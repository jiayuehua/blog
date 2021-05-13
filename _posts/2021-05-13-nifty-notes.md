---
layout: post
title: "Nifty notebooks, and _Almazar_"
date: 2021-05-13 00:01:00 +0000
tags:
  adventure
  castlequest
  digital-antiquaria
---

Thanks to my posts on _Castlequest_, I've been receiving the occasional email
from people with memories of the game. Most recently, I received a message from
John P. Wesson (RPI classes of '71, '76, and '88). He'd seen the _Castlequest_
news via the _wrpi-alumns_ mailing list, and wanted to share copies of his notes
on _Castlequest_ and two other games, made "while babysitting lab
equipment during 24-hour runs" as a grad student in the 1980s.

His notes are really awesome! For each game there's a complete map, a list of items,
and typically some hints on the puzzles. Wesson numbers all the rooms on the map;
his numbers are often spookily close to those in the games' own data files, but
_not_ identical: evidence that his maps are the genuine product of late-night gameplay,
not source-diving.

There's also a page of "trophies": printed clippings demonstrating for posterity
that John had, indeed, beaten each game with a perfect score.

|:------------------------------------------------------------:|:------------------------------------------------------:|
| [![Adventure](/blog/images/2021-05-13-adventure.jpg)][1]     | [![Almazar](/blog/images/2021-05-13-almazar.jpg)][2]   |
| [![Castlequest](/blog/images/2021-05-13-castlequest.jpg)][3] | [![Trophies](/blog/images/2021-05-13-trophies.jpg)][4] |

John's notes on _Adventure_ (the original 350-point version) fit on one sheet.
Though still clearly _very_ practiced, they're the messiest of the lot.
The three-dimensional geography starts in pencil in the upper right corner, and meanders
down and west until the Hall of the Mountain King (which is both "down from" and "north of"
the Hall of Mists) appears twice with a bit of white-out where he apparently decided to
jump to a new place on the page.
At the bottom of the page is a hint to the Last Lousy Point, and another indication
of a work-still-in-progress: "? Try depositing at Y2, also key!"

John's carefully drafted maps of _Castlequest_ occupy four pages, and include
a complete map of the maze of twisty passages, with a table of shortest routes from
each entrance to each other exit.

Finally, there's two pages of notes on _Almazar_ — a game I'd never heard of before!
So of course I went looking for it online... and quickly found several versions.
The full name of the game is _The Search for Almazar, Part I: The Proving Ground,_
by Winston M. Llamas, who was then a CS undergrad at RPI. The game was written in BASIC;
although Wesson played it on a mainframe (maybe an IBM 370?), the versions I found
were all associated with [microcomputers](https://en.wikipedia.org/wiki/Microcomputer)
such as the [TRS-80](https://en.wikipedia.org/wiki/TRS-80).

One version was published in _80 Micro_ magazine's 1983 Special Anniversary Issue,
pages 288–297 ([archived here](https://archive.org/details/80-microcomputing-magazine-1983-SE/page/n287/mode/2up)).
This version is specifically billed as "the first of a projected series of programs
whose central theme is the continuing search for the super being Almazar."

Another version was distributed in source-code form on "SIG/M Volume 142" (1983-10-07).
This version had been slightly modified by one Bob Liddelow in order to run on the
[Osborne 1](https://en.wikipedia.org/wiki/Osborne_1), and perhaps also to remove
the "first of a series" wording to make it a bit more of a standalone adventure.
(And also to introduce some typos that make it literally unwinnable.
I still don't understand exactly how _that_ happened.)

Intriguingly, penciled on the second page of Wesson's _Almazar_ notes is a sketch
marked "Almazar addition," depicting eight or nine additional rooms on the near
side of the river with names like "fork in river," "swept into subterranean cavern,"
and "subterranean lake." These rooms do not appear in the _80 Micro_ or SIG/M
versions of the game. Wesson guesses that these rooms were added by Llamas (or somebody)
circa 1983–84, but doesn't really remember what the deal was or how he would have known
at the time that they were an "addition."


## Playing _Almazar_

I've put copies of the microcomputer _Almazar_ sources (along with their provenance)
on [github.com/Quuxplusone/Almazar](https://github.com/Quuxplusone/Almazar).

It might be possible to use them with any sufficiently retro BASIC interpreter
native to your preferred OS; I haven't tried. What I _have_ tried, successfully,
is to run them in Microsoft BASIC v5.21 for CP/M, using
Iván Izaguirre's [iz-cpm](https://github.com/ivanizag/iz-cpm/releases) emulator
(on a MacBook running MacOS 10.14). The only thing that doesn't work there is
the `SAVE` command: it invariably gives a "Disk full" error for some reason.

Serendipitously, just yesterday another CP/M emulator for MacOS was on the
front page of Hacker News: Tom Harte's [CP/M for OS X](https://github.com/TomHarte/CP-M-for-OS-X/releases).
_Almazar_ seems to work fine in that, too. But iz-cpm is more suited to
my command-line aesthetic.

So far, I've detected and fixed four significant typos in the code; there may be
more I haven't yet detected.

More on _Almazar_ later, perhaps.

[1]: /blog/images/RPI-Adventure-Notes.pdf
[2]: /blog/images/RPI-Almazar-Notes.pdf
[3]: /blog/images/RPI-Castlequest-Notes.pdf
[4]: /blog/images/RPI-computer-games.pdf
