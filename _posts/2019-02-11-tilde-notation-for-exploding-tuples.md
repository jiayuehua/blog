---
layout: post
title: "Packs of packs"
date: 2019-02-11 00:01:00 +0000
tags:
  kona-2019
  language-design
  metaprogramming
  variadic-templates
---

Jason Rice's interesting (but IMHO too novel) proposal
[P1221 "Parametric Expressions"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1221r1.html)
is in the pre-Kona mailing. It's one of these big proposals that claims to solve a whole
bunch of perennial problems at once. I bet it _would_ solve maybe half of what it claims,
and if you poke a hole in one part, that wouldn't make the remainder unsalvageable.
I'm just going to poke a hole in a hypothetical extension of one part, for the record,
so that I don't have to keep repeating myself on Slack.

P1221 proposes that [this](https://godbolt.org/z/g2xkMb) should work:

    using make_pack(using auto... Is) {
        return Is;
    }

    int arr[] = {
        make_pack(1,2,3) ...
    };

Here `make_pack(1,2,3)` calls `make_pack` and returns not a _tuple_, but an actual _pack_ of
values, back to its caller.
(This is superficially similar to some other languages'
"multiple return values," but it's not really the same. Packs are still
[harder to manipulate](https://godbolt.org/z/6vkmJo)
than actual values, even in P1221's world.) In standard C++, packs are only ever permitted
to appear as _parameter packs_ — or as a special case, lambda-captures. Being able to
return a pack from a function would be a big change.

Back in February 2016, something like P1221's idea of "naked" packs was discussed on the
std-proposals list under the title
["Improve fundamentals of parameter packs."](https://groups.google.com/a/isocpp.org/forum/#!topic/std-proposals/ajLcDl8GbpA).
My first message in that thread referred to a previous discussion
titled ["RFC: Unpacking tuples to value sequences"](https://groups.google.com/a/isocpp.org/d/topic/std-proposals/KW2FcaRAasc/discussion)
(January 2016).
See also a later discussion of ["Variadic decomposition"](https://groups.google.com/a/isocpp.org/d/topic/std-proposals/5-PGd0AptEs/discussion)
(October 2016), in which Vicente J. Botet Escribá points to Mike Spertus's paper
[P0341 "parameter packs outside of templates"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0341r0.html)
(May 2016). I guess 2016 was a big year for naked packs!

In the January thread, I floated the idea of a core-language syntax `t~` where the postfix
tilde would mean "take this tuple-like object and explode it into a naked pack of its elements."
That is, I proposed that the P1221 example above ought to be writeable as

    int arr[] = {
        std::make_tuple(1,2,3)~ ...
    };

where `make_tuple` creates a `std::tuple` object, `~` explodes it back into a pack, and then
`...` expands that pack via the usual expression-rewriting rules.
Or, vice versa, we could express my postfix-tilde thing in terms of P1221's primitives
([Godbolt](https://godbolt.org/z/hmB7IE)):

    template<class> struct int_seq_;

    template<size_t... Is>
    struct int_seq_<std::index_sequence<Is...>> {
      static using apply() {
        return Is;
      }
    };

    using indices_of(using auto t) {
      return int_seq_<
          std::make_index_sequence<std::tuple_size_v<decltype(t)>>
      >::apply();
    }

    using postfix_tilde(using auto t) {
        return std::get<indices_of(t)>(t);
    }

    int arr[] = {
        postfix_tilde(std::make_tuple(1,2,3)) ...
    };

However, this tilde-notation idea runs into trouble with nested pack expansions. Consider:

    template<class> struct int_seq_;

    template<size_t... Is>
    struct int_seq_<std::index_sequence<Is...>> {
      static using apply() {
        return Is;
      }
    };

    using iota(using auto N) {
      return int_seq_<
          std::make_index_sequence<N>
      >::apply();
    }

    auto sum = [](auto... ints) {
        return (0 + ... + ints);
    };

    template<size_t N>
    constexpr int foo() {
        return sum(iota(N) ...);
    }

So far so good, right? `iota(N)` should return a pack, and then we expand that pack with `...` and
give its elements to `sum`, which adds them up.

    static_assert(foo<2>() == 3);
    static_assert(foo<3>() == 6);
    static_assert(foo<4>() == 10);

But now we spring the trap. If `iota(N)` takes a non-pack (`N`) and produces a pack from it,
then `iota(Ns)` must take a pack and produce from it a... pack of packs?

    template<size_t... Ns>
    void bar() {
        return sum(iota(Ns) ...);  // Uh-oh!
    }

Clearly `bar<1,2,3>()` will not compile as written. But what would we write if we _did_ want to
compute, say, `sum(iota(1)..., iota(2)..., iota(3)...)`?

Or what about this snippet?

    template<size_t N>
    void baz() {
        return sum(iota(iota(N)) ...);  // Uh-oh!
    }

This is the same trap as the previous example. `baz<3>()` should return `sum(iota(iota(3)) ...)`;
and we know that `iota(3)` is a pack containing the integers 0, 1, and 2; so `baz<3>()` should
return the same thing as `bar<0,1,2>()`. But what is that?

----

The equivalent in my tuple-exploding tilde notation would be

    template<size_t... Is>
    constexpr auto iota_helper(std::index_sequence<Is...>) {
        return std::make_tuple(Is...);
    }
    template<size_t N>
    constexpr auto iota() {
        return iota_helper(std::make_index_sequence<N>());
    }

    template<size_t N>
    constexpr int foo() {
        return sum(iota<N>()~ ...);  // Easy peasy
    }

    static_assert(foo<2>() == 3);
    static_assert(foo<3>() == 6);
    static_assert(foo<4>() == 10);

    template<size_t... Ns>
    void bar() {
        return sum(iota<Ns>()~ ...);  // Uh-oh! Does ... bind to Ns, or to ~?
    }

    template<size_t N>
    void baz() {
        return sum(iota< iota<N>()~ >()~ ...);  // Uh-oh x2!
    }

----

The power of C++11 parameter packs is that they are "vectorized" in only one dimension.
There's a set of primitive operators that can be applied to non-pack values — for example, `+` —
and there's a completely disjoint set of primitive operators that can be applied to packs —
namely, `...` and `sizeof...` (and that's all). So when the compiler sees `Ns + 1`, it knows
that you aren't really adding 1 _to the pack_; what you want to do is crack open the pack and
add 1 to each of its elements, producing a new pack. When the compiler sees `Ns + Ms`, it knows
that you aren't really adding _a pack and a pack_; what you mean is to crack open each pack
and add their corresponding elements, producing a new pack.

And when the compiler sees `f(Ns ...)`, it knows that you don't really mean to crack open the pack
and apply `...` _to each element_ (because `...` is an operator that applies only to packs).
What you must mean is to apply `...` _to the pack itself_, which causes a rewrite of the expression
into a comma-separated snippet like `elt1, elt2, elt3`, and then evaluation of `f(elt1, elt2, elt3)`
can proceed from there.

If C++ were ever to permit "packs of packs," then `Ns ...` would become a source of ambiguity.
(As we just saw.)
In `Ns ...`, do you mean to apply `...` to the pack itself, or do you mean to crack it open
and apply `...` to each of its elements, which are themselves also packs?
Either way, the meaning of the result would not be obvious: you'd end up with an expression rewritten
in terms of comma-separated unexpanded packs (whatever that means),
or else a pack of snippets like `elt1, elt2, elt3` (whatever that means).

Idle thought: Maybe C++17's [fold-expressions](https://en.cppreference.com/w/cpp/language/fold) and
APL's [inner-product notation](https://aplwiki.com/LearnApl/AplOperators)
can produce a baby that makes the whole thing make sense.
But I wouldn't hold my breath for that messiah.

TLDR: If you permit a function to "return a pack," then you permit the moral equivalent of `iota(N)`.
And if you permit that, then you permit the moral equivalent of `iota(Ns)` / `iota(iota(N))`.
And then you're in uncharted territory.
