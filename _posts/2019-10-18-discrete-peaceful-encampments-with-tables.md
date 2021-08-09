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
that heuristically finds best-known solutions (not necessarily best-possible solutions) to the
"peaceable coexisting queens" problem, for any size board $$n$$ and any number of colors $$c$$.

I have [ported Dmitry's Java program to C++14](/blog/code/2019-10-18-discrete-encampments.cpp)
and made it concurrently run on many sizes of problem and periodically write its best results to
a file on disk.

Here's a table of the largest integers $$f(n,c)$$ such that $$c$$ armies of $$f(n,c)$$
queens each can all be encamped peaceably on an $$n\times n$$ board.

    c=       1  2  3  4  5  6  ...
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
    n=18   324 47 23 16 12  6  5  4  3  2  2  2  1  1  1  1  1  1  0
    n=19   361 52 25 18 12  7  6  4  3  2  2  2  1  1  1  1  1  1  1  0
    n=20   400 58 28 20 16  9  6  4  3  3  2  2  2  1  1  1  1  1  1  1  0

Column $$c=2$$ of this table is [OEIS A250000](https://oeis.org/A250000).
Column $$c=3$$ of this table is [OEIS A328283](https://oeis.org/A328283).
Diagonal $$c=n$$ represents the existence of solutions to the $$n$$-queens problem
(that is, it's 1 for all $$n$$ except 2 and 3).

These numbers are merely my best lower bounds based on Dmitry Kamenetsky's program;
any numbers not already listed in OEIS should not be taken as gospel.

The clever solution for $$f(10,5)=4$$ is just the 2x2 tessellation of the solution to $$f(5,5)=1$$;
and likewise the clever solution to $$f(14,7)=4$$.

----

Here's a table of the largest integers $$g(n,c)$$ such that
$$c-1$$ armies of $$f(n,c)$$ queens each, plus one army of $$g(n,c)$$ queens,
can all be encamped peaceably on an $$n\times n$$ board.
By definition, $$g(n,c)\geq f(n,c)$$.

    c=       1  2  3  4  5  6  ...
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
    n=14   196 29 14 10 10  9  4  8  3 20 12  8  4  1  0
    n=15   225 34 17 12  9 12  6 13  7  2 18 12  8  4  1  0
    n=16   256 37 19 13 16 10  9  4 11  5 24 17 12  8  4  1  0
    n=17   289 42 24 15 17  7 12  5 13  8  3 22 16 11  8  4  1  0
    n=18   324 48 24 16 16 11  6  4  4 10  7  2 23 18 12  8  4  1  0
    n=19   361 53 28 19 17 14  9  6  6 13 11  4 27 22 16 11  8  4  1  0
    n=20   400 59 31 20 16 13 12  9  9  3 11  7  3 30 20 17 11  7  4  1  0

Column `c=2` of this table is [OEIS A308632](https://oeis.org/A308632).

Again, these numbers are merely my best guesses based on Dmitry Kamenetsky's program;
they should not be taken as gospel. They are neither upper bounds nor lower bounds!
For example, on a 12x12 board you can definitely encamp 4+4+4+4+10 queens, so 4 is a hard
lower bound for $$f(12,5)$$; but $$g(12,5)$$ might be either greater than 10 or
(if it turns out that $$f(12,5)>4$$) less than 10.

Also notice that for example on an 11x11 board you can encamp 8+8+11 queens
or 8+9+10 queens, but not (as far as I know) 8+9+11. So $$f(11,3) = 8$$ and $$g(11,3) = 11$$,
but it would be reasonable to imagine defining some related sequence $$h$$ such that $$h(11,3) = 10$$.
Define a set of army sizes to be more "equitable" than another if it's lexicographically greater
when sorted, so for example (8,9,9) is more equitable than either (8,8,8) or (8,8,100). Then we
can collect solutions to the discrete-peaceful-encampments problem that maximize equitability, instead
of maximizing the size of the "richest" army as the expense of the "middle class."

Here's a table of the largest integers $$h(n,c)$$ such that the most equitable
way peaceably to encamp $$c$$ armies of at least $$f(n,c)$$ queens each on an $$n\times n$$ board
results in a smallest army of size $$f(n,c)$$ and a largest army of size $$h(n,c)$$.
By definition, $$f(n,c)\leq h(n,c)\leq g(n,c)$$.

    c=       1  2  3  4  5  6  ...
          .
    n=1   .  1  0
    n=2   .  4  0  0
    n=3      9  2  0  0
    n=4     16  3  3  1  0
    n=5     25  4  4  3  1  0
    n=6     36  6  3  2  2  1  0
    n=7     49  7  4  3  2  2  1  0
    n=8     64 10  6  4  4  2  4  1  0
    n=9     81 12  7  5  4  2  2  2  1  0
    n=10   100 15  8  5  4  4  2  2  2  1  0
    n=11   121 19 10  7  6  4  3  2  2  2  1  0
    n=12   144 21 11  7  6  4  4  2  2  2  4  1  0
    n=13   169 25 12  9  9  4  4  4  2  2  2  4  1  0
    n=14   196 29 14 10  9  5  4  4  3  2  2  2  2  1  0
    n=15   225 34 17 12  9  5  6  4  4  2  2  2  2  2  1  0
    n=16   256 37 19 13 12  6  6  4  3  3  2  3  2  2  2  1  0
    n=17   289 42 22 15 12  7  9  4  4  4  3  2  2  2  2  2  1  0
    n=18   324 48 24 16 16  8  6  5  4  4  3  2  3  4  4  2  2  1  0
    n=19   361 53 26 19 16  8  9  6  5  4  3  4  2  3  4  2  2  2  1  0
    n=20   400 59 30 20 16 13  9  6  4  3  3  3  3  3  2  3  2  2  4  1  0

Column `c=2` of this table is still [OEIS A308632](https://oeis.org/A308632), since in the two-army case
the second-smallest army _is_ the biggest army.

Empirical note: At the start of March 2020, my previous best solution for $$f(21,8)$$ had been 4 queens per army,
with one "rich" army of size $$g(21,8)=9$$. As soon as I started trying to maximize equitability, the computer
quickly found a better solution where all eight armies had size $$f(21,8)=h(21,8)=5$$. That's because going from
$$(4,4,4,4,4,4,4,9)$$ to $$(5,5,5,5,5,5,5,5)$$ in one random step is quite improbable; but if
our metric is equitability, then we can step from $$(4,4,4,4,4,4,4,9)$$ to $$(4,4,4,4,4,4,5,8)$$,
and then to $$(4,4,4,4,4,5,5,6)$$, and so on, because each step now counts as a measurable improvement
over the previous one.

----

I have ported Dmitry's Java program to C++14 and made it compute the entire triangle
(that is, compute all the entries in parallel and periodically write its best results to a file on disk).

* The C++14 source code itself is [here](/blog/code/2019-10-18-discrete-encampments.cpp), and also
    [on GitHub](https://github.com/Quuxplusone/MetaSudoku/blob/master/discrete-encampments-kamenetsky-heuristic.cc).

* A complete listing of its best $$g(n,c)$$ solutions (maximizing the smallest army, and then the biggest) is [here](/blog/code/2019-10-18-discrete-encampments-best-results.txt),
    and also [on GitHub](https://github.com/Quuxplusone/MetaSudoku/blob/master/dek-out.txt).

* A complete listing of its best $$h(n,c)$$ solutions (maximizing the smallest army, and then the next smallest, and so on) is [here](/blog/code/2020-03-21-discrete-encampments-bernie-results.txt),
    and also [on GitHub](https://github.com/Quuxplusone/MetaSudoku/blob/equitable/dek-out.txt).
