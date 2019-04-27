---
layout: post
title: "This one weird trick for customization by class template (partial) specialization"
date: 2019-04-26 00:02:00 +0000
tags:
  concepts
  customization-points
  library-design
  metaprogramming
---

Last night I attended the [New York City C++ meetup](https://www.meetup.com/nyccpp/events/260701006/)
(of which I am now a co-organizer!). Our speaker was JeanHeyd Meneide, giving a sneak preview of
his upcoming C++Now talk
["The Plan for Tomorrow: Extension Points in C++ Applications."](https://cppnow2019.sched.com/event/Mj2n/the-plan-for-tomorrow-extension-points-in-c-applications)
(By the way, there is still time and space for you to attend C++Now 2019! And your local C++ meetup
is always looking for presenters, too!)

One of the first extension mechanisms discussed in JeanHeyd's talk is the classic "class template
specialization," which we all know because of `std::hash`. Classically, it looks
[like this](https://godbolt.org/z/MfAhDX). The `Library` author provides a primary template,
declared but perhaps not defined.

    namespace Library {
        template<class T> struct Hash;
    }

The `User` invokes the functionality by asking for a specific specialization `Library::Hash<T>`.

    namespace User {
        template<class T>
        size_t hash_a_thing(T t) {
            return Library::Hash<T>::doit(t);
        }

        int test() {
            Client::A a;
            return hash_a_thing(42) + hash_a_thing(a);
        }
    } // namespace User

The business of providing these specific specializations is left up to the authors of `Library`
and/or `Client` code:

    namespace Client {
        struct A {};
    }

    template<>
    struct Library::Hash<Client::A> {
        static size_t doit(Client::A) { return 2; }
    };

(Notice that the specialization of `Library::Hash<Client::A>` must appear outside of `namespace Client`,
even though it is being provided by the author of `namespace Client`. This is extremely awkward, and is
the subject of [P0665R1 "Allowing Class Template Specializations in Associated Namespaces"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0665r1.pdf)
(Tristan Brindle, May 2018). For the meaning of "associated namespaces," see
[my previous post on ADL](/blog/2019/04/26/what-is-adl/).)

----

The downside of the above technique is that you have to specialize `Library::Hash<X>` individually
for every class `X` in your code.

Sure, you can get some mileage out of partial specializations, like this:

    namespace Client {
        template<class A, class B>
        struct Pair { A a; B b; };
    }

    // This partial specialization covers all kinds of Pairs!
    template<class A, class B>
    struct Library::Hash<Client::Pair<A, B>> {
        static size_t doit(const Client::Pair<A, B>& p) {
            return Library::Hash<T>::doit(p.a) +
                   Library::Hash<T>::doit(p.b);
        }
    };

But if you have a whole zoo of similar types, you might find yourself wanting to eliminate some duplication.
Consider this tedious morass:

    namespace Client {
        struct Tag {};
        struct RedTag : Tag {};
        struct GreenTag : Tag {};
        struct BlueTag : Tag {};
    }

    template<>
    struct Library::Hash<Client::RedTag> {
        static size_t doit(Client::RedTag) { return 3; }
    };
    template<>
    struct Library::Hash<Client::GreenTag> {
        static size_t doit(Client::GreenTag) { return 3; }
    };
    template<>
    struct Library::Hash<Client::BlueTag> {
        static size_t doit(Client::BlueTag) { return 3; }
    };

With the class template `Library::Hash` as described above, we're stuck in this tedium.
But JeanHeyd showed a beautifully simple fix, which has been used extensively
in his successful Lua-binding library [sol2](https://github.com/ThePhD/sol2).

> Although I see what look like
> [a slew of examples](https://github.com/ThePhD/sol2/blob/92f12952efb2225ac96dd5a1b9ea276874daeffe/sol/stack_core.hpp#L500-L525)
> in sol2's codebase, it sounds like sol2 may have lately moved away from customization-via-template-specialization;
> these templates may or may not be intended as user-facing customization points in the current version of sol2.
> I've never used sol2; please don't cite me as an expert on how it works!

The fix is, you add a _second template parameter_.

    namespace Library {
        template<class T, class Enable = void> struct Hash;
    }

The sole purpose of this otherwise unused parameter is to give the client a place to hang SFINAE constraints.
It has a default value, so if a client-provided specialization just omits it entirely, that's fine too.
For example, this specialization will continue to work:

    namespace Client {
        template<class A, class B>
        struct Pair { A a; B b; };
    }

    // This partial specialization covers all kinds of Pairs!
    template<class A, class B>
    struct Library::Hash<Client::Pair<A, B>> {
        static size_t doit(const Client::Pair<A, B>& p) {
            return Library::Hash<T>::doit(p.a) +
                   Library::Hash<T>::doit(p.b);
        }
    };

With the aid of the extra parameter, we can escape from our `Tag` tedium!
[Godbolt:](https://godbolt.org/z/c_ahNx)

    namespace Client {
        struct Tag {};
        struct RedTag : Tag {};
        struct GreenTag : Tag {};
        struct BlueTag : Tag {};
    }

    template<class T>
    struct Library::Hash<T, std::enable_if_t<std::is_base_of_v<Client::Tag, T>>> {
        static size_t doit(T) { return 3; }
    };

The specialization above is "enabled" only for types `T` where `std::is_base_of_v<Client::Tag, T>`;
for all other types, this specialization will suffer substitution failure (which Is Not An Error)
and vanish from consideration. So it does exactly what we want!

And the client programmer can hang as many conditions on their partial specializations as they want,
as long as those conditions remain mutually exclusive, and as long as they ultimately evaluate
(when they are evaluable) to `void`. For example:

    template<class T>
    struct Library::Hash<T, std::enable_if_t<std::is_integral_v<T>>> {
        static size_t doit(T t) { return size_t(t); }
    };

    template<class T>
    struct Library::Hash<T, std::void_t<decltype(T::sh()), typename T::is_widget>> {
        static size_t doit(const T&) { return T::sh(); }
    };

The conclusion is that any time we have a "customization by class template specialization" scenario,
we should consider adding that one extra template parameter and default it to `void`. It could save some
client programmer a lot of grief!

----

"But wait — could we achieve the same goal sans parameter, via constrained type aliases?"
Regular readers of this blog will remember constrained type aliases from
["Stopping the cascade of errors"](/blog/2018/08/23/stop-cascading-errors/#in-the-comments-sergey-vidyuk-su)
(August 2018).

On GCC, yes, we can use constrained type aliases even in partial specializations.
But on any other compiler, sadly, they don't work in this context. So I would advise against trying
to use them this way. [Godbolt:](https://godbolt.org/z/TQHGy7)

    namespace Client {
        struct Tag {};
        struct RedTag : Tag {};
        struct GreenTag : Tag {};
        struct BlueTag : Tag {};

        // here's our type alias with a constraint attached...
        template<class T, class = std::enable_if_t<std::is_base_of_v<Tag, T>>>
        using MustBeTag = T;
    }

    template<class T>
    struct Library::Hash<Client::MustBeTag<T>> {
        static size_t doit(T) { return 3; }
    };

And just to be contrary, if we try to use C++2a Concepts `requires`-clauses to express
our constraints, then _Clang_ accepts and _GCC_ rejects! (I don't know which one is
more correct according to the C++2a draft standard.) [Godbolt:](https://concepts.godbolt.org/z/mMYBgu)

    namespace Client {
        struct Tag {};
        struct RedTag : Tag {};
        struct GreenTag : Tag {};
        struct BlueTag : Tag {};

        template<class T>
        concept MustBeTag = std::is_base_of_v<Tag, T>;
    }

    template<class T> requires Client::MustBeTag<T>
    struct Library::Hash<T> {
        static size_t doit(T) { return 3; }
    };

So for the foreseeable future, `enable_if` is the way to go, and giving every customizable
class template an extra defaulted-to-`void` parameter is worth considering. Heck, if I were writing
a template-heavy library, I might even try to use the presence of that extra parameter as a hint to
the user:

> Class templates with an "open" `=void` hook are intended for you to customize.
> Templates lacking that hook are "closed" for a reason; do not try to extend them.

----

Finally, I feel obliged to repeat my usual warning whenever `enable_if` comes up: Watch out for the
difference between `enable_if` and `enable_if_t`!

Can you spot the reason
[this code silently returns `1` instead of `3`?](https://godbolt.org/z/_osgU2)

    namespace Library {
        template<class T, class = void> struct Hash {
            static size_t doit() { return 1; }
        };
    }

    template<class T>
    struct Library::Hash<T, std::enable_if<std::is_integral_v<T>>> {
        static size_t doit() { return 2; }
    };

    namespace Client {
        struct Tag {};
        struct RedTag : Tag {};
        struct GreenTag : Tag {};
        struct BlueTag : Tag {};
    }

    template<class T>
    struct Library::Hash<T, std::enable_if<std::is_base_of_v<Client::Tag, T>>> {
        static size_t doit() { return 3; }
    };

    int main() {
        return Library::Hash<Client::RedTag>::doit();
    }
