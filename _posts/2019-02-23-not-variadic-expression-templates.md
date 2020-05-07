---
layout: post
title: "Not variadic expression templates"
date: 2019-02-23 00:01:00 +0000
tags:
  kona-2019
  metaprogramming
  pitfalls
  variadic-templates
---

At the Kona committee meeting, it was voted (by someone, probably LEWG, I forget the story,
I wasn't in the room) to eliminate the overloaded `operator()` from C++2a `std::span`.
Now it has only `operator[]`.

The overloaded `operator()` was there merely for consistency with `std::mdspan`,
which isn't slated for C++2a.

`std::mdspan` needed an overloaded `operator()` because it couldn't use overloaded `operator[]`
because `operator[]` is a purely binary operator: `s[a]` means `s.operator[](a)`, but
`s[a,b]` means `s.operator[]((a,b))` which means the same thing as `s.operator[](b)` because
that's what the comma operator does.

According to the minutes of the Kona meeting (again I wasn't in the room), Corentin Jabot's
[P1161R2 "Deprecate uses of the comma operator in subscripting expressions"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1161r2.html)
(October 2018) was forwarded to CWG for inclusion in C++2a. This is great news!

Because the next thing that could happen, after a period of deprecation, is that C++2b might
finally allow multi-argument `operator[]` overloads, the same way C++98 allowed multi-argument
`operator()` overloads. Then `mdspan` (which is slated for C++2b) could use a multi-argument
`operator[]`: you could index a single span as `s[a]` and a multi-dimensional mdspan as `s[a,b]`.
(Of course then we'd *really* have to argue about whether C++ has multidimensional arrays or
just single-dimensional arrays of arrays. I think the right answer is the latter. But anyway...)

----

Talking about multi-argument operator overloads over breakfast made me think up
[the following code snippet](https://godbolt.org/z/c6QKTd).

    #include <type_traits>

    struct A {
        int value = 0;
        A(int v): value(v) {}

        template<class... Ts,
                 class = std::enable_if_t<(std::is_same_v<Ts, A> && ...)>>
        A operator+(Ts... ts) {
            return A((this->value + ... + ts.value));
        }
    };

    A a = 1;
    A b = a;
    A c = a + b;
    A d = a + b + c;
    A e = a + b + c + d;

Here I've given `A` a _variadic_ overloaded `operator+` that accepts _any number_ of `A` objects
and adds them together using a C++17 fold-expression. And Clang is totally happy with this code.

But if your coworker wrote this code, hoping for some expression-templatey kind of thing,
they'd be sadly mistaken! [This example](https://godbolt.org/z/4o08Iy) might make clearer what's
going on: we have here an `operator+` that can _accept_ a variadic number of parameters, but the
compiler will never _give_ it more than the usual two at a time. All we've done by making it
variadic is we've permitted the same operator to be called as both a binary operator `(a+b)`
and a unary operator `(+a)`. There's no such thing in C++ as a "ternary operator `+`"!

----

See also: [the prefix-or-postfix-I-don't-care increment operator.](https://godbolt.org/z/bymrOW)
