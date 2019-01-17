---
layout: post
title: "Is this `std::vector`?"
date: 2019-01-17 00:02:00 +0000
tags:
  metaprogramming
---

![Is this std::vector?](/blog/images/2019-01-17-is-this-std-vector.jpg){: .float-right}
Consider [this snippet](https://godbolt.org/z/bX5sSC), whose behavior is
not surprising at all:

    #define Tc template<class...> class

    template<Tc T, Tc U> struct is_same : std::false_type {};
    template<Tc T> struct is_same<T, T> : std::true_type {};

    namespace A {
        using std::vector;
    }
    namespace B {
        template<class... Ts> using vector = std::vector<Ts...>;
    }

    static_assert(is_same<std::vector, std::vector>::value);
    static_assert(is_same<A::vector, std::vector>::value);
    static_assert(not is_same<B::vector, std::vector>::value);

----

And [this snippet](https://godbolt.org/z/VyyO0X):
just because `C<T>` is `vector<T>` doesn't mean `C<T>` is *always* `vector<T>`!

    #include <vector>
    #include <type_traits>

    template<template<class...> class C>
    struct Foo {
        static_assert(std::is_same_v<C<int>, std::vector<int>>);
        static_assert(not std::is_same_v<C<float>, std::vector<float>>);
    };

For example:

    template<class T> using Bar =
        std::conditional_t<std::is_same_v<T, int>, std::vector<int>, void>;

    template struct Foo<Bar>;  // OK
