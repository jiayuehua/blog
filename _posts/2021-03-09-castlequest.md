---
layout: post
title: "_Castlequest_ exhumed!"
date: 2021-03-09 00:01:00 +0000
tags:
  adventure
  castlequest
  digital-antiquaria
---

"Lost game" _Castlequest_ (1980) has been found!

## Background

In the 1990s, my parents had a subscription to the online service "GEnie".
I fondly remember the "GEnie Games" area. There you could play either 350-point _Adventure_
or David Platt's 550-point _Adventure_ (and as a kid I never knew there was a difference,
which made it extra special and spooky to stumble into one of Platt's new rooms that I
just _knew_ wasn't there yesterday) — and also several games that
[are now lost](http://www.club.cc.cmu.edu/~ajo/in-search-of-LONG0751/readme.html#others),
such as _Black Dragon_ and _Castlequest_.

_Castlequest_, written by Mike Holtzman and Mark Kershenblatt (but ported
and credited-on-GEnie-to Bob Maples), was a parser adventure similar to, and
inspired by, _Adventure_ (but coded differently; Holtzman had never seen
_Adventure_'s actual source code). _Castlequest_ opens like this:

    You are in a large, tarnished brass bed in an old, musty bedroom.
    cobwebs hang from the ceiling.  A few rays of light filter through
    the shutters.  There is a nightstand nearby with a single wooden
    drawer.  The door west creaks in the breeze.  A macabre portrait
    hangs to the left of an empty fireplace.
    The shutters are closed.
    There is a silver bullet here.

Similar to _Adventure_, there are wandering nuisance monsters — but this
time they're werewolves. One of the most memorable bits of _Castlequest_ is
what happens when you kill one:

    You killed a werewolf. An old gypsy woman
    appears and drags away the body.

Anyway, after the demise of GEnie (and The Source and Compuserve), it seemed that
the game had been lost forever. _But in fact it had not!_


## Give me the source code

[Here's the code.](https://github.com/Quuxplusone/Castlequest)
This code is _still under copyright_ to Mike Holtzman and Mark Kershenblatt,
and provided by Mark's extensive and generous assistance.

In fact, had this code _not_ been copyrighted, it would still be lost today!
Funny story...

See, one of the few references to _Castlequest_ on the Internet prior to
this decade was on the website of the U.S. Copyright Office. Back in 1981,
Mike Holtzman had submitted the program for archival — a service the USCO
still provides, even though all creative works are automatically _protected_
under U.S. copyright law whether they're submitted for archival or not.
So it was overkill on Holtzman's part, legally speaking... but I'm sure glad
he did it! The documentation submitted by Holtzman was assigned the
number [TXu000091366](https://cocatalog.loc.gov/cgi-bin/Pwebrecon.cgi?Search_Arg=TXu000091366&Search_Code=REGS&CNT=10&HIST=1),
and remained on file somewhere in the depths of Washington, D.C., for
forty years. However, the USCO entry doesn't say what kind of documentation
was submitted. (I assumed it would be a game transcript or walkthru.)

I made contact with Mike Holtzman for just a couple of months in 2016,
through an email address which is now defunct. He answered some questions
about the history of the game, but I failed to interest him in retrieving
the deposit. (He, also, assumed it was just a transcript: it was so long ago
that he'd forgotten.)

Then, in 2020, I got an email from the _other_ author, Mark Kershenblatt,
whom I _did_ persuade to attempt retrieval! The retrieval process required
USCO employees to search the physical archive, which meant that it was
very slow: the coronavirus pandemic had reduced the USCO's headcount and
hours. But just a week ago, Mark emailed me to say that the records had
been found, copied, and snail-mailed to him!

And guess what? Mike Holtzman's 1981 copyright deposit wasn't just
a transcript — it was the full Fortran source code and data files for the
entire game, all 78 pages, printed out in _remarkably_ high quality!
(Naturally, Mike did it on his office printer. Times haven't changed a bit.)

So the USCO sent Mark copies of all 78 pages; Mark went to Staples,
scanned them in, and uploaded them to Dropbox; I downloaded them and...

...well, it turns out that even 16 years after the launch of Google Books,
the state of the art of [OCR](https://en.wikipedia.org/wiki/Optical_character_recognition)
is still ridiculously bad. After a miserably failed attempt to
[get Google Drive to OCR the PDF](https://webapps.stackexchange.com/questions/111691/how-do-i-make-google-drive-perform-ocr-on-a-pdf-i-upload),
I just went the manual route. Three days of eyestrain later, I had a
plain-text version of the code!


## Timeline

- 1979: The initial "release" of _Castlequest_ at [RPI](https://en.wikipedia.org/wiki/Rensselaer_Polytechnic_Institute),
    according to Mike Holtzman's best recollection. Mark Kershenblatt guesses
    "between September 1979 and May 1980." (Holtzman was in the graduating class
    of 1980.)

- 1980-02-??: The date on the first page of the source code.

- 1981-10-22 ("81.295"): Holtzman, now employed at Grumman Data Systems,
    prints out the source code for the deposit.

- 1981-11-06: The USCO stamps Holtzman's deposit.

- 1983-07-??: _Compute!_ magazine publishes a BASIC game by Timothy G. Baldwin,
    coincidentally also named "Castle Quest". Holtzman (and Kershenblatt?)
    prank the magazine with "a cease and desist letter on a fake legal letterhead";
    _Compute!_'s October issue carries an apology for having "inadvertently used
    the name _Castle Quest_," crediting the wrong authors, which they then have
    to correct _again_ in their January 1984 issue. Good times.

- 2016-01-06: I post my ["In Search of LONG0751"](http://www.club.cc.cmu.edu/~ajo/in-search-of-LONG0751/readme.html) webpage,
    with a mention of _Castlequest_.

- 2016-08-19: Mike Holtzman first emails me (from an openlink.com address).
    He sends four emails through September, filling in the history of the game.

- 2016-09-06: Holtzman's last email to me.

- 2018-07-11: Holtzman's openlink.com address bounces.

- 2020-07-28: Mark Kershenblatt first emails me (from a gmail.com address).

- 2020-10-06 (less "a couple of days"): Mark Kershenblatt begins the USCO retrieval process.

- 2021-03-02: Mark Kershenblatt receives the packet of source code from the USCO.

- 2021-03-09: I finish transcribing Mark's scanned PDF and upload the code to GitHub.


### Bug bounty program

I certainly introduced some mistakes during that manual transcription
process. Therefore I'm offering a "bug bounty" of $5 per error to anyone who reports
mistakes in the transcription. Note that my goal is fidelity to the original,
so "mistakes" in the original (e.g. the word `STAUS` on page 1) do not count.
Vice versa, if you point out where my fingers accidentally _corrected_
the spelling of a word that was misspelled in the original, you get a bounty!
Submit reports to me via email, or as GitHub pull requests.


## So, can it be compiled?

Sadly, the code can't be compiled right out of the box. If you're an expert
in antiquarian Fortran and would like to assist, send me a pull request and/or
an email!

Known pain points, as of this writing, are:

(1) The use of character strings such as `'BULL'` in `DATA` statements
where an array of two integers is expected. My impression is that this
was common in old Fortran code. _Adventure_ famously packed five 7-bit characters
into a 36-bit word on the [PDP-10](https://en.wikipedia.org/wiki/PDP-10),
which is why it looks at only the first five characters of each
vocabulary word. But gfortran 4.9 doesn't approve:

    Error: Incompatible types in DATA statement at (1);
    attempted conversion of CHARACTER(1) to INTEGER(4)

(2) How to read the data files. The original code calls out to a
(VS Fortran?) builtin to hook up the `HINT` file to input unit
number 8, and then reads it a record at a time. To modernize
this code is probably not difficult, but isn't something I know
how to do off the top of my head.

          CALL CMS('FI      ','8       ','DISK    ',
         2         'HINT    ','CQDATA  ','(       ',
         3         'LRECL  ','80      ','RECFM   ',
         4         'F       ')
     [...]
          DO 110 I=1,50
               READ(8,1001,END=120,ERR=120)(HINT(I,J),J=1,20)
      110 CONTINUE

I'm also puzzled by the way that many (but not all) strings, both in the
data files and in the code, begin with the digit character `0`.

(3) The use of builtins for getting the time
and date (`TOD`), initializing the random number generator (`RDMIN`),
generating random real numbers (`RDM`), and maybe a couple
other builtins I failed to notice.

----

UPDATE: The game _can_ be compiled! See:

* ["Making _Castlequest_ compilable"](/blog/2021/03/11/castlequest-update/) (2021-03-11)
