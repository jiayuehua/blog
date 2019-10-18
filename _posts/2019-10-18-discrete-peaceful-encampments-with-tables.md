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

Here's a tabular display of the largest integer $$f(c,n)$$ such that $$c$$ armies of $$f(c,n)$$
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
    n=12   144 21 10  7  4* 3  2  2  1  1  1  1
    n=13   169 24 12  8  6  4  2* 2  1  1  1  1
    n=14   196 28 14 10  6  4  4  2  2  1  1  1
    n=15   225 32  . 11  7  4* 4  2  2  1* 1  1
    n=16   256  .  . 13  9  5  4  3  2  2  1* 1

Column $$c=2$$ of this table is [OEIS A250000](https://oeis.org/A250000).
Column $$c=3$$ of this table is [OEIS A328283](https://oeis.org/A328283).
Diagonal $$c=n$$ represents the existence of solutions to the $$n$$-queens problem
(that is, it's 1 for all $$n$$ except 2 and 3).

Columns for $$c\ge 4$$ are merely my best lower bounds based on Dmitry Kamenetsky's program;
they should not be taken as gospel.

The numbers with asterisks are ones that feel surprisingly low to me.
The clever solution for $$f(5,10)=4$$ is just the 2x2 tessellation of the solution to $$f(5,5)=1$$;
and likewise the clever solution to $$f(7,14)=4$$.

----

Here's a tabular display of the largest integer $$g(c,n)$$ such that
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
    n=8     64 10  8  4  4  6  4  1  0
    n=9     81 12  9  5  6  2  7  4  1  0
    n=10   100 15  8  5  4  4 11  8  4  1  0
    n=11   121 19 11  7  4  6  2 12  7  3  1  0
    n=12   144  . 11  .  6  3  5  2  9  6  3  1
    n=13   169  .  .  .  .  .  8  3 14 10  7  4
    n=14   196  .  .  .  .  .  4  5  2 14  9  6
    n=15   225  .  .  .  .  .  .  9  4 20 14 10
    n=16   256  .  .  .  .  .  .  3  6  3 19 15

Row `c=2` of this table is [OEIS A308632](https://oeis.org/A308632).

Columns for $$c\ge 3$$ are merely my best guesses based on Dmitry Kamenetsky's program;
they should not be taken as gospel. They are neither upper bounds nor lower bounds!
For example, on a 12x12 board you can definitely encamp 6+4+4+4+4 queens, so my lower bound for
$$f(c,n)$$ is 4; but if it turns out that $$f(c,n)=5$$, then $$g(c,n)$$ will have to be revised
_downward_.

Also notice that for example on an 11x11 board you can encamp 8+8+11 queens
or 8+9+10 queens, but not (as far as I know) 8+9+11. So $$f(3,11) = 8$$ and $$g(3,11) = 11$$,
but it would be reasonable to imagine defining some related sequence $$h$$ such that $$h(3,11) = 10$$.
