---
layout: post
title: "Discrete Peaceful Encampments, with tables"
date: 2019-10-18 00:02:00 +0000
tags:
  math
  puzzles
---

Previously on this blog: ["Discrete Peaceful Encampments"](/blog/2019/01/24/discrete-peaceful-encampments/)
(2019-01-24).

Dmitry Kamenetsky has written [a Java program](https://oeis.org/A250000/a250000_1.java.txt)
that heuristically finds solutions (not necessarily best-possible solutions) to the "peaceable
coexisting queens" problem, for any size board $$n$$ and any number of colors $$c$$.

Here's a table of the largest integers $$f(c,n)$$ such that $$c$$ armies of $$f(c,n)$$
queens each can all be encamped peaceably on a $$c\times c$$ board.

    k=       1  2  3  4  5  6  ...
          .
    n=1   .  1  0
    n=2   .  4  0  0
    n=3      9  1  0  0
    n=4     16  2  1  1  0
    n=5     25  4  1  1  1  0
    n=6     36  5  2  2  1  1  0
    n=7     49  7  3  2  1  1  1  0
    n=8     64  9  4  3  2  1  1  1  0
    n=9     81 12  5  4  2  2  1  1  1  0
    n=10   100 14  7  5  4  2  1  1  1  1  0
    n=11   121 17  8  6  4  2  2  1  1  1  1  0
    n=12   144 21 10  7  4  3  2  2  1  1  1  1  0
    n=13   169 24 12  8  6  4  2  2  1  1  1  1  1  0
    n=14   196 28 14 10  6  4  4  2  2  1  1  1  1  1  0
    n=15   225 32 16 11  9  4  4  2  2  2  1  1  1  1  1  0
    n=16   256 37 18 13  9  5  4  3  2  2  1  1  1  1  1  1  0
    n=17   289 42 20 14  9  6  4  3  2  2  2  1  1  1  1  1  1  0
    n=18   324 47 23 16 12  6  4  3  3  2  2  2  1  1  1  1  1  1  0
    n=19   361 52 25 18 12  7  5  4  3  2  2  2  1  1  1  1  1  1  1  0
    n=20   400 58 28 20 16  8  6  4  3  3  2  2  2  1  1  1  1  1  1  1  0

Column $$c=2$$ of this table is [OEIS A250000](https://oeis.org/A250000).
Column $$c=3$$ of this table is [OEIS A328283](https://oeis.org/A328283).
Diagonal $$c=n$$ represents the existence of solutions to the $$n$$-queens problem
(that is, it's 1 for all $$n$$ except 2 and 3).

These numbers are merely my best lower bounds based on Dmitry Kamenetsky's program;
any numbers not already listed in OEIS should not be taken as gospel.

The clever solution for $$f(5,10)=4$$ is just the 2x2 tessellation of the solution to $$f(5,5)=1$$;
and likewise the clever solution to $$f(7,14)=4$$.

----

Here's a table of the largest integers $$g(c,n)$$ such that
$$c-1$$ armies of $$f(c,n)$$ queens each, plus one army of $$g(c,n)$$ queens,
can all be encamped peaceably on a $$c\times c$$ board.
By definition, $$g(c,n)\geq f(c,n)$$.

    k=       1  2  3  4  5  6  ...
          .
    n=1   .  1  0
    n=2   .  4  0  0
    n=3      9  2  0  0
    n=4     16  3  3  1  0
    n=5     25  4  7  3  1  0
    n=6     36  6  6  2  4  1  0
    n=7     49  7  6  4  7  4  1  0
    n=8     64 10  8  4  4  7  4  1  0
    n=9     81 12  9  5  9  2  7  4  1  0
    n=10   100 15  8  5  4  5 12  7  4  1  0
    n=11   121 19 11  7  6  8  3 12  9  4  1  0
    n=12   144 21 11  7 10  4  7  2 12  8  4  1  0
    n=13   169 25 12  9  9  4 10  4 18 13  8  4  1  0
    n=14   196 29 14 10 10  9  4  8  3 18 12  7  4  1  0
    n=15   225 34 17 12  9 12  6 13  7  2 18 12  8  4  1  0
    n=16   256 37 19 13 16 10  5  4 11  5 24 17 12  7  4  1  0
    n=17   289 42 24 15 17  6  7  5 11  8  3 21 16 11  7  4  1  0
    n=18   324 48 24 16 16 11  9  7  4 10  6  2 20 15 11  8  4  1  0
    n=19   361 53 28 19 16 14  6  6  6 13  8  4 27 21 16 11  7  4  1  0
    n=20   400 59 31 20 16 12 11  8  9  3 11  7  3 29 20 17 10  7  4  1  0

Row `c=2` of this table is [OEIS A308632](https://oeis.org/A308632).

Again, these numbers are merely my best guesses based on Dmitry Kamenetsky's program;
they should not be taken as gospel. They are neither upper bounds nor lower bounds!
For example, on a 12x12 board you can definitely encamp 10+4+4+4+4 queens, so 4 is a hard
lower bound for $$f(5,12)$$; but $$g(5,12)$$ might be either greater than 10 or
(if it turns out that $$f(5,12)>4$$) less than 10.

Also notice that for example on an 11x11 board you can encamp 8+8+11 queens
or 8+9+10 queens, but not (as far as I know) 8+9+11. So $$f(3,11) = 8$$ and $$g(3,11) = 11$$,
but it would be reasonable to imagine defining some related sequence $$h$$ such that $$h(3,11) = 10$$.

----

I have ported Dmitry's Java program to C++14 and made it compute the entire triangle
(that is, compute all the entries in parallel and periodically write its best results to a file on disk).
A complete listing of its "best" solutions is [here](/blog/code/2019-10-18-discrete-encampments-best-results.txt),
and the C++14 source code itself is [here](/blog/code/2019-10-18-discrete-encampments.cc).
