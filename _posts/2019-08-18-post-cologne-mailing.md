---
layout: post
title: "The post-Cologne mailing is out"
date: 2019-08-18 00:01:00 +0000
tags:
  allocators
  copy-elision
  coroutines
  rant
  wg21
---

The post-meeting mailing from WG21's Cologne meeting was released just the other day.
I wasn't in Cologne, but the post-meeting mailing contains a couple of my contributions.

First of all, the new draft standard [N4830](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/n4830.pdf)
includes wording from my [P1155 "More Implicit Moves"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1155r3.html)
which permits the compiler to "implicitly move" in situations like

    struct base { ... };
    struct derived : base {};

    base f(base b) {
        throw b;    // move-from, no copy
        derived d;
        return d;   // move-from, no copy
    }

I was rather surprised to see that EWG had changed its mind again about David Stone's
[P0527 "Implicitly move from rvalue references in return statements"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0527r1.html) —
last I knew, EWG had voted down that proposal, but it came back as
[P1825 "Merged wording for P0527R1 and P1155R3"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1825r0.html)
and was adopted.

The difference between P1155 and P0527/P1825/N4830 is that the latter now _also_ permits

    base f(base&& b) {
        throw b;    // move-from, no copy
        auto&& b2 = base();
        return b2;   // move-from, no copy (but no copy-elision, either)
    }

There's no prior art for this kind of implicit move from rvalue reference types (as opposed to P1155,
which was mainly standardizing behaviors that various compiler vendors had already been doing for years).
It will be... interesting... to see whether C++2a's implicit move from rvalue references is hailed as a
simplification or a complexification.

