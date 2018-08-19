---
layout: post
title: 'Announcing "trivially relocatable"'
date: 2018-07-18 00:01:00 +0000
tags:
  proposal
  relocatability
---

Frequent users of Compiler Explorer, a.k.a. [godbolt.org](https://godbolt.org/),
may have noticed that a few days ago a new compiler appeared in its dropdown menu.
"x86-64 clang (experimental P1144)" is a branch of Clang which I have patched
to support the concept of "trivially relocatable types," as described in my
[C++Now 2018 talk](https://www.youtube.com/watch?v=MWBfmmg8-Yo) and
as proposed for standardization in my upcoming paper
P1144 "Object relocation in terms of move plus destroy" (coauthored
with Mingxin Wang).

The implementation consists of several new language and library features,
most of which are displayed in [this snippet](https://godbolt.org/g/zUUAVW):

    struct [[trivially_relocatable]] Widget {
        Widget(Widget&&);
        ~Widget();
    };

    struct R {
        std::optional<std::string> name;
        std::unique_ptr<int> p, q;
        Widget w;
    };

    static_assert(std::is_trivially_relocatable_v<R>);

Here we see an assertion that objects of type `R` are _trivially relocatable_
— i.e., that the relocation operation for `R` is trivial —
i.e., that the operation of "moving an `R` and then immediately destroying
the original" is equivalent to `memcpy`.

The library type-trait `std::is_trivially_relocatable<T>` can be used to
ask the compiler whether any given type `T` has this property or not. The
compiler knows exactly which types have this property, in the same way
that it knows which types have the property "trivially copyable."
The compiler tracks trivial relocatability, even for built-in types
such as closures.

    auto lama = [a = 1](){};
    static_assert(std::is_trivially_copyable_v<decltype(lama)>);

    auto lamb = [b = std::string("hello world")](){};
    static_assert(std::is_trivially_relocatable_v<decltype(lamb)>);

So that's how we *ask* the compiler about the trivial relocatability
of a given type. What about the reverse? How do we *tell* the compiler that
we *know* a given user-defined type is trivially relocatable, even though it
has user-provided special member functions? For that, we provide an attribute:

    struct [[trivially_relocatable]] Widget {
        Widget(Widget&&);
        ~Widget();
    };

(For those keeping score, this attribute is [ignorable](/blog/2018/05/15/the-ignorable-attributes-rule)
in exactly the same sense that `[[no_unique_address]]` is ignorable.)

Finally, the compiler on Godbolt is hooked up to my own branch of libc++,
where we have *told* the compiler (using the above attribute) that all of
the most popular standard types are in fact trivially relocatable. This is
important, because *by default* the compiler cannot assume that any type
is trivially relocatable unless it follows the Rule of Zero. This
ensures that we don't break any existing code. [Example:](https://godbolt.org/g/aYoh86)

    // You can rely on this.
    static_assert(!std::is_trivially_relocatable_v<boost::interprocess::offset_ptr<int>>);

    // This happens to be true, thanks to my patch.
    static_assert(std::is_trivially_relocatable_v<std::unique_ptr<int>>);

    // This happens to be false, since I did not patch Boost.
    static_assert(!std::is_trivially_relocatable_v<boost::movelib::unique_ptr<int>>);

Observe that while `boost::movelib::unique_ptr<int>` can *in fact* safely be relocated
via `memcpy`, it has not clearly communicated that fact to the *compiler*
(and thence to consumers of the type-trait, such as `vector::reserve`,
who could have optimized based on that fact). It would be easy to patch
Boost just as I patched libc++, to add the attribute and thus enable the optimization.

By the way: In many cases the attribute must be added *conditionally*.
The implementation takes care of this for you. [Godbolt](https://godbolt.org/g/XGuW7W):

    template<class P>
    struct Dummy {
        using pointer = P;
        void operator()(P) {}
    };

    using P1 = std::unique_ptr<int, Dummy<int *>>;
    using P2 = std::unique_ptr<int, Dummy<offset_ptr<int>>>;

    static_assert(std::is_trivially_relocatable_v<P1>);
    static_assert(!std::is_trivially_relocatable_v<P2>);

This produces about a 3x speed boost on operations that relocate ranges of
trivially relocatable objects (such as happens in `vector::reserve`, or any
time you move a `fixed_capacity_vector`). For library writers who want to
get this optimization but don't want to write all the tag-dispatching logic
themselves, we provide a new standard algorithm, `std::uninitialized_relocate`,
which can be used as a building block. [Godbolt](https://godbolt.org/g/iHnxuD):

    struct S {
        std::any a;
        std::variant<std::string, int> v;
    };

    void foo(S *src, int n, S *dst) {
        std::uninitialized_relocate_n(src, n, dst);
    }

Take a look, play around with it, let me know what you think!
If you come up with compelling or thought-provoking examples,
I'd like to add them to the paper.

P1144 has not yet appeared in print. You should expect to see P1144R0
in the pre-meeting mailing for San Diego. In the meantime, [here is D1144R0 draft
revision 7](/blog/code/object-relocation-in-terms-of-move-plus-destroy-draft-7.html).

----

P.S. — Many, many thanks to Matt Godbolt for letting me put this implementation online!
