---
layout: post
title: "On `function_ref` and `string_view`"
date: 2019-05-10 00:01:00 +0000
tags:
  cppnow
  library-design
  parameter-only-types
  pitfalls
  type-erasure
---

At C++Now 2018, I wrote three blog posts during the conference.
This year I was somehow so busy at C++Now that I merely wrote a list of things I _ought_ to blog about
once I got the time. So here's the first one.

[On Wednesday afternoon](https://cppnow2019.sched.com/event/Mj3N/higher-order-functions-and-functionref),
Vittorio Romeo gave a very good summary of his proposed `function_ref`. It's a very well-defined type;
because it is non-owning and trivially copyable, it can sidestep the majority of design decisions that
are the bugbear of `std::function`'s awkward design. (See
["The space of design choices for `std::function`,"](/blog/2019/03/27/design-space-for-std-function)
2019-03-27.)

However, during the talk, David Sankel made an interesting observation. Consider:

    template<class T>
    T one(std::reference_wrapper<T> arg) {
        T local = arg;
        return local;
    }

    int main() {
        std::string s = one("hello"s);
        std::cout << s << "\n";
    }

`arg` is a reference to a temporary, but by converting it to `T local`, we get a copy
_by value_.

Or consider:

    std::string two(std::string_view arg) {
        std::string local(arg);
        return local;
    }

    int main() {
        std::string s = two("hello"s);
        std::cout << s << "\n";
    }

`arg` is a view to a temporary, but by converting it to `std::string local`, we get a copy
_by value_. (Notice that this conversion is marked `explicit`, so `std::string local = arg;`
won't compile — thanks to Jason Cobb for the correction!)

But consider:

    std::function<int()> three(std::function_ref<int()> arg) {
        std::function<int()> local = arg;
        return local;
    }

    int main() {
        std::function<int()> f = three([x=42](){ return x; });
        std::cout << f() << "\n";
    }

`arg` is a view to a temporary. By converting it to `std::function<int()> local`,
we get _a copy of the view_ — the resulting `std::function` has _reference_ semantics,
and when we call it on the last line of `main`, we dereference a dangling reference and
produce undefined behavior.

----

This interaction is cute, but I definitely don't want to imply that `function_ref` (or `function`)
should be derailed over it. The conclusion falls out fairly intuitively from the fact that
`string_view` is a view over _data_, whereas `function_view` is a view over _code_.

Data can be serialized; therefore it can be copied, and also compared for equality
(remember, "copies compare equal").
Therefore the conversion from `string_view` to `string` has value semantics, and both
`string` and `string_view` provide an `operator==`.

Code cannot be serialized; therefore it cannot be copied, nor can it be compared for equality.
Therefore the conversion from `function_ref` to `function` (or from `reference_wrapper` to `function`)
has reference semantics, and neither `function` nor `function_ref` provide an `operator==`.

----

So, to a first approximation, `function_ref` is to `function` as `string_view` is to `string` — both are
parameter-only types with reference semantics. But watch out for the inherent difference between
"views over data" and "views over code"!
