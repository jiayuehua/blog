---
layout: post
title: "Is `__int128` integral? A survey"
date: 2019-02-28 00:01:00 +0000
tags:
  implementation-divergence
  standard-library-trivia
---

How well is `__int128` supported, and what are its
properties? A Godbolt-based survey of the Big Four compilers.
Spoiler alert: libc++ thinks it's integral, libstdc++ thinks it's integral
only in `-std=gnu++XX` mode(!), and MSVC thinks it doesn't exist at all.

Many thanks to Rein Halbersma for alerting me to libstdc++'s different behavior
in `-std=gnu++XX` mode versus `-std=c++XX` mode.


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


## Sneaky macros

The story on libstdc++ is complicated. Libstdc++ doesn't do anything special for
`__int128`, but it does provide _macro hooks_
([link](https://github.com/gcc-mirror/gcc/blob/87798f3f2cf97b30e75110b5a8d2964aef62be02/libstdc%2B%2B-v3/include/std/type_traits#L285-L293))
so that for example
`__GLIBCXX_TYPE_INT_N_0` will be treated as an integral type, whatever that macro
expands to. (If it doesn't expand to anything, it'll be ignored.)

If you compile with GCC in `-std=c++XX` mode (for any `XX`),
`__GLIBCXX_TYPE_INT_N_0` won't be defined and so libstdc++ will consider
`__int128` a non-integral type.

But if you compile with GCC in `-std=gnu++XX` mode (for any `XX`), then the compiler
will pre-define these two macros:

    #define __GLIBCXX_BITSIZE_INT_N_0 128
    #define __GLIBCXX_TYPE_INT_N_0 __int128

[As of July 2016-ish](https://bugs.llvm.org/show_bug.cgi?id=23156), Clang also
defines these two macros in `-std=gnu++XX` mode.

By the way, you can use the options `-dM -E` to view all your compiler's pre-defined macros:

    clang++ -std=c++17 -dM -E test.cc

The upshot is, if you compile in `-std=gnu++XX` mode, libstdc++ will treat `__int128` and
`unsigned __int128` as integral types with the proper numeric limits; but if you
compile in the usual `-std=c++XX` mode, libstdc++ will not do any of that.

On the other hand, libc++ treats `__int128` as an integer type, full stop.

> I've worked on several projects that compiled with `-std=c++17 -D_GNU_SOURCE`. That
> will give you a lot of GNU library extensions, but it will not give you `__int128` support.
> libstdc++ is looking specifically for the two macros defined above, and `-D_GNU_SOURCE`
> doesn't enable those macros.


## Relationship to integer types and `intmax_t`

    static_assert(std::is_integral_v<__int128>);
        // TRUE on libc++, "it depends" on libstdc++

libstdc++ (in standard, non-`gnu++XX` mode) leaves `is_integral_v<__int128>` as `false`.
This makes a certain amount of sense from the library implementor's point of view,
because `__int128` is not one of the _standard_ integral types, and furthermore,
if you _call_ it integral, then you have to face the consequence that `intmax_t`
(which is 64 bits on every ABI that matters) is kind of lying about being the
"max."

In `-std=gnu++XX` mode, libstdc++ makes `is_integral_v<__int128>` come out to `true`,
as described in the previous section on sneaky macros.

Meanwhile, libc++ simply
[sets](https://github.com/llvm-mirror/libcxx/blob/aeeba70ab15e1469841c0f3f1f9c78211a541699/include/type_traits#L738-L741)
sets `is_integral_v<__int128>` to `true`, unconditionally.

Yes, this means that on libc++ (or on libstdc++ with `-std=gnu++17`), there exist
integral values that do not fit in `intmax_t`! ([Godbolt.](https://godbolt.org/z/lMD4Iu))

> In case you're wondering: libstdc++ in standard C++17 mode considers `__int128` a "compound" type.
> The definition of "compound type" is simply "anything that's not a fundamental type,"
> where "fundamental" encompasses the arithmetic types (both integral and floating-point), `void`,
> and `nullptr_t`. So for example `int*` is also a compound type.


## Numeric limits

In standard (non-`gnu++XX`) mode, libstdc++ does not specialize `numeric_limits` for `__int128` at all.
This [has the unfortunate effect](https://godbolt.org/z/RpaD-7) of

    constexpr __int128 int128_max =
        std::numeric_limits<__int128>::max();
    static_assert(int128_max == 0);  // TRUE on libstdc++ in standard mode

libc++ (and libstdc++ in `gnu++XX` mode) provides the appropriate specialization, so that
`numeric_limits<__int128>::min()` and `numeric_limits<__int128>::max()`
have the appropriate values instead of zero.

On libc++ [`numeric_limits` reports `__int128` as an integral type](https://godbolt.org/z/ZR_nn6),
whereas on libstdc++ in standard mode it doesn't (again because there is no specialization for
`__int128` at all).

    static_assert(std::numeric_limits<I128>::is_integer);
        // TRUE on libc++, "it depends" on libstdc++


## Signedness

libc++ reports that `is_signed_v<__int128>` and `is_unsigned_v<unsigned __int128>`.
libstdc++ in standard mode of course denies both, because only arithmetic types can be
signed or unsigned, and libstdc++ denies that `__int128` is arithmetic.

As for `make_signed`, libc++ (and libstdc++ in `gnu++XX` mode) reports what you'd expect —
`make_signed<__uint128_t>::type` is `__int128_t`, and `make_unsigned<__int128_t>::type` is `__uint128_t`.

Trying to instantiate `make_signed<__uint128_t>` on libstdc++ in standard mode
[results in](https://godbolt.org/z/j7BvKB) a hard error which is not SFINAE-friendly.
(It's undefined behavior to instantiate `make_signed` of any type which is neither
integral nor an enumeration type.)

Incidentally, today I learned that `is_signed_v<float> == true` but [`make_signed_t<float>`
is a hard error](https://godbolt.org/z/t6v1CJ).

----

For more on `__uint128_t` arithmetic, see
["The abstraction penalty for wide integer math on x86-64"](/blog/2020/02/13/wide-integer-proof-of-concept/) (2020-02-13).
