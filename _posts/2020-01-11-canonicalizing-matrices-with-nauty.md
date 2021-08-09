---
layout: post
title: 'Canonicalizing {0,1}-matrices with Nauty'
date: 2020-01-11 00:01:00 +0000
tags:
  math
---

Previously on this blog:

- ["Wolves and sheep"](/blog/2019/04/17/wolves-and-sheep/) (2019-04-17)

- ["Wolves and sheep, with tables"](/blog/2020/01/10/wolves-and-sheep-with-tables/) (2020-01-10)

In the process of deciding that I ought to build a file of small $$d$$-separable matrices,
I went down a rabbit-hole that in hindsight wasn't all _that_ related. I decided that it might
be useful to have some way of telling whether a given testing strategy for $$t(n,d)$$ (that is,
a given $$t\times n$$ matrix) was "novel" or merely equivalent to a matrix that I'd already found.

Asking "Is testing strategy $$T_1$$ equivalent to testing strategy $$T_2$$?" is equivalent to asking
["Can the $$t\times n$$ matrix $$T_2$$ be derived from $$T_1$$ merely by permuting $$T_1$$'s rows and then its columns?"](https://math.stackexchange.com/questions/518436/normalizing-a-matrix-with-row-and-column-swapping)
And _that_ question is equivalent to asking "Is graph $$G_1$$ isomorphic to graph $$G_2$$?",
where $$G_1$$ is the bipartite graph on $$t+n$$ vertices whose adjacency matrix is $$T_1$$,
and $$G_2$$ is the bipartite graph on $$t+n$$ vertices whose adjacency matrix is $$T_2$$.

Graph isomorphism is a hard
([but not NP-hard](https://cstheory.stackexchange.com/questions/8034/what-is-the-current-known-hardness-of-graph-isomorphism))
problem. Fortunately, as a popular practical problem, there exist software tools for it!

So I downloaded [Nauty/Traces](https://users.cecs.anu.edu.au/~bdm/nauty/), an open-source
software library by Brendan McKay and Adolfo Piperno, and I wrote some code to canonicalize
a Wolves and Sheep testing strategy (or any other {0,1}-matrix). Put in two matrices that are
equivalent under row-and-column-permutation, get out two identical matrices. Neat!
Find my code [on GitHub here](https://github.com/Quuxplusone/wolves-and-sheep/blob/master/canonicalize_matrix.cpp).

I also submitted the `nauty` package to [Homebrew](https://github.com/Homebrew/homebrew-core)
and got it accepted, so if you're on Mac OSX you can just type `brew install nauty`
and you'll be ready to compile my matrix-canonicalization code.

----

For more info on Nauty, see:

- ["Canonical Labelings with Nauty"](https://computationalcombinatorics.wordpress.com/2012/09/20/canonical-labelings-with-nauty/) (Derrick Stolee, September 2012)

- [The user manual (PDF)](http://pallini.di.uniroma1.it/nug26.pdf)

----

Nauty's C API is reasonably clean, but it took me a lot of experimentation to figure out its quirks.

The main entry point for our purposes is `densenauty(g, lab, ptn, orbits, options, stats, m, n, canong)`.
(There's also a `sparsenauty` API, but I didn't try to use it.)

`m` is simply `SETWORDSNEEDED(n)` (i.e., `n` divided by the word size and rounded up). I don't really
understand why this needs to be its own parameter, but it is.

`g` is the graph you're trying to canonicalize, represented as an adjacency matrix. Except that
Nauty doesn't do matrices; it does _sets_. So `g` is really an array of `n` sets, where each set
consists of `m` SETWORDs. To see whether vertex `vi` is adjacent to vertex `vj`, you would test
`ISELEMENT(g[vi], vj)`... except that Nauty doesn't do multidimensional arrays either! So actually
`g` is an array of `n*m` SETWORDs, and you have to index into it manually. What you actually write
to test `vi`'s adjacency to `vj` is `ISELEMENT(g[vi*m], vj)`.

`lab` and `ptn` define the graph's vertex coloring, but in a weird way (which, to be fair, is
_mostly_ documented). Each of `lab` and `ptn` is an array of `int`. For example:

    lab: 2 5 4 0 3 1 6
    ptn: 1 1 0 1 0 1 0

This example makes vertices {2,5,4} red, vertices {0,3} blue, and vertices {1,6} green.
`lab` should be a permutation of the integers $$[0,n)$$, and `ptn` should consist of
blocks of zero-or-more consecutive 1s followed by single 0s. Each `ptn[i]==0` indicates that
a color-partition ends with element `i`. (If you set `ptn[n-1]!=0`, then I assume bad things happen.)

Nauty's default behavior is to make all the vertices the same color.
_It will do this even if you pass in a different coloring!_ That's right, Nauty will
by default _ignore_ what you pass in for `lab` and `ptn`! To make Nauty respect your settings
of `lab` and `ptn`, you must set `options.defaultptn=false`.

So if you want all your vertices to be the same color, can you just pass `nullptr` for `lab` and `ptn`?
_No, of course not._ Nauty wants to use those two arrays for scratch space. You must pass in
non-null buffers, and they must be writeable (non-const), even if you don't care about vertex
coloring. Also, be aware that Nauty will trash `lab` and `ptn`
as part of its operation. (Specifically, it may change any non-zero element of `ptn`. It will never
change any element of `ptn` whose value you have set to 0.)

`orbits` we don't care about, but again it cannot be `nullptr` — it's used as scratch space.

`options` we can mostly leave alone, except that we must set `options.defaultptn=false`.
The documentation shows another promising-looking
option — `options.getcanon=true` — but it turns out that this is a red herring. Even though we are
trying to canonicalize our graph, we _do not want_ to set `options.getcanon`, nor do we want to pass
in anything for the `canong` parameter. (That's right, `canong` is the one parameter which _is_
allowed to be `nullptr`!)

When `densenauty` returns, it will have permuted our input labeling, `lab`, into a canonical ordering.
Each color-partition will be permuted only within itself. So in our example above, we might end up
with:

    lab: 5 4 2 0 3 6 1

Don't look at `ptn`; it's garbage at this point. And don't look at `canong` — it's a red herring.
Look at the original graph `g`, which (as far as I can tell) Nauty does _not_ trash or modify in any
way. If you take its vertices in the order given by `lab`, then you get the canonical graph we're
after!

This description is probably about as confusing as [the real docs](http://pallini.di.uniroma1.it/nug26.pdf) at this point;
so if you got here looking for Nauty/Traces code to canonicalize a (non-directed, dense,
perhaps bipartite) graph, you'll probably want to read my code
[on GitHub here](https://github.com/Quuxplusone/wolves-and-sheep/blob/master/canonicalize_matrix.cpp).
