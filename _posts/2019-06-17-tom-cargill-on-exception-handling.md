---
layout: post
title: '"Exception Handling: A False Sense of Security"'
date: 2019-06-17 00:01:00 +0000
tags:
  exception-handling
  wg21-folkloristics
---

This is the 25th-anniversary year of Tom Cargill's article ["Exception Handling: A False Sense of
Security,"](http://ptgmedia.pearsoncmg.com/imprint_downloads/informit/aw/meyerscddemo/DEMO/MAGAZINE/CA_FRAME.HTM)
which appeared in _C++ Report_ magazine (November/December 1994).

> I suspect that most members of the C++ community vastly underestimate the skills needed to program with exceptions
> and therefore underestimate the true costs of their use. The popular belief is that exceptions provide a
> straightforward mechanism for adding reliable error handling to our programs. On the contrary, I see exceptions
> as a mechanism that may cause more ills than it cures. Without extraordinary care, the addition of exceptions
> to most software is likely to diminish overall reliability and impede the software development process.
>
> This "extraordinary care" demanded by exceptions originates in the subtle interactions among language features
> that can arise in exception handling. Counter-intuitively, the hard part of coding exceptions is not the explicit
> throws and catches. The really hard part of using exceptions is to write all the intervening code in such a way
> that an arbitrary exception can propagate from its throw site to its handler, arriving safely and without
> damaging other parts of the program along the way.
>
> In the October 1993 issue of the _C++ Report_, David Reed argues in favor of exceptions [...]
> To illustrate my concerns concretely I will examine the code that appeared in Reed's article.

Cargill's article is well worth reading from beginning to end. (Read it at the link above; or
[here (PDF)](https://pdfs.semanticscholar.org/467a/e223730836328c9fe1a27ed12bc0034efb7c.pdf);
or it's reprinted on paper in Stan Lippman's
[_C++ Gems: Programming Pearls from The C++ Report_](https://www.amazon.com/Gems-Programming-Pearls-Reference-Library/dp/0135705819/ref=as_li_ss_tl?keywords=C+++Gems:+Programming+Pearls+from+The+C+++Report&qid=1560813995&s=books&sr=1-1&linkCode=sl1&tag=arthurodwye01-20&linkId=0e1dbd4626a9392c2e2f250a7c963027&language=en_US)
(1997).)
The state of C++ programming has improved _dramatically_ in the past 25 years;
but I dare say that most published code still would not survive a close reading, with respect to exception-safety.
(See also: ["Fetishizing class invariants"](/blog/2019/02/24/container-adaptor-invariants/) (February 2019).)

Unfortunately, the article Cargill was rebutting — David R. Reed's "Exceptions: pragmatic issues with a new language feature"
(October 1993) — doesn't seem to be accessible these days; it's not reprinted in Lippman's book and it's not
online anywhere that I'm aware of. If you know where to find a copy, drop me a line!
