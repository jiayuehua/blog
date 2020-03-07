---
layout: post
title: 'Concepts as door-opening robots'
date: 2018-09-24 00:01:00 +0000
tags:
  c++-style
  concepts
  cppcon
  paradigm-shift
---

I'm at CppCon this week. It's great. You should be here too.
(If you are — come say hello in person! I hope you're having fun
so far.)

The opening keynote was from Bjarne Stroustrup, advertising C++2a Concepts.
I won't try to characterize it too much, because I'm sure I'd inadvertently
misrepresent his views on the subject, but I'd say that it seems like he's
a big fan of the _notion_ of concepts and generic algorithms (just like I am),
and he is excited that C++2a is finally getting some _syntax_ to match those
notions, so that we can teach the notions more easily.

He gave some guidelines for the usage of Concepts. One guideline was
"Beware concepts with names ending in `-able`." `concept Addable` is probably
just as bad as `concept HasPlus`. _(Pay no attention to the `Sortable`
and `Mergeable` behind the curtain!)_ Concepts should represent full-fledged
and well-rounded notions such as `Numeric` (which may require definitions for
`+`, `-`, `*`, `/`, and so on). Bjarne raised and grappled-slightly-with the
question of whether this would inconvenience people with "addable but not
multipliable" types.

It occurs to me to tie together the question of "granularity of concepts"
with the question of "semantic constraints." When we say `concept HasPlus`
is a bad concept, part of the reason is that we don't have a full-fledged
model of what "having `+`" really means. `std::string` has `+` but it's not
commutative. Did we mean for `+` to be commutative? (`Numeric` types presumably
have commutative `+`. Although _that_ might not even be true, for things like
[ordinal numbers](https://en.wikipedia.org/wiki/Ordinal_number).)

I'm a big fan of Don Norman's book [_The Design of Everyday Things_](https://amzn.to/39INVrT).
The takeaway from that book is that everyday objects present certain visual or tactile
appearances that connote behaviors — what Norman calls _affordances_. A door whose handle
is a long vertical bar _affords_ pulling-to-open. A long horizontal bar _affords_ pushing.
Have you ever encountered a door with a long horizontal bar that seemed like a "push" but
was actually a "pull"? Wasn't it frustrating?

So suppose I'm designing a robot to roll around and go through doors in a human environment.
I need the robot to be able to open doors. I'd probably make it look at the shape of the
door handle, right?  If the handle is a horizontal bar or a
flat plate, the robot would try _pushing_, because a horizontal bar or flat plate _affords_
pushing-to-open. If the robot encountered a door with a horizontal bar that actually needs to
be _pulled_, then the robot might just get really confused and not be able to proceed.
This should sound completely plausible to the engineers in the audience, right?

Now, suppose field testing discovers that my robot is spending a lot of time _pushing_ on
flat plates that aren't actually part of doors at all. I might need to tweak its door-detecting
heuristic by adding some more constraints: maybe it should try pushing only on
six-by-three-foot rectangles made of wood, for example.

Now switch modes. You're not a robot designer anymore; you're a _door_ designer. You want
to design a door for your office building. If you want my robot to be able to open it,
you'd darn well better make your door a six-by-three-foot wooden rectangle!

----

A generic algorithm — say, `template<Door D> void open(D& d)` — is like that robot.
The robot designer may well _overconstrain_ their robot's notion of "door-ness"
so that it does not even attempt to open many things that are in fact doors.

    template<class D>
    concept Door =
        (handle_of<D>::is_flat_plate || handle_of<D>::is_horizontal_bar) &&
        width_of<D> == std::feet(3) &&
        height_of<D> == std::feet(6);

    inline struct Robot {
        template<Door D> void open(D&);
    } robot;

    template<class T> constexpr auto is_openable(T&&, ...)
        { return false; }
    template<class T> constexpr auto is_openable(T& d)
        -> decltype(robot.open(d), true) { return true; }

Now, on the one hand, this is awkward and inconvenient for library users, because
[it generates](https://concepts.godbolt.org/z/BflGSu) a lot of false negatives:

    my::triangular_door t;
    robot.open(t);   // ERROR: Door<triangular_door> is false

    static_assert(not is_openable(t));  // SURPRISING

But the alternative would be worse: the alternative would be lots of _false positives!_
Suppose we removed the silly extra requirements from our `Door` and just used the
very basics: flat plate or horizontal bar, no size check. Then

    my::electrical_outlet o;
    robot.open(o);   // CASCADE OF ERRORS: Door<electrical_outlet> is true
                     // but it is not actually openable

    static_assert(is_openable(o));  // SURPRISING and possibly DEADLY

When the visual _appearance_ of an everyday object does not match its actual behavior —
when it seems visually to _afford_ some action that it does not actually support — our
robot gets confused.  Erring on the side of caution — not trying to open anything that
doesn't appear to be a six-by-three rectangle — is a good survival strategy.

When the syntactic _appearance_ of a C++ type does not match its semantic behavior —
when it seems syntactically to _afford_ some invariant that it does not actually provide —
our generic algorithms get confused. Erring on the side of caution — not trying to `accumulate`
anything that doesn't syntactically provide an `operator/`, unary `operator-`, and so on —
is a good survival strategy.
