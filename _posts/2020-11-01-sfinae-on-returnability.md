---
layout: post
title: "SFINAE and `E1.E2`"
date: 2020-11-01 00:01:00 +0000
tags:
  concepts
  implementation-divergence
  templates
---

The other day on Slack someone asked whether there was anything that was
easier to express using classic SFINAE techniques than using C++20 Concepts
techniques. I suggested

    template<class T, class U>
    auto plus(T t, U u) -> decltype(t + u)
    {
        return t + u;
    }

This expresses the SFINAE constraint that this function shall not participate in
overload resolution unless `t + u` is a well-formed expression. To do the same
with Concepts, idiomatically, you might say something like

    template<class T, class U>
    concept Plussable = requires(T t, U u) { t + u; };

    template<class U, Plussable<U> T>
    decltype(auto) plus(T t, U u)
    {
        return t + u;
    }

And even then, I wondered, did the Concepts version really capture _everything_
about the original? The original expresses as part of its signature that its
_return type_ is the type of `t+u`; could this have a SFINAE effect?

I think the answer is "no," in the case of `t+u`. But consider:

    template<class T>
    auto dot1(T t) -> decltype(t.u)
    {
        return t.u;
    }

    template<class T> requires requires(T t) { t.u; }
    decltype(auto) dot2(T t)
    {
        return t.u;
    }

    void dot1(...);
    void dot2(...);

    struct A {
        static int u[];
    };
    struct A10 {
        static int u[10];
    };
    struct F {
        static int u(int);
    };
    struct MF {
        int u(int);
    };

[Godbolt yields](https://godbolt.org/z/4fYeYj) the following results:

|:-------------:|:-----------:|:--------------------------:|:----------------------:|:----:|
|               | GCC         | Clang                      | MSVC                   | ICC  |
|:-------------:|:-----------:|:--------------------------:|:----------------------:|:----:|
| `dot1(A())`   | `dot1(...)` | `dot1(...)`                | `dot1<A> -> int[]`     | `dot1(...)`
| `dot2(A())`   | error       | error                      | error                  | N/A
| `dot1(A10())` | `dot1(...)` | `dot1(...)`                | `dot1<A10> -> int[10]` | `dot1(...)`
| `dot2(A10())` | error       | error                      | error                  | N/A
| `dot1(F())`   | `dot1(...)` | `dot1<F>() -> int(&)(int)` | error                  | `dot1(...)`
| `dot2(F())`   | error       | `dot2<F>() -> int(&)(int)` | error                  | N/A
| `dot1(MF())`  | `dot1(...)` | `dot1(...)`                | error                  | `dot1(...)`
| `dot2(MF())`  | `dot2(...)` | error                      | error                  | N/A

I agree with GCC's behavior in all of these cases.

- `dot1<T>` should SFINAE away whenever `auto (T) -> decltype(t.u)` is not
    a well-formed function type, and that includes whenever `decltype(t.u)` is
    an array, function, or member-function type.

- Contrariwise, `dot2<T>` participates in overload resolution whenever `t.u`
    is well-formed, and so it's correct for each of its uses to give a hard
    (non-SFINAE-friendly) error.

The final case, `dot2(MF())`, is different from the rest. Here, the expression `t.u`
itself is ill-formed per [[expr.ref]/6.3.2](http://eel.is/c++draft/expr.ref#6.3.2):

> [When `E2` refers to a non-static member function], `E1.E2` is a prvalue.
> The expression can be used only as the left-hand operand of a member function call.

All four compilers agree that in normal code, even discarding the value counts as a "use":

    struct MF { int u(int); };
    void f1() { MF mf; mf.u; }  // error
    void f2() { MF mf; (void)mf.u; }  // error

However, every compiler has its own opinion about how this plays out in
a `requires`-expression. [Godbolt](https://godbolt.org/z/4aEndq):

    template<class T> concept C1 =
        requires (T t) { t.u; };
    template<class T> concept C2 =
        requires (T t) { (void)t.u; };

    static_assert(C1<MF> && C2<MF>);   // MSVC
    static_assert(C1<MF> && !C2<MF>);  // Clang
    static_assert(!C1<MF> && !C2<MF>); // GCC

Again, my impression is that GCC is correct and the others are (at time of writing) wrong.
