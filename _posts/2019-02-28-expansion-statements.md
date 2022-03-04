---
layout: post
title: 'Thoughts on P1306 "Expansion Statements"'
date: 2019-02-28 00:02:00 +0000
tags:
  kona-2019
  language-design
  metaprogramming
  variadic-templates
---

Daveed Vandevoorde's [P1306R1 "Expansion Statements"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1306r1.pdf)
proposes three interesting extensions to the language: `for... (auto arg : pack)` and `for... (constexpr int i : array)`
and `for... (auto arg : tuple)`.
I was in EWG in Kona when this paper was presented, and observed that the proposed syntax is slightly messed up
by the way it conflates two of these three distinct cases. But upon reflection, I'm not sure it's possible to solve the third
case without some conflation anyway.

Here are three examples of functions (or function templates) using P1306R1's proposed syntax.


## Example 1. Pack-expansion without constexpr.

    template<class... Ts>
    void f1(Ts&&... args) {
        for... (Ts rt : args) {
            std::cout << rt << std::endl;
        }
    }

The above snippet accepts a pack `args` and pack-expands it using P1306's `for...` construct. So it would
be almost exactly equivalent to [this working C++17 code](https://godbolt.org/z/n8lV0N):

    template<class... Ts>
    void f1a(Ts&&... args) {
        ([rt = Ts(args)]() {
            std::cout << rt << std::endl;
        }() , ...);
    }

Notice that if we're expanding a pack of `args`, we must have received that pack from a function parameter list, and
therefore the `args...` cannot possibly be constant expressions. Function parameters are never constant expressions.


## Example 2. Constexpr without pack-expansion.

    template<int N>
    constexpr std::array<int, N> make_iota() {
        std::array<int, N> result{};
        for (int i=0; i < N; ++i) result[i] = i;
        return result;
    }

    template<class Tuple>
    void f2(const Tuple& tuple) {
        constexpr auto arr = make_iota<std::tuple_size_v<Tuple>>();
        for... (constexpr int CT : arr) {
            std::cout << std::get<CT>(tuple) << std::endl;
        }
    }

The above snippet creates a constexpr array of indices and then uses them as arguments to `std::get`. So it would be essentially
equivalent to this C++17 code:

    template<class Tuple>
    void f2a(const Tuple& tuple) {
        std::cout << std::get<0>(tuple) << std::endl;
        std::cout << std::get<1>(tuple) << std::endl;
        std::cout << std::get<2>(tuple) << std::endl;
        [...]
    }

except that it would magically know how many indices to unroll for. Notice that in this case there is no pack-expansion taking
place within `f2`, nor even within `iota`. This snippet contains no variadic templates and no parameter-packs at all.
Therefore, it seems awkward and "wrong" that it contains the pack-expanding ellipsis `...` syntax.


## Example 3. Constexpr and pack-expansion together in the same example.

At the moment there is no way to get a heterogeneous pack of arguments that is also usable in constant expressions, because
packs can appear only as _parameter_ packs, and function parameters are never usable in constant expressions.

But you might reasonably ask, "How do I iterate over the elements of a `constexpr tuple` and do something with each of them?"
Tuples don't have `begin()` and `end()`, so the usual ranged-`for` loop won't work for them. But they also aren't packs,
so the kind of `for...` we used in Example 2 won't work for them.

You can turn a tuple back into a parameter-pack using `std::apply`:

    constexpr std::tuple<int, float, bool> tuple {42, 5.0f, true};

    auto f = [](auto... args) {
        // etc.
    };
    std::apply(f, tuple);

But as soon as `f` receives `args`, you lose the `constexpr`-ness of `args`! So we're back to Example 1.

If we want to iterate over a `constexpr tuple`, it seems that we must make up a special case.
P1306R1 uses the same `for...` grammar for Example 3 as it uses for Examples 1 and 2. We explained how removing the `...`
from Example 2 seems like a good idea; but even if we come up with nice natural syntax for Examples 1 and 2, we still
have to shoehorn Example 3 in somehow.

    void f3() {
        constexpr std::tuple<int, float, unsigned long> tuple {42, true, 15uL};
        for... (constexpr auto CT : tuple) {
            std::integral_constant<decltype(CT), CT> constant;  // OK
        }
    }

I don't have a great example for this one, because I can't immediately think of a situation where you'd want heterogeneity
*and* be able to achieve constexprness. However, you can see what's going on in this snippet: we make a `constexpr std::tuple`
and then do something with each of its elements. P1306 proposes that the feature work sort of like structured binding:

    void f3a() {
        constexpr std::tuple<int, float, unsigned long> tuple {42, true, 15uL};
        auto&& [args...] = tuple;
        for... (constexpr auto CT : args) {
            std::integral_constant<decltype(CT), CT> constant;  // OK
        }
    }

except that of course you can't say `auto&& [args...] = tuple;` in the working draft yet, either.

The other confusing thing about this third case is I wonder if you're supposed to be able to mix and match among the
conflated cases. For example:

    template<class... Ts>
    void f3b() {
        constexpr std::tuple<Ts...> tuple {42, true, 15uL};
        for... (constexpr Ts CT : tuple) {
            std::integral_constant<Ts, CT> constant;  // OK??
        }
    }

Here `CT`'s _type_ comes from a pack-expansion (Example 1), but its _value_ comes from structured-binding on a tuple (Example 3).


## Packs of packs, revisited

Because of its special case for `constexpr tuple`, P1306 hits
[a familiar problem: packs of packs](/blog/2019/02/11/tilde-notation-for-exploding-tuples/).

    template<class... Tuples>
    void f4(Tuples... tuples) {  // A
        for... (auto elt : tuples) {
            do_something_with(elt);
        }
    }

    void use() {
        f4(std::make_tuple(1, 2.0f), std::make_tuple(3, 4.0f));
    }

This clearly means to `do_something_with({1, 2.0f})` and then `do_something_with({3, 4.0f})`. The mention of `tuples` on line A
is referring to the pack, and `elt` is an element of the pack (that is, `elt` has type `tuple<int, float>`).
But:

    template<class... Tuples>
    void f5(Tuples... tuples) {
        ([&]() {
            for... (auto elt : tuples) { // A
                do_something_with(elt);
            }
        }() , ...)
    }

    void use() {
        f5(std::make_tuple(1, 2.0f), std::make_tuple(3, 4.0f));
    }

Now, the mention of `tuples` on line A must be referring to a single element of the pack (that is, `tuples` has type
`tuple<int, float>`) and `elt` must be referring to an element _of that tuple_ (that is, `elt` has type `int` or `float`).

I suspect that this can be used to create complicated ambiguities similarly to
["Packs of packs"](/blog/2019/02/11/tilde-notation-for-exploding-tuples/) (2019-02-11),
but I haven't fully thought it out yet.

You could definitely remove all ambiguity by eliminating the special case for `constexpr tuple`, and un-conflating
the cases for `constexpr array` and non-`constexpr` pack — that is, making the pack-expansion case use `for...` (but not
`constexpr` because packs are never `constexpr`) and making the array case use `for (constexpr auto i : arr)`.
It is unclear to me how frequently programmers want to iterate over constexpr tuples.
