---
layout: post
title: "Peaceful Encampments, round 2"
date: 2019-01-21 00:01:00 +0000
tags:
  math
  puzzles
  web
---

> Consider a plain represented by the unit square. On this plain we want to "peacefully encamp"
> two armies of point-sized soldiers — one army red and one army green. Each soldier "attacks"
> chess-queen-wise: horizontally, vertically, and diagonally in all directions. The puzzle is
> to maximize the size of the equal armies (equivalently, maximize the size of the smallest army),
> given the constraint that no pair of opposing soldiers can be placed attacking each other.

[Previously on this blog](/blog/2019/01/10/happy-mmxix/#i-met-dr.-knuth-a-couple-of-time):
Back in 2014 I had conjectured that the optimum solution for two armies was the following.
(Each region has size $$\left(1 - {\sqrt{3}\over 2}\right) \approx 0.13397$$.) This morning
I learned that I was _very_ wrong about that!

|:---------------------------------------------------------------------------------:|
| [![Two encampments of size 0.13397 each](/blog/images/2019-01-10-1340.png)][1] |

Last night I was noodling on this problem again (for the more-than-two-army case) and came up with
this knight's-move-inspired solution for 4 armies. Each home-plate-shaped region has size
exactly 0.05.

|:--------------------------------------------------------------------------------:|
| [![Four encampments of size 0.05 each](/blog/images/2019-01-21-one-20th.png)][2] |

In the above image, I used two colors to illustrate the four-army solution — so the four-army
solution (also [here][3]) happens to double as a two-army solution of size 0.1. But what if we take that solution and
rearrange the colors so that the friendly armies are next to each other instead of diametrically
across the board from each other? I expected a modest increase, but after making the switch and
jiggling it a bit, I arrived at _this_ interesting configuration!

|:-------------------------------------------------------------------------------------:|
| [![Two encampments of size 0.1458 each](/blog/images/2019-01-21-1458.png)][4] |

The vertical lines divide the unit square into fourths; the slashing lines divide the sides of the unit
square at the 1/3 and 2/3 marks. So each home-plate-sized region has size
$$\frac{19}{288} \approx 0.06597$$,
and each slashing region has size $$\frac{23}{288} \approx 0.07986$$,
for a total army size of $$\frac{7}{48} \approx 0.1458$$. Wow!

If we take the same four-army solution and color _just two_ armies with the same color,
then we get a three-army version. Jiggling that version arrives at these two solutions for
three armies. The left-hand solution has army size $$\frac{1}{15} \approx 0.0667$$; the
better right-hand solution has army size $$\approx 0.0718$$. I have not yet done the math
to describe this solution algebraically.

|:------------------------------------------------------------------------------------------------:|:---------------------------------------------------------------------------------------:|
| [![Three encampments of size 0.0667 each](/blog/images/2019-01-21-three-army-suboptimal.png)][5] | [![Three encampments of size 0.0718 each](/blog/images/2019-01-21-three-armies.png)][6] |

Finally, here's my best (and only) solution for five armies. Each encampment here has size
$$\approx 0.0311$$. Again I haven't done the math to describe it algebraically.

|:----------------------------------------------------------------------------------------:|
| [![Five encampments of size 0.0311 each](/blog/images/2019-01-21-five-army-0311.png)][7] |


[1]: http://club.cc.cmu.edu/~ajo/disseminate/encamp.html?q=%7B%22v%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.422649730810374%7D%5D%2C%22h%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.42265%7D%5D%2C%22s%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A1%7D%5D%2C%22b%22%3A%5B%7B%22minInvariant%22%3A-0.2113248654051871177%2C%22maxInvariant%22%3A0.2113248654051871177%7D%5D%7D
[2]: http://club.cc.cmu.edu/~ajo/disseminate/encamp.html?q=%7B%22v%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.195%7D%2C%7B%22minInvariant%22%3A0.801%2C%22maxInvariant%22%3A1%7D%5D%2C%22h%22%3A%5B%7B%22minInvariant%22%3A0.215%2C%22maxInvariant%22%3A0.809%7D%5D%2C%22s%22%3A%5B%7B%22minInvariant%22%3A0.606%2C%22maxInvariant%22%3A1.416%7D%5D%2C%22b%22%3A%5B%7B%22minInvariant%22%3A-1%2C%22maxInvariant%22%3A-0.391%7D%2C%7B%22minInvariant%22%3A0.394%2C%22maxInvariant%22%3A1%7D%5D%7D
[3]: http://club.cc.cmu.edu/~ajo/disseminate/encamp4.html?q=%7B%22v%22%3A%5B%7B%22minInvariant%22%3A0.2%2C%22maxInvariant%22%3A0.5%2C%22color%22%3A%22green%22%7D%2C%7B%22minInvariant%22%3A0.5%2C%22maxInvariant%22%3A0.8%2C%22color%22%3A%22blue%22%7D%2C%7B%22minInvariant%22%3A0.8%2C%22maxInvariant%22%3A1%2C%22color%22%3A%22red%22%7D%5D%2C%22h%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.20%2C%22color%22%3A%22blue%22%7D%2C%7B%22minInvariant%22%3A0.5%2C%22maxInvariant%22%3A0.8%2C%22color%22%3A%22red%22%7D%2C%7B%22minInvariant%22%3A0.8%2C%22maxInvariant%22%3A1%2C%22color%22%3A%22green%22%7D%5D%2C%22s%22%3A%5B%7B%22minInvariant%22%3A0.6%2C%22maxInvariant%22%3A1%2C%22color%22%3A%22blue%22%7D%2C%7B%22minInvariant%22%3A1%2C%22maxInvariant%22%3A1.4%2C%22color%22%3A%22green%22%7D%2C%7B%22minInvariant%22%3A1.4%2C%22maxInvariant%22%3A1.8%2C%22color%22%3A%22red%22%7D%5D%2C%22b%22%3A%5B%7B%22minInvariant%22%3A-0.8%2C%22maxInvariant%22%3A-0.4%2C%22color%22%3A%22blue%22%7D%2C%7B%22minInvariant%22%3A-0.4%2C%22maxInvariant%22%3A0%2C%22color%22%3A%22red%22%7D%2C%7B%22minInvariant%22%3A0.4%2C%22maxInvariant%22%3A1%2C%22color%22%3A%22green%22%7D%5D%7D
[4]: http://club.cc.cmu.edu/~ajo/disseminate/encamp.html?q=%7B%22v%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.25%7D%2C%7B%22minInvariant%22%3A0.5%2C%22maxInvariant%22%3A0.75%7D%5D%2C%22h%22%3A%5B%7B%22minInvariant%22%3A0.5%2C%22maxInvariant%22%3A1%7D%5D%2C%22s%22%3A%5B%7B%22minInvariant%22%3A0.667%2C%22maxInvariant%22%3A1%7D%2C%7B%22minInvariant%22%3A1.333%2C%22maxInvariant%22%3A2%7D%5D%2C%22b%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A1%7D%5D%7D
[5]: http://club.cc.cmu.edu/~ajo/disseminate/encamp3.html?q=%7B%22v%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.23%2C%22color%22%3A%22red%22%7D%2C%7B%22minInvariant%22%3A0.77%2C%22maxInvariant%22%3A1%2C%22color%22%3A%22green%22%7D%5D%2C%22h%22%3A%5B%7B%22minInvariant%22%3A0.172%2C%22maxInvariant%22%3A0.5%2C%22color%22%3A%22green%22%7D%2C%7B%22minInvariant%22%3A0.5%2C%22maxInvariant%22%3A0.828%2C%22color%22%3A%22red%22%7D%5D%2C%22s%22%3A%5B%7B%22minInvariant%22%3A0.568%2C%22maxInvariant%22%3A1%2C%22color%22%3A%22red%22%7D%2C%7B%22minInvariant%22%3A1%2C%22maxInvariant%22%3A1.432%2C%22color%22%3A%22green%22%7D%5D%2C%22b%22%3A%5B%7B%22minInvariant%22%3A-1%2C%22maxInvariant%22%3A-0.368%2C%22color%22%3A%22green%22%7D%2C%7B%22minInvariant%22%3A0.368%2C%22maxInvariant%22%3A1%2C%22color%22%3A%22red%22%7D%5D%7D
[6]: http://club.cc.cmu.edu/~ajo/disseminate/encamp3.html?q=%7B%22v%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.246%2C%22color%22%3A%22red%22%7D%2C%7B%22minInvariant%22%3A0.246%2C%22maxInvariant%22%3A0.566%2C%22color%22%3A%22green%22%7D%5D%2C%22h%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.291%2C%22color%22%3A%22green%22%7D%2C%7B%22minInvariant%22%3A0.53%2C%22maxInvariant%22%3A0.889%2C%22color%22%3A%22red%22%7D%5D%2C%22s%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.699%2C%22color%22%3A%22green%22%7D%2C%7B%22minInvariant%22%3A0.699%2C%22maxInvariant%22%3A1.107%2C%22color%22%3A%22red%22%7D%5D%2C%22b%22%3A%5B%7B%22minInvariant%22%3A-0.441%2C%22maxInvariant%22%3A0%2C%22color%22%3A%22green%22%7D%2C%7B%22minInvariant%22%3A0.345%2C%22maxInvariant%22%3A1%2C%22color%22%3A%22red%22%7D%5D%7D
[7]: http://club.cc.cmu.edu/~ajo/disseminate/encamp4.html?q=%7B%22v%22%3A%5B%7B%22minInvariant%22%3A0.197%2C%22maxInvariant%22%3A0.407%2C%22color%22%3A%22green%22%7D%2C%7B%22minInvariant%22%3A0.407%2C%22maxInvariant%22%3A0.603%2C%22color%22%3A%22magenta%22%7D%2C%7B%22minInvariant%22%3A0.603%2C%22maxInvariant%22%3A0.806%2C%22color%22%3A%22blue%22%7D%2C%7B%22minInvariant%22%3A0.806%2C%22maxInvariant%22%3A1%2C%22color%22%3A%22red%22%7D%5D%2C%22h%22%3A%5B%7B%22minInvariant%22%3A0%2C%22maxInvariant%22%3A0.199%2C%22color%22%3A%22blue%22%7D%2C%7B%22minInvariant%22%3A0.404%2C%22maxInvariant%22%3A0.596%2C%22color%22%3A%22magenta%22%7D%2C%7B%22minInvariant%22%3A0.596%2C%22maxInvariant%22%3A0.805%2C%22color%22%3A%22red%22%7D%2C%7B%22minInvariant%22%3A0.805%2C%22maxInvariant%22%3A1%2C%22color%22%3A%22green%22%7D%5D%2C%22s%22%3A%5B%7B%22minInvariant%22%3A0.609%2C%22maxInvariant%22%3A0.869%2C%22color%22%3A%22blue%22%7D%2C%7B%22minInvariant%22%3A0.869%2C%22maxInvariant%22%3A1.142%2C%22color%22%3A%22magenta%22%7D%2C%7B%22minInvariant%22%3A1.142%2C%22maxInvariant%22%3A1.407%2C%22color%22%3A%22green%22%7D%2C%7B%22minInvariant%22%3A1.407%2C%22maxInvariant%22%3A1.825%2C%22color%22%3A%22red%22%7D%5D%2C%22b%22%3A%5B%7B%22minInvariant%22%3A-0.821%2C%22maxInvariant%22%3A-0.402%2C%22color%22%3A%22blue%22%7D%2C%7B%22minInvariant%22%3A-0.402%2C%22maxInvariant%22%3A-0.138%2C%22color%22%3A%22red%22%7D%2C%7B%22minInvariant%22%3A-0.138%2C%22maxInvariant%22%3A0.138%2C%22color%22%3A%22magenta%22%7D%2C%7B%22minInvariant%22%3A0.402%2C%22maxInvariant%22%3A1%2C%22color%22%3A%22green%22%7D%5D%7D
