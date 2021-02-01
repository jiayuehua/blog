---
layout: post
title: "Conway's Soldiers"
date: 2020-10-17 00:01:00 +0000
tags:
  blog-roundup
  puzzles
---

The other day I learned about [Conway's Soldiers](https://en.wikipedia.org/wiki/Conway%27s_Soldiers):
Consider playing peg solitaire on an infinite grid of holes, where initially an infinite number of pegs
fill the entire half-plane south of the line $$y=0$$. The goal is to advance one peg as far north as
possible, by repeatedly jumping and removing pegs in the usual peg-solitaire fashion.

[![Image rendered by Simon Tatham](/blog/images/2020-10-17-soldiers.gif)](https://www.chiark.greenend.org.uk/~sgtatham/solarmy/)

John Horton Conway explored the problem and proved the surprising fact that no matter how many pegs you
jump, it's impossible to advance any peg beyond the line $$y=4$$ in a finite number of moves!

However, [Simon Tatham observes](https://www.chiark.greenend.org.uk/~sgtatham/solarmy/)
that it is possible to advance a single peg to $$y=5$$, if you are
permitted an _infinite_ number of moves to do so. He provides this animation
of the winning strategy, which provably must use every single one of the pegs:

[![Image rendered by Simon Tatham](/blog/images/2020-10-17-solution.gif)](https://www.chiark.greenend.org.uk/~sgtatham/solarmy/solution.gif)

----

Incidentally, Conway died earlier this year, of COVID-19, causing me to update
["Hello Muddah, Hello Faddah (Coronavirus Version)"](/blog/2020/04/08/hello-muddah/) (2020-04-08)
with another verse:

> Rest in peace, John / Horton Conway,  
> You have gone [A- / nacreon](https://en.wikipedia.org/wiki/The_Anacreontic_Song) way;  
> You've ascended / Gardner's column,  
> And now Heaven has another angel problem!
