---
layout: post
title: "Befunge and Flobnar"
date: 2019-05-02 00:01:00 +0000
tags:
  esolang
---

A week-ish ago I discovered [Flobnar](https://github.com/catseye/Flobnar),
an esoteric programming language invented by Chris Pressey circa 2011 and
described either as "a functional dual of [Befunge-93](https://esolangs.org/wiki/befunge)" or
"what happens when you get Befunge-93 drunk."

Within a couple of hours, I had hacked together a Flobnar interpreter in C++;
you can find it [on my GitHub](https://github.com/Quuxplusone/Flobnar/tree/cplusplus/cppsrc).
Chris Pressey's reference implementation is written [in Haskell](https://github.com/catseye/Flobnar/tree/master/src);
there's another version [in Rust](https://github.com/Reconcyl/flobnar) which is used by
[tio.run](https://tio.run/##fZHRboIwFIbv@xR/UnYDGsGwKc0J2UPssmnEiG4ZAYOs3vju7LSW6WayE6Dt/53z97Tsm27bVv04FgW23TA09QndHtu67tG1GN5rnKumEY9YvFWfNefU2HXndoZjdTrhY0DVd1/tThTrf/0EgGfF3zIG4S6INbOMftaWsXGaOsxuiQmBSJO@pkcpCz40NKcDkZEObHBxNSqKiI0w46lZeTOUWhs8kd8BCfdhYrsg/eooizKCJWSelprXOesywEzBviRH7yklS9wHigDZy8aUJmT9PgaytIgD5HPZdag0c5Zynk22iqFCitTBjKWYIdsqcz0cLfh1BzIIlYnAdCvGDRkCdH0vhVpO9zIF3KPJinAzt1C@LoUVUpXa3KPNxQ8LEqvk989yG66M1qREnOFPpNw/lZiLHI/htWwcvwE).

This is what a (non-[conforming](http://www.99-bottles-of-beer.net/lyrics.html))
"99 bottles of beer" program looks like in Flobnar:

    99 bottles of beer on the wall
    99 bottles of beer
    Take one down, pass it around
    98 bottles of beer on the wall

       5:
      >* <            <
      ^2$         v < ^
      ^:g,<         +< <<\<\ <
      ^$0 +<<<<<<\ \< ^  $^#
      ^` |<   :$$< v  ,  :^7
      ^: >\\^ %<   v  +  >^*v/<\@
      ^   #$ v< 1  v  >\  #4  ^#
      ^   1: v6+p< v  ## ^ \ < 9
      ^   +  v*<0+<v  ^: ^ #>v *
      ^   $  v8+p< v  ^- ^ 4 v #
      ^   :  v: 0 0v  ^1 ^ * v 9
     :^<<<<<<</<<<|<  ^  ^ 4 v +
     g,<     ^<   1   ^  ^   v 2
    :2 +<<<<<<<<<<<<<< << <\<v
    ` |<             :^  ^ 0 v
    #:>\^            `|     /<
    7+ <             1  7^\\<:
    *1                0 * <> -
    4                   4    1

And for comparison, this is what a similar "99 bottles of beer" program looks like in Befunge.
I wouldn't say this is idiomatic Befunge; rather, it's me trying to be clever and employ
"code reuse" and "a functional style" in a language that isn't really designed for it.
My original idea was to make a "direct port" of the Flobnar code into Befunge; but I quickly
realized that that was going to be a real pain, and settled for the version you see here.

    v
     99 bottles of beer on the wall
     99 bottles of beer
     Take one down, pass it around
    >               v
       v   ,*25$<   >92+9*v
    50< >:65*0-`|            0< 1 <
    50^#^+1,g1: <             ^   ^
    30>>08g76p      28g39pv   ^   ^
    51v^09g76p19g96p29g39p 08g 97p^
       >1-16g96p26g39p19g9 7pv     @
      > 27g39p                   :!|
            v:p12+*86%*25:<<<< ,*25<
            >52*/:68*+11p!1+  ^

In the Flobnar code, the subexpression `5*2` (in the upper left corner) is reused four times.

In the Befunge code, the most interesting cell is (x=3, y=9).
