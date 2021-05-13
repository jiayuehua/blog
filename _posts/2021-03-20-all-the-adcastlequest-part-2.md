---
layout: post
title: "Playing _Castlequest_ (1980), Part 2"
date: 2021-03-20 00:01:00 +0000
tags:
  castlequest
  digital-antiquaria
---

Previously on this blog: ["Playing _Castlequest_ (1980), Part 1"](/blog/2021/03/19/all-the-adcastlequest-part-1/)
(2021-03-19). So far we've found eight treasures —
silver cross, Cuban cigar, gold statue, champagne,
jade figure, sapphire, crystal swan, large ruby —
and our main (only?) unsolved puzzle was where to use this
rope tied to a grappling hook.

But wait! Only at this point do I notice that the Cuban cigar's
description doesn't feature an exclamation point, which means
it's not a treasure. So we're down to _seven_ actual treasures;
plus one cigar, plus one grappling hook in search of a cliff.

The precipice in question is just south of the room where we found
the sapphire. Throw the hook, then `UP` the cliff we go. Atop the cliff
we find another bizarre grab-bag of rooms:

    This is the disco room.  Multicolored lasers pulsate
    wildly to the beat of badly mixed music.  A stairway
    down is barely visible through the glare.  A large
    passage exits south, and a smaller one leads west.
    > S
    You have entered the land of the living dead, a
    large, desolate room.  Although it is apparently
    uninhabited, you can hear the awful sounds of
    thousands of lost souls weeping and moaning.
    In the east corner are stacked the remains of
    dozens of previous adventurers who were less
    fortunate than yourself.  To the north is a
    foreboding passage.  A path goes west.
    > W
    You are in a tall tunnel leading east and west.  A small
    trail goes SE.  An immense wooden door heads south.
    There is a fairly large cyclops staring at you.
    > W
    You are in a tremendous cavern divided by a white line
    through its center.  The north side of the cavern is
    green and fresh, a startling change from the callous
    terrain of the cave.  A sign at the border proclaims
    this to be the edge of the wizard's realm.

Well, there's the cyclops and wizard I knew had to be around here somewhere!
Being somewhat spoiled on the solution to the cyclops puzzle, I gave
the game a sporting chance to hint me toward the intended action:
fruitlessly.

    > WAVE AXE
    Nothing happens.
    > BREAK DOOR
    You are not strong enough to break it.
    > TIE CYCLOPS
    That would be a neat trick.
    > FEED CYCLOPS
    Boy are you dumb!  A cyclops doesn't eat food.
    > KILL CYCLOPS
    The cyclops hurls you against the wall and chuckles quietly.
    > THROW AXE
    The cyclops flings you across the room and laughs hysterically.
    > SHOOT CYCLOPS
    The cyclops does not even feel the impact of the bullet.
    > ULYSSES
    I don't think I understand.
    > HELP
    To aid you in your travels, you may ask for a hint by
    saying "HINT object", where "object" is the item that you
    need help with (e.g. "HELP CROSS").  Saying "HELP ROOM"
    will give you some help concerning the room you're in.
    > HINT CYCLOPS
    It will cost you five points.
    Do you still want the hint?.
    > YES
    Try "smoking" him out.

Fine. I have no idea how I was supposed to get here without the hint,
but I'll play along.

    > THROW CIGAR
    The cyclops turns to you and says:
       "Hey buddy!.  Got a light??"
    > LIGHT CIGAR
    You have nothing to light it with.
    > LIGHT MATCH
    The light is burning dimly.
    > LIGHT CIGAR
    The cyclops chokes from the rancid tobacco, and
    crashes through the door in search of water.
    There is a cyclops-shaped hole in the door.

On the other side of the door we find an ivory-handled sword, which
begins glowing dimly as we approach the wizard's realm. Holding the sword,
we're able to pass through some sort of magical force field and
confront the wizard. `STAB WIZARD` results in an ignominious death.
Restore and `THROW SWORD`: ignominy. On the other hand, the wizard
doesn't react to `THROW AXE` and `KILL WIZARD`. Even

    > SHOOT WIZARD
    Nothing happens.

