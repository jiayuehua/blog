---
layout: post
title: "`decltype` of a non-static member"
date: 2021-02-10 00:01:00 +0000
tags:
  concepts
  implementation-divergence
  name-lookup
---

Today I learned ([[expr.prim.id]/3](http://eel.is/c++draft/expr.prim.id#general-3)):

> An _id-expression_ that denotes a non-static data member or non-static member function of a class can only be used:
>
> - as part of a class member access in which the object expression refers to the member's class or a class derived from that class, or
>
> - to form a pointer to member, or
>
> - if that _id-expression_ denotes a non-static data member and it appears in an unevaluated operand.

(Thanks to Jason Cobb on Slack for finding that standardese so quickly!)

The example in [expr.prim.id] clarifies the meaning of that last bullet point:

    struct S {
        int m;
    };
    int j = sizeof(S::m + 42);      // OK

Because the argument of `decltype` is also an unevaluated context, this pokes a
surprising hole in the ["You must write it three times"](https://www.youtube.com/watch?v=I3T4lePH-yA)
style of SFINAE:

    void foo(...);

    template<class T>
    auto foo(T) -> decltype((T::m + 42)) {
        return (T::m + 42);
    }

    int main() { foo(S{}); } 

    error: invalid use of non-static data member 'm'
        return (T::m + 42);
                ~~~^

----

Where it really gets confusing is that the second and third bullet points conflict with each other,
without indicating any kind of priority order. What happens if the _id-expression_ denotes a
non-static data member, and appears in an unevaluated operand, _and_ it is (perhaps) used to
form a pointer to member? How can the compiler tell whether an expression is intended to
"form a pointer to member"?

Consider [this Clang bug filed April 2019](https://bugs.llvm.org/show_bug.cgi?id=41561)
([Godbolt](https://godbolt.org/z/v4M1ah)):

    decltype(&S::m)   mp;  // all vendors agree this is `int S::*`
    decltype(&(S::m)) ip;  // GCC, MSVC, ICC agree this is `int*`,
                           // but Clang thinks it's still `int S::*`

Extra weird: If you give `S::m` an overloaded unary `operator&`, then Clang starts agreeing
that `&(S::m)` should be the result of applying `&` to `m`, rather than a pointer-to-member
expression. ([Godbolt.](https://godbolt.org/z/d1q981))

Vice versa, within a member function, MSVC thinks that `&m` denotes a pointer-to-member
([Godbolt](https://godbolt.org/z/PYTTKo)) — again, unless `S::m` has an overloaded `operator&`!

    struct S {
        int m;
        int mf();

        static void f() {
            decltype(&m) ip;  // GCC, Clang, ICC agree this is `int*`,
                              // but MSVC thinks it's `int S::*`

            decltype(&mf) fp;  // MSVC thinks this is `int (S::*)()`;
                               // other vendors think it's ill-formed

        }
    };

----

Lénárd Szolnoki points out that `requires`-expressions are defined in terms of
the behavior of `decltype((e))`, so the same corner cases appear there,
as well ([Godbolt](https://godbolt.org/z/9Kvf79)):

    template<class T>
    concept C = requires {
        { T::m + 42 } -> std::same_as<int>;
    };

    static_assert(C<S>);

Fortunately `UniformRandomBitGenerator`'s `min` and `max` members are member functions,
not member variables, so you would have to do some really silly stuff in order to
fool those constraints (even if `UniformRandomBitGenerator` were a standard concept,
which it's not). [Godbolt](https://godbolt.org/z/qMGhEj):

    template<class G, class R = typename G::result_type>
    concept UniformRandomBitGenerator = requires (G& g) {
        requires std::unsigned_integral<R>;
        { G::min() } -> std::same_as<R>;
        { G::max() } -> std::same_as<R>;
        { g() } -> std::same_as<R>;
    };

    struct Evil {
        using result_type = unsigned;
        unsigned operator()();
        static constexpr unsigned min_() { return 0; }
        static constexpr unsigned max_() { return 127; }
        unsigned (*min)() = min_;
        unsigned (*max)() = max_;
    };

    static_assert(UniformRandomBitGenerator<Evil>);

----

See also:

* ["SFINAE and `E1.E2`"](/blog/2020/11/01/sfinae-on-returnability)