Note that P0527/P1155 were accepted as [DRs](/blog/2019/08/02/the-tough-guide-to-cpp-acronyms/#dr),
which means that future compilers will implement "implicit move from rvalue references"
even in C++11 mode. Exciting times.

----

[P1612 "Relocate Endian's Specification,"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1612r1.pdf)
which I coauthored with Walter Brown (it was mostly his work, really), also made it into the N4830 draft.
This long-overdue cleanup paper moves Howard Hinnant's `std::endian` enumeration out of `<type_traits>`
and into `<bit>`. (And yes, I assume the paper's title is a pun on other work by this coauthor. I
didn't pick the title.)

----

[P1808R0 "Contra P0339 '`polymorphic_allocator<>` as a vocabulary type'"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1808r0.html)
(Arthur O'Dwyer, July 2018). The highlight here is

> A significant number of C++ developers are already confused about which of `polymorphic_allocator`
> and `memory_resource` are templates, which are type-erased, and which are classically polymorphic.
> Allowing these developers to write `std::pmr::polymorphic_allocator a;` as if it were a concrete
> class type does them a grave disservice.

(Using CTAD to permit writing `polymorphic_allocator` without its template argument is the main point
of P0339, which was voted into C++2a at the previous meeting.) This paper needs a National Body to
sponsor a comment, since it is officially "too late" for anyone but a National Body to affect C++2a.

----

Finally, [P1837R0 "Remove NTTPs of class type from C++20"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1837r0.html)
(Arthur O'Dwyer, August 2018). The highlight here is

> P0732 was premised on an erroneous conflation of "`==` equality" and "NTTP identity."
> These are similar — but distinguishable — notions. Conflating them causes subtle inconsistencies
> which will be very hard for any future work in the area to fix.
>
> We should not ship class-typed NTTPs in C++20 without thoroughly exploring the consequences.
> Once P0732 has appeared in a published standard, it will be too late to fix it.

[N4826](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/n4826.pdf) indicates that Jorg Brown's related
[P1714R1 "NTTP are incomplete without float, double, and long double"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1714r1.html)
was brought up for a vote in plenary. The vote was not "do we want this feature," but rather
"do we want to even _think_ about these arguments in the C++2a timeframe." It was voted down, 39–14.

My P1837 cross-references another paper, P1821 "The Spaceship Needs To Be Grounded," which I expected to see
in the post-Cologne mailing after it was [discussed in EWG](http://wiki.edg.com/bin/view/Wg21cologne2019/D1821R0-EWG).
However, I guess the author abandoned the paper after that mostly fruitless discussion.
([EDIT: At this point I quoted a sentence from that discussion, without attribution.
The quotation involved the phrase "too late." I have removed the quotation after objection on Reddit.]
This mantra never ceases in WG21: individually we don't like the direction we're going,
but we're powerless to stop it because it is always _too late_.)


## The release of C++2a should be delayed past 2020

Also in the post-Cologne mailing is an updated revision of Herb Sutter's
[P1000 "C++ IS schedule."](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1000r2.pdf)
This paper has been, and continues to be, the guiding force behind WG21's rapid three-year release
cycles in this decade (following the previous two decades of nine-year release cycles).
The countervailing whisper, such as it is, is these postscripts on my blog. ;)

Herb and I have discussed the release cycle, and I am thoroughly convinced that he and many others
are good-faith believers in the benefits of a rapid release cycle. Believers
in the rapid cycle generally view the period 1995–2010 as a Dark Ages of stagnation,
lack of vitality, a wilderness of vendor extensions, in which innovation was done by
third parties like Qt and Boost instead of by the core committee.
Returning to those Dark Ages has been called "the fastest way to kill C++ I can think of."

Vice versa, I view 1995–2010 as a Golden Age of compiler stability and portability, in which the
C++ community spoke "C++" with no caveats or footnotes. You could learn C++ in 2006 from a book written
in 2001. You could make it all the way through a four-year degree, as I did, without having the
language change out from under you! Compiler vendors could implement the whole language in a
couple of years and then move on to _create new value_ by adding extensions driven by the market,
rather than spending all their time playing catch-up to the paper standard.
(These days libc++ has a general policy of not accepting library extensions unless they have already
been proposed and favored by WG21 — an inversion of the old motto "standardize existing practice.")
Meanwhile, because the compilers were stable and well-tested, the library-building could be outsourced
to mere programmers, leading to utility collections such as Boost.

Every year at C++Now, there's an event called [Library in a Week](https://cppnow2019.sched.com/event/Miye/library-in-a-week).
At least two of the past three years, it's been suggested (and half-heartedly pursued) to pick a
"Boost Classic" library and update it to use modern features such as alias templates,
constexpr, structured binding, and CTAD. But it never really goes anywhere, because what's the point?
Updating code to a later standard doesn't add value — if anything, it _decreases_ value by decreasing
portability. And what's the point of updating the code to use C++14 `enable_if_t` or C++17 `is_same_v`,
when you already foresee updating it next year to use C++2a `requires Same`
([I mean `requires same`](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1754r1.pdf))?
Programmers are [lazy](http://wiki.c2.com/?LazinessImpatienceHubris); if we _see_ the
[Red Queen's race](https://en.wikipedia.org/wiki/Red_Queen%27s_race), we subconsciously
try to avoid joining it.

----

Many of the arguments in P1000R2 engage with (hypothetical?) people who say "I want to delay
C++2a in order to push _more_ features into it."

> If we had just another meeting or two, we could add <feature> which is almost ready ...
>
> What if a major feature by a prominent committee member was “almost ready”...
>
> What about ... flexibility to take “a little” extra time when we feel we need it for a feature?

P1000R2's first bullet point, though, does engage head-on with my primary technical argument
for a longer release cycle: that the rapid cycle leads to the rapid standardization of
poorly baked, poorly explored features.

> There are bugs in the draft standard, should we delay C++NN?
>
> Of course, and no.
> Fixing bugs is the purpose of the final year, and it’s why this schedule set the feature freeze
> deadline for C++NN in early 20NN-1 (a year before), to leave a year to get a round of
> international comments and apply that review feedback and any other issue resolutions and bug fixes.

This is a good explanation of how WG21 has chosen to use its limited time wisely —
two years for innovation (while most compiler vendors are busy implementing the previous standard anyway),
then budget a whole year for fixing the bugs introduced in the previous pell-mell two years. If you only
have three years between releases, then that's a reasonable way to budget. But imagine for a moment
what would happen if we delayed C++NN!  With _five_ years between releases,
we could keep that "one year for bug-fixing," but give four years for innovation.
Now, maybe that'd just lead to twice as much crap going in and needing to be bug-fixed; but I think
we'd see something else happen, too. With a longer release cycle, I think we'd see people become
less panicked about their particular feature's chances; it wouldn't always be a
firing-on-all-cylinders race to get your camel's nose into CWG/LWG before the feature window closed,
followed by _another_ race to fix all the bugs before the bug-fixing window closed.
More time: more exploration, more discussion, less burnout. And from the point of view of the
working programmer, who sees only what _is_ released: more stability, more teachability,
a smaller language with fewer facepalms and footguns.

Interestingly, C++0x Concepts officially lasted in the draft standard [from November 2008 until
July 2009](http://ecee.colorado.edu/~siek/concepts_effort.pdf) (eight months), and of the vote to remove them,
Jeremy Siek says, "Most of the committee members felt [in July 2009] it was too late for major changes
[to Concepts]." So, 26 months before the release of C++11, people were already using
the "too late" mantra! (Now, C++11 had no really fixed release schedule; those people might
have assumed that a release was right around the corner.)

For comparison,

- `operator<=>` has been in the draft standard since November 2017 (21 months).

- P0732 class-typed NTTPs have been in the draft standard since June 2018 (14 months).

- Coroutines has been in the draft standard since March 2019 (five months), with a TS in December 2017 (20 months total).

- Modules has been in the draft standard since March 2019 (five months), with a TS in May 2018 (15 months total).

P1000R2 writes:

> No matter how long we delay the standard, there will be interactions we can’t discover until much later.

This is trivially true; but I claim that there's a continuum here. On the one end we might imagine a
super-rapid two-year release cycle (the shortest cycle permitted by ISO rules) that would discover
practically _no_ interactions and footguns; on the other we might imagine a multi-decade release cycle
that would discover practically (but not _literally_) all the interactions. In between, we have
points on the continuum. The modern three-year rapid release cycle is one point.
A five-year "ISO standard" release cycle would be another point. A nine-year
C++03/C++11-inspired release cycle would be yet another. Each successive point
increases the odds of finding flaws and fixing them before release. And from the point of view
of the working programmer, each successive year between standards increases the stability of the
language simply by _not destabilizing it._

When I say "C++2a should be delayed beyond 2020," I also mean "C++2b should be delayed beyond 2023,
and C++2c beyond 2026." I suggest C++2a = 2023 (although obviously it is as always "too late" for
that to be realistic), C++2b = 2029. I think the rapid release cycle with its concomitant
half-baked features causes _more_ harm to C++'s reputation and viability than would be caused
by a statelier, more deliberated pace.