So the sword is the operative element here.

    > WAVE SWORD
    The walls of the cavern tremble as you unleash the
    terrible power contained in the sword.
    The wizard, sensing a stronger power than his own,
    flees in a blinding flash and a cloud of smoke.
    > W
    You are in the wizard's cache, a large room whose walls
    are inlaid with jewels.  A majestic marble walk leads to
    the east.
    There is lots of money here!

That makes nine treasures and 255 points, and now it _really_ seems
like we've seen all the puzzles. Unless we need to keep trying to
get to the village?

_Castlequest_'s hint system is pretty neat. It doesn't seem to notice
when you're "stuck" in a particular area for too long, the way _Adventure_
does; but I like the context-dependent notes the butler gives you
(you get one of these per game), and I like the `HINT [item]` mechanic.
The problem is that at this point I don't know which item or room
to get a hint about! `HINT ROOM` in the vault produces the message
"Sorry, not available." So, I went and looked at the source code.


## Cheating our way to victory

Looking at `object.dat` showed me that _again_ I had misconstrued
the treasure list. The Cuban cigar isn't a treasure, but the skeleton key
_is_ one! So we need to drop the key in the vault as well. That rounds
out our score to 264 points (the key is worth only 9 points, unlike
all the other treasures), and then when we go back up in the elevator:

    > U
    The elevator has screeched to a halt between two floors.
    Your axe is trembling slightly.
    > WAVE AXE
    Nothing happens.
    > THROW AXE
    You can't be serious.
    > JUMP
    Jump from where??
    > OUT
    There is no way to go in that direction.
    > OPEN DOOR
    I see no door here.
    > HINT AXE
    Sorry, not available.
    > HINT ROOM
    Sorry, not available.

Well, I guess I shouldn't expect `HINT` to work in what is clearly the
"master game"...

I kick myself when I decipher the source code and find that what I should
do with the axe, when trapped in an elevator, is obviously `CHOP DOOR`.

    > CHOP DOOR
    There is a passable hole in the door.
    > OUT
    You are in a room of mammoth proportions which seems
    to be some sort of warehouse.  On a nearby table are
    several clipboards and a massive pile of order forms.
    To your right is a large loading dock and a truck bay.
    The room opens to the north, east and west.
    Your axe is trembling slightly.

Okay, _now_...

    > WAVE AXE
    The letter "H" appears for an instant on the wall.

Waving the axe in each of the four corners of this warehouse reveals
the letters of the magic word, coincidentally one of the few words
I haven't used from the game's vocabulary list.

    > WAVE AXE
    A large "K" emerges from the floor.
    > HONK
    The floor erupts violently, swallowing you in a sea of molten lava.
    You scored  256 out of  300 points.
    You are a MASTER at CASTLEQUEST.

Wait, wait, let's try that again!

    You are in the elevator, stuck between floors.
    There is a passable hole in the door.
    Your axe is trembling slightly.
    > HONK
    You feel the elevator jump as you are wisked up towards
    ground level.  You emerge in the open air in the village
    square amidst cheers from the local villagers.  Banners
    proclaiming the death of count Vladimir hang from most
    of the old buildings around the square.  The mayor
    presents you with a key to the city and makes your
    birthday a holiday.  You watch the sun rise as you
    bask in your newfound fame.
    You scored  279 out of  300 points.
    You are a MASTER at CASTLEQUEST.

It turns out that the game rewards you for a few inessential tasks:
you get 5 points for reading the butler's note, 15 points for viewing
the combination behind the kitchen door, and 10 points for waving the
axe all around the warehouse at the end. (You also get bonus points
for some essential tasks, such as getting the bedroom window open,
opening the combination lock, and killing the Count.)

Finally, _Castlequest_ has a Last Lousy Point. I don't think it's
clued at all — but you get 1 point for leaving the empty bottle
on the island. (Thematic, right? Shipwreck, message in a bottle... sure.)

Anyway, when you do all of these things in the right order,
within 250 moves, you get the top score:

    You scored  300 out of  300 points.
    This qualifies you as a "CLASS A" MASTER!

Note that (just as in _Adventure_) some commands don't count as "moves,"
so even my [298-line walkthru](/blog/code/2021-03-20-castlequest-walkthru.txt)
counts as only about 210 moves.

Tomorrow: some observations on the source code.

* ["Observations on _Castlequest_'s code"](/blog/2021/03/21/castlequest-vs-adventure/) (2021-03-21)
