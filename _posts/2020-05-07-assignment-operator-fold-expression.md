---
layout: post
title: "Folding over `operator=`"
date: 2020-05-07 00:01:00 +0000
tags:
  blog-roundup
  order-of-evaluation
  variadic-templates
excerpt: |
  Jonathan Müller [posted](https://foonathan.net/2020/05/fold-tricks/#content) a
  "Nifty Fold Expression Trick" the other day:

      template<class F, class... Ts>
      void reverse_for_each(F f, Ts... ts) {
          int dummy;
          (dummy = ... = (f(ts), 0));
      }

  For example, `reverse_for_each(putchar, 'a', 'b', 'c')` prints `cba`.

  However, as I puzzled out each step of the process, I realized that there
  were several subtleties to this "simple" expression!
---

Jonathan Müller [posted](https://foonathan.net/2020/05/fold-tricks/#content) a
"Nifty Fold Expression Trick" the other day:

    template<class F, class... Ts>
    void reverse_for_each(F f, Ts... ts) {
        int dummy;
        (dummy = ... = (f(ts), 0));
    }

For example, `reverse_for_each(putchar, 'a', 'b', 'c')` prints `cba`.

However, as I puzzled out each step of the process, I realized that there
were several subtleties to this "simple" expression!


## False start #1

Most C++ programmers don't need to know anything about fold-expressions.
But even among those who do know something about them, the "layman's view"
of fold-expressions is basically that `(x + ... + E(ts))` expands to
`(x + E(t0) + E(t1) + E(t2))`. If we apply this "layman's version" of fold-expressions
to the above code, we get this instantiation for three `char` parameters:

    // Layman's version -- INCORRECT!
    template<>
    void reverse_for_each(decltype(puts) *f, char a, char b, char c) {
        int dummy;
        (dummy = (f(a), 0) = (f(b), 0) = (f(c), 0));
    }

[But this clearly shouldn't compile!](https://godbolt.org/z/qqzPXC)
We know that assignments _can_ be "chained" like this: `x = y = z` means "assign `z` to `y`, then assign `y` to `x`."
But `(f(a), 0)` is an rvalue, not an lvalue.

    (f(a), 0) = 42;  // ERROR, can't assign to an rvalue

What went wrong with our layman's version of fold-expressions?


## False start #2

It turns out that in C++, fold-expressions bring their own associativity with them.
The expression `(x + ... + E(ts))` is called a [_binary left fold_](https://en.cppreference.com/w/cpp/language/fold),
and is equivalent to

    (((x + E(t0)) + E(t1)) + E(t2))

The expression `(E(ts) + ... + x)` is called a _binary right fold_, and is equivalent to

    (E(t0) + (E(t1) + (E(t2) + x)))

So Jonathan's binary left fold over `=` is actually expanded by the compiler
into this:

    // Actual expansion -- CORRECT
    template<>
    void reverse_for_each(decltype(puts) *f, char a, char b, char c) {
        int dummy;
        (((dummy = (f(a), 0)) = (f(b), 0)) = (f(c), 0));
    }

That is, what we have here is not structured like `x = y = z`; it's structured
like `(x = y) = z`. First we assign `y` to `x`; then we assign `z` to `x`.

But wait! If `(x = y) = z` is basically equivalent to `x = y; x = z`, then
why does Jonathan's fold-expression seem to evaluate `z` _before_ `y`?


## Guaranteed order of evaluation

The final trick here is C++17's guaranteed order of evaluation.
"Order of evaluation" differs from "order of operations" (a.k.a. "precedence"),
which of course C++ has always provided. "Order of operations" is
about which of `*` and `+` is executed first in an expression like `f() * g() + h()`.
"Order of evaluation" is about which of `f()` and `g()` is executed first.

In C++17, it is guaranteed that an assignment expression like `x = y`
proceeds in the following order: Evaluate `y`; then evaluate `x`; then
assign `y` to `x`. This ensures that an expression like

    a[++i] = b[i];

has defined behavior in C++17: if `i` is initially `42`, then this
expression assigns `b[42]` to `a[43]`. Certain other operators, notably
`<<` and `>>`, have guaranteed left-to-right order of evaluation. Most
operators have no guaranteed order of evaluation: `f() * g()` might evaluate
`g()` before `f()`, or vice versa.

So these two C++ constructs produce the same output:

    // Example 1
    a() = b() = c();

    // Example 2
    auto& ref = (b() = c());
    a() = ref;

But these two constructs produce different output:

    // Example 3
    (a() = b()) = c();

    // Example 4
    auto& ref = (a() = b());
    ref = c();

Example 3 _evaluates_ `c()` first; then evaluates `(a() = b())`; then assigns the result
of `c()` to `ref`.

Example 4 _evaluates_ `(a() = b())` first; then evaluates `c()` and assigns the result to `ref`.

----

So Jonathan's binary left fold

    (dummy = ... = (f(ts), 0));

actually does end up assigning `(f(a), 0)` to `dummy`, and then assigning
`(f(b), 0)` to `dummy`, and then assigning `(f(c), 0)` to `dummy`. The _assignment operations_
do happen in that order, because of the associativity implied by a left fold.
But because of C++17's guaranteed right-to-left order of _evaluation_,
the right-hand sides of those assignments are actually _evaluated_ in right-to-left
order regardless. In other words:

    (((dummy = (f(a), 0)) = (f(b), 0)) = (f(c), 0));

means roughly

    auto& refc = (f(c), 0);
    auto& refb = (f(b), 0);
    auto& refa = (f(a), 0);
    (((dummy = refa) = refb) = refc);


## Alternative formulation of `reverse_for_each`, and a guideline

It seems to me that if you're doing a "reverse-for-each", you should prefer
to use a _right_ fold expression ([Godbolt](https://godbolt.org/z/L4iR32)):

    template<class F, class... Ts>
    void reverse_for_each(F f, Ts... ts) {
        int dummy;
        ((f(ts), dummy) = ... = 0);
    }

This removes the first layer of subtlety in Jonathan's version, because
now we can return to our "layman's intuition" about fold-expressions:

    (E(ts) = ... = x);

is in all respects equivalent to

    E(t0) = E(t1) = E(t2) = x;

This seems like a good candidate for a style guideline.

> Use left folds only for left-associative operators.  
> Use binary right folds only for right-associative operators.


## Bonus advanced guideline

> Use unary right folds only for right-associative operators,
> the short-circuiting logical operators, and comma.

For the short-circuiting operators and comma, a unary right fold does exactly
the same thing as a unary left fold, but right fold is slightly easier to read
(in my opinion).

    Unary right fold: (ts || ...)  ==> (t0 || (t1 || t2))

    Unary left fold:  (... || ts)  ==> ((t0 || t1) || t2)

Coincidentally, the short-circuiting logical operators and comma are also the only ones
that fully support the unary fold syntax even for empty packs. For every other
operator, if `sizeof...(ts)` might be zero, then you _must_ use a binary fold.

Notice that this applies only to the _built-in_ operators `||` and `&&` and `,`!
If `ts...` might overload these operators to do user-defined things, then the
difference between a right fold and a left fold is again observable —
and I think you should prefer left fold because it matches our "layman's intuition"
about order of evaluation.
