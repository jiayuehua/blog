---
layout: post
title: 'WG21 and the National Popular Vote Compact'
date: 2018-04-26 00:01:00 +0000
tags:
  us-politics
  wg21
---

There is a thing going on in the United States right now (since 2006) called the
[National Popular Vote Interstate Compact](https://en.wikipedia.org/wiki/National_Popular_Vote_Interstate_Compact).
It is an agreement among several U.S. states (currently MD, NJ, IL, HI, WA, MA, DC, VT, CA, RI, NY) that
*currently* has no effect, but which specifies that *once the Compact has been ratified by enough states*
that their combined voting power exceeds 50% of the Electoral College, the group *as a whole* will throw all
of their Electoral College votes directly to the winner of the national popular vote in every Presidential
election.

(Political aside— Notice that all of the signatories so far have been deeply [blue](https://en.wikipedia.org/wiki/Red_states_and_blue_states) states;
the maps on those two Wikipedia pages match up almost exactly. The Democratic Party is confident in its ability
to retain the popular vote, as they have done in 1992, 1996, 2000, 2008, 2012, and 2016. The Republican
Party is equally confident in its ability to retain the Electoral College vote, as they have done in
1980, 1984, 1988, 2000, 2004, and 2016. —end political aside)

![On the left, green means blue and yellow means nothing](/blog/images/2018-04-26-maps.png)

This is a neat way of dealing with a thorny problem: how to efficiently and effectively deal with a
rules change that *might* be popular, but would be very hard to make via traditional channels.

Also consider the paradox of [the economists and the $20 bill on the sidewalk](https://financingefficiency.wordpress.com/2011/10/19/the-20-bill-on-the-sidewalk/):

> The young economist looks down and sees a $20 bill on the street and says, "Hey, look! A twenty-dollar bill!"
> Without even looking, his older and wiser colleague replies, "Nonsense. If there had really been a twenty-dollar bill
> lying in the street, somebody would have picked it up by now."


## Now for the WG21-geek part

I wonder if there is room for procedural improvements in how WG21 deals with this pattern that seems to come up fairly often...

Some number of vocal and knowledgeable people all say that

- language feature X was a mistake and should have been specified as not-X instead

- but apparently the committee as a whole agreed with X back in the day, and it's unclear how many pro-Xers (if any) have changed their mind

- so the anti-Xers altruistically agree that it would be unproductive to rehash that discussion right now

- especially since it's only a *few* vocal and knowledgeable people who have said out loud that they are anti-X.

The paradoxical result is that everyone altruistically agrees to continue with the status quo, even though the
participants in the discussion all believe that the status quo is *worse* than the alternative.

Recently, this pattern came up in a discussion of how:

- Unwinding an exception out of a `noexcept` function is specified to call `std::terminate`.
  It should have been specified as a precondition violation, i.e., undefined behavior.
  (But there can't *really* be a $20 bill lying in this street, because if there were, C++11 would have picked it up, right?)

And I feel like it's come up fairly often in the past, although I'm blanking on any other examples. I'll add them
here as I think of them, if I think of them.

Modest proposal: Maybe we need a central clearinghouse for these ideas, modeled after the National Popular Vote
Interstate Compact. We could list ideas, and people could sign onto them (or onto the "over my dead body" side,
of course), but ideas on the list would be prohibited from taking up any actual time at WG21 meetings until a certain
critical mass of supporters (and critical *lack* of objectors) was reached.

Notice that *in theory*, this is no different from how WG21 already works. Want to make something happen? Write
it down (in a paper) and get a critical mass of supporters (coauthors) and mollify the opposition.
And *in theory*, the National Popular Vote Interstate Compact is no different from how the U.S. Congress works.
But [*in practice*, there's a difference](https://en.wikiquote.org/wiki/Jan_L._A._van_de_Snepscheut) — or at least
it feels like there is one.
