---
layout: post
title: 'A compile-time benchmark for `enable_if` and `requires`'
date: 2021-09-04 00:01:00 +0000
tags:
  concepts
  llvm
  metaprogramming
---

A lot of templates in the C++ standard library are specified with _Constraints:_ elements,
which means that the library vendor must ensure that they drop out of overload resolution
when their constraints are not met — but the Standard does not mandate any particular
mechanism by which the vendor must implement that dropping-out. Two possible mechanisms are
SFINAE using `std::enable_if`, and adding C++20 constraints using the `requires` keyword.

libc++ used to use `std::enable_if` all over the place; but back in 2019 Eric Fiselier
switched libc++ from the standard `enable_if` to a slightly different technique:

    template<bool B> struct _MetaBase {};
    template<bool B> struct _MetaBase<true> {
        template<class T> using _Type = T;
    };
    template<bool B, class T = void>
    using _EnableIf = typename _MetaBase<B>::template _Type<T>;

Notice that the standard `enable_if_t` instantiates a brand-new struct type `enable_if<Bn,Tn>`
for each of the different `Tn`s you use it with; whereas `_EnableIf` only ever instantiates
two structs — `_MetaBase<true>` and `_MetaBase<false>`. Everything else is done via type aliases,
which according to the
[Rule of Chiel](https://www.reddit.com/r/cpp/comments/6gur2x/the_rule_of_chiel_aka_compiletime_cost_of/)
are supposed to be relatively cheaper.
See ["SCARY metafunctions"](/blog/2018/07/09/scary-metafunctions/) (2018-07-09).

In C++20, it's tempting to start using Concepts `requires`-clauses instead of
any version of `enable_if`. However, are `requires`-clauses any faster than the old way?
And was that `_EnableIf` rewrite actually providing any performance benefit? I decided to find out.

The average C++ program doesn't mess around with constrained templates nearly enough to register
on a compile-time benchmark, so I wrote a Python script ([here](/blog/code/2021-09-04-benchmark.py))
to generate translation units that stress `enable_if` in particular. I'd previously done something
similar for ["Don't blindly prefer `emplace_back` to `push_back`"](/blog/2021/03/03/push-back-emplace-back/#the-benchmark-program)
(2021-03-03).

My generator produces translation units that look like this:

    template<int N> requires (N==0) auto f(priority_tag<0>) -> A<0>;
    template<int N> requires (N==1) auto f(priority_tag<1>) -> A<1>;
    template<int N> requires (N==2) auto f(priority_tag<2>) -> A<2>;
    void test() {
        f<0>(priority_tag<3>{});
        f<1>(priority_tag<3>{});
        f<2>(priority_tag<3>{});
    }

except that it varies the number of overloads. (Shown: N=3. The actual number of overloads
in my benchmark varied from N=15 up to N=410.) The generator also varies the style of
SFINAE in four ways:

    template<int N> requires (N==42)
    auto f(priority_tag<42>) -> A<42>;

    template<int N>
    auto f(priority_tag<42>) -> enable_if_t<N==42, A<42>>;

    template<int N, class = enable_if_t<N==42>>
    auto f(priority_tag<42>) -> A<42>;

    template<int N, enable_if_t<N==42, int> = 0>
    auto f(priority_tag<42>) -> A<42>;

For the three non-`requires` versions, we have two different options for how to
implement `enable_if_t`: the standard way via `enable_if<B,T>::type`,
and the SCARY way via `_MetaBase<B>::_Type<T>`. And for each of _those_ we
have one more choice: Do we use the primary template for `B=false`
and make a custom specialization for `B=true`, or vice versa?

So that gives us 13 different ways to implement our SFINAE. Let's time the
compilation of each of these translation units and see if any particular way
outperforms the others.

## Benchmark results

Here's Clang trunk (the future Clang 14), running on my MacBook. The best-fit
curves shown here are quartics, and it's pretty amazing how exactly they fit the data.

![](/blog/images/2021-09-04-clang-results.png)

Here's GCC 10.3, running on RHEL 7.1. This data is much noisier, perhaps because the
machine is shared. The best-fit curves here are merely quadratics, because the best-fit
quartics ended up super wiggly.

![](/blog/images/2021-09-04-gcc10-results.png)

I observe only two significant results here:

* On Clang, C++20 `requires` is significantly _worse_ than the rest.

* On GCC 10.3, the extra-value-parameter method is significantly _better_ than the rest.

I have no particular explanation for either of these observations.

----

I don't have the ability to run this benchmark on MSVC, but if you run it,
send me the numeric results as a text file and I'll credit you here!

* Get my Python script [here](/blog/code/2021-09-04-benchmark.py).
