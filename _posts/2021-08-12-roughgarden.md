---
layout: post
title: 'Two counterintuitive results'
date: 2021-08-12 00:01:00 +0000
tags:
  math
  puzzles
  science
---

Via Hacker News: [lecture notes](https://timroughgarden.org/notes.html)
from several of Tim Roughgarden's courses at Stanford, particularly CS269I "Incentives in Computer Science."
They're worth reading from start to finish. Here are two gems I thought were especially neat.


## Braess's paradox

From [Lecture 7](http://timroughgarden.org/f16/l/l7.pdf): Suppose a heavy weight is
suspended from a fixed hook by a system of springs and strings. (A string's length is
constant as long as it carries any load; a spring's length is roughly proportional to
the load it carries.) The springs and strings are interconnected among themselves in
some clever way, but ultimately they all end up connected to the fixed hook at the top,
and all connected to a single point on the weight at the bottom. So the whole assembly
is pretty much vertically linear.

Now find one of the strings and cut it. If it was carrying no load, then we expect nothing
to happen. If it was loaded, then its load is suddenly redistributed among the other
strings and springs, in some way that depends on how the system
is interconnected. We might reasonably expect the weight to drop a little bit further
down as a result. (Or crash to the floor, if the string we cut was the [min-cut](https://en.wikipedia.org/wiki/Minimum_cut).)

But would you think that the weight could _rise_ as a result of cutting one of the loaded
strings? It turns out that it totally can — and [here's video proof!](https://www.youtube.com/watch?v=gwS8jiVOWps)

The trick is that the loaded string is the last thing keeping two springs in series;
cutting the string puts the springs in parallel, where they can each take half the load,
and become shorter as a result. Here's Tim Roughgarden's diagram of the situation:

![](/blog/images/2021-08-12-braess-paradox.png)

Vice versa, if you start with the system on the right, you'll find that "hitching up"
two pieces of the system with a short piece of string causes the weight to drop further
down. (Anyone got a video of that?) This paradox has practical applications for the design
of roads and other networks: Counterintuitively, adding a few high-speed links _might_ alter
traffic patterns so that the whole network gets slower, and removing a link _might_ make a
slow network faster. Of course, it _might_ not work that way either — certainly we don't
want it to work that way, if we do our jobs right — but the paradox shows that we really
need to think about how to do it right.


## Two urns

From [Lecture 10](http://timroughgarden.org/f16/l/l10.pdf), another paradox involving
"Herding and Information Cascades."

Suppose we have two urns.
The "red urn" has two red balls and one blue ball.
The "blue urn" has two blue balls and one red ball.
The experimenter picks one of these urns uniformly at random, and throws away the other one
— we'll never see it again.

Now here come an infinite number of players. The first player draws a random ball from the urn,
privately observes its color, and puts it back. The first player then guesses out loud whether the
urn is the "red urn" or the "blue urn." Then, the second player does the same thing (but notice
that the second player has slightly more information than the first player: he knows what the
first player guessed). Then the third player does the same as the first two (but with yet more
knowledge). And so on, forever. Each player's (selfish) goal is to maximize their own likelihood
of guessing correctly.

Obviously the first player should just guess the color of the ball they observed; they'll
have a 2/3 chance of guessing correctly. The second player, knowing this, will have the benefit
of two independent observations: Player 1's and his own. If his observation agrees with Player 1's
(both red or both blue), obviously he'll guess that color; if his observation disagrees, then
we've got one red ball and one blue, so he might as well guess the color he observed.
Player 2 ends up always guessing his own ball's color, and thus also guesses right 2/3 of the time.

Player 3 knows that Players 1 and 2 simply guessed the colors they saw. If they disagreed, then
Player 3 doesn't really have any more information than Player 1, and should just guess the color
he observed. But if the first two players _agreed_ in their observations, then Player 3 has the
benefit of _three_ independent observations! If those observations were RRR or RRB, then Player 3
should logically guess "red"; if BBR or BBB, he should logically guess "blue." In other words,
if Players 1 and 2 agreed, then Player 3 should disregard his own observation and simply follow
the herd. (And then Player 4 follows the same logic as Player 3, and so on ad infinitum.)

The results are that each individual player maximizes their chances of guessing correctly;
and in fact the _expected value_ averaged over all the players goes up as well; but the
distribution of outcomes changes drastically.

![](/blog/images/2021-08-12-urns.png)

In four-ninths of the games, Players 1 and 2
guess correctly, and thus so does everyone else — 100% success! In one-ninth of the games,
Players 1 and 2 both happen to guess incorrectly (by unluckily drawing the odd ball), and
thus _so does everyone else_ — 100% failure!

> The other four-ninths of the time, Players 1 and 2 disagree, and so we end up recursing:
> 16/81 of the time we score only one wrong guess; 4/81 of the time we score only one right
> guess; and 16/81 of the time Players 3 and 4 disagree, so we end up recursing... and so on.
> To generate the histograms above, I used [this C++ and Python code](/blog/code/2021-08-12-urns.cpp)
> to do some quick simulations with only 100 players instead of an infinite number.

So the players' strategy is both the selfishly optimal and (as far as I know) the globally optimal strategy;
yet, it has this awkward property that as often as one-ninth of the time _literally nobody_
guesses the urn correctly — something that would never happen if all the players guessed
individually without communicating.

This paradox (or whatever you'd call it) has practical applications for restaurant reviews,
conference submission reviews...
