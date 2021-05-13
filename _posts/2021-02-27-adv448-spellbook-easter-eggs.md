---
layout: post
title: "Adventure 448 (SWEN0448): All the easter eggs"
date: 2021-02-27 00:01:00 +0000
tags:
  adventure
  digital-antiquaria
---

This week Jason Dyer played Adventure 448, in a couple of blog posts:

* ["Adventure 448 (1978–1979)"](https://bluerenga.blog/2021/02/21/adventure-448-1978-1982/)

* ["Adventure 448: The Shape of a God"](https://bluerenga.blog/2021/02/26/adventure-448-the-shape-of-a-god/)

I also played through it, just to make sure the max score of 448 was accurate.
(It is.) Whereas Jason seems to play unspoiled and enjoy the authentic sense of
discovery, I _very quickly_ go to the source code and get my sense of discovery
from the code. So, we have probably had interestingly different experiences of the game.

I was going to leave a long comment on Jason's second post above, but then I thought,
hey, I've got a blog too! I should just write a _really_ long comment over here!
So here's a brain dump on Adventure 448.

## Adventure 448's history (conjectured)

[The source code says:](https://github.com/PDP-10/its/blob/master/src/games/adv4ma.3#L22-L27)

    C  Modified for the Brown University system, April 1978
    C     Dave Wallace '78
    C     Dave Nebiker '79
    C     Eric Albert  '80
    C     LES  WU      '82 (ALSO MADE CONVERSIONS FOR A NBS-10)
    C  Modified for the ITS system, July 1979 by EJS@MC

Notice that since Les Wu was in the (four-year undergraduate) class of 1982,
he didn't arrive on campus until _fall_ of 1978. His name is also
in ALL CAPS, stylistically different from the other credits; so I infer that his
work came a bit later than the bulk of the expansion work — sometime in the ’78–’79
school year.

> Warning: Massive spoilers for both SWEN0448 and LONG0501 from here on.

The Brown University game (which I convinced Nathanael Culver to
[taxonomize as BROW0448](http://advent.jenandcal.familyds.org/#BROW0448))
included at least five new treasures — the ring, crown, sword, mithril mail,
and Brown scholarship. I conjecture that it also included the sixth and last
new treasure — the crystal figure — bringing its maximum score to 448 points.

> I conjecture thusly, because
> [the "maze" in which the crystal figure is found](https://github.com/PDP-10/its/blob/master/src/games/adv4db.2#L366-L374)
> is placed numerically _before_ the hexagonal room where you find the
> dagger to kill the magician, and the magician is pretty integral to
> the plotline that gets you the Brown scholarship. You find the broom in his
> tower, and [the broom is in the noun table](https://github.com/PDP-10/its/blob/master/src/games/adv4db.2#L1227-L1233)
> _before_ the dagger, throne, and staff.

Okay, that's BROW0448. Then, in July 1979, the game made its way to MIT.
`EJS@MC` is Eric J. Swenson, on the [MIT-MC](https://news.ycombinator.com/item?id=13518273)
machine (a KL10 running ITS — and yes I am parroting these words with little real
idea what they mean; this was all very much before my time). Swenson did whatever porting
work was necessary for ITS, and also changed the description of the "scholarship"
treasure from a _Brown_ scholarship into an _MIT_ scholarship.

> How do I know this? [He forgot to update the noun table.](https://github.com/PDP-10/its/blob/master/src/games/adv4db.2#L1267-L1269)

I conjecture that this minor modification was Swenson's only "feature work"
on the game itself  (which I convinced Nathanael Culver to
[taxonomize as SWEN0448](http://advent.jenandcal.familyds.org/#SWEN0448)).
The new description of the scholarship is amusing—

    You are in a large room full of dusty rocks.  There is a big hole in
    the floor.  There are cracks everywhere, and a passage leading east.

    > SWEEP

    Your sweeping stirs up the dust and reveals a piece of paper on the
    ground.

    > READ PAPER

    "Congratulations.  Due to your extraordinary abilities as an
    adventurer, you have won a full four-year scholarship to the
    College of your choice -- limited to Cambridge Massachusetts.  Sorry,
    but Harvard excluded.  Void where prohibited by law."

Notice that this "broom to sweep up the dust in the dusty
rock room" trope also occurs in LONG0501, although with a different outcome.
One of these days I'll do a post where I Venn-diagram all the different
tropes that recur between different versions — the throne room, the sword-in-stone
puzzle, the genie in the lamp (or otherwise), and so on.


## Speak friend and enter

Now the reason I started writing this post!
Jason Dyer [writes:](https://bluerenga.blog/2021/02/26/adventure-448-the-shape-of-a-god/)

> The source code has the text "Speak, friend, to enter"
> but I wasn’t able to get it to appear; READ DOOR or READ RUNES don’t work.

The secret is to use the Magician's Book of Spells, which you find in the tower above.
Admittedly it is _not obvious_ that the book is even portable — the map isn't! —
and then even if you do take the book, `READ BOOK` gives a very discouraging response:

    > READ BOOK

    The book seems to be a big book of fairy tales.  This particular tale
    concerns an adventurer wandering around in a cave.

And likewise, the elven door isn't very inviting:

    You're in the Magician's Chamber, a large pentagonal room with a
    spiral staircase leading up from the center of the room.

    There is a massive stone Elven door set into the western wall.

    > READ DOOR
    I SEE NO DOOR HERE.

(This unnaturally mechanical response is because there _is_ a noun called DOOR —
the iron door north of the Giant Room — but it's not here with us right now.)

    > READ RUNES

    What?

    > READ ELVEN DOOR

    I'm afraid I don't understand.

Aha! This isn't the randomized "I don't know what you mean" response; this is
the _I don't think that object is readable_ response. Now that we know _what_
to read, we can think about _how_ to read it. Let's consult our arcane tome...

    > READ SPELLBOOK

    OK.
    > READ ELVEN DOOR

    Speak, Friend, and enter

    > SAY FRIEND

    You're in a long straight corridor ending in a massive slab of stone
    to the east. [...]

That's right: you need to read the spellbook _at this particular location_
in order to get the proper effect. Ugh!


## The other spellbook effects

I did some source-diving to learn
[these effects](https://github.com/PDP-10/its/blob/master/src/games/adv4ma.3#L2185-L2207),
and then reproduced them all in-game.

Reading the book while in the presence of a living dragon does this:

    You are in a secret canyon which exits to the north and east.

    A huge green fierce dragon bars the way!

    The dragon is sprawled out on a Persian rug!!

    > READ SPELLBOOK

    The earth rumbles and opens up beneath the dragon.  Both the dragon and
    the rug vanish down the hole which closes behind them.

    You are in a secret canyon which exits to the north and east.

Yes, this loses the rug.

Reading the book while in the Giant Room accomplishes the same
thing as FEE FIE FOE FOO. However, oddly enough, there is a second change
to the FEE FIE FOE FOO condition itself:
[the spell doesn't work if you are carrying the magician's staff!](https://github.com/PDP-10/its/blob/master/src/games/adv4ma.3#L2136)
So if you want to get the eggs back via this odd route, you need to
bring the book all the way to the Giant Room but _not_ still be carrying
the staff when you read it.

Reading the book in the Druid Temple Room is the last and oddest
easter egg of them all:

    You are in a large chamber decorated like an ancient Druid temple
    continuing to the west with passages leading off to the north and south.
    A large stone dominates the center of the room.

    The sword is firmly imbedded in the stone!

    > READ SPELLBOOK

    You hear a rumble from deep within the earth and the room shakes a bit.

That's it. Just that message. No other in-game effect.


## The exits from the magician's tower

Jason also writes:

> If you go up one floor, going east kills you [...]
> going one room up again — with an identical staircase room — and trying to go east leads to death
> [...] the preponderance of confusion and deathtraps feels slightly off from 350-point Adventure.

I agree that the likely observed effect on the player is "going east from room 151
(the room just below the magician's tower room) means death."
But the source code is fiddlier than that — unfortunately, since most actual players
would never have seen these effects!

See, first of all, SWEN0448 (and I assume BROW0448) has ten dwarves, not just the
original five. (The eleventh dwarf is special; that's the pirate. But it's
[still the sixth dwarf](https://github.com/PDP-10/its/blob/master/src/games/adv4ma.3#L895)
who generates the faint rustling noises from the darkness behind you.)
In fact there's a twelfth dwarf, too, who seems to start out dead but comes to life
if the game thinks you're "cheating" by messing with the savefile or something —
I didn't really understand [that codepath](https://github.com/PDP-10/its/blob/master/src/games/adv4ma.3#L927-L939).
But anyway, the first five dwarves start wandering on turn 1. Dwarves six through ten
remain dead until either the game thinks you're cheating _or_ you try to go through
the eastern door in that one staircase room.
 [The code says:](https://github.com/PDP-10/its/blob/master/src/games/adv4ma.3#L1368-L1383)
"IF THE NEW DWARVES HAVE BEEN RELEASED AND ARE NOT ALL DEAD, HE GETS THROUGH.
OTHERWISE HE DIES."

The overall effect is that the _first_ time you go through this door it kills you and
releases five new dwarves into... the crack above the Hall of Mists? I think the
intent is that you should then be able to go through the door safely (in your
next life), but in reality it doesn't seem to work that way for some reason.

Oh, and if the cave is closing, then all the dwarves (and pirate and dragon and troll)
are gone home for the day, so in that case you get through the door no problem.

----

Meanwhile, going east from room 148 (the room just above where you enter the staircase)
is _crazy_ fiddly! Look at
[this travel table](https://github.com/PDP-10/its/blob/master/src/games/adv4db.2#L988-L996):

    148 148171     43
    148 955170     43
    148 956170     43
    148 167143     43
    148 157114     43
    148 168157     43
    148  60019     43
    148  30149     43
    148    151     43

This means: When you go EAST (verb 43) from room 148,...

- If TOTING(STAFF), go to 171, which takes you to 150, the magician's tower.
    This is the same place you'd get if you just walked up the stairs far enough!

        > EAST

        As you enter the ethereal plane, you seem to be twisted and pulled
        at and lose all sense of orientation.

        You're in the magician's tower, a small cluttered room filled with
        all manner of strange artifacts, the purpose of which cannot be
        kenned immediately.

- Otherwise, if WEARING(CROWN) or WEARING(RING), go to 170 and death.
    This is what happened to Jason.

        You enter a thick, cold, gray mist.  You seem to be falling forever
        and the light slowly fades away.

        Oh dear, you seem to have gotten yourself killed.  I might be able to
        help you out, but I've never really done this before.  Do you want me
        to try to reincarnate you?

- Otherwise, if TOTING(SWORD), go to 143, the Druid Temple Room (which
    is where you got the sword in the first place).
    This is almost certain loss, because the only way out of there is
    to use the ring, and we know you're not wearing it. (But you might be
    just carrying it, I guess.)

- Otherwise, if TOTING(CHEST), go to 114, the pirate's lair (which is
    where you got the chest in the first place).

- Otherwise, if TOTING(MITHRIL), go to 157, the Dwarves' Great Hall
    (which is roughly where you got the mithril).

- Otherwise, if none of those cases hold, go to the Hall of the Mountain King (60%)
    or room 149 (another staircase room — 30%) or 151 (the room where going east
    runs into all those dwarves).

By the way: If TOTING(STAFF), then using any of the three "travel"
magic words (XYZZY in either direction, PLUGH in either direction, or PLOVER _from_ Y2 only)
will pull you to the ethereal plane and the tower instead of having its usual
effect.
