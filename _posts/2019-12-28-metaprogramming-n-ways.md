---
layout: post
title: 'Comparative TMP #1: MPL, Mp11, Kvasir, Hana, Metal'
date: 2019-12-28 00:01:00 +0000
tags:
  metaprogramming
  templates
---

I thought it'd be interesting to take some well-known template metaprogramming libraries
and show how a simple (but non-trivial) program looks, in each respective style. So here's
my first entry in that vein. I've solved the same problem, in roughly the same way, in
five different metaprogramming libraries:

- Boost.MPL (Aleksey Gurtovoy and Dave Abrahams, 2001—)

- Boost.Mp11 (Peter Dimov, 2015—)

- Metal (Bruno Dutra, 2015—)

- Kvasir.MPL (Odin Holmes and Chiel Douwes, 2017—)

- Boost.Hana (Louis Dionne, 2013—)

MPL, Mp11, and Hana are all part of mainline Boost. Both Boost and Kvasir.MPL are
preinstalled on Godbolt Compiler Explorer. Metal is _not_ preinstalled,
but since it's single-header, you can include it [like this](https://godbolt.org/z/_qHpeC).

    #include <https://raw.githubusercontent.com/brunocodutra/metal/standalone/metal.hpp>

Neat, right?!

----

For a similar qualitative comparison of Kvasir, Mp11, Metal, Meta, and Brigand,
see ["Tradeoffs of TMP MPL design"](https://odinthenerd.blogspot.com/2017/03/tradeoffs-of-tmp-mpl-design.html)
(Odin Holmes, March 2017).

If you liked this post, you might also like
["Four versions of Eric's Famous Pythagorean Triples Code"](/blog/2019/03/06/pythagorean-triples/) (2019-03-06).


## The problem statement

Define an alias template `SortedAndFiltered` that takes a user-defined typelist,
filters out empty classes, and then sorts its input in descending size order.
([Here's the test cases fleshed out in Godbolt.](https://godbolt.org/z/RMKnKx))

    template<class...> struct TypeList;  // incomplete
    struct Z {};
    static_assert(std::is_same_v<
        SortedAndFiltered<TypeList<int[2], Z, int[1], int[4], int[3], Z>>,
        TypeList<int[4], int[3], int[2], int[1]>
    >);
    static_assert(std::is_same_v<
        SortedAndFiltered<TypeList<Z, Z, char, Z, Z, double, Z, Z>>,
        TypeList<double, char>
    >);

I'm not terribly concerned with performance (either compile-time or runtime);
in this comparison I'm concerned mainly with readability and with trying to
showcase code that is "idiomatic" in each style. But be warned: I have
essentially zero experience with any of these libraries, so my sense of what
is "idiomatic" is likely to be miscalibrated! If you see something that could
be expressed more cleanly, please shoot me an email or something.


## Hand-rolled

This hand-rolled solution "cheats" in at least two ways. First of all, it
uses a "recursive template" to implement `cat`, which is going to be pretty slow.
(People who took my [_STL from Scratch_ course](/blog/2019/06/21/stl-from-scratch-at-cppcon-2019/)
at previous CppCons will remember that we can implement `tuple_cat` to do
all its work at once, non-recursively, which should be a lot faster for large lists.)
Secondly, it uses another recursive template to do a selection sort. I initially wrote a
"counting sort," which is non-recursive and slightly shorter in lines-of-code,
but it ran the compiler out of memory if you fed it `TypeList<int[1], int[1'000'000]>`.

    template<class... Ts> struct typelist {
        template<template<class...> class Tc>
        using as = Tc<Ts...>;
    };
    template<class...> struct cat;
    template<> struct cat<> { using type = typelist<>; };
    template<class Head, class... Tail>
    struct cat<Head, Tail...> {
        template<class... Hs, class... Ts>
        static typelist<Hs..., Ts...> f(typelist<Hs...>, typelist<Ts...>);
        using type = decltype(f(Head{}, typename cat<Tail...>::type{}));
    };
    template<bool> struct if_ {
        template<class T> using maybe = typelist<T>;
    };
    template<> struct if_<false> {
        template<class T> using maybe = typelist<>;
    };

    struct Max {
        int value;
        constexpr Max operator*(Max b) const {
            return Max{value > b.value ? value : b.value};
        }
    };

    template<class> struct SortedAndFilteredImpl;
    template<template<class...> class Tc>
    struct SortedAndFilteredImpl<Tc<>> { using type = Tc<>; };
    template<template<class...> class Tc, class... Ts>
    struct SortedAndFilteredImpl<Tc<Ts...>> {
        static constexpr int biggest_size = (Max{sizeof(Ts)} * ... * Max{0}).value;

        using big_ones = typename cat<
            typename if_<sizeof(Ts) == biggest_size && !std::is_empty_v<Ts>>::template maybe<Ts> ...
        >::type;
        using small_ones = typename cat<
            typename if_<sizeof(Ts) != biggest_size>::template maybe<Ts> ...
        >::type;

        using type = typename cat<
            big_ones,
            typename SortedAndFilteredImpl<small_ones>::type
        >::type::template as<Tc>;
    };

    template<class TL>
    using SortedAndFiltered = typename SortedAndFilteredImpl<TL>::type;


## Boost.MPL

[Boost.MPL](https://github.com/boostorg/mpl) is super classical TMP,
full of `::type`s and weird tricks with placeholders such as `boost::mpl::_`.
Notably, it does not seem to have any way to turn an `mpl::vector<Ts...>`
back into a `TypeList<Ts...>` except through hand-rolled code. Another
disadvantage of MPL for blogging purposes is that
[it has no "all.hpp" header](https://www.boost.org/doc/libs/1_72_0/libs/mpl/doc/tutorial/physical-structure.html);
you have to `#include` a whole zoo of little headers.

Peter Dimov informs me that my `ToTc`-in-terms-of-`mpl::at` is unidiomatic,
or at least anachronistic; remember that Boost.MPL was designed for C++03,
which has no variadic templates. I'm not sure how to do it "idiomatically"
in a way that still passes all my test cases.

    #include <boost/mpl/at.hpp>
    #include <boost/mpl/greater.hpp>
    #include <boost/mpl/int.hpp>
    #include <boost/mpl/size.hpp>
    #include <boost/mpl/sizeof.hpp>
    #include <boost/mpl/sort.hpp>
    #include <boost/mpl/remove_if.hpp>
    namespace lib = boost::mpl;

    template<template<class...> class, class, class> struct ToTcImpl;

    template<template<class...> class Tc, class V, size_t... Is>
    struct ToTcImpl<Tc, V, std::index_sequence<Is...>> {
        using type = Tc<
            typename lib::at<V, lib::int_<Is>>::type ...
        >;
    };

    template<template<class...> class Tc, class V>
    using ToTc = ToTcImpl<
        Tc, V, std::make_index_sequence<lib::size<V>::value>
    >;

    template<class> struct SortedAndFilteredImpl;

    template<template<class...> class Tc, class... Ts>
    struct SortedAndFilteredImpl<Tc<Ts...>> {
        using type = typename ToTc<
            Tc,
            typename lib::sort<
                typename lib::remove_if<
                    lib::vector<Ts...>,
                    std::is_empty<lib::_>
                >::type,
                lib::greater<lib::sizeof_<lib::_1>, lib::sizeof_<lib::_2>>
            >::type
        >::type;
    };

    template<class TL>
    using SortedAndFiltered = typename SortedAndFilteredImpl<TL>::type;


## Boost.Mp11

[Mp11](https://github.com/boostorg/mp11) is perhaps the
most portable and battle-tested of the C++11 libraries shown here.
It is also one of the most readable, thanks to the way it transparently
treats any `Tc<Ts...>` as a list. Every other solution
has to spend at least two lines of code transforming `TypeList<Ts...>`
into `lib::something<Ts...>` and back again. In Mp11, _it just works._

(Thanks to Ilya Popov for improving this code!)

    #include <boost/mp11/algorithm.hpp>
    #include <boost/mp11/integral.hpp>
    namespace lib = boost::mp11;

    template<class T1, class T2>
    using SizeofGT = lib::mp_bool<(sizeof(T1) > sizeof(T2))>;

    template<class TL>
    using SortedAndFiltered = lib::mp_sort<
        lib::mp_remove_if<TL, std::is_empty>,
        SizeofGT
    >;


## Metal

[Metal](https://github.com/brunocodutra/metal) has the best documentation
of any of these libraries; see the docs [here](https://brunocodutra.github.io/metal/index.html).
It also wins the readability contest, due to its plethora of handy adaptors such as
`metal::as_lambda` and `metal::trait`: it eliminates almost all of the helper classes
that the other solutions needed.

    #include <metal/metal.hpp>
    namespace lib = metal;

    template<class T1, class T2>
    using SizeofGT = lib::number<(sizeof(T1) > sizeof(T2))>;

    template<class TL>
    using SortedAndFiltered = lib::apply<
        lib::as_lambda<TL>,
        lib::sort<
            lib::remove_if<
                lib::as_list<TL>,
                lib::trait<std::is_empty>
            >,
            lib::lambda<SizeofGT>
        >
    >;


## Kvasir.MPL

[Kvasir.MPL](https://github.com/kvasir-io/mpl) isn't super well documented.
The best documentation I've found is
[here](http://kvasir.io/mpl/doc/standardese/standardese_entities.html).
Notice Kvasir's distinctive "continuation" style: to express that we want to
`unpack` and _then_ `remove_if`, we pass `remove_if<...>` as the continuation
parameter of `unpack` — and `sort<...>` as the continuation parameter of
`remove_if`, and so on. This means that the nesting of the primitives appears
"reversed" relative to the more traditional MPL/Mp11/Metal.

    #include <kvasir/mpl/mpl.hpp>
    namespace lib = kvasir::mpl;

    template<class T1, class T2>
    using SizeofGT = lib::bool_<(sizeof(T1) > sizeof(T2))>;

    template<class> struct AsLambda;
    template<template<class...> class Tc, class... Ts>
    struct AsLambda<Tc<Ts...>> : lib::cfe<Tc> {};

    template<class TL>
    using SortedAndFiltered = lib::call<
        lib::unpack<
            lib::remove_if<
                lib::cfe<std::is_empty>,
                lib::sort<
                    lib::cfe<SizeofGT>,
                    AsLambda<TL>
                >
            >
        >,
        TL
    >;


## Boost.Hana

Louis Dionne's [Hana](https://github.com/boostorg/hana)
has the most distinctive appearance of any of these libraries.
([Docs here.](https://www.boost.org/doc/libs/1_61_0/libs/hana/doc/html/index.html#tutorial-quickstart))
The helper that we had to out-of-line as `SizeofGT` in other versions
can be done in Hana as a plain old generic lambda.

I doubt that my way of unpacking `TL` into `Ts...` is idiomatic;
this is the version I most expect to have to update in response to Reddit comments.
For the difference between `hana::type` and `hana::basic_type`, see
["How `hana::type<T>` disables ADL"](/blog/2019/04/09/adl-insanity-round-2/) (2019-04-09).

    #include <boost/hana.hpp>
    namespace lib = boost::hana;

    template<template<class...> class Tc, class... Ts>
    constexpr auto SortAndFilter(lib::basic_type<Tc<Ts...>>) {
        auto filtered = lib::remove_if(
            lib::tuple_t<Ts...>,
            lib::traits::is_empty
        );
        auto sorted = lib::sort(
            filtered,
            [](auto t1, auto t2) {
                return lib::sizeof_(t1) > lib::sizeof_(t2);
            }
        );
        return lib::unpack(sorted, lib::template_<Tc>);
    }

    template<class TL>
    using SortedAndFiltered = typename decltype(
        SortAndFilter(lib::type_c<TL>)
    )::type;
