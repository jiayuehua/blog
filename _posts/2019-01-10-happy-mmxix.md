---
layout: post
title: "Happy birthday, Donald Knuth! and Peaceful Encampments"
date: 2019-01-10 00:01:00 +0000
tags:
  adventure
  celebration-of-mind
  knuth
  litclub
  math
  puzzles
  web
---

Today (January 10th) is Donald Knuth's 81st birthday. Happy birthday, Dr. Knuth!

Readers of this blog will already know [Donald Knuth](https://www-cs-faculty.stanford.edu/~knuth/)
as the primary author of TeX, Metafont, and WEB;
the author of the ongoing "Art of Computer Programming" (TAOCP) series; and one of the two Dons
behind the 1998 collaboration that produced a translation of _Adventure_ into a CWEB "literate program."
The beautiful (and fairly informative, and fairly entertaining) result is
[here](http://literateprogramming.com/adventure.pdf), and is also included as pages 235–395 of
Knuth's [_Selected Papers on Fun & Games_](https://amzn.to/2RhoZTb) (2011).

I met Dr. Knuth a couple of times in real life. The first time was at the Martin Gardner
Celebration of Mind at UC Berkeley in 2014. (In those days ambigram expert Scott Kim, familiar to readers of
Douglas Hofstadter's [_Gödel, Escher, Bach_](https://amzn.to/2SJTwW8), did a new logo for the
Celebration of Mind every year. See ["Scott Kim's rotational ambigrams"](/blog/2020/10/18/scott-kim-gardner-ambigrams/) (2020-10-18).)

Knuth's pet puzzle of the day, that day, was a sort of continuous version of a
[nonattacking queens](https://math.stackexchange.com/questions/687298/maximum-nonattacking-black-and-white-queens-on-infinite-chessboard)
problem which he called "Peaceful Encampments." Paraphrased by me:

> Consider a plain represented by the unit square. On this plain we want to "peacefully encamp"
> two armies of point-sized soldiers — one army red and one army green. Each soldier "attacks"
> chess-queen-wise: horizontally, vertically, and diagonally in all directions. The puzzle is
> to maximize the size of the equal armies (equivalently, maximize the size of the smallest army),
> given the constraint that no pair of opposing soldiers can be placed attacking each other.

Back in 2014, I went and wrote a JavaScript visualizer for the problem, and — purely by noodling
around, no rigor at all — came up with a few solutions, such as the following five.
I conjectured the bottom-right one to be the best possible solution.

|:------------------------------------------------------:|:------------------------------------------------------:|
| [![Encampments](/blog/images/2019-01-10-1111.png)][1]  | [![Encampments](/blog/images/2019-01-10-1111b.png)][2] |
| [![Encampments](/blog/images/2019-01-10-1250.png)][3]  |                                                        |
| [![Encampments](/blog/images/2019-01-10-1320.png)][4]  | [![Encampments](/blog/images/2019-01-10-1340.png)][5]  |

[1]: http://club.cc.cmu.edu/~ajo/disseminate/encamp4.html?q=%7B%22v%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.1666%7D%2C%7B%22minInvariant%22%3A0.8333%2C%22maxInvariant%22%3A1%7D%5D%2C%22h%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.1666%7D%2C%7B%22minInvariant%22%3A0.8333%2C%22maxInvariant%22%3A1%7D%5D%2C%22s%22%3A%5B%7B%22minInvariant%22%3A0.000%2C%22maxInvariant%22%3A0.3333%7D%2C%7B%22minInvariant%22%3A0.8333%2C%22maxInvariant%22%3A1.1666%7D%2C%7B%22minInvariant%22%3A1.6666%2C%22maxInvariant%22%3A2%7D%5D%2C%22b%22%3A%5B%7B%22minInvariant%22%3A-1%2C%22maxInvariant%22%3A-0.6666%7D%2C%7B%22minInvariant%22%3A-0.1666%2C%22maxInvariant%22%3A0.1666%7D%2C%7B%22minInvariant%22%3A0.6666%2C%22maxInvariant%22%3A1%7D%5D%7D
[2]: http://club.cc.cmu.edu/~ajo/disseminate/encamp4.html?q=%7B%22v%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.257%7D%2C%7B%22minInvariant%22%3A0.757%2C%22maxInvariant%22%3A1%7D%5D%2C%22h%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.257%7D%2C%7B%22minInvariant%22%3A0.757%2C%22maxInvariant%22%3A1%7D%5D%2C%22s%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.522%7D%2C%7B%22minInvariant%22%3A1.491%2C%22maxInvariant%22%3A2%7D%5D%2C%22b%22%3A%5B%7B%22minInvariant%22%3A-0.1666%2C%22maxInvariant%22%3A0.1666%7D%5D%7D
[3]: http://club.cc.cmu.edu/~ajo/disseminate/encamp4.html?q=%7B%22v%22%3A%5B%7B%22minInvariant%22%3A0.0%2C%22maxInvariant%22%3A0.5%7D%5D%2C%22h%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.5%7D%5D%2C%22s%22%3A%5B%7B%22minInvariant%22%3A0.0%2C%22maxInvariant%22%3A1.0%7D%5D%2C%22b%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.5%7D%5D%7D
[4]: http://club.cc.cmu.edu/~ajo/disseminate/encamp4.html?q=%7B%22v%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.482%7D%5D%2C%22h%22%3A%5B%7B%22minInvariant%22%3A0.31%2C%22maxInvariant%22%3A0.701%7D%5D%2C%22s%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.936%7D%5D%2C%22b%22%3A%5B%7B%22minInvariant%22%3A0.065%2C%22maxInvariant%22%3A1%7D%5D%7D
[5]: http://club.cc.cmu.edu/~ajo/disseminate/encamp4.html?q=%7B%22v%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.423%7D%5D%2C%22h%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.423%7D%5D%2C%22s%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A1%7D%5D%2C%22b%22%3A%5B%7B%22minInvariant%22%3A-0.211%2C%22maxInvariant%22%3A0.211%7D%5D%7D

I wrote at the time:

> The square in the upper left-hand corner has a side of
> $$a = \left(1 - {1\over\sqrt{3}} \right)$$
> and the big right isosceles triangle wedged into the lower left corner has a side of
> $$b = 1 - {a\over 2}.$$
> This maximizes the size of the (equal) armies, which is
> $$b^2 - a^2 = \left(1 - {\sqrt{3}\over 2}\right) \approx 0.13397$$.

My description of the puzzle continues:

> Once you've solved that, the next puzzle is to peacefully encamp *three* armies, four armies, etc...
> all the way to infinity. Knuth had a raggedy-looking conjectured solution for three armies,
> and nothing for four or higher.

I'd be interested to know if any further progress has been made on this problem since 2014.

----

UPDATE, 2019-01-21: My conjectured solution above is _not_ the best possible. [I've found a better
solution](/blog/2019/01/21/peaceful-encampments-round-2) with armies of size approximately 0.1458.
