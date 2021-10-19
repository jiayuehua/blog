---
layout: post
title: "Overload arrangement puzzles"
date: 2021-10-29 00:02:00 +0000
tags:
  cppcon
  overload-resolution
  puzzles
---

I contributed some puzzles to the online venue for CppCon 2021.
Some of my contributions were just C++ Pub Quiz questions recycled from 2019;
but I also contributed a set of logic puzzles that I think are fairly novel.
I'm going to call this type of puzzle "overload arrangement."

You're presented with a set of lines of C++ code.

    int f(short);       int A = f(0);
    int f(const char*); int B = f("");
    int f(void*);       int C = f(nullptr);

Your goal is to rearrange these lines into a valid translation unit.
The given order, A-B-C, doesn't work because the compiler rejects

    int C = f(nullptr);

as an ambiguity: `f(const char*)` and `f(void*)` are equally good matches.
But if we rearrange to put line C first:

    int f(void*);       int C = f(nullptr);
    int f(short);       int A = f(0);
    int f(const char*); int B = f("");

then `A = f(0)` becomes ambiguous: `f(void*)` and `f(short)` are
equally good matches for `0`.

The solution to this three-line puzzle is... given at the bottom
of this page.

Now that you've seen an example, here are a few more puzzles
sans solutions. Of course this kind of puzzle is easy to crack
if you resort to a compiler, and even easier if you use a Python script
to try all the possible permutations automatically. But I flatter
myself that it's kind of challenging — and maybe it'll even be entertaining —
to solve these by hand.


## Puzzle One

    int f(auto);  int A = f();
    int f(long);  int B = f(0);
    int f(void*); int C = f({});
    int f();      int D = f(nullptr);


## Puzzle Two

    int f(int);   int A = f();
    int f(int=0); int B = f(0);
    int f(...);   int C = f({});
    int f();      int D = f(nullptr);


## Puzzle Three

    int f(auto);  int A = f();
    int f();      int B = f(0);
    int f(int);   int C = f({});
    int f(int=0); int D = f(nullptr);


## Puzzle Four

    int f(int);   int A = f();
    int f(int=0); int B = f(0);
    int f();      int C = f({});
    int f(void*); int D = f('\0');
    int f(long);  int E = f(nullptr);


## Puzzle Five

    int f(decltype(f(0))); int A = f();
    decltype(f(0)) f(...); int B = f();
    char f(short);         int C = f(0);
    short f(int);          int D = f('0');


## Analysis...?

These puzzles can be expressed in terms of constraint satisfaction,
sort of like a variation on
["Who owns the zebra?"](https://en.wikipedia.org/wiki/Zebra_Puzzle)
but with many fewer dimensions and many more "positional" constraints —
I mean, constraints of the form "The green house is somewhere to the
right of the ivory house."

For example, Puzzle Two above is isomorphic to the following rather
boringly simple logic puzzle about heights:

* Alice, Bob, Carol, and Dave all have different heights.
* Alice is shorter than exactly one other person.
* Carol is shorter than Alice and/or shorter than Bob.
* Dave is shorter than Carol.

And Puzzle Three:

* Alice, Bob, Carol, and Dave all have different heights.
* Alice is shorter than either Bob or Dave, but not both.
* Bob is not the tallest.
* Dave is shorter than Alice.

Here "is shorter" corresponds to "appears later (lower down)
in the translation unit."

C++ puts some restrictions on what we can express; for example,
it's easy to express "Alice is not the tallest" but impossible
(in general) to express "Alice is not the shortest." And we can
express some interesting constraints, such as: "If Alice is taller than
Bob, then she is shorter than exactly one of Carol or Dave."
We see some of those interesting constraints in Puzzle Four:

* Alice, Bob, Carol, Dave, and Ernie all have different heights.
* Alice is intermediate in height between Bob and Carol.
* If Dave is not shorter than exactly one of Alice and Ernie, then Dave is shorter than exactly one of Bob and Ernie.
* Unless there is exactly one person taller than Carol, there are exactly two people taller than Carol and they are Alice and Bob.
* Ernie is shorter than Dave.

Puzzle Five doesn't submit to this kind of rephrasing as easily,
in my opinion.

Which do you find more interesting: the C++ phrasing or the
logic-puzzle-about-heights phrasing? And which do you find
easier to solve?


## Answer to the sample puzzle

The correct ordering — the only one that produces a valid translation unit — is "A-C-B."

    int f(short);       int A = f(0);
    int f(void*);       int C = f(nullptr);
    int f(const char*); int B = f("");
