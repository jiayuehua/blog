---
layout: post
title: "The Star Wars cellular automaton"
date: 2020-06-29 00:01:00 +0000
tags:
  cellular-automata
  esolang
  pretty-pictures
  web
---

You've probably heard of [Conway's Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life),
a cellular automaton which produces intriguingly chaotic patterns from simple rules. (Stephen Wolfram would
call it a ["class 4 CA"](https://www.wolframscience.com/nks/p235--four-classes-of-behavior/) as if that
meant something.) This blog post is on another intriguingly chaotic cellular automaton known as
"[Star Wars](https://www.conwaylife.com/wiki/OCA:Star_Wars),"
which was discovered by Mirek Wójtowicz circa 1999.

It makes a pretty nice screensaver to stare at.

<script type="text/javascript" src="/blog/code/2020-06-29-starwars.js"></script>
<div>
    <canvas id="RandomCanvas" width="640" height="400"></canvas>
    <script>
    window.addEventListener('load', function () {
        starWarsWithButtons(document.getElementById('RandomCanvas'), {
            buttons: ['play', 'step', 'clear'],
            busyBorders: true,
            initialPattern: [[]],
            initiallyAdvance: 50,
        });
    });
    </script>
</div>

Many [CAs in the general vicinity of Conway's Life](https://en.wikipedia.org/wiki/Life-like_cellular_automaton)
can be described via a compact notation pioneered by
[Mirek Wójtowicz](https://en.wikipedia.org/wiki/Mirek%27s_Cellebration). Life itself is "B3/S23": a new cell
is <b>B</b>orn when a dead cell is surrounded by 3 live cells, and a living cell <b>S</b>urvives when it
is surrounded by 2 or 3 other living cells. (This assumes an eight-cell
[Moore neighborhood](https://en.wikipedia.org/wiki/Moore_neighborhood).)

One way to extend the rules of Life-like automata is to give cells a "refractory period." When a cell in
such a CA dies, it cannot immediately become alive again; instead, the dead cell enters a "countdown" of
a certain number of timesteps, after which it becomes eligible for life again. That is, in these CAs,
there are Life-like rules for transitioning from state 0 to state 1, and from state 1 to state 2, but then
any cell in state 2 unconditionally enters state 3, any cell in state 3 unconditionally enters state 4,
and so on, until finally any cell in state N enters state 0 and is once again subject to a Life-like rule.
The compact notation for this kind of automaton is its Life-like rules for states 0 and 1, followed by
the _total number of states_ in the automaton (that is, the number of refractory states plus two).

[Brian's Brain](https://en.wikipedia.org/wiki/Brian%27s_Brain) is the most famous refractory Life-like CA;
its rule happens to be B2/S/3. (The lone "S" indicates that live cells in Brian's Brain never survive for
multiple turns; the "3" indicates that it has three states: dead, alive, and refractory.)

Star Wars is a refractory Life-like automaton with the rule "B2/S345/4" (that is, it has two refractory states).

For more specimens of refractory Life-like rules, see [the old MCell home page](http://psoup.math.wisc.edu/mcell/rullex_life.html).

Refractory states tend to lead to tiny light-speed spaceships.
In the same way that the "head" and "tail" states in Brian Silverman's [Wireworld](https://en.wikipedia.org/wiki/Wireworld)
create "electrons" that move in the direction of their head, Brian's Brain and Star Wars are full of
asymmetric constructs that birth living cells ahead of them as their "tail" enters the refractory state.
In Star Wars a single pair of cells, with refractory cells behind them, makes a glider.
If you curl up the ends of the glider into parentheses, you don't even need the refractory cells.

<div>
    <canvas id="GliderCanvas" width="120" height="120"></canvas>
    <script>
    window.addEventListener('load', function () {
        starWarsWithButtons(document.getElementById('GliderCanvas'), {
            buttons: ['play', 'step', 'reset'],
            pixelsPerCell: 10,
            wrapBorders: true,
            initialPattern: [
              [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
              [0,0,1,0,0,0,0,0,0,3,3,0,0,1,0,0,1,0,0,0,0,0,0,0],
              [0,1,3,0,0,0,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0],
              [1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
              [1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                    [0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                    [0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                    [0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                    [0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                    [0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                    [0,0,0,0,0,0,0,0,0,0,0,0,0,0],
            ],
        });
    });
    </script>
</div>

One of the most common naturally occurring spaceships is a "fireball" that travels
at light speed. (I see that [Catagolue knows about this entity](https://catagolue.appspot.com/census/g4b2s345/C1/xq4),
but I don't really have any idea what that means.) Here's the fireball, plus another
slightly larger and rarer naturally occurring spaceship below it:

<div>
    <canvas id="FlamingCanvas" width="250" height="210"></canvas>
    <script>
    window.addEventListener('load', function () {
        starWarsWithButtons(document.getElementById('FlamingCanvas'), {
            buttons: ['play', 'step', 'reset'],
            pixelsPerCell: 10,
            wrapBorders: true,
            initialPattern: [
                [0,0,0,0,0,5,0,0,5,0],
                [0,0,0,6,0,5,0,5,5,5],
                [0,0,0,0,0,7,5,5,5,5],
                [0,0,0,0,0,0,0,5,0,0],
                [0,0,0,0,0,0,0,0,0,0],
                [0,0,0,0,0,0,0,0,0,0],
                [0,0,0,0,0,0,0,0,0,0],
                [0,0,0,0,0,0,0,0,0,0],
                [0,0,0,7,6,5,0,0,0,0],
                [0,7,6,0,0,7,6,5,0,0],
                [0,0,5,6,7,0,5,0,5,0],
                [7,7,0,0,7,0,5,6,5,5],
                [6,0,0,0,7,0,5,6,5,5],
                [0,0,7,0,0,6,0,5,0,0],
                [0,0,0,0,0,7,6,5,0,0],
            ],
        });
    });
    </script>
</div>

Here's an extensible ship that paddles along like a centipede, and a ship that gallops along like a centaur:

<div style="display:inline-block;">
    <canvas id="CentipedeCanvas" width="240" height="80"></canvas>
    <script>
    window.addEventListener('load', function () {
        starWarsWithButtons(document.getElementById('CentipedeCanvas'), {
            buttons: ['play', 'step', 'reset'],
            pixelsPerCell: 8,
            wrapBorders: true,
            initialPattern: [
                [0,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,0],
                [0,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,0],
                [3,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                [0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1],
                [0,2,3,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,2,1,1],
                [0,0,0,0,3,0,1,2,0,0,2,3,0,1,3,0,1,2,0,0,2,3,0,1,0,0],
            ],
        });
    });
    </script>
</div>
<div style="display:inline-block;">
    <canvas id="CentaurCanvas" width="200" height="120"></canvas>
    <script>
    window.addEventListener('load', function () {
        starWarsWithButtons(document.getElementById('CentaurCanvas'), {
            buttons: ['play', 'step', 'reset'],
            pixelsPerCell: 8,
            wrapBorders: true,
            initialPattern: [
                [0,0,0,0,0,0,0],
                [0,0,0,7,6,5,0],
                [0,0,0,7,6,5,0],
                [0,0,0,0,0,0,0],
                [0,5,5,5,5,5,5],
                [6,5,0,6,6,5,5],
                [5,0,5,0,5,0,0],
                [0,0,7,6,5,0,0],
            ],
        });
    });
    </script>
</div>

Star Wars also has aesthetically pleasing still lifes and blinkers, in the form of "planets."
A cross of five live cells makes a stable still life. But if a ship comes too close to the
planet, the planet may gain a satellite — destroying the ship in a cloud of flaming debris!

<div>
    <canvas id="CaptureCanvas" width="210" height="150"></canvas>
    <script>
    window.addEventListener('load', function () {
        starWarsWithButtons(document.getElementById('CaptureCanvas'), {
            buttons: ['play', 'step', 'reset'],
            pixelsPerCell: 10,
            initialPattern: [
                [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0],
                [0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0],
                [0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,0],
                [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                [0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0],
                [0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0],
                [0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0],
                [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
            ],
        });
    });
    </script>
</div>

Planets may have up to three satellites at once; and a two-planet system may be
stabilized by four satellites.

<div>
    <canvas id="TriplePlanetCanvas" width="100" height="160"></canvas>
    <script>
    window.addEventListener('load', function () {
        starWarsWithButtons(document.getElementById('TriplePlanetCanvas'), {
            buttons: ['play', 'step'],
            pixelsPerCell: 10,
            initialPattern: [
                [0,2,0,0,0],
                [1,0,1,0,1],
                [0,1,1,1,2],
                [0,0,1,0,0],
                [0,2,1,0,0],
                [0],
                [0],
                [0,0,0,0,0,0,0,0,0,0],
                [0,0,1,0,0,0,0,0,0,0],
                [0,2,0,1,0,2,1,0,0,0],
                [0,0,1,1,1,0,1,0,0,0],
                [0,0,0,1,0,1,1,1,0,0],
                [0,0,0,1,2,0,1,0,2,0],
                [0,0,0,0,0,0,0,1,0,0],
            ],
        });
    });
    </script>
</div>

----

For the big "random exploration/screensaver" grid at the top of this post, I decided to add
some interest to the visual display by color-coding each living cell as part of a "faction."
Recall that in Star Wars, the only way to birth a new cell is for the cell to have exactly
two living neighbors. So, I've assigned each new cell to the "fire" faction, the "water" faction,
or the neutral "magenta" faction, according to whether its two parent cells are themselves
majority-fire, majority-water, or neutral/tied. This produces a vivid display of strife
between the factions, even though the rules of the underlying CA have not changed at all.
(See ConwayLife's wiki page on [Colourised Life](https://www.conwaylife.com/wiki/Colourised_Life),
which is the same kind of thing.)

Also, the north and south edges of the grid (about 10 cells off-screen in either direction) are
constantly seeded with random noise, producing a steady influx of "immigrants" to the
galaxy.

[LifeWiki](https://www.conwaylife.com/wiki/OCA:Star_Wars) quotes John M. G. Elliott as
observing that Star Wars is "active to a degree almost reminiscent of [Brian's] Brain,
with the distinction that it likes to build fixed Lego-like skeletal structures."
If you leave it running long enough, the galaxy will ossify — the Lego-like structures
will accumulate like plaque and stifle anything interesting that the spaceships
might try to do. But it does take quite a while to get to that point!

----

Participants on the ConwayLife forum have invented some pretty cool Wireworld-esque
circuits and other gadgets in Star Wars; see [here](https://www.conwaylife.com/forums/viewtopic.php?f=11&t=507).
The "electron-to-glider" gadget depicted here is due to ConwayLife poster ["calcyman"](https://www.conwaylife.com/forums/viewtopic.php?f=11&t=507&start=25#p3452)
and the general design of the wires is due to poster ["ebcube"](https://www.conwaylife.com/forums/viewtopic.php?f=11&t=507#p3433).

<div>
    <canvas id="WireCanvas" width="260" height="145"></canvas>
    <script>
    window.addEventListener('load', function () {
        starWarsWithButtons(document.getElementById('WireCanvas'), {
            buttons: ['play', 'step','reset'],
            pixelsPerCell: 5,
            initialPattern: [
[0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[1,1,1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,1,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,1,1,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0],
[0,1,1,1,0,0,0,1,1,0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0],
[0,0,1,0,0,0,0,0,1,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0],
[0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,1,1,0,0,0,0,1,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,1,0,0,0,0,0,1,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,1,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,1,0,0,7,6,5,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,1,0,0,0,7,6,5,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,1,1,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,1,1,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
[0,9,10,11,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
            ],
        });
    });
    </script>
</div>

ConwayLife poster "knightlife" found a very cool "bug" that crawls around the inside
of a slanted rectangular enclosure; see [here](https://www.conwaylife.com/forums/viewtopic.php?f=11&t=507&start=50#p3734).

What sorts of things might _you_ discover in Star Wars?
