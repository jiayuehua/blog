---
layout: post
title: 'Conditional `explicit` is not the same thing as forwarding `explicit`'
date: 2019-07-05 00:01:00 +0000
tags:
  metaprogramming
  templates
---

The C++2a Working Draft adds "conditional `explicit`" to the core language. So rather than writing
some icky constructor overload set like

    template<class T>
    struct Wrapped {
        T t;

        template<class U, std::enable_if_t<std::is_constructible_v<T, U&&> && std::is_convertible_v<U&&, T>, int> = 0>
        Wrapped(U&& u) : t(static_cast<U&&>(u)) {}

        template<class U, std::enable_if_t<std::is_constructible_v<T, U&&> && !std::is_convertible_v<U&&, T>, int> = 0>
        explicit Wrapped(U&& u) : t(static_cast<U&&>(u)) {}
    };

you can now write just one overload:

    template<class T>
    struct Wrapped {
        T t;

        template<class U, std::enable_if_t<std::is_constructible_v<T, U&&>, int> = 0>
        explicit(std::is_convertible_v<U&&, T>) Wrapped(U&& u) : t(static_cast<U&&>(u)) {}
    };

I noticed today (while working on the constructors for a `co_optional` for my
[`coro` examples library](/blog/2019/07/03/announcing-coro-examples/)) that "conditional `explicit`"
(no matter whether you do it the old-school way or the C++2a way)
is not quite the same thing as "forwarding `explicit`." That is, we have successfully constrained
this template's _interface_, but we have not constrained the implicit constructor's _implementation_.

[Godbolt:](https://godbolt.org/z/KT0f5B)

    struct A {
        A(int);
        explicit A(long);
    };

    using B = Wrapped<A>;

    A a = 42L; // A is constructed via A(int)
    B b = 42L; // A is constructed via A(long)

Generally speaking, C++ has problems with "passing around" things that aren't first-class citizens in the language.
(Or, likely, my notion of what it means for a thing to be "first-class" in a language is based on whether it's
possible to pass that thing around.)

Passing _values_ such as `42` from one place to another has worked basically since they invented computers.
Passing _references_ works great in C++, because references are first-class citizens. Passing value-category
(a.k.a. "perfect forwarding") _almost_ works, except that we've accumulated some inconsistencies around prvalues
as their definition has shifted post-C++11. Forwarding true/false notions such as `noexcept`-ness and _existence_
(in the [SFINAE-space](https://www.youtube.com/watch?v=ybaE9qlhHvw&t=1m19s) sense) tends to work great.
But forwarding overload sets? Forget about it.
Forwarding explicitness (per this blog post), which boils down to forwarding overload sets? Forget about it.
Forwarding C++2a Concepts constraints (in a way that preserves subsumption relationships)? Forget about it.

There's no particular moral here; I just thought it was noteworthy.
