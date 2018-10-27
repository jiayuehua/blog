---
layout: post
title: "Cheaply relocatable, but not trivially relocatable"
date: 2018-10-26 00:01:00 +0000
tags:
  relocatability
---

On [GCC bug 87106](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=87106), Marc Glisse raises
the following interesting example:

> I think it might be useful to be able to specify how to relocate an object
> that is not trivially relocatable. Relocating a `pair<A,B>` where `A`
> is trivially relocatable and `B` is not can still benefit from doing
> piecewise relocations so it avoids `A`'s super-costly move constructor.

Expressed in [P1144R0](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1144r0.html) syntax:

    struct [[trivially_relocatable]] A {
        A(A&&);
        ~A();
    };
    static_assert(is_trivially_relocatable_v<A>);

    struct B {
        B(B&&);
        ~B();
    };
    static_assert(not is_trivially_relocatable_v<B>);

    using C = std::pair<A, B>;
    static_assert(not is_trivially_relocatable_v<C>);

Ideally, `C`'s "relocate" operation would use `memcpy` for the `A` component
but then dispatch to `B(B&&)` and `~B()` for the `B` component. So it'd call
`memcpy`, `B(B&&)`, and `~B()`.

Since P1144 does not propose any first-class "relocate operation," there's no
way for the designer of `C` (that is, the designer of `std::pair`) to indicate
that this is how "relocation" ought to work for `C`. And `C` is definitely not
*trivially* relocatable, because of the `B` component. So the result is that
relocating a `C` falls back to the only fallback we know: calling `C`'s
move constructor and destructor.
So it ultimately calls `A(A&&)`, `B(B&&)`, `~A()`, and `~B()`.

So this is an example of a situation where a first-class "relocate" operation
— such as the one proposed by Denis Bider in P0023 "Relocator: Efficiently
moving objects" (April 2016) — would produce more benefit than P1144's
focus on *trivially* relocatable objects.

----

*TL;DR:*

    using CheaplyButNotTriviallyRelocatable =
        tuple<string, string, string, string, list<int>>;
