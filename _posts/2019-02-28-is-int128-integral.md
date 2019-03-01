---
layout: post
title: "Is `__int128` integral? A survey"
date: 2019-02-28 00:01:00 +0000
tags:
  standard-library-trivia
---

How well is `__int128` supported, and what are its
properties? A Godbolt-based survey of the Big Four compilers.
Spoiler alert: libc++ thinks it's integral, libstdc++ thinks it's not integral,
and MSVC thinks it doesn't exist at all.


## Compiler support

    using I128 = __int128;

GCC, Clang, and Intel ICC all support a built-in `__int128` type.
Microsoft MSVC does not support any 128-bit integral type as far as I'm aware.

On GCC, Clang, and ICC, `__int128` is a token similar to `int`: you can modify
it with `unsigned` to produce `unsigned __int128`. However, all three front-ends
also support the built-in synonyms `__int128_t` and `__uint128_t`.

    static_assert(std::is_same_v<__int128_t, __int128>);
    static_assert(std::is_same_v<__uint128_t, unsigned __int128>);
        // PASSES on all three compilers

Both `__int128_t` and `__uint128_t` are safe to use in non-type template parameters
and constant expressions, on all compilers that support them.


## Relationship to integer types and `intmax_t`

    static_assert(std::is_integral_v<__int128>);
        // TRUE on libc++, FALSE on libstdc++

libstdc++ (as seen on Godbolt's GCC and ICC) leaves `is_integral_v<__int128>` as `false`.
This makes a certain amount of sense from the library implementor's point of view,
because `__int128` is not one of the _standard_ integral types, and furthermore,
if you _call_ it integral, then you have to face the consequence that `intmax_t`
(which is 64 bits on every ABI that matters) is kind of lying about being the
"max."

MSVC's headers take the same approach as libstdc++, since MSVC has no `__int128` in
the first place.

On the other hand, libc++ [sets](https://github.com/llvm-mirror/libcxx/blob/aeeba70ab15e1469841c0f3f1f9c78211a541699/include/type_traits#L738-L741)
`is_integral_v<__int128>` to `true`. Yes, this means that on libc++, there are
integral values that do not fit in `intmax_t`! ([Godbolt.](https://godbolt.org/z/lMD4Iu))

> In case you're wondering: libstdc++ considers `__int128` a "compound" type. The definition
> of "compound type" in the Standard is simply "anything that's not a fundamental type," where
> "fundamental" encompasses the arithmetic types (both integral and floating-point), `void`,
> and `nullptr_t`. So for example `int*` is also a compound type.


## Numeric limits

libstdc++ does not specialize `numeric_limits` for `__int128` at all.
This [has the unfortunate effect](https://godbolt.org/z/iyBz0X) of

    constexpr __int128 int128_max =
        std::numeric_limits<__int128>::max();
    static_assert(int128_max == 0);  // TRUE on libstdc++

libc++ provides the appropriate specialization, so that
`numeric_limits<__int128>::min()` and `numeric_limits<__int128>::max()`
have the appropriate values instead of zero.

On libc++ [`numeric_limits` reports `__int128` as an integral type](https://godbolt.org/z/ZR_nn6),
whereas on libstdc++ it doesn't (again because there is no specialization for `__int128` at all).

    static_assert(std::numeric_limits<I128>::is_integer);
        // TRUE on libc++, FALSE on libstdc++


## Signedness

libc++ reports that `is_signed_v<__int128>` and `is_unsigned_v<unsigned __int128>`.
libstdc++ of course denies both, because only arithmetic types can be signed or unsigned,
and libstdc++ denies that `__int128` is arithmetic.

As for `make_signed`, libc++ reports what you'd expect — `make_signed<__uint128_t>::type`
is `__int128_t`, and `make_unsigned<__int128_t>::type` is `__uint128_t`.

Trying to instantiate `make_signed<__uint128_t>` on libstdc++ [results in](https://godbolt.org/z/j7BvKB)
a hard error which is not SFINAE-friendly. (It's undefined behavior to instantiate
`make_signed` of any type which is neither integral nor an enumeration type.)

Incidentally, today I learned that `is_signed_v<float> == true` but [`make_signed_t<float>`
is a hard error](https://godbolt.org/z/t6v1CJ).

----

[I have also done some comparison of GCC and Clang codegen for `__uint128_t` arithmetic](https://github.com/Quuxplusone/WideIntProofOfConcept#wide-integer-math-proof-of-concept),
but I have not yet written it up in a blog post.
