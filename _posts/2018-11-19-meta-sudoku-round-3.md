---
layout: post
title: "Meta-sudoku, round 3: D'oh!"
date: 2018-11-19 00:02:00 +0000
tags:
  math
  puzzles
---

[Previously on this blog:](/blog/2018/10/26/sudoku-stories) the notion of a "meta-sudoku" puzzle
as a configuration of 81 squares, each labeled "filled" or "unfilled." The goal of the puzzle is to find
an assignment of numbers to the "filled" squares so that the resulting (partially filled) grid
is a solvable Sudoku puzzle.

[Yesterday on this blog:](/blog/2018/11/18/meta-sudoku-round-2) a relatively brute-force solver
that was able to prove that multiple distinct assignments existed for both the "moose" Sudoku
configuration and the 17-clue configuration for which I had such high hopes.

Just now I realized that *of course* both of those grids had multiple solutions!

|:------------------------------------------------------:|:----------------------------------------------------------:|
| ![Sudoku grid](/blog/images/2018-11-19-moose-swap.png) | ![Sudoku grid](/blog/images/2018-11-19-seventeen-swap.png) |

In each case, we can swap the two highlighted columns and produce a new, distinct Sudoku grid.
In the moose case, the new grid follows all our rules for introducing the digits in order.
In the 17-clue case, the new grid introduces 4 before 3; but we can follow up by swapping
the digits 3 and 4 everywhere they appear, and obtain this grid:

![Sudoku grid](/blog/images/2018-11-18-seventeen-c.png)

...which was indeed one of the four distinct grids found by my program.

So, just by looking at the configuration, without even filling in the grid at all, I should have
known that neither the moose nor this particular 17-clue grid could possibly have a unique
meta-solution! No complicated computer program needed!

----

Today I learned that Gordon Royle of the University of Western Australia
[maintains an exhaustive list](http://staffhome.ecm.uwa.edu.au/~00013890/sudokumin.php)
of all known 17-clue grids (up to rotation of the numbers, and up to row/column swaps
similar to those described above). His list currently has 49151 grids, representing
34446 meta-sudoku configurations. 26250 of these grids do not match the configuration
of any other grid in his collection. 3227 of these configurations have "obvious"
row/column swaps like what I just showed. I have set my program to investigate one
of the other 23023 configurations.
