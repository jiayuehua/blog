---
layout: post
title: '_The Design of Everyday Things_'
date: 2020-03-07 00:01:00 +0000
tags:
  cppcon
  litclub
  sre
---

Today I re-read Don Norman's 1988 book [_The Design of Everyday Things_](https://amzn.to/39INVrT).
I continue to highly recommend it to anyone who does engineering (especially software engineering).
In particular, it's the book that introduced me to the term _affordances_, as in "glass _affords_
seeing-through, and breaking; a flat panel _affords_ pushing but not pulling."
I felt like the term started to break out into the mainstream C++ community at CppCon 2019,
what with its forming the core of my ["Back to Basics: Type Erasure"](https://www.youtube.com/watch?v=tbUCHifyT24)
and also being name-dropped by Titus Winters in the B2B track's closing keynote
["What is C++?"](https://www.youtube.com/watch?v=LJh5QCV4wDg&t=8m11s)

(For links to all of CppCon 2019's B2B talks, see [here](/blog/2019/09/12/cppcon-2019-b2b-track/).)

Previously on this blog:

* ["Concepts as door-opening robots"](/blog/2018/09/24/concepts-as-door-opening-robots/) (2018-09-24)

----

_The Design of Everyday Things_ contains my preferred formulation of Murphy's Law
(often phrased as "Anything that can go wrong, will").  Murphy's Law is often seen as a negative,
cynical thing; I prefer to see it as a useful guarantee that helps us reason about systems design.
Norman writes:

> If an error is possible, someone will make it.
>
> The designer must assume that all possible errors will occur and design so as to minimize
> the chance of the error in the first place, or its effects once it gets made.
> Errors should be easy to detect, they should have minimal consequences, and, if possible,
> their effects should be reversible.

A few pages earlier he gives an example of design that takes this guarantee into account:

> A simple example of good design is [the 3.5-inch magnetic diskette](https://en.wikipedia.org/wiki/Floppy_disk#%E2%80%8B3_1%E2%81%842-inch_disk)
> [...] The diskette has a square shape: there are apparently eight possible ways
> to insert it into the machine, only one of which is correct. What happens if I do it wrong?
> I try inserting the disk sideways. Ah, the designer thought of that. A little study shows
> that the case really isn't square: it's rectangular, so you can't insert a longer side.
> I try backward. The diskette goes in only part of the way. Small protrusions, indentations,
> and cutouts prevent the diskette from being inserted backward or upside down: of the eight
> ways one might try to insert the diskette, only one is correct, and only that one will fit.
> An excellent design.

----

Norman quotes this story from Mike King, "a designer who works for a telephone company." I think
it is perfectly applicable to a lot of software design, including the design of C++:

> It is very hard to remove features of a newly designed product that had existed in an earlier version.
> It's kind of like physical evolution. If a feature is in the genome, and if that feature is not
> associated with any negativity (i.e., no customers gripe about it), then the feature hangs on for
> generations.
>
> It is interesting that things like the 'R' button [on a desk telephone] are largely determined
> through examples. Somebody asks, "What is the 'R' button used for?" and the answer is to give
> an example: "You can push 'R' to access loudspeaker paging." If nobody can think of an example, the
> feature is dropped. Designers are pretty bright people, however. They can come up with a
> plausible-sounding example for almost anything. Hence, you get features, many many features,
> and these features hang on for a long time. The end result is complex interfaces for essentially
> simple things.

Previously on this blog:

* ["How DNS got so complicated"](/blog/2018/11/20/dns-complexity/) (2018-11-20)

----

The main thing that distinguishes Don Norman's material from superficially similar collections
of "tech horror stories" (like [this disappointing Scott Meyers talk from 2014](https://www.youtube.com/watch?v=5tg1ONG18H8))
is that Norman's horror stories are curated each to provide a unique and profound moral
(as in the Murphy's Law example above).

I particularly like Chapter Three's description of a car where, to close the sunroof, you had to either
turn on the ignition _or_ turn the headlights on high beam:

> Mental models let people derive appropriate behavior for situations that are not remembered
> (or never before encountered). [...] That is why designers should provide users with appropriate
> models: when they are not supplied, people are likely to make up inappropriate ones.

And Chapter Three's discussion of stovetop burner arrangements:

> If a design depends upon labels, it may be faulty. Labels are important and often necessary,
> but the appropriate use of natural mappings can minimize the need for them. Whenever labels
> seem necessary, consider another design.

And Chapter Five's discussion of "the non-working key," an allegory familiar to all C++ programmers:

> Someone goes to his or her car and the key won't work.
> The first response is to try again, perhaps holding the key more level or straight.
> Then the key is reversed, tried upside down. When that fails, the key is examined and perhaps
> another tried in its stead. Then the door is wiggled, shaken, hit. Finally, the person [...]
> walks around the car to try the other door; at which point it is suddenly clear that
> this is the wrong car.
>
> In all the situations I have examined, the error correction mechanism seems to start at the
> lowest possible level and slowly work its way higher.

Previously on this blog:

* ["PSA: `shared_ptr<T>()` is not `make_shared<T>()`"](/blog/2019/03/06/shared-ptr-vs-make-shared-pitfall/) (2019-03-06)

----

Point is, [_The Design of Everyday Things_](https://amzn.to/39INVrT) is a great book and you should read it!
