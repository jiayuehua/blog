---
layout: post
title: 'Meta-sudoku, round 2'
date: 2018-11-18 00:01:00 +0000
tags:
  math
  puzzles
---

[Previously on this blog:](/blog/2018/10/26/sudoku-stories) the notion of a "meta-sudoku" puzzle
as a configuration of 81 squares, each labeled "filled" or "unfilled." The goal of the puzzle is to find
an assignment of numbers to the "filled" squares so that the resulting (partially filled) grid
is a solvable Sudoku puzzle.

[I wrote a little program](https://github.com/Quuxplusone/MetaSudoku) to run through
all the possible assignments to the following configuration:

![Sudoku grid](/blog/images/2018-11-18-seventeen-meta.png)

First, I asked my program how many viable assignments existed. By "viable," I just mean that
the assignment is unique up to permutation, and it doesn't contain any _obvious_ contradictions
out of the gate. For example: The grid below left is not viable because you can swap all the 1s
for 2s (and vice versa) to create a "lower-numbered" grid. The grid below right is not viable
because it contains an obvious contradiction: two 1s in the top row.

|:-------------------------------------------------------------:|:-------------------------------------------------------------------:|
| ![Sudoku grid](/blog/images/2018-11-18-seventeen-invalid.png) | ![Sudoku grid](/blog/images/2018-11-18-seventeen-contradiction.png) |


According to my program, there are 2,659,328,730 viable assignments. To compute just this
_number_ of possible assignments, without inspecting the resulting grids for solvability, took
my program 67 seconds.

Note in passing that if we completely brute-forced it, by testing all 9<sup>17</sup> possible
assignments for viability, we'd have to test 9<sup>17</sup> = 16,677,181,699,666,569 assignments!
Even if we could test assignments for viability at the astonishing rate of four per nanosecond,
it would still take us _48 days_ of continuous computation in order to test all 9<sup>17</sup> of them.
So short-circuiting large swaths of the search space turns out to be super important here.

----

Now how about validating that an assignment is a valid Sudoku — which boils down to Sudoku-solving?

On my dual-core, hyperthreaded MacBook, my program can test grids for solvability at the rate
of about 50,000 grids per second. At this rate, it takes something like 15 hours of continuous
computation to inspect all 2.6 billion possible viable assignments for just this _one_ meta-sudoku
configuration. So I did that — at least, long enough for it to find some solutions.

It turns out that this configuration can be filled in to create a valid Sudoku grid in at least
these four fundamentally different ways:

|:-------------------------------------------------------:|:-------------------------------------------------------:|
| ![Sudoku grid](/blog/images/2018-11-18-seventeen-a.png) | ![Sudoku grid](/blog/images/2018-11-18-seventeen-b.png) |
| ![Sudoku grid](/blog/images/2018-11-18-seventeen-c.png) | ![Sudoku grid](/blog/images/2018-11-18-seventeen-d.png) |

So, in terms of my original puzzle, we would say that this particular meta-sudoku configuration
is _not_ a valid meta-sudoku puzzle: there is more than one possible meta-solution. (Namely,
there are at least the four solutions above.)

----

How about the moose-shaped meta-sudoku?
With 24 filled squares, brute force would look at 9<sup>24</sup> grids —
that's 80 thousand billion billion.
From partial runs of my program in "counting" mode, I know it actually has somewhere between
480 billion and 87698 billion viable grids.

But my program quickly turned up many distinct _solvable_ Sudoku grids for the moose configuration,
which means that the moose, too, is not a valid meta-sudoku. Here are some of the distinctly
solvable Sudokus you can form from the moose configuration:

|:---------------------------------------------------:|:---------------------------------------------------:|
| ![Sudoku grid](/blog/images/2018-11-18-moose-a.png) | ![Sudoku grid](/blog/images/2018-11-18-moose-b.png) |
| ![Sudoku grid](/blog/images/2018-11-18-moose-c.png) | ![Sudoku grid](/blog/images/2018-11-18-moose-d.png) |

I still don't know if there exist any "meta-sudoku" configurations which are really "valid"
in the sense of having only _one_ distinct solution. If you find one, let me know!
