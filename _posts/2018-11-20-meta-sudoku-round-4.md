---
layout: post
title: "Meta-sudoku, round 4: A valid meta-puzzle"
date: 2018-11-20 00:02:00 +0000
tags:
  math
  puzzles
excerpt: |
  My program informs me that there is exactly one way to complete the following
  configuration (assigning a number 1 through 9 to the five marked cells)
  to form a valid Sudoku puzzle. Can you find that one way?

  ![Sudoku grid](/blog/images/2018-11-20-metapuzzle.png)
---

BREAKING NEWS: I've found a meta-sudoku!

My program informs me that there is exactly one way to complete the following
configuration (assigning a number 1 through 9 to the five marked cells)
to form a valid Sudoku puzzle. Can you find that one way?

![Sudoku grid](/blog/images/2018-11-20-metapuzzle.png)

A (technically unnecessary) hint: You'll end up using the numbers 1 through 5 exactly once each.
And a reminder that I have no idea how to solve the 17-clue Sudoku that forms the basis of this
puzzle, except by brute force; so this puzzle is probably *not* one that you can solve using
unassisted brain power!

Previously on this blog: [(3)](/blog/2018/11/19/meta-sudoku-round-3),
[(2)](/blog/2018/11/18/meta-sudoku-round-2), [(1)](/blog/2018/10/26/sudoku-stories).

----

Mind-bendingly, the grid below left — identical except for one additional given —
is *not* a valid meta-sudoku puzzle: There are *two different* ways you could fill in
the five marked cells to form a valid Sudoku puzzle.

Yet the grid below right — identical except for a *different* additional given —
remains a valid meta-sudoku puzzle. Its solution also features the digits 1 to 5... but in a
*different arrangement* from the solution to the puzzle above!

|:------------------------------------------------------------:|:-------------------------------------------------------------:|
| ![Sudoku grid](/blog/images/2018-11-20-not-a-metapuzzle.png) | ![Sudoku grid](/blog/images/2018-11-20-also-a-metapuzzle.png) |

These puzzles featuring a mix of marked cells and givens are different from the "pure"
meta-sudokus I have been looking at up to now. They certainly do behave weirdly!
