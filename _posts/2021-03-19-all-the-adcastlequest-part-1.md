---
layout: post
title: "Playing _Castlequest_ (1980), Part 1"
date: 2021-03-19 00:01:00 +0000
tags:
  adventure
  castlequest
  digital-antiquaria
excerpt: |
  Earlier this week I posted that the source code for _Castlequest_ (Holtzman and Kershenblatt, 1980)
  had been found — ["_Castlequest_ exhumed!"](/blog/2021/03/09/castlequest/) (2021-03-09).
  By now it's totally playable (at least if you have the ability to install GNU Fortran).

  I had been seeking _Castlequest_ because I had fond memories of the game on
  [GEnie](https://en.wikipedia.org/wiki/GEnie). I vaguely remembered the wandering werewolf,
  and the vampire in the attic that you have to kill with a tomato stake before the sun sets...
  But it turns out that the game is actually much _much_ longer than I ever knew as a kid!
  I'm going to try the ["All the Adventures"](https://bluerenga.blog/all-the-adventures/) thing here,
  and describe my experience replaying the game this week.

  <b>This post contains major spoilers for all plot points and puzzles!</b>
---

Earlier this week I posted that the source code for _Castlequest_ (Holtzman and Kershenblatt, 1980)
had been found — ["_Castlequest_ exhumed!"](/blog/2021/03/09/castlequest/) (2021-03-09).
By now it's totally playable (at least if you have the ability to install GNU Fortran).

I had been seeking _Castlequest_ because I had fond memories of the game on
[GEnie](https://en.wikipedia.org/wiki/GEnie). I vaguely remembered the wandering werewolf,
and the vampire in the attic that you have to kill with a tomato stake before the sun sets...
But it turns out that the game is actually much _much_ longer than I ever knew as a kid!
I'm going to try the ["All the Adventures"](https://bluerenga.blog/all-the-adventures/) thing here,
and describe my experience replaying the game this week.

<b>This post contains major spoilers for all plot points and puzzles!</b>

> For softer spoilers, you might check out
> [this thread on IntFiction.org](https://intfiction.org/t/castlequest-1980-text-adventure-recovered/49860)
> where some people are playing through the game, asking and giving hints as they go.

Immediately prior to playing, I'd just spent three days transcribing the source code,
so I'd seen literally every line of the game — and yet I was still relatively unspoiled.
Where Crowther and Woods' _Adventure_ uses a lot of comments and mnemonic variable names,
_Castlequest_ tends to use opaque numbers. Here's _Adventure_:

    C  DISCARD OBJECT.  "THROW" ALSO COMES HERE FOR MOST OBJECTS.  SPECIAL CASES FOR
    C  BIRD (MIGHT ATTACK SNAKE OR DRAGON) AND CAGE (MIGHT CONTAIN BIRD) AND VASE.
    C  DROP COINS AT VENDING MACHINE FOR EXTRA BATTERIES.
     9020   IF(TOTING(ROD2).AND.OBJ.EQ.ROD.AND..NOT.TOTING(ROD))OBJ=ROD2
            IF(.NOT.TOTING(OBJ))GOTO 2011
            IF(OBJ.NE.BIRD.OR..NOT.HERE(SNAKE))GOTO 9024
            CALL RSPEAK(30)
            IF(CLOSED)GOTO 19000
            CALL DSTROY(SNAKE)
    C  SET PROP FOR USE BY TRAVEL OPTIONS
            PROP(SNAKE)=1

And here's _Castlequest_:

    C     ---THROW---
          IF (ROOM .EQ. 47 .AND. OBJECT .EQ. 26) GOTO 391
          IF (OBJECT .EQ. 16) GOTO 241
          [...]
          IF (OBJECT .NE. 25) GOTO 240
               IF (ITEMS(25) .NE. -1) GOTO 720
               NUMB = NUMB - 1
               IF (ROOM  .NE. 1) GOTO 232
               IF (SHUTTR .EQ. 0) GOTO 238
                   WIND1 = 3
                   WRITE(6,1061)
                    ITEMS(25) = 0
                   GOTO 25

Both games use a smattering of named global variables: _Adventure_ has `CLOSED`,
_Castlequest_ has `SHUTTR` and `WIND1`. But _Adventure_ also uses mnemonic names for
its items (`BIRD`, `ROD`) and named predicates such as `HERE(SNAKE)` in lieu of most room numbers,
whereas _Castlequest_ uses simple magic numbers (`25` for the flask of acid,
`1` for the bedroom). Finally, _Adventure_ uses named helper functions such as `TOTING(ROD)`
and `DSTROY(SNAKE)`, where _Castlequest_ open-codes `ITEMS(25) .NE. -1` (`-1` being
the magic "in my inventory" room number) and `ITEMS(25) = 0` (`0` being the
magic "nowhere/destroyed" room number).

So I was spoiled really only in that I'd seen all the messages (so I knew to throw acid
on the bars, and I knew to melt the glacier and give something smokable to the cyclops
and tie the rope to the bed), and in that I had seen all the verbs and nouns in the game,
like, once. As for how all the rooms would connect together, I was totally in the dark.


## The early game (a.k.a. what I knew from GEnie)

    Welcome to CASTLEQUEST!! Would you like instructions?
    > YES
    You are in a remote castle somewhere in Eastern Europe.
    I will be your eyes and hands.  Direct me with words such
    as "LOOK", "TAKE", or "DROP".  To move, enter compass points
    (N,NE,E,SE,S,SW,W,NW), UP, or DOWN.  To get a list of what
    you are carrying, say "INVENTORY".  To save the current game
    so it can be finished later say "SAVE".  Say "RESTORE" as
    your first command to finish a game that had been saved.
    The object of the game is to find the master of the castle
    and kill him, while accumulating as many treasures as possible.
    You get maximum points for depositing the treasures in the
    vault.  Notice that the descriptions of treasures have an
    exclamation point.  Be wary, as many dangers await you in
    in the castle.
    Would you like more detailed instructions?
    > NO

It's mildly noteworthy that the instructions are split onto three pages.
The second line of the instructions is lifted straight from _Adventure_:
"I will be your eyes and hands. Direct me with commands of 1 or 2 words."
Holtzman was a fan of _Adventure_, but had never seen its source code;
therefore (as we've seen) the code of _Castlequest_ looks pretty different.
(I'll do a post on the code sometime.)

You awake in aforementioned remote castle:

    You are in a large, tarnished brass bed in an old, musty bedroom.
    cobwebs hang from the ceiling.  A few rays of light filter through
    the shutters.  There is a nightstand nearby with a single wooden
    drawer.  The door west creaks in the breeze.  A macabre portrait
    hangs to the left of an empty fireplace.
    The shutters are closed.
    There is a silver bullet here.
    > OUT
    You are in a dim corridor lit by gaslight.  Doors exit
    to the east and west.  A stairway leads down.
    > D
    You are in the parlor, an old fashioned sitting room.  A display
    case of dueling pistols hangs over the mantle. Stairs lead up to
    a dimly lit corridor.  Open double doors lead west.  Two wide
    hallways lead north and south.
    There is some "HORROR HOTEL" writing paper here.
    There is an old gun here.

The game doesn't recognize `GET` as a verb, so you have to `TAKE`
everything. `T` is recognized as a shorthand. We can `LOAD` the gun with the
bullet in order to `SHOOT` any wandering werewolves that pester us.

The exits of the castle's rooms are all clearly and un-trickily marked; we
quickly map the bedroom, hallway, parlor, armory, dining room, kitchen, pantry,
foyer, workshop, smoking room, laundry room, storage room, upstairs hallway,
library, boudoir, L-shaped corridor, and mirror maze. Yes, this castle
(or Victorian mansion or whatever it is) comes with a mirror maze! But the maze
is just one room, and wandering randomly always seems to lead you out into
a random room of the castle — never anywhere new.

In the process, you find a silver cross and a Cuban cigar; the gun-with-silver-bullet,
a bloody axe, some tasty food, an empty bottle (did I mention Holtzman was a fan of _Adventure_?),
some "reusable matches" and kerosene, writing paper and a quill pen,
and a book in the library that you can't seem to do much with.

    This is the library.  All four walls are lined with bookcases.
    The room is brightly lit, although there is no apparent source
    of light.
    A copy of Shakespeare's "HAMLET" lies on the desk.
    > TAKE BOOK
    Sorry, but you don't have your library card.
    > READ BOOK
    A literary classic, but we don't have time to read.

However, there must be _something_ special about this book.

    You are in the foyer.  An umbrella near the door is dripping on the
    thick pile carpet.  A black cape is draped neatly over the banister
    of a grand staircase leading up.  A magnificentl archway leads north.
    Corridors lead south and southeast, a small hallway heads west,
    and a narrow stairway goes down.
    The butler is sound asleep.
    > WAKE BUTLER
    The butler is motioning that he wants to write you a note.
    > THROW PEN
    OK
    > THROW PAPER
    The butler is holding out a note.
    > READ NOTE
    "The master loves Shakespeare".

The trick is that you have to type `OPEN BOOK`. If you think to do that:

    A skeleton key falls out of the book.
    > TAKE KEY
    OK

Oh, also, there are six rooms outside the mansion, including a garden where
you find a tomato stake handily lying about, and a moat with a rowboat, which
you can cross by doing [`TAKE BOAT`](https://qwantz.com/index.php?comic=778)
and then `CROSS MOAT`. But on the other side all you find is a sign that says
"You can't reach the village from here"; and it seems to be telling the truth.

Re-entering the house gives a funny pastiche on _Adventure_'s plover passage:

    You are in the garden.
    > E
    Something you're carrying won't fit into the house.
    > DROP BOAT
    OK
    > E
    You're in foyer.

I am fairly confident that I, as a child, never said `OPEN BOOK` and therefore
never found the skeleton key at all, which means the only other room in the game
I ever saw is the attic entrance:

    You are in a musty room that appears to be an entrance to
    an attic.  A small passage leads north and stairs descend
    down behind you.
    > N
    A huge vampire bat hangs from the doorframe and blocks your way.
    > KILL BAT
    With what?? Your bare hands??
    > YES
    Attack the bat??  That's gross.  I won't do it.
    > THROW AXE
    The blade does not penetrate the bat's thick hide.
    > SHOOT BAT
    The bullet does not penetrate the bat's thick hide.

If you don't figure out the skeleton key, _this is all of the game you will ever see._
And that's exactly what happened to me as a kid! I wonder if this game would have
been so memorable if I hadn't felt so stumped. (The other game that fascinated me
in that way was David Malmberg's [_World's Hardest Adventure_](https://archive.org/details/msdos_Worlds_Hardest_Adventure_The_1993),
which I later disassembled to find that as a kid I'd seen the whole thing and it
was just literally impossible to win.)

But notice: we have a _stake_ and we have a _vampire bat_, and we know that
"the object of the game is to find the master of the castle and kill him."
Even though I never managed to solve the puzzle of the bat, I assumed I knew how the game would play out.

Boy, was I wrong.


## Killing Count Vlad

Now that we have the skeleton key from the book, we can open a boarded-up
door in the kitchen:

    You are now in the kitchen.  Twelve Swanson's frozen entrees rest
    on the counter, below a microwave oven.  "THE BEGINNER'S GUIDE TO
    COOKING" lies on a small table.  A swinging door exits south.
    Other doors lead east and north.
    > E
    The door is boarded up.
    > OPEN DOOR
    The door is boarded up.
    > UNLOCK DOOR
    OK
    > E
    The door is closed.
    > OPEN DOOR
    The door opens to a brick wall.  ---DEAD END---
    A note on the wall reads "L 8 R 31 L 59".

(Why does our skeleton key work on a boarded-up door? And why _doesn't_
chopping with the axe work? I don't know.
More on _Castlequest_'s handling of doors and windows in that code post,
whenever I get around to writing it.)

Now let's go back toward the bedroom:

    You're in the parlor.
    > U
    You have fallen through a trap door and find...
    You are in a dark stone E/W passage.

This secret passage for some reason triggers only when you are carrying the key.
Down here are another ten rooms, including a torture chamber occupied
by a nasty hunchback who seems to work just like the bear in _Adventure_ —
you can `THROW FOOD` and then `TAKE HUNCHBACK` to have him follow you around.
(He doesn't seem to do anything except ward off one werewolf attack and
then die.) Item-wise, we find a flask of acid, an acetylene torch, a rope,
and "a small pool of blood." We can `FILL BOTTLE` from the pool —
did I mention Holtzman was a fan of _Adventure_? — and then:

    A huge vampire bat hangs from the doorframe and blocks your way.
    > FEED BAT
    The bat gulps down the blood and flitters away.
    > N
    You are in an old attic filled with old-fashioned clothes, a pile
    of newspapers and some antiques.  An entrance to a cedar closet
    is to the east and there is a door to a crawlspace to the west.
    > W
    A combination lock bars the door.

We use the combination from the kitchen. (Mark Kershenblatt tells me that 8-31-59
is Mike Holtzman's birthday. Compare David Long's _New Adventure_,
in which a safe is opened with the combination 7-22-34 — the date John Dillinger was shot.)

    The lock is now open.
    > W
    You are crawling along a low passage that leads east and west.
    > W
    It is now pitch dark. If you procede you may stumble and fall.
    > E
    You fall in the dark and break your neck.
    My, My.  You seem to have bitten the dust.
    I can attempt to reincarnate you, but I'm
    not very good at it. Should I try?

_Castlequest_ doesn't play around with random numbers here. If you
walk in the dark, you're dead! And when you die, your inventory is
scattered randomly about the house — which is so annoying to deal with
that I can't imagine anyone not just restarting (or restoring to
their last save) on death.

Restart or restore, get the brass lamp from the cedar closet, and try again:

    > W
    You are in the chamber of the master of the castle,
    Count Vladimir!  Pictures depicting scenes of tranquil
    Transylvanian countrysides line the walls.  A huge
    portrait of Vladimir's brother, Count Dracula, hangs
    upon the near wall.  In the center of the room is a
    large, ominous, mahogany coffin.
    The coffin is closed.
    > OPEN COFFIN
    The Count is asleep in the coffin.
    > WAKE COUNT
    The Count sits up and prepares for breakfast-namely you!
    > STAB COUNT
    The vampire clutches at the stake and dies,
    leaving only a pile of dust.
    A note materializes on the wall which reads:
    EMERGENCY EXIT--The mirror maze will lead you
    to the locked door.  The exit lies within.

Remember the dim corridor outside our bedroom? There was a door to the west there,
but it was locked. Going upstairs from the parlor takes us back to the dim corridor,
_unless_ we're carrying the key, in which case it dumps us in the secret passage.
So we have been unable to try the key on that locked door.

But now, exiting the mirror maze takes us somewhere different!

    You are wandering around the mirror maze.
    > W
    You are in the dim corridor.
    > UNLOCK DOOR
    OK
    > OPEN DOOR
    OK
    > W
    A cool wind blows up a stone stairway which descends
    down into a large stone room.  A note written in blood
    reads "VERY CLEVER OF YOU TO MAKE IT THIS FAR".
    The door leads east, back to the hall.

Well, hang on. Wasn't the object of the game to find the master of the castle
and kill him? Didn't we do that a while ago? Did the game's instructions _lie_
to us?


## The Great Underground E(verything-but-the-va)mpire

This is a good place to exercise the game's `SAVE` command, by the way.

    You are in a perfectly square room carved out of solid
    rock.  Stone steps lead up.  An arched passage exits
    south.  Above the arch is carved the message:
       "ABANDON HOPE ALL YE WHO ENTER HERE".
    > S
    You are in a long sloping N/S passage.  The darkness
    seems to thicken around you as you walk.
    > S
    You are in a narrow room which extends out of sight
    to the east.  Sloping paths exit north and south.
    It is getting warmer here.
    > S
    This is the fire room.  The stone walls are gutted
    from centuries of evil fires.  It is very hot here.
    A low trail leads west and a smaller one leads NE.
    A sloping trail goes north.
    A wall of fire bars the way to the NE.
    > W
    You are in the blue room.  The entire room is a deep
    shade of royal blue.  Exits go north, south and east.
    > S
    You are in a narrow E/W passage.  A faint noise of
    rushing water can be heard.  A small crawl goes south.
    > S
    You are in a long bending tunnel which leads north
    and east.  The walls are damp here, and you can
    distinctly hear the sound of rushing water.
    > E
    You are at the base of a magnificent underground waterfall.
    A cool mist rising off the surface of the water almost obscures
    a small island.  A tunnel goes west and stone steps lead up.

Climbing the waterfall leads to a dead end whose scenery rivals _Adventure_'s Volcano View.
Meanwhile, throwing water on the wall of fire lets us pass to find a... hotel lobby?
With... elevators? In the basement we find what might be our new objective:

    This is the safe deposit vault, an immense room with polished
    steel walls.  A closed circuit T.V. camera hums quietly above
    you as it pans back and forth across the room.  To the east is
    an open elevator.  Engraved on the far wall is the message:
            "DEPOSIT TREASURES HERE FOR FULL CREDIT"

So far we've found a silver cross, a Cuban cigar, and a gold statue in the house,
a jade figure and a bottle of champagne in the cave.
North of the lobby is a sort of Y2 room that lets us teleport back
to the boudoir by saying `POOF`.
Next to the Blue Room is a maze of winding little passages.

This maze is a cute (if diabolical) twist on Woods' "maze of twisty little passages,
all different." The rooms' interconnections are pretty much nonsensical,
but each room's description is slightly different.
Also, it can be mapped the same way as Crowther's "all alike" maze:
by dropping items.

    You're in a long and winding maze of passages.
    > DROP BOTTLE
    OK
    > N
    You're in a winding maze of long passages.
    > DROP MATCHES
    OK
    > NW
    You're in a maze of short and winding passages.
    An empty bottle is discarded nearby.

...wait, what?

    > LOOK
    You're in a long and winding maze of passages.
    An empty bottle is discarded nearby.

That's right: the _long_ description of the room (printed the
first time you enter it) is significantly different from the _short_
description (printed on subsequent visits). There are seven rooms
in this maze, and seven different room descriptions, but the
descriptions are unnaturally shuffled and repeated:
"You're in a maze of short and winding passages"
is the short description of room 57, and the long description
of both room 62 and room 63. The "necessary" core of the maze
looks like this:

![Map of the maze](/blog/images/2021-03-19-maze-map.jpg)

One exit from the maze is "four tunnel junction," near which
we find a sparkling sapphire. The other exit is the glacier room:

    This is the glacier room.  The walls are covered with
    dazzling shapes of ice which reflect the light from your
    lamp in a million colors.  In the far side of the room
    are magnificent ice sculptures of animals unknown to
    Mankind.  A faint "X" is scratched in the ice on one wall.
    Icy passages exit SW and east.  A steep trail goes up.

We can use our acetylene torch to melt a hole in the glacier:

    > LIGHT MATCH
    The light is burning dimly.
    > LIGHT TORCH
    The torch is burning noisily.
    > MELT ICE
    Some ice has melted, leaving a large hole.
    You are standing in a small puddle of water.
    > IN

Don't `MELT ICE` a second time or you'll flood the cave!
Also, it is probably important to `EXTINGUISH` both the match
and the torch afterward; these matches are somehow "reusable,"
but they do eventually exhaust themselves after burning for a
long time. (So you can't explore the whole cave just on matches;
you need the lamp too. If the lamp starts getting dim,
you can refill it with kerosene from the storage room.)
Anyway, inside the glacier we find our seventh treasure: a delicate crystal swan.

Up the steep trail from the glacier room (I notice this cave is
full of "trails" and "paths," as opposed to Crowther's "crawls"
and "canyons") we find the underside of the tomato garden,
and eventually (if you remembered to bring the key) we
come out again above ground:

    You are in the remains of an old wine cellar, apparantly the
    victim of a cave in.  Casks of once fine wine lie crushed in
    the rubble.  A battered keg of GENESEE sits off in the corner.
    The room smells like a Rathskellar band party.  A muddy path
    goes east, and steps lead up to a door in the ceiling.
    > UNLOCK DOOR
    OK
    > OPEN DOOR
    OK
    > UP
    You are on the far side of the moat.  You can see
    a full view of the castle here in all its deadly
    splendor.  A small town can be glimpsed far off in
    the distance.  An old sign nailed to a tree reads:
         "YOU CAN'T REACH THE VILLAGE FROM HERE!"
    There is a large opening in the ground.

Go back through the mansion and cross the moat again and take
the boat down into the cave through the cellar! With the boat,
you can `CROSS` under the waterfall to find an island with a
large ruby.

(The rowboat won't `POOF` upstairs — just like the emerald in _Adventure_ —
but you can carry it up the stairs just fine.)


## What's next?

At this point we've found eight treasures —
silver cross, Cuban cigar, gold statue, champagne,
jade figure, sapphire, crystal swan, large ruby —
for a total of 215 points. I know from transcribing the messages
that there's more to find:

- We have a rope tied to a grappling hook, but no precipice to descend/ascend.

- There's a cyclops around here somewhere.

- There's a wizard around here somewhere.

But I seem to have run out of areas to explore, again.
To be continued...

* ["Playing _Castlequest_ (1980), Part 2"](/blog/2021/03/20/all-the-adcastlequest-part-2/) (2021-03-20)
