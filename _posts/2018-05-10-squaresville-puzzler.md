---
layout: post
title: 'Puzzle: Policing Squaresville'
date: 2018-05-10 00:01:00 +0000
tags:
  blog-roundup
  math
  puzzles
---

If I blog these math puzzles, I can close the browser tabs containing them and un-nerd-snipe
myself...


## Puzzle #1

[Can the policeman shoot the thief?](https://puzzling.stackexchange.com/questions/36565/can-the-policeman-catch-the-thief)
This is (claimed to be) a puzzle from the 1978 Moscow Math Olympiad. We have a square
city of 2 by 2 city blocks (so, 3 north-south streets and 3 east-west streets).

![Squaresville city map](https://i.stack.imgur.com/odSRh.jpg){: .float-right}

Suppose an (alleged) thief is moving around on these streets, somewhere in the city.
There's also a single policeman for the whole city; he can run 2.00001 times as fast
as the thief can. Initially the policeman has no idea where the thief is, nor what the
thief's movement strategy is. Work out a strategy by which the policeman will eventually,
inevitably, spot the thief.

[Milo Brandt has written up a very complete solution.](https://puzzling.stackexchange.com/a/36567/3896)


## Puzzle #1.5

Incidentally, suppose there *may or may not* be a thief in the city. How long does
our policeman have to patrol the streets in order to conclude definitely that no thief
is present? (Again, assuming that our policeman's cruising speed is 2.0001 times the
maximum speed of any hypothetical thief.)


## Puzzle #2

[In what kinds of cities can the policeman catch up to the thief?](https://puzzling.stackexchange.com/questions/36648/can-the-policeman-actually-catch-the-thief-instead-of-shooting)
In this variation, we assume that the policeman *does* initially know where the thief
is — the thief starts somewhere in his line of sight. And whenever the thief exits
the policeman's line of sight, the policeman is keen enough to notice which direction
the thief was moving.

In our 2x2 Squaresville, the thief's initial position can never be more than 2 blocks
from the policeman, and the policeman can cover those 2 blocks faster than the thief
can cover 1 block; so the policeman will arrive at the (alleged!) thief's original
position before the thief has managed to get around the next corner; and so the policeman
will catch the thief easily.

The policeman also seems to have winning strategies in 3x3 and 4x4 cities, as detailed
in the StackExchange answers.

Does the policeman have a winning strategy in the 5x5 city? The StackExchange answers
show that the strategy, if it exists, must be non-trivial.

What about a 4x5 city? 3x5? 1xN? (Obviously the policeman has a winning strategy in
every 0xN city — that's a city with one Main Street and no side streets at all.)

If you discover anything non-trivial here, I encourage you to [post it as an answer
on StackExchange!](https://puzzling.stackexchange.com/questions/36648/can-the-policeman-actually-catch-the-thief-instead-of-shooting)


## Other mathematical puzzles

If you like this kind of puzzle, you might be interested in
[fivethirtyeight's "The Riddler"](https://fivethirtyeight.com/tag/the-riddler/),
which posts a new mathematical puzzle every week.
