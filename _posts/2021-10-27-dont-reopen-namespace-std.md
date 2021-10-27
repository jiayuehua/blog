---
layout: post
title: "Don't reopen `namespace std`"
date: 2021-10-27 00:01:00 +0000
tags:
  antipatterns
  c++-style
  cppcon
  sd-8-adjacent
  slogans
  templates
---

Today (in two different contexts) I saw people reopening `namespace std`
in order to introduce a specialization of a standard template.
For example:

    struct Widget {};

    namespace std {  // Danger!
        template<>
        struct hash<Widget> {
            size_t operator()(const Widget&) const;
        };
    }

Don't do this! Instead, prefer to define template specializations
using their qualified names, like so:

    struct Widget {};

    template<>
    struct std::hash<Widget> {
        size_t operator()(const Widget&) const;
    };

Here are my reasons.


## It's simpler and clearer

Opening `namespace std` takes two extra lines, which you don't need.

Also, by keeping all parts of the qualified name together in the source code,
it's easier to see that what we're doing here is specializing `std::hash` —
not just `hash`. Explicit is good.


## Name lookup inside `namespace std` works differently

Consider this contrived example ([Godbolt](https://godbolt.org/z/MhnvPfsKf)):

    #include <ranges>

    struct topping {};
    struct slice { topping *begin() const; topping *end() const; };
    struct pizza {};

    namespace std::ranges {  // Danger!
        template<> constexpr bool enable_view<slice> = true;
    }

    static_assert(std::ranges::view<slice>);

This works in isolation, but watch what happens when we `#include <valarray>`
(perhaps transitively, from another header):

    error: static_assert failed
        static_assert(std::ranges::view<slice>);
        ^                          ~~~~~~~~~~~
    note: because 'slice' does not satisfy 'view'
        static_assert(std::ranges::view<slice>);
                                   ^
    __ranges/concepts.h:83:5: note: because 'enable_view<slice>' evaluated to false
        enable_view<_Tp>;
        ^

Outside `namespace std`, the name `slice` refers to `::slice`; but _inside_
the namespace, it refers to `std::slice`, an obscure utility type associated
with `std::valarray`. To work around this, we could cruft up our template
specialization a little more:

    namespace std::ranges {  // Danger!
        template<>
        constexpr bool enable_view<::slice> = true;
    }

Or, we could just use the simpler syntax:

    template<>
    constexpr bool std::ranges::enable_view<slice> = true;

----

However, you should still be aware that if you're specializing a class template
like `std::hash` — or anything that requires you to type curly braces — then
everything inside the curly braces is still considered "inside" `namespace std`
for the purposes of name lookup. ([Godbolt.](https://godbolt.org/z/zMeeqeMPo))

    template<>
    struct std::hash<slice> {
        size_t operator()(const slice& w) const {
            return 0;
        }
    };

The compiler says:

    error: no matching function for call to object of type 'std::hash<slice>'
        return h(s);
               ^
    note: candidate function not viable: no known conversion from 'slice'
    to 'const std::slice' for 1st argument
        size_t operator()(const slice& w) const {
               ^

So, sometimes it can still be necessary to `::`-qualify names inside the curly braces.
This is highly unfortunate, and I'm not aware of any simple workaround. But _most_
names won't collide with anything in the standard library. If you run into a
scenario requiring `::`-qualification in real-world code, I'd be interested to
hear about it!


## Qualified names don't get confused by aliases

Consider this hypothetical library code (via Lewis Baker):

    namespace std {
        template<class P = void> struct coroutine_handle;
        template<class R, class... Args> struct coroutine_traits;
        struct suspend_never;
        // etc. etc.
    }
    namespace std::experimental {  // for backward compatibility
        using std::coroutine_handle;
        using std::coroutine_traits;
        using std::suspend_never;
    }

I don't recommend that library vendors do this, but _hypothetically_,
let's suppose that one did. Then consider the situation from the client's
point of view:

    #include <experimental/coroutine>

    struct MyResumable {
        explicit MyResumable(std::experimental::coroutine_handle<>);
        void resume();
    };

    struct MyPromise {
        auto get_return_object() {
            return MyResumable(std::experimental::coroutine_handle<MyPromise>::from_promise(*this));
        }
        auto initial_suspend() { return std::experimental::suspend_never{}; }
        auto final_suspend() noexcept { return std::experimental::suspend_never{}; }
        void return_void() {}
        void unhandled_exception() {}
    };

    MyResumable f() { co_return; }

Now, this is really not the best way to write `MyResumable` and `MyPromise`!
We _should_ just rename `MyPromise` to `MyResumable::promise_type`, as a nested type
definition, and then all our problems go away. But for the sake of this example,
we'll assume that `MyResumable` does not define a nested `promise_type`.

So the client must specialize `std::experimental::coroutine_traits<MyResumable>`.
They might try like this:

    namespace std::experimental {  // Danger!
        template<>
        struct coroutine_traits<MyResumable> {
            using promise_type = MyPromise;
        };
    }

This would work if `coroutine_traits` were actually a class template defined in
`namespace std::experimental`. But it's not! It's a class template defined in
`namespace std`. So, this code doesn't compile. What the client should have
written is

    template<>
    struct std::experimental::coroutine_traits<MyResumable> {
        using promise_type = MyPromise;
    };

The compiler performs qualified name lookup to decide what class template
is being referred to as "`std::experimental::coroutine_traits`." Name lookup
looks through the `using`-declaration in `std::experimental` and decides that
the class template being referred to is `std::coroutine_traits`. So this
code works fine.

As in our `slice` example, we see that reopening `namespace std`
causes an unfamiliar compiler error, but when we _don't_ reopen the namespace,
everything Just Works.

Both of these last two examples involve nested namespaces (`std::experimental`, `std::ranges`),
but that's coincidental. We would have the same problems if the "client's preferred name"
for the template is located directly in `std`. It's just harder to come up
with realistic-looking examples for that scenario. ([Godbolt](https://godbolt.org/z/qbx4v58K3).)


## Fewer moving parts means less bikeshedding

What happens when you need to specialize more than one standard template?
Should you reopen `namespace std` just once...

    namespace std {
        template<>
        struct hash<Widget> {
            size_t operator()(const Widget&) const { return 1; }
        };
        template<>
        struct tuple_size<Widget> : integral_constant<2> {};
        template<>
        struct tuple_element<0, Widget> : type_identity<int> {};
        template<>
        struct tuple_element<1, Widget> : type_identity<int> {};
        template<>
        struct hash<Gadget> {
            size_t operator()(const Gadget&) const { return 2; }
        };
    }

or once per associated type...

    namespace std {
        template<>
        struct hash<Widget> {
            size_t operator()(const Widget&) const { return 1; }
        };
        template<>
        struct tuple_size<Widget> : integral_constant<2> {};
        template<>
        struct tuple_element<0, Widget> : type_identity<int> {};
        template<>
        struct tuple_element<1, Widget> : type_identity<int> {};
    }

    namespace std {
        template<>
        struct hash<Gadget> {
            size_t operator()(const Gadget&) const { return 2; }
        };
    }

or once per template specialization (which I won't write out because
it would take 28 lines)... or — how about never! Prefer to write:

    template<>
    struct std::hash<Widget> {
        size_t operator()(const Widget&) const { return 1; }
    };

    template<> struct std::tuple_size<Widget> : std::integral_constant<2> {};
    template<> struct std::tuple_element<0, Widget> : std::type_identity<int> {};
    template<> struct std::tuple_element<1, Widget> : std::type_identity<int> {};

    template<>
    struct std::hash<Gadget> {
        size_t operator()(const Gadget&) const { return 2; }
    };


## Reopening `namespace std` is _usually_ wrong; make it _always_ wrong

Back in the bad old days, before there were so many blogs and conference talks
about C++, I remember seeing people do things like this:

    struct Widget {
        void swap(Widget&);
    };

    namespace std {  // Danger!
        void swap(Widget& a, Widget& b) {
            a.swap(b);
        }
    }

    int main() {
        Widget a, b;
        {
            using std::swap;
            swap(a, b);  // OK
        }
        {
            std::swap(a, b);  // OK
        }
    }

That is, instead of defining a hidden-friend `swap` for their type `Widget`,
they'd reopen `namespace std` and insert an overload of `std::swap` that works
specifically for `Widget`s! If you learned C++ from reading the standard library,
this would seem like a reasonable idea, because this is how types like `std::string`
do it: they add extra overloads of `std::swap`, rather than hidden friends.
But, for user code like `Widget`, this is [undefined behavior](https://en.cppreference.com/w/cpp/language/extending_std)!
Users are not allowed to add new overloads of functions in the `std` namespace.

See ["What is the `std::swap` two-step?"](/blog/2020/07/11/the-std-swap-two-step/) (2020-07-11).

If you see someone reopening `namespace std` in order to add a function definition
(such as an overloaded `swap`), that's a bug, and they should stop it.

If you see someone reopening `namespace std` for _another_ reason... Well, let's
just make a blanket rule that "Thou shalt not reopen `namespace std`." Then, we
don't even have to think about _why_ they're doing it. We can just say "Either
that's actually undefined behavior, or (at best) it's simply unnecessary and error-prone. Stop it."

----

I've previously mentioned my mantra "Never reopen `namespace std`." This is
just one special case of it.

See also:

* ["Don't explicitly instantiate `std` templates"](/blog/2021/08/06/dont-explicitly-instantiate-std-templates/) (2021-08-06)
