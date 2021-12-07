---
layout: post
title: "`operator<=>` doesn't obsolete the hidden friend idiom"
date: 2021-10-22 00:01:00 +0000
tags:
  argument-dependent-lookup
  hidden-friend-idiom
  c++-style
  operator-spaceship
  stl-classic
---

C++20 introduces new expression-rewriting rules for comparison operators, such that
the compiler can rewrite
`a < b` as either `(a <=> b) < 0` or `0 < (b <=> a)`, and
`a != b` as either `!(a == b)` or `!(b == a)`. Today, someone
asked whether these new rules decrease the attractiveness of the hidden friend idiom
for overloaded operators — at least for `operator<=>`, should we just declare it
as a member function? The answer is <b>no.</b> Observe ([Godbolt](https://godbolt.org/z/e3eE39xhW)):

    struct Good {
        friend auto operator<=>(const Good&, const Good&) = default;
    };

    struct Bad {
        auto operator<=>(const Bad&) const = default;
    };

    static_assert(std::totally_ordered<Good>);
    static_assert(std::totally_ordered<Bad>);

    static_assert(std::totally_ordered<std::reference_wrapper<Good>>);
    static_assert(not std::totally_ordered<std::reference_wrapper<Bad>>); // !!

Consider an expression such as

    Good g;
    auto rg = std::ref(g);
    auto og = rg < rg;  // OK

The compiler will rewrite `rg < rg` into `(rg <=> rg) < 0`, and then
use ADL to look up a declaration for `<=>` in the places associated with
`std::reference_wrapper<Good>`. (See ["What is ADL?"](/blog/2019/04/26/what-is-adl/) (2019-04-26).)
ADL will find the hidden friend of `Good`. Both operands have a user-defined
implicit conversion to `const Good&`, so the hidden friend is viable; and so
everything's fine.

But in the `Bad` case...

    Bad b;
    auto rb = std::ref(b);
    auto ob = rb < rb;  // ill-formed

The compiler does the same rewrite into `(rb <=> rb) < 0`, but this time,
ADL doesn't find any friends of `Bad`. Lookup also finds no member
functions named `operator<=>`, because `std::reference_wrapper<Bad>` has no
such member functions. (The member functions of `Bad` itself are not consulted,
because neither operand is actually of type `Bad`.)

The hidden friend idiom has some engineering benefits as well, such as
reducing the size of the overload set the compiler sees (that's the
"hidden" part) and making it harder to forget to apply `const` symmetrically
to both operands. So it's just all around a good idea. The point of this
post is simply to show that it also has a real effect on your types'
observable API. C++20's new expression-rewriting rules don't change that fact.

----

See also:

* ["An example of the Barton–Nackman trick"](/blog/2020/12/09/barton-nackman-in-practice/) (2020-12-09)
