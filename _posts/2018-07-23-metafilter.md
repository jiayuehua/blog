---
layout: post
title: 'Template metaprogramming: Iteration is better than recursion'
date: 2018-07-23 00:02:00 +0000
tags:
  compile-time-performance
  metaprogramming
  templates
  variadic-templates
---

Thanks to Jeff Trull for [this example](https://github.com/jefftrull/coroutine_experiments/blob/f86c720/meta.hpp#L62-L80)
of metaprogramming. Our goal is to make a `filter` metafunction that takes a typelist and returns a new typelist with
all non-matching elements filtered out. Here's our unit test:

    using Input = std::tuple<int, int&, char, char&>;
    using Output = std::tuple<int&, char&>;
    static_assert(std::is_same_v<
        filter<std::is_reference, Input>::type,
        Output
    >);

![expanding brain phase 1](/blog/images/2018-07-23-expanding-brain-phase-1.jpg){: .float-right}
And here's our original metaprogram (courtesy of Jeff). Most C++11 metaprograms, in my experience,
end up looking something like this. There's a forward declaration of `filter`, and then a recursive
case, and then a base case. Classic recursion.

    //
    // Filter types in a std::tuple by a template "predicate"
    //

    template<template<class> class Pred, class Sequence>
    struct filter;

    // recursive case
    template<template<class> class Pred, class H, class... Ts>
    struct filter<Pred, std::tuple<H, Ts...>> {
        using type = std::conditional_t<
            Pred<H>::value,
            decltype(std::tuple_cat(
                std::declval<std::tuple<H>>(),
                std::declval<typename filter<Pred, std::tuple<Ts...>>::type>()
            )),
            typename filter<Pred, std::tuple<Ts...>>::type
        >;
    };

    // base case
    template<template<class> class Pred>
    struct filter<Pred, std::tuple<>> {
        using type = std::tuple<>;
    };

![expanding brain phase 2](/blog/images/2018-07-23-expanding-brain-phase-2.jpg){: .float-right}
I observe that calling `conditional_t<B, T, F>` always fully evaluates both `T` and `F`,
even though it's going to throw out half of that work. In particular, even when the `Pred<H>::value`
is `false`, we are still instantiating `tuple_cat` — and function instantiation is basically the slowest
thing you can ask a compiler to do. So it should be more efficient to implement our branching in terms
of partial specializations rather than `conditional_t`:

    template<template<class> class Pred, class Sequence>
    struct filter;

    template<bool>
    struct helper {
        template<template<class> class Pred, class H, class... Ts>
        using type = decltype(std::tuple_cat(
            std::declval<std::tuple<H>>(),
            std::declval<typename filter<Pred, std::tuple<Ts...>>::type>()
        ));
    };

    template<>
    struct helper<false> {
        template<template<class> class Pred, class H, class... Ts>
        using type = typename filter<Pred, std::tuple<Ts...>>::type;
    };

    // recursive case
    template<template<class> class Pred, class H, class... Ts>
    struct filter<Pred, std::tuple<H, Ts...>> {
        using type = typename helper<Pred<H>::value>::template type<Pred, H, Ts...>;
    };

    // base case
    template<template<class> class Pred>
    struct filter<Pred, std::tuple<>> {
        using type = std::tuple<>;
    };

Now we instantiate `tuple_cat` only exactly as many times as `Pred<H>::value` is `true`.
(Here I am also trying to demonstrate my newfound appreciation for
[SCARY metafunctions](/blog/2018/07/09/scary-metafunctions/).)

![expanding brain phase 3](/blog/images/2018-07-23-expanding-brain-phase-3.jpg){: .float-right}
But we can still do better!

A really good rule of thumb for variadic-template metaprogramming is that

> Iteration is always cheaper than recursion

because recursion involves lots of "throwaway" intermediate entities, whereas iteration
often goes straight to the point. We should look for a way to avoid instantiating
all those intermediate specializations of `filter` and `tuple_cat`. And lo and behold,
[we find such a way!](https://wandbox.org/permlink/x9MKTFdc6DJXkGsj)

    template<template<class> class Pred, class Sequence>
    struct filter;

    template<bool>
    struct zero_or_one {
        template<class E> using type = std::tuple<E>;
    };

    template<>
    struct zero_or_one<false> {
        template<class E> using type = std::tuple<>;
    };

    // iterative case
    template<template<class> class Pred, class... Es>
    struct filter<Pred, std::tuple<Es...>> {
        using type = decltype(std::tuple_cat(
            std::declval<typename zero_or_one<Pred<Es>::value>::template type<Es>>()...
        ));
    };

----

In case you're wondering, no we did *not* just push the problem down another level: `std::tuple_cat` is
generally also implemented with iteration, not recursion.

By the way, implementing an iterative `tuple_cat`
is one of the 12 classroom exercises in my [upcoming two-day class at CppCon 2018](https://cppcon.org/the-standard-library-from-scratch/).
So if you want to be able to say "I implemented `tuple_cat` in 20 minutes at CppCon 2018," please consider signing up!
