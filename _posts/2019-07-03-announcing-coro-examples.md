---
layout: post
title: 'Announcing `Quuxplusone/coro`, single-header C++2a example code'
date: 2019-07-03 00:02:00 +0000
tags:
  concepts
  coroutines
  ranges
  web
---

My new GitHub repo [`Quuxplusone/coro`](https://github.com/Quuxplusone/coro) is a collection
of single-header library facilities for working with C++2a coroutines. Here's an example:
[https://coro.godbolt.org/z/4rzMI9](https://coro.godbolt.org/z/4rzMI9)

    #include <https://raw.githubusercontent.com/Quuxplusone/coro/master/include/coro/shared_generator.h>
    #include <stdio.h>
    #include <tuple>
    #include <range/v3/view/take.hpp>

    namespace rv = ranges::view;

    auto triples() -> shared_generator<std::tuple<int, int, int>> {
        for (int z = 1; true; ++z) {
            for (int x = 1; x < z; ++x) {
                for (int y = x; y < z; ++y) {
                    if (x*x + y*y == z*z) {
                        co_yield std::make_tuple(x, y, z);
                    }
                }
            }
        }
    }

    int main() {
        for (auto&& triple : triples() | rv::take(10)) {
            printf(
                "(%d,%d,%d)\n",
                std::get<0>(triple),
                std::get<1>(triple),
                std::get<2>(triple)
            );
        }
    }

Notice the first line of this code. It's including a URL! Yes, Godbolt Compiler Explorer supports
`#include`-by-URL. But the fetching and textual-pasting-in of the `#include`d file is done entirely
in client-side JavaScript. So if the header you're including happens to `#include` some other header
which is not installed on Godbolt's virtual machine, then you're out of luck.

So I decided the other day that what the world needed was a collection of single-header library
facilities — one facility per header file — implementing simple coroutine primitives such as `task`
and `generator`. So I made that.

Or, more accurately, I stole that! Most of the code in my repo so far is only slightly different from Lewis Baker's
[`llvm/coroutine_examples`](https://github.com/lewissbaker/llvm/tree/master/coroutine_examples) repo,
or from Eric Niebler's [`range/v3/experimental`](https://github.com/ericniebler/range-v3/tree/master/include/range/v3/experimental/utility).
They're the ones who know how to write this code from scratch, really. The only thing I'm adding is
the ability to `#include` these files from Compiler Explorer.

Lewis Baker is also the author of `cppcoro`, which is available in the Libraries dropdown on Compiler Explorer.
So you can get a lot of stuff simply by adding `cppcoro` to your Libraries and writing lines like

    #include <cppcoro/generator.hpp>

However, as of this writing, `cppcoro::generator<T>` is a move-only type (which makes perfect sense:
it manages a coroutine, which is a non-copyable resource). Move-only types don't satisfy `ViewableRange`,
which means they don't work with any of the range adaptors in the C++2a Ranges library.

Eric Niebler's `ranges::experimental::generator<T>` is copyable and thus viewable, but if you try
to compile it on Godbolt, [Clang runs out of memory and crashes](https://coro.godbolt.org/z/j9nUmw).

So I believe `Quuxplusone/coro` has at least some value-added. I hope some readers of this blog
find it useful as well!

In the coming weeks, I hope to add more facilities to it, such as an awaitable type that can interleave
execution within a thread pool, an awaitable `optional`, and a few manageable examples that use the
`co_await` syntax in a meaningful way.
If you know of good examples that show off the potential of `co_await`, please send them my way!


## The release of C++2a should be delayed past 2020

Speaking of move-only range types, Corentin Jabot has a proposal currently before LWG that introduces
"move-only iterator types":
[P1207R2 "Movability of Single-pass Iterators"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1207r2.pdf)
(June 2019). Move-only iterators are something that's never been tried before;
in classic C++, iterators are lightweight and copyable by definition. But Corentin correctly draws
an analogy between `istream_iterator<int>` and `auto_ptr<int>`: both are copyable only because C++98
didn't have move semantics. Neither one _should_ be copyable!

Casey Carter's [P1456R0 "Move-only views"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1456r0.pdf) (January 2019)
proposes a similar feature for range types.

In a world with coroutines, move-only iterators and ranges are going to become more important.
(Uncertainty re WG21's direction in this area is one reason `cppcoro` doesn't yet provide
any copyable/viewable generator type.) These both seem like very fruitful directions to explore.
Today on Slack Corentin said of P1207 and P1456, "Both need to be in [C++]20, or never" — these are
directions that if we do not explore them _before_ shipping C++2a, we will likely never be able to
act on them at all.

WG21 has no plans to add C++2a Coroutines support to the C++2a standard library. The current synopsis
of the `<coroutine>` header is [startlingly short](http://eel.is/c++draft/coroutine.syn) and I have it on
good authority that it will stay that way (unless Coroutines is ripped out of C++2a or unless
additions to C++2a are entertained). There are semi-active proposals for a `std::task<T>` type similar to what
you get from `cppcoro` or `Quuxplusone/coro`, but these proposals (Lewis Baker and Gor Nishanov's
[P1056](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1056r1.html),
Gor's [P1681](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1681r0.pdf)) are no longer targeting C++2a.

[Setting realistic deadlines is maybe an art, maybe a science](https://hackernoon.com/deadlines-that-are-doomed-from-the-beginning-21fd6960cd7e),
but regardless, the timetable must match the amount of work
to be done. C++2a has a _very_ big amount of work to be done, and work items of the form
"explore, implement, and get user feedback on _____" aren't necessarily parallelizable.

When is the best time to catch bugs: while the product is in development, or after it has been released to customers?
