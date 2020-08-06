---
layout: post
title: "PSA: `<random>`'s distributions are stateful"
date: 2019-10-22 00:01:00 +0000
tags:
  pitfalls
  random-numbers
  rant
  standard-library-trivia
---

Twice in the past week I've run into this issue (once on [Code Review StackExchange](https://codereview.stackexchange.com/)
and once on [Slack](https://cppalliance.org/slack/)), so I thought I'd make a blog post about it.

> Distributions are stateful!

Consider the following code:

    std::mt19937 g;
    std::cout << std::uniform_int_distribution<int>(0, 10)(g) << '\n';

This code works fine. But, a student may think, let's pull out the distribution into a named
local variable:

    std::mt19937 g;
    const auto dist = std::uniform_int_distribution<int>(0, 10);
    std::cout << dist(g) << '\n';

This code [does not compile](https://godbolt.org/z/tJmS1K) (except on MSVC, which is how I think
students get lulled into thinking it's correct). The problem is that `dist`'s `operator()` is
not const-qualified, so you can't call it on a const-qualified object.
(See ["`const` is a contract"](/blog/2019/01/03/const-is-a-contract/) (2019-01-03).)

The `operator()` is not const-qualified because — just like the `operator()` of `std::mt19937`
itself — a call to that operator may modify the internal state of the distribution.
That's right, distributions are just as stateful as random number engines!

Why might a distribution want to keep internal state? Well, the most popular algorithm for producing
numbers in a bell-shaped [normal distribution](https://en.wikipedia.org/wiki/Normal_distribution)
is the [Box–Muller transform](https://en.wikipedia.org/wiki/Box–Muller_transform), which produces
two independently distributed outputs in a single computation.
libc++ ([here](https://github.com/llvm-mirror/libcxx/blob/f6e8515214de2b9c47c0d262971c978f3667b04d/include/random#L4312-L4341))
uses the [Marsaglia polar method](https://en.wikipedia.org/wiki/Marsaglia_polar_method), which
similarly produces two results at a time. Having computed those two results, it would be wasteful
to throw the second one away; so `std::normal_distribution` will cache the second result inside
itself until the user asks for another output.

For `uniform_int_distribution`, which produces one value at a time, there's no need to keep
any internal state (other than the parameters `min` and `max`); but all vendors except MSVC
continue to mark `uniform_int_distribution::operator()()` as non-const, for two reasons:

- The Standard mandates that they do it. Vendors are not free to arbitrarily add `const`
    to random member functions. [EDIT: Oops! [[member.functions]/2](http://eel.is/c++draft/conforming#member.functions-2)
    seems to say that vendors _are_ free to do that! Really? Thanks to Tim Song for bringing
    this to my attention.)

- It protects against [Hyrum's Law](https://www.hyrumslaw.com). If your code compiles with
    `uniform_real_distribution` but [fails to compile](https://godbolt.org/z/FAWIP-) when you change
    it to `normal_distribution`, that's brittle code. It's beneficial for all distributions to have
    the same API and conform to the same concepts.

----

So, engines and distributions are both stateful.
You can observe the effects of these two different levels of statefulness with a test program
like this ([Godbolt](https://godbolt.org/z/H-1mlu)):

    template<class G, class D, class F>
    void f(G& g, D& d, F reset_some_stuff) {
        g.seed(1);
        d.reset();
        printf("Expected output:                            ");
        printf("%0.2f ", d(g));
        printf("%0.2f ", d(g));
        printf("%0.2f ", d(g));
        printf("%0.2f ", d(g));
        printf("%0.2f ...\n", d(g));
        g.seed(1);
        d.reset();
        reset_some_stuff();
        printf("%0.2f ", d(g));
        printf("%0.2f ", d(g));
        printf("%0.2f ...\n", d(g));
    }

    int main() {
        std::mt19937 g;
        std::normal_distribution<float> dist(0.5f, 1.0f / 6);

        f(g, dist, [&]() {
            dist(g);
            printf("Grab one, then reset both:                  ");
            g.seed(1); dist.reset();
        });
        f(g, dist, [&]() {
            dist(g);
            printf("Grab one, then reset just the engine:  ");
            g.seed(1);
        });
        f(g, dist, [&]() {
            dist(g);
            printf("Grab one, then reset just the distribution:           ");
            dist.reset();
        });
        f(g, dist, [&]() {
            dist(g);
            printf("Grab one, then reset neither:                    ");
        });
    }

Interestingly enough, you can also observe that the `normal_distribution`s of libstdc++ and libc++
produce their outputs using the same algorithm... but produce them _in reversed order!_ This
illustrates another common `<random>` pitfall (and a personal pet peeve of mine): the standard
library's distributions are not portable from one implementation to another. If you are generating
random numbers for use in simulations or games where you need _reproducible_ results, you can
get away with using the standard random number engines
([if you don't care about performance](https://arxiv.org/abs/1910.06437)), but you should
never use the standard _distributions_ to produce your random results, if you care about
being able to reproduce those results later on someone else's machine.
