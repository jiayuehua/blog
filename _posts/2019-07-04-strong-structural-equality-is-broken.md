---
layout: post
title: 'Enums break strong structural equality'
date: 2019-07-04 00:01:00 +0000
tags:
  operator-spaceship
  templates
  wg21
---

In early June 2019, Richard Smith came up with [this example C++03 code](https://godbolt.org/z/OV6olk).

    struct A {
        enum E { RED = 1, BLUE = 2 };

        friend const char *operator==(E, E) { return "hello world"; }
        friend double operator!=(E, E) { return 0.0; }
    };

    template<A::E Value>
    struct Example {};

    Example<A::RED> e1;
    Example<A::BLUE> e2;

Here `e1` and `e2` have different types. `Example<A::RED>` and `Example<A::BLUE>` are different types.
But that's not because `A::RED != A::BLUE`!  In fact `A::RED != A::BLUE` is `0.0`. And
`A::RED == A::BLUE` is `"hello world"`.

This new observation seriously shakes the foundation of C++2a's support for non-type template parameters (NTTPs)
of user-defined type. That foundation is a relatively novel notion called _strong structural equality_.
(See my previous post ["P0732R0 and trivially comparable"](/blog/2018/03/19/p0732r0-and-trivially-comparable/) (2018-03-19).)
Strong structural equality was supposed to give a compiler-checkable assurance that the user-defined type's
`operator==` actually implemented _equality_, as opposed to, say, returning `"hello world"`.
(That's one reason C++ doesn't let you use floating-point types as non-type template parameters: `double`'s
`operator==` doesn't implement _equality_ unless you handwave away `-0.0` and `NaN`.)
But for enum types, Richard observes, we don't have that assurance even today.

For the record, I was a big fan of P0732 strong structural equality (I merely lobbied to change its name away from
the original proposal's "trivial comparison"). However, given that it doesn't get us what we want, should it be scrapped?

C++2a strong structural equality is propagated in the same way as trivial destructibility.
So the following is a valid program according to the C++2a Working Draft right now.
(Plus [P1614R1](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1614r1.html) for `has_strong_structural_equality_v`.)

    struct A {
        enum E { RED = 1, BLUE = 2 };
    };
    static_assert(std::has_strong_structural_equality_v<A::E>);

    constexpr bool operator==(A::E, A::E) { return true; }
    constexpr bool operator!=(A::E, A::E) { return true; }

    // The value of `has_strong_structural_equality` doesn't change after the type is complete.
    static_assert(std::has_strong_structural_equality_v<A::E>);

    struct B {
        A::E value;
        constexpr B(A::E v) : value(v) {}
        auto operator<=>(const B&) const = default;
    };
    static_assert(std::has_strong_structural_equality_v<B>);

    template<B V>
    struct Boom {
        static_assert(V != B{A::RED});
    };

    int main() {
        Boom<B{A::RED}> t;  // Surprise! This is OK!
    }

----

There are other ideas floating around the Committee as to how to do NTTPs in a more general (and now,
"less inconsistent"?) way. The problem we need to avoid is still... well, something like the example
above, but in reverse.

    struct Rational {
        int numerator;
        int denominator;
        constexpr bool operator==(const Rational& rhs) const {
            return numerator * rhs.denominator == denominator * rhs.numerator;
        }
    };

    template<Rational X> int denom_of = X.denominator;

    constexpr Rational one_half{1,2};
    constexpr Rational three_sixths{3,6};
    static_assert(one_half == three_sixths);

In normal C++ programming, equals can be substituted for equals; and this
extends to template programming. If `1+1 == 2`, then `entity_of<1+1>`
should be exactly the same entity as `entity_of<2>`.

    static_assert(&denom_of<one_half> == &denom_of<three_sixths>);

But if the two instantiations of `denom_of` are the same entity, then they
_must_ have the same value —

    static_assert(denom_of<one_half> == denom_of<three_sixths>);

— and that's clearly nonsense!

[P0732 "Class types in non-type template parameters"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0732r2.pdf)
solved the issue by saying, more or less, "Look, we know a subset of types where this problem
doesn't ever arise, because for these types `==` _does_ mean actual substitutability.
Let's permit just these types for now, and punt on the remainder (such as `Rational`).
This is a conservative position..."

> since `(a <=> b) == 0` is equivalent to `a == b` for all valid non-type template arguments in C++17. [page 4]

Now that the position has been shown to be non-conservative after all, is it appropriate to revisit P0732?

To be clear: I wish the issue with `A::E` didn't exist! I wish the quotation from P0732 above
were actually true!  But `A::E` does exist, so now we know the quotation above is not true.
Should we incorporate this new knowledge into the design?


## The release of C++2a should be delayed past 2020

[Setting realistic deadlines is maybe an art, maybe a science](https://hackernoon.com/deadlines-that-are-doomed-from-the-beginning-21fd6960cd7e),
but regardless, the timetable must match the amount of work
to be done. C++2a has a _very_ big amount of work to be done, and work items of the form
"explore, implement, and get user feedback on _____" aren't necessarily parallelizable.

When is the best time to catch bugs: while the product is in development, or after it has been released to customers?
