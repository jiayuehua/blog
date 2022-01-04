---
layout: post
title: 'The `FORCE_CONSTEXPR` macro'
date: 2018-08-07 00:02:00 +0000
tags:
  constexpr
  pearls
  preprocessor
  sufficiently-smart-compiler
---

    #define FORCE_CONSTEXPR(expr) [&]() \
        { constexpr auto x = (expr); return x; }()

Wrap any expression in `FORCE_CONSTEXPR(...)` to *force* the compiler to evaluate it
completely at compile time (or fail the compilation if it cannot do so).

[This macro has a significant effect](https://godbolt.org/g/qXowwt) at `-O0` and `-O1`.
At `-O2` or higher, compilers seem good enough to do the constexpr evaluation
on their own (essentially as an emergent effect of the inliner, I think).

----

If you were — God knows why — doing this for real,
you'd need a separate macro to handle the global scope:

    #define GFORCE_CONSTEXPR(expr) []() \
        { constexpr auto x = (expr); return x; }()

    int global = GFORCE_CONSTEXPR(constexpr_sqrt(42.0));

Because Clang (alone out of the Big Three compilers) [does not support](https://godbolt.org/g/dqYfnx)
lambdas with capture-defaults at global scope!
