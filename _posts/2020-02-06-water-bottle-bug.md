---
layout: post
title: "A bug in _Adventure_'s endgame"
date: 2020-02-06 00:01:00 +0000
tags:
  adventure
  digital-antiquaria
  knuth
  war-stories
---

WARNING: This post contains <b>major spoilers</b> related to
the endgame of Crowther and Woods' original _Adventure_!
Do not proceed unless you're okay with this.

The other day, Jonathan Ellis reported to me a bug in my C port of
Crowther and Woods' 350-point _Adventure_. (You can play my port online
[here](https://quuxplusone.github.io/Advent/).) The bug had to do with
bookkeeping the player's carrying capacity. I fixed it and then went over
the code with a fine-toothed comb to verify the absence of other similar
bugs... and lo and behold, I found a previously unknown(?) bug in
the endgame of Crowther and Woods' _original_ game!

The first thing to know about _Adventure_'s bookkeeping is that it tracks
the player's inventory in two redundant ways. First, location `-1` is the
"in hand" location. If you want to find out whether the player is toting
the gold nugget, you just check whether `PLACE(NUGGET).EQ.-1`. (In fact,
Don Woods [refactored this into a subroutine](https://github.com/Quuxplusone/Advent/blob/master/WOOD0350/advent.for#L78)
so you can just check `TOTING(NUGGET)`.) Second, the game tracks the
absolute number of objects toted by the player; each time the player picks
up or drops an item, we [increment](https://github.com/Quuxplusone/Advent/blob/master/WOOD0350/advent.for#L2406-L2429)
or decrement the global variable `HOLDNG`. This second method was added
by Don Woods; in the only surviving copy of Crowther's Fortran code,
`HOLDNG` does not exist. Woods added the two features of _Adventure_
that depend on `HOLDNG`: the player's seven-object carrying capacity
and the Plover Room puzzle.

> Spoiler alert: We're going to find a way to get `HOLDNG` out of sync with `TOTING`.

The original _Adventure_ has exactly two "containers" — the wicker cage
and the water bottle. The cage can contain only the bird; the bottle can
contain either water or oil. (In Crowther's source, you could empty
the bottle via `POUR` or `DRINK`, but then it would remain empty forever after.
Woods added both the `OIL` object, and the ability to refill the bottle in
various locations.)

Crowther's cage-and-bird system is pretty simple: if you `GET BIRD` when the bird
is already in the cage, then you also pick up the cage, and vice versa.
(And similarly for dropping the cage.) In all other respects, the bird and
cage behave like ordinary totable items. In particular, they each individually
increment and decrement `HOLDNG`. If you're carrying six objects and you try to
`GET CAGE`, you'll succeed (because you haven't yet reached your inventory limit
of 7 objects), resulting in `HOLDNG.EQ.8`. This is fine.

Crowther's bottle-and-liquids system works completely differently! In Crowther's
original source code, `BOTTLE` and `WATER` were synonymous; there was no "water"
in the game except for what started in the bottle.
Whether the bottle was "full of water" or not was controlled by `PROP(WATER)` —
`0` meant "full" (its initial state), and `1` meant "empty."

When Woods added the ability to refill the bottle via `GET WATER` — and for that
matter to `DRINK WATER` from the stream in the absence of any bottle — it suddenly
became important to have two distinct nouns `BOTTLE` and `WATER`. So Woods renamed
Crowther's `WATER` object to `BOTTLE`, and added new objects for the two liquids
`WATER` and `OIL`. The liquid contents of the bottle are controlled by
`PROP(BOTTLE)` — `0` means "full of water," `1` means "empty," and `2` means "full of oil."
But, in addition, the game updates `PLACE(WATER)` and `PLACE(OIL)` so that they
are "in hand" at the appropriate times. When the bottle is full of water and you
pick up the bottle, `PLACE(WATER)` becomes `-1` (the "in hand" location).
When you drop the bottle or empty it, `PLACE(WATER)` becomes `0` (the "limbo" location).

The liquid objects `OIL` and `WATER` do not contribute to the player's `HOLDNG` count!
You can `FILL BOTTLE` (or even `GET WATER`) even when `HOLDNG.GE.7`.

This means that Woods had to do some "clever" fixups in various places. For example,
[in the code for the `DROP` verb](https://github.com/Quuxplusone/Advent/blob/master/WOOD0350/advent.for#L1276-L1282):

    9021    K=LIQ(0)
            IF(K.EQ.OBJ)OBJ=BOTTLE
            IF(OBJ.EQ.BOTTLE.AND.K.NE.0)PLACE(K)=0
            IF(OBJ.EQ.CAGE.AND.PROP(BIRD).NE.0)CALL DROP(BIRD,LOC)
            IF(OBJ.EQ.BIRD)PROP(BIRD)=0
            CALL DROP(OBJ,LOC)
            GOTO 2012

Calling [`DROP(OBJ,LOC)`](https://github.com/Quuxplusone/Advent/blob/master/WOOD0350/advent.for#L2433-L2451)
decrements `HOLDNG`. Notice that if you `DROP CAGE` while the bird is in the cage,
we call `DROP` twice; but if you call `DROP BOTTLE` while the water is in the bottle,
we call `DROP` only once, and then use direct assignment to `PLACE(K)` to send the `WATER`
object back to limbo without decrementing `HOLDNG`.

This cleverness needs to be replicated in every place that puts liquids into your inventory
or takes them out again — `GET`, `DROP`, `THROW`, `FILL`, `POUR`, and even
[death](https://github.com/Quuxplusone/Advent/blob/master/WOOD0350/advent.for#L1180-L1195).

            IF(NUMDIE.EQ.MAXDIE.OR..NOT.YEA)GOTO 20000
            PLACE(WATER)=0
            PLACE(OIL)=0
            IF(TOTING(LAMP))PROP(LAMP)=0
            DO 98 J=1,100
            I=101-J
            IF(.NOT.TOTING(I))GOTO 98
            K=OLDLC2
            IF(I.EQ.LAMP)K=1
            CALL DROP(I,K)
    98      CONTINUE

The code for death first sends `WATER` and `OIL` to limbo; then turns off your lamp; then drops
every toted object in the place where you died, except for the lamp, which (if toted) gets dropped
at the starting location instead. It is very important that we set `PLACE(WATER)=0` _before_ looping
over your possessions — otherwise we'd drop `WATER` in the place where you died, which would violate
our invariant that the liquid `WATER` is only ever found in location `0` ("limbo") or location `-1`
("in hand"). Plus, if we ever called `DROP(I,K)` with `I.EQ.WATER`, we'd decrement `HOLDNG`, and
that breaks our invariant that liquids don't contribute to `HOLDNG`.

Where else do we `DROP` everything toted by the player?
[_When we teleport the player into the endgame repository!_](https://github.com/Quuxplusone/Advent/blob/master/WOOD0350/advent.for#L1917-L1945)

            LOC=115
            OLDLOC=115
            NEWLOC=115
            [...]
            PROP(MIRROR)=PUT(MIRROR,115,0)
            FIXED(MIRROR)=116

            DO 11010 I=1,100
            IDONDX=I
    11010   IF(TOTING(IDONDX))CALL DSTROY(IDONDX)

            CALL RSPEAK(132)
            CLOSED=.TRUE.
            GOTO 2

`DSTROY(OBJ)` is a synonym for `MOVE(OBJ,0)`, which is implemented in terms of `DROP`.
So, if when the flash of light happens the player is carrying a non-empty bottle — and therefore
`TOTING` some liquid — the liquid will be `DSTROY`ed instead of manually sent to limbo;
which means the player's `HOLDNG` count will be decremented one too many times; which means
that the player will enter the endgame `TOTING` zero objects but with `HOLDNG.EQ.-1`.

A player in this situation effectively has their inventory limit increased by 1!

And, coincidentally, there _just happen_ to be exactly eight totable objects in the endgame:
the bottle, oyster, lamp, cage, bird, pillow, and both black rods. So this bug's effects are
observable — just barely!

Here are screenshots I took of the original game emulated on Scott Healey's excellent
[gobberwarts.com](http://www.gobberwarts.com/index-350.html),
as seen previously in
["Colossal Cave Adventure: open world challenge"](/blog/2019/01/28/mcdo0551-sandbox-game/)
(2019-01-28). Notice the inventory lists in the lower right corner: in one case I'm able to
pick up the second rod as my eighth item, and in the other case I'm not.

|:-------------------------------------------------------:|:-------------------------------------------------------:|
| ![Failure](/blog/images/2020-02-06-get-rod-failure.png) | ![Success](/blog/images/2020-02-06-get-rod-success.png) |

I have fixed this bug in my own C port — as well as the bug Jonathan Ellis actually reported,
which was that my own port had stupidly had this same kind of bug any time you dropped a non-empty
bottle at all. (My bug not only could be exploited to increase your carrying capacity _ad infinitum_,
but caused very obvious misbehavior of the plover passage. Woods' endgame bug has no such dramatic
applications.)

I have also reported the bug "upstream" to Donald Knuth, whose
[brilliantly annotated CWEB version of _Adventure_](http://literateprogramming.com/adventure.pdf)
served as the starting point for my own port. Knuth's version faithfully replicated Woods' original bug.
