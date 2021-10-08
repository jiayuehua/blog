---
layout: post
title: "A macro for migrating to `static operator()`"
date: 2021-10-07 00:01:00 +0000
tags:
  pearls
  preprocessor
  proposal
---

[P1169 "`static operator()`"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p1169r2.html)
(Barry Revzin and Casey Carter, August 2021) proposes that we should be able to define
`operator()` as a static member function. This is a very good idea.
As of this writing, it also looks likely to get into C++2b. So instead of e.g.

    struct ByAuthor {
        bool operator()(const Book& a, const Book& b) const {
            return a.author() < b.author();
        }
    };

we'll soon be able to write simply

    struct ByAuthor {
        static bool operator()(const Book& a, const Book& b) {
            return a.author() < b.author();
        }
    };

This may avoid the overhead of passing an unused `this` pointer (when the call
isn't simply inlined away), and it certainly can't hurt.

Jonathan Müller recently posed a question about migration strategy. If your codebase
currently uses non-static `operator() const`, but wants to use static `operator()`
in C++2b, then how should you write your code so that it will be valid
in both C++20 and C++2b?

I responded:

    #if __cplusplus > 202002L
     #define CALL_OPERATOR(...) static operator()(__VA_ARGS__)
    #else
     #define CALL_OPERATOR(...) operator()(__VA_ARGS__) const
    #endif

    struct ByAuthor {
        bool CALL_OPERATOR(const Book& a, const Book& b) {
            return a.author() < b.author();
        }
    };

This works because C++'s declaration syntax treats `bool static f()`
and `static bool f()` synonymously.

----

P1169 does have some downsides: for example, it proposes that captureless
lambdas' call operators must be _non-static_ by default, unless you manually
cruft them up with yet another trailing keyword: `[](int x) static { return x + 1; }`.
I'd rather that compilers and/or platform-ABI owners be free to do this optimization
themselves, without any additional keywords. But P1169 is definitely a step forward
for the non-lambda case, and maybe lambdas will catch up in a few years.

> CppCon Pub Quiz practice: Which six C++20 keywords can _already_
> appear in trailing position, directly after a lambda's parameter-list?
>
> [UPDATES, 2021-10-08: Changed "three" to "four," then again to "six."
> Thanks to Reddit commenters "SentientChowMein" and "i_lack_chromosomes"
> for pointing out that while there are currently only three _decl-specifiers_
> that can appear in trailing position, there are several more _keywords_
> that can appear there.]

----

See also:

* ["`static constexpr unsigned long` is C++’s 'lovely little old French whittling knife'"](/blog/2021/04/03/static-constexpr-whittling-knife/) (2021-04-03)
