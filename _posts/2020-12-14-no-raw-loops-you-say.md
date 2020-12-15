---
layout: post
title: "The STL is more than `std::accumulate`"
date: 2020-12-14 00:01:00 +0000
tags:
  cppcon
  memes
  ranges
  rant
  stl-classic
excerpt: |
  Conor Hoekstra gives great talks on algorithms. Notably, ["Algorithm Intuition"](https://www.youtube.com/watch?v=48gV1SNm3WA)
  (C++Now 2019) and ["Better Algorithm Intuition"](https://www.youtube.com/watch?v=2MXyNS33t2k) (code::dive 2019).
  However, every time I watch one of his talks where he uses STL algorithms
  to solve some programming problem, I come away feeling like

  ![Let's see who's really under that mask! Why, it's old Mr. Accumulate and his brother For_each!](/blog/images/2020-12-14-why-its-old-mister-accumulate.png){: .meme}

  Let's look at two examples of "using STL algorithms to solve problems" in this limited sense,
  and how else we might solve them. Both of these examples are taken from Conor's CppCon 2020 talk
  ["Structure and Interpretation of Computer Programs: SICP."](https://www.youtube.com/watch?v=7oV7hiAsVTI)
---

Conor Hoekstra gives great talks on algorithms. Notably, ["Algorithm Intuition"](https://www.youtube.com/watch?v=48gV1SNm3WA)
(C++Now 2019) and ["Better Algorithm Intuition"](https://www.youtube.com/watch?v=2MXyNS33t2k) (code::dive 2019).
However, every time I watch one of his talks where he uses STL algorithms
to solve some programming problem, I come away feeling like

![Let's see who's really under that mask! Why, it's old Mr. Accumulate and his brother For_each!](/blog/images/2020-12-14-why-its-old-mister-accumulate.png){: .meme}

Or maybe

![You: for / Me, an intellectual: std::ranges::for_each](/blog/images/2020-12-14-me-an-intellectual.png){: .meme}

Let's look at two examples of "using STL algorithms" in this limited sense,
and how we might solve the same problems differently (maybe using the STL, maybe not).
Both of these examples are taken from Conor's CppCon 2020 talk
["Structure and Interpretation of Computer Programs: SICP."](https://www.youtube.com/watch?v=7oV7hiAsVTI)


## Sum of two largest squares

[Circa 36m55s](https://www.youtube.com/watch?v=7oV7hiAsVTI&t=36m55s),
Conor finishes polishing a function to compute the sum of the squares
of the two largest integers in a list.

    // IMP1
    auto solution(auto const& v) {
        auto const [a, b] = std::accumulate(
            std::cbegin(v),
            std::cend(v),
            std::pair{0, 0},
            [](auto acc, auto e) {
                auto [a, b] = acc;
                if      (e > a) { b = a; a = e; }
                else if (e > b) { b = e; }
                return std::pair{a, b};
            });
        return a * a + b * b;
    }

> "This code makes me really happy.  Some would argue that it's not as
> nice as a `for` loop, and I would have to vehemently disagree."

To me, that's not "STL" code; that's just open-coding with extra steps.
You know how in a pre-Coroutines world we have to maintain the "state" of our state machine by hand?
That's basically what we're doing here, via `acc` — it's just a manual way
of managing the local variables `a` and `b` in a way that can be threaded
through `accumulate`. We could use `for` to write that _exact same_ algorithm
much more simply:

    // IMP2
    auto core_solution(const auto& v) {
        int a = 0;
        int b = 0;
        for (auto&& e : v) {
            if (e > a) { b = a; a = e; }
            else if (e > b) { b = e; }
        }
        return a * a + b * b;
    }

Conor's `IMP1` using `std::accumulate` with an ad-hoc accumulator function
is analogous to a C++17 state machine that manually manages stack frames;
`IMP2`, using core-language `for`, is analogous to a C++20 function
using core-language `co_await`. It lets us unkink our control flow and stop
manually managing our stack frames.

----

Meanwhile, here's what I would call the actually "STL classic" solution to Conor's puzzle:

    // IMP3
    auto stl_solution(const auto& v) {
        int top[2] = {};
        std::ranges::partial_sort_copy(v, top, std::greater());
        return std::inner_product(top, std::end(top), top, 0);
    }

This directly translates the problem statement: "Copy `v`'s top 2 elements
to `top`; then compute the sum of the squares of `top`'s elements."

Unfortunately, both libstdc++ and libc++ produce absolutely awful code for this
formulation: their `partial_sort_copy` implementations are very inliner-hostile.
In fact, libstdc++'s `std::ranges::partial_sort_copy` is somehow worse than their
`std::partial_sort_copy` — as of this writing, you can actually generate strictly better code
by replacing

    std::ranges::partial_sort_copy(v, top, std::greater());

with

    std::partial_sort_copy(std::begin(v), std::end(v),
        top, std::end(top), std::greater());

You're probably wondering why I bothered to write `std::end(top)`
when I could have written `top+2`, right? Well, I wanted to highlight
that it is trivial to extend `stl_solution` to compute the sum of squares of
the top 3 values, or the top 10 values. All you have to do is change the bound
of the array variable `top` in one place.
Everything else in this (three-line) function just keeps working.
Consider how much you'd need to change either of the open-coded solutions to sum
the squares of, say, the top 10 values.

_This_, to me, is the benefit of using STL algorithms and containers. It's not that
they will give you the absolute best codegen. It's that they'll give you an appropriate
level of maintainability, while generating code that costs _no more_ than whatever
you might have written by hand (with that same level of maintainability).

STL algorithms, used correctly, are _self-documenting._ Compare `IMP2` to `IMP3`;
which one is easier to understand? (I claim the answer is "`IMP3`," because it
is a line-by-line translation of the problem statement.) Now compare
`IMP2` to `IMP1`; which one is easier to understand? I claim that _at best_ they
are equally comprehensible. In practice, `IMP1` is harder to understand, because it
has all the stateful-variable ickiness of `IMP2` _plus_ the reader must understand
what's happening with that `std::accumulate`.


## Leibniz pi approximation

[Circa 49m38s](https://www.youtube.com/watch?v=7oV7hiAsVTI&t=49m38s), Conor
presents this implementation of a pi-calculating algorithm:

    // IMP1
    auto leibniz_pi_approximation(int n) {
        return (rv::iota(0, n)
              | rv::transform([](auto e){ return 1 + 2 * e;})
              | rv::chunk(2)
              | rv::transform([](auto rng){ return 1.0 / (rng[0] * rng[1]); })
              | hs::accumulate(0.0, std::plus{})) * 8;
    }

[At 52m42s](https://www.youtube.com/watch?v=7oV7hiAsVTI&t=52m42s),
there's an "iteratively refined" version of the same algorithm.
Conor doesn't explicitly say so, but the big advantage of this version
over the previous is that this one produces decent codegen.

    // IMP2
    auto leibniz_pi_approximation2_alt(int n) {
        return (rv::iota(0, n)
              | rv::transform([s = -1.0](auto e) mutable {
                  s *= -1;
                  return s / (1 + 2 * e); })
              | hs::accumulate(0.0, std::plus{})) * 4;
    }

Notice that this code has the same overall outline as the "sum of squares"
program: we're iterating over our input sequence and mutating a manually
managed "stack frame" of variables (in this case just `s`) which conceptually
live outside the loop.

In "core-language" C++, the same algorithm might look something like this:

    // IMP3
    double leibniz_pi_approximation(int n) {
        double sum = 0.0;
        double s = -1.0;
        for (int e = 0; e < n; ++e) {
            s = -s;
            sum += s / (1 + 2 * e);
        }
        return sum * 4;
    }

And we could re-obfuscate the code using `std::accumulate` like this:

    // IMP4
    double leibniz_pi_approximation(int n) {
        auto [sum, s] = rv::iota(0, n)
                      | hs::accumulate(
            std::pair(0.0, 1.0),
            [](auto acc, auto e) {
                return std::pair(acc.first + acc.second/(1+2*e), -acc.second);
            });
        return sum * 4;
    }

All of these versions have basically the same codegen at `-O3` (but of course
the non-Ranges version `IMP3` is vastly faster to compile).

By the way, you don't need mutability just to get a repeating sequence
of `+1, -1` in range-v3. You could use `rv::cycle`, like this:

    // IMP5
    double leibniz_pi_approximation(int n) {
        int signs[] = {+1, -1};
        return (rv::zip_with(
                  [](auto e, double s) { return s / (1+2*e); },
                  rv::iota(0), rv::cycle(signs))
              | rv::take_exactly(n)
              | hs::accumulate(0.0)) * 4;
    }

However, if you do this, then again the codegen suffers, compared to the baseline
`for`-loop (`IMP3`) or the two "manual stack frame management" solutions (`IMP2`,
`IMP4`).

Is there a "classic STL" version of this algorithm? I don't think so, because it's really
just a numeric operation: it takes in _one_ number (not a collection of elements), and it
produces _one_ number (not a collection). I think `IMP3` is the best solution, not just
because its codegen is better, but because it most clearly explains what it's doing.
The closest I can get to a "classic STL" solution would be, like,

    // IMP6
    double leibniz_pi_approximation(int n) {
        std::vector<double> terms(n);
        std::iota(terms.begin(), terms.end(), 0);
        for (auto&& e : terms) { e = 1.0 / (1+2*e); }
        for (auto&& e : terms | rv::stride(2)) { e = -e; }
        return -4 * std::accumulate(terms.begin(), terms.end(), 0.0);
    }

But this is obviously not an acceptable solution, because it heap-allocates for a problem
(computing pi) that fundamentally doesn't require heap-allocation.

Here are the line counts I saw for each version of `leibniz_pi_approximation`
[on Godbolt](https://godbolt.org/z/sode8d), using GCC 10.2 at `-O2`:

| Version | C++17 | C++20 |
|:-------:|:-----:|:-----:|
|  `IMP1` |  227  |  219  |
|  `IMP2` |   49  |   49  |
|  `IMP3` |   35  |   35  |
|  `IMP4` |   49  |   49  |
|  `IMP5` |   83  |   83  |
|  `IMP6` |  180  |  172  |

----

By the way, I think the problem with `IMP1` is that the parameter to `rv::chunk(2)`
is a runtime function parameter, whereas we really want it to be a compile-time
(template) parameter. If there were an `rv::chunk<2>()`, we could eliminate a
whole heap-allocation there. Something like this:

    template<int N>
    struct chunk {
        friend auto operator|(auto&& lhs, const chunk& rhs) {
            return rv::zip_with(
                [](auto x, auto y) { return std::array{x, y}; },
                lhs | rv::stride(2),
                lhs | rv::drop(1) | rv::stride(2)
            );
        }
    };

This modification chops `IMP1`'s codegen from 219 lines to 176 —
in the vicinity of `IMP6` — but it's still an order of magnitude worse
than the "good" versions.
