---
layout: post
title: "Decompiling GOET0580"
date: 2020-06-30 00:01:00 +0000
tags:
  adventure
  digital-antiquaria
  esolang
---

This is a step-by-step description of how to extract the A-code source code of Mike Goetz's
580-point _Adventure_ expansion (taxonomized in the
[Adventure Family Tree](http://advent.jenandcal.familyds.org/) as GOET0580).

The IF Archive has two versions of this game:

* [http://ifarchive.org/if-archive/games/cpm/cpm-advent.zip](http://ifarchive.org/if-archive/games/cpm/cpm-advent.zip)
* [http://ifarchive.org/if-archive/games/cpm/cpm-advent-b03.zip](http://ifarchive.org/if-archive/games/cpm/cpm-advent-b03.zip)

Download, unzip, and compile the `advdis` tool out of the former. (This tool is
copyright 2000 John Elliott — a _completely different_ John Elliott from the one
quoted in [yesterday's post](/blog/2020/06/29/star-wars-ca/)!)

    mkdir -p /tmp/work ; cd /tmp/work
    curl -O http://ifarchive.org/if-archive/games/cpm/cpm-advent.zip
    unzip cpm-advent.zip
    cd advdis
    less advdis.doc  # Always read the instructions first!
    gcc advdis.c -o advdis

Now you can disassemble the A-code from versions B00, B01, and B02!
You should find no differences between B00 and B01. Version B02 adds the
computer center and two new treasures, in addition to fixing many typos.

    cd ../adv-B00
    ../advdis/advdis -t > disassembled.txt
    cd ../adv-B01
    ../advdis/advdis -t > disassembled.txt
    cd ../adv-B02
    ../advdis/advdis -t > disassembled.txt


## Disassembling version B03 (an open question)

Now download and disassemble version B03:

    mkdir adv-b03 ; cd adv-b03
    curl -O http://ifarchive.org/if-archive/games/cpm/cpm-advent-b03.zip
    unzip cpm-advent-b03.zip
    ../advdis/advdis -t > disassembled.txt

This disassembly is mostly garbage. We can fix the first bunch of errors
by borrowing version B02's "symbol.tab" file.

    cp ../adv-b02/symbol.tab .
    ../advdis/advdis -t > disassembled.txt

The next problem is that all the messages are encrypted. This clearly
has something to do with the 384-byte "DECODE.DAT" file sitting in this
directory, but I don't know what to do with it.

It seems to me that the messages are _also_ compressed, because e.g.
`INFROMOUT` (a 109-character message) occupies only 43 bytes in version B03.
(And they're compressed more aggressively than in version B02, because
version B02's `INFROMOUT` occupies 86 bytes.)

So, I'm pretty much stymied at this point. Maybe someone reading this blog post will have an idea of
how to proceed with decrypting the messages in version B03. We know (most of) the plaintext; we have
the ciphertext; we have some (but not all) knowledge of the compression algorithm; but we don't know
the encryption algorithm at all (except insofar as one might reverse-engineer it from the Z80 machine
code in "adv.com"). That's a job for Not Me.

----

Postscript, 2020-07-06: While I haven't made any progress on decrypting the munged
data back into A-code, the question is now pretty much moot, because I've gotten
in touch with Mike Goetz and received a copy of the _original_ A-code source files
(and the original munger and interpreter, both written in K&R C) for Goetz's version C01.1,
circa March 1987. These files are now on their way to the
[Interactive Fiction Archive](https://www.ifarchive.org/indexes/if-archive/unprocessed/)
and I'll update this post again once they're available.

Postscript, 2020-07-29: The files are now available in the IF Archive!
The path is [/games/cpm/cpm-advent-c01.zip](https://www.ifarchive.org/indexes/if-archive/games/cpm/).
