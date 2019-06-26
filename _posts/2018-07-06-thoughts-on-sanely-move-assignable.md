---
layout: post
title: 'Thoughts on "sanely move-assignable"'
date: 2018-07-06 00:02:00 +0000
tags:
  concepts
  relocatability
---

The "relocate" operation to be proposed in my upcoming P1144 (not yet published) is a high-level
operation consisting of a move-construct and a destroy. This is analogous to how most libraries
today define "swap" as a high-level operation consisting of a move-construct, two move-assigns,
and a destroy.

[The C++17 standard defines `std::swap`'s effects only vaguely](http://eel.is/c++draft/utility.swap#3):

> Exchanges values stored in two locations.

Typically, library vendors implement `swap` as

    template<class T>
        // requires MoveConstructible<T>
        // requires MoveAssignable<T>
        // requires Destructible<T> (implied by MoveConstructible<T>)
    void swap(T& a, T& b) noexcept(OMITTED)
    {
        T tmp(std::move(a));
        a = std::move(b);
        b = std::move(tmp);
    }

[Mingxin Wang has observed](https://groups.google.com/a/isocpp.org/d/msg/std-proposals/HGCHVSRwSMk/k7Ir-rmxBgAJ)
that "swap" can also be expressed in terms of "relocate". That is, library vendors could reasonably
implement a faster `std::swap` using

    template<class T>
        // requires MoveConstructible<T>
        // requires MoveAssignable<T>
        // requires Destructible<T> (implied by MoveConstructible<T>)
    void swap(T& a, T& b) noexcept(OMITTED)
    {
        alignas(T) char buffer[sizeof (T)];
        __uninitialized_relocate(&b, &b + 1, (T*)buffer);
        __uninitialized_relocate(&a, &a + 1, &b);
        __uninitialized_relocate((T*)buffer, (T*)buffer + 1, &a);
    }

    template<class T>
        // requires MoveConstructible<T>
        // requires Destructible<T> (implied by MoveConstructible<T>)
    void __uninitialized_relocate(T *first, T *last, T *d_first) noexcept(OMITTED)
    {
        std::uninitialized_move(first, last, d_first);
        std::destroy(first, last);
    }

(This is an suboptimally generic definition of `__uninitialized_relocate`, but it suffices
in this context.)

[Godbolt shows](https://godbolt.org/g/RCQDks) that for `std::string`, the relocate-based
`swap` (46 instructions on GCC, 50 on Clang) is vastly more efficient than the standard assignment-based
`swap` (174 on GCC, 156 on Clang). It is even more efficient than
[libstdc++'s insanely complicated `string::swap` method](https://github.com/gcc-mirror/gcc/blob/3f6823abf8d0ce23804dfbfe32c6250824501ef6/libstdc%2B%2B-v3/include/bits/basic_string.tcc#L56-L128)
(78 instructions on GCC; 76 on Clang) — but less efficient than libc++'s (14 instructions on Clang).

This raises a thought-provoking question: If reimplementing `swap<T>` in this super-efficient way
actually stops calling `T::operator=`, isn't that an observable side-effect? Imagine this type:

    struct Evil {
        int i;
        Evil(Evil&&) = default;
        ~Evil() = default;
        Evil& operator=(Evil&& rhs) {
            i = rhs.i + 1;
            puts("I see you");
            return *this;
        }
    };

Notice that `Evil` is trivially relocatable; but we might say that in some sense
it is not "trivially swappable."

The standard already seems to *assume* that the visible side-effects of
"one move-construction, two move-assignments, and one destruction" must not
significantly differ from the visible side-effects of "three move-constructions
and three destructions," or else it would have specified `swap`'s
effects actually in terms of "one move-construction, two move-assignments,
and one destruction," rather than in the vague "Exchanges values" way that it
actually did.

However, some paranoid programmer might want to ask,
"Is the standard's assumption actually true of my type `T`?"
That is, "Is my `T` relocatable *and also*
does move-assigning dst from src perform essentially the same operation
as destroying dst and then move-constructing dst from src?" (In the `Evil` case
above, it does not, for at least two reasons.)

The part of the question about the behavior of move-assignment might be
separated out and given a name such as `is_sanely_move_assignable`,
at which point we could write the definition

    template<class T>
    struct is_trivially_swappable : bool_constant<
        is_trivially_relocatable_v<T> and
        is_sanely_move_assignable_v<T>
    > {};

But!

In a C++2a Concepts world, it is extremely reasonable to claim that
"`is_sanely_move_assignable`" essentially encapsulates the *semantic requirements*
of the `MoveAssignable` syntactic concept. In Concepts, since
semantic requirements cannot be indicated in source code, the library
*must* assume that any `MoveAssignable` type also
follows `MoveAssignable`'s semantic requirements.
C++2a Concepts essentially assumes that when I write the syntax
`x = std::move(y)`, I get the semantics of a move-assignment;
when I write the syntax `x + y`, I get the semantics of an addition;
when I write the syntax `*x`, I get the semantics of a pointer; and so on.

In short, in a Concepts world, the library *must assume* that
"`is_sanely_move_assignable<T>` if-and-only-if `is_move_assignable<T>`."
So, in a Concepts world, there is no need for `sanely` variants;
every operation must be *assumed* sane by definition.

Therefore, if syntactic-Concepts-with-semantic-requirements
remains largely unchanged in C++2a, we safely conclude that

    template<class T>
    struct is_trivially_swappable : bool_constant<
        is_trivially_relocatable_v<T> and
        is_move_assignable_v<T>
    > {};

----

Notice that if `is_trivially_swappable_v<T>`, then
`swap` will check, but never use, the fact that
`is_move_assignable<T>`. Perhaps `MoveAssignable` should be dropped
from the requirements of the `swap` template?

No, it should not!

[One particular kind of type](/blog/2018/03/27/string-view-is-a-borrow-type/)
is often trivially relocatable but not
move-assignable. In the standard, `string_view` is assignable and
`optional<T&>` doesn't exist, but you can imagine versions of
both of those where `operator=` was deleted (to avert misuse,
or simply to avert bikeshedding over what it should do). If the programmer
has already gone out of his way to disable `a = std::move(b)`,
a library's heroic efforts to reenable `std::swap(a, b)` are
unlikely to be appreciated.
