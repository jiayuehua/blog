---
layout: post
title: "Does `throw x` implicit-move? Let's ask SFINAE"
date: 2021-03-18 00:01:00 +0000
tags:
  implementation-divergence
  implicit-move
---

> Language lawyers may want to read the UPDATE at the bottom of this post
> before the rest. I think everything discussed here is, technically speaking,
> ill-formed.

As of C++20, thanks to [P1155 "More implicit move,"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1155r3.html)
`throw x` gets the same implicit-move semantics as `return x`. So you can do, for example,

    std::unique_ptr<int> a(std::unique_ptr<int> p)
    {
        auto v = std::make_unique<int>(1);
        return v;  // OK: implicit move (since C++11ish)
        return p;  // OK: implicit move (since C++11ish)
    }

    void b(std::unique_ptr<int> p)
    {
        auto v = std::make_unique<int>(1);
        throw v;  // OK: implicit move (since C++14ish)
        throw p;  // OK: implicit move (since C++20)
    }

Here's a handy table of compiler support for implicit move in each of these situations;
i.e., does the compiler accept the given expression for a move-only type like `unique_ptr`?
Answer: Yes, in all modes — but GCC's dev team likes to complicate things. GCC alone sticks
to the formal C++17 standard and rejects `throw p` pre-C++20.

|------------|-----|-------|------|-----|
| Expression | GCC | Clang | MSVC | ICC |
|------------|-----|-------|------|-----|
| return v   | ∞   | ∞     | ∞    | ∞   |
| return p   | ∞   | ∞     | ∞    | ∞   |
| throw v    | ∞   | ∞     | ∞    | ∞   |
| throw p    | 20+ | ∞     | ∞    | ∞   |

This week Erich Keane pushed me to think about how the (past and future) changes to "implicit move"
may affect SFINAE. Consider the following test program ([Godbolt](https://godbolt.org/z/1197dM)):

    template<class T>
    auto f(T p, int) -> decltype(throw p)
    {
        puts("one");  // #1
        throw p;
    }

    template<class T>
    auto f(T p, long) -> void
    {
        puts("two");  // #2
        throw p;
    }

    int main() {
        f(std::make_unique<int>(42), 42);
    }

The best-matching overload is #1, but it participates only when `throw p` is well-formed. (Btw: when
`throw p` _is_ well-formed, its decltype is invariably `void`.) When `throw p` is ill-formed, #1 drops out
of the overload set; #2 is the best remaining match.

* Clang (in all modes) accepts the program and calls #1.

* MSVC (in all modes) accepts the program and calls #1.

* Intel ICC 2021.1.2 (in all modes) accepts the program and calls #2.

* GCC (in all modes, including C++20) gives a hard error trying to check the well-formedness of `throw p`.
    It reports that the expression attempted to call `unique_ptr`'s deleted copy constructor — which is
    correct — but I'm surprised that GCC doesn't consider this to be an "immediate context"
    inside which "substitution failure is not an error."

See, the well-formedness of `throw p` depends on whether you think it's allowed to implicit-move from `p`.
If implicit move is allowed on the operand of `throw`, inside an unevaluated expression, then it's well-formed
(this is what Clang and MSVC think); otherwise it's ill-formed (this is what ICC thinks); and I have no idea
what GCC is thinking.

> ICC 19.0.1's behavior actually matched GCC's: hard error in all modes!

There's a disturbing parallel here to `decltype(co_await x)`. The Committee decided that `co_await` was
context-sensitive enough that C++20 forbids all use of the `co_await` operator inside unevaluated expressions.
`throw` will never reach `co_await`'s extreme level of context-dependence; but it is a _little_
context-dependent, and that is bad because it produces implementation divergence.


## Looking ahead to P2266 "Simpler implicit move"

Fortunately, the divergence has already happened. ([The program above](#this-week-erich-keane-pushed-me) is only C++14,
but gives three different answers on Clang/MSVC, ICC, and GCC.) My hope is that by
simplifying the rules around "implicit move" (particularly, junking the "two overload resolutions" dance),
[P2266 "Simpler Implicit Move"](https://wg21.link/p2266) will lead to implementation _convergence_.

Here is a program that uses SFINAE to detect whether the compiler is fully in P2266-world.
(Note that in real life, you wouldn't use metaprogramming for this; you'd just check the feature-test macro.)
[Godbolt](https://godbolt.org/z/q5Mf8c):

    struct AutoPtr {
        AutoPtr() = default;
        AutoPtr(AutoPtr&) {}
    };

    template<class T>
    auto f(T p, int) -> decltype(throw p, 1) { return 1; }

    template<class T>
    int f(T p, long) { return 2; }

    int main() {
        return f(AutoPtr(), 42);
    }

In C++20, all vendors agree that `main` returns 1. In P2266-world, the intent is that `throw p` be
ill-formed (because `p` is a move-eligible _id-expression_, therefore an xvalue; but `AutoPtr` is
constructible only from lvalues), and so `main` should return 2.

At least, that's how I think `decltype(throw p)` should work. What do you think?

----

UPDATE, 2021-03-19: Reddit commenter "scatters" points out that C++20 `requires`-expressions
permit us to express the trouble even more cleanly:

    template<class T> requires requires (T p) { throw p; }
    void f(T p) { return 1; }

    template<class T>
    void f(T p) { return 2; }

    int main() { return f(AutoPtr()); }

Clang and MSVC call #1; GCC hard-errors; ICC doesn't support `requires` yet.

Interestingly, MSVC believes even that `requires (T& p) { throw p; }` is true,
although it clearly is not. This discovery reminded me that actually _everything discussed
in this entire post is ill-formed, diagnostic required._ See
["MSVC can’t handle move-only exception types"](/blog/2019/05/11/msvc-what-are-you-doing/)
(2019-05-11) and [[except.throw/5](http://eel.is/c++draft/except.throw#5)].
