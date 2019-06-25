---
layout: post
title: '"The STL From Scratch" is back!'
date: 2019-06-21 00:01:00 +0000
tags:
  conferences
  cppcon
---

Registrations for CppCon 2019 are open, and so are registrations for my two-day
pre-conference class, "[The STL from Scratch](https://cppcon.org/class-2019-stl-from-scratch/)."

"The STL from Scratch" arose from my series of CppCon talks such as
"[Lambdas from Scratch](https://www.youtube.com/watch?v=WXeu4fj3zOs)" (2015),
"[Futures from Scratch](https://www.youtube.com/watch?v=jfDRgnxDe7o)" (2015),
and "[`dynamic_cast` from Scratch](https://www.youtube.com/watch?v=QzJL-8WbpuU)" (2017).
Each of these talks takes an initially "scary" and "magic" feature of C++ and breaks it down to show how
it can in fact be implemented by an intermediate-level programmer. Once CppCon started running pre- and
post-conference training courses, I decided to offer a course covering the entire STL in this
style.

Now, it does turn out that the C++ standard library is too big to cover in two days! :) So the course
ends up covering the "most magic" highlights, on various axes of magic. On one axis, I show how to implement
`mutex` from scratch in terms of Linux futexes. On another, I show how `tuple` and `variant` are implemented,
and how `visit` and `tuple_cat` share the same underlying constexpr techniques.

The course comes with lab exercises after each lecture. In 2017 and 2018, the exercises were as follows:

* Test your knowledge of lambdas.
* Modify an implementation of `std::function` to create an implementation of `std::any`.
* Find and fix a concurrency bug.
* Implement `shared_ptr`'s atomic reference counting.
* Modify an implementation of `future` to create an implementation of `shared_future`.
* Implement `future::then`.
* Implement a new [affordance](/blog/2019/03/18/what-is-type-erasure/) in a small-buffer-optimized `unique_function`.
* Implement `tuple_cat`.
* Implement `std::count`, and consider the pros and cons of taking `const value_type&` versus `const T&`.
* Test your knowledge of `std::hash` specializations and heterogeneous comparators.

In 2019, I plan to mix up the second day a bit and replace some of the lectures and exercises.
Heck, this might be the year I start doing a unit on
"[Niebloids](https://en.cppreference.com/w/cpp/algorithm/ranges/all_any_none_of) from Scratch."

-----

If you're reading this blog entry because you like my blogging and/or my conference videos — or because you've
attended one of my other courses in the past — I bet you would enjoy the CppCon 2019 edition of "The STL from Scratch"!
You can sign up [here](https://cppcon.org/registration/).

* The five-day CppCon itself has a price tag of $1400.
    With Early Bird registration, that drops to $1150.
    The deadline for Early Bird registration is June 30th!

* My class is offered pre-conference (September 14 and 15). All pre- and post-conference classes
    have the same price tag: $1000 for two days. This includes lunch, which in the past
    has been a _very_ good sit-down affair. There is no Early Bird discount for classes.

* You can attend both a pre-conference class and a post-conference class, if you want! There are
    [13 pre-conference offerings](https://cppcon.org/classes-2019/) and [7 post-conference offerings](https://cppcon.org/classes-2019/),
    making this CppCon's busiest year yet.

* This is CppCon's first year in Aurora, CO instead of Bellevue, WA. Remember this when making your travel arrangements!

Many companies gladly pay for their employees to attend CppCon, because of how educational the main conference
can be — both the sessions and the hallway discussions between. If your company is already sending you to CppCon,
see if they'll add on an extra two days of professional-quality training.

If your employer _isn't_ paying your way to CppCon, maybe you'll sway their opinion by pointing out
that you have the opportunity to take a professional-quality training course. Suppose your employer
sponsors just that course (plus your travel and lodging), leaving you with $1150 out of pocket; that's
still a cheaper deal for you — and a better deal for your employer, honestly! — than if you went alone,
paid for everything yourself, and _didn't_ take the course.

What are you waiting for? Early Bird registration closes on June 30th!

-----

P.S. — If you submitted a session proposal to CppCon 2019, then don't worry: submitters always get the Early Bird price.
This is always special-cased for submitters because many submitters base "will I attend?" on "is my session accepted?"
and accept/reject decisions aren't announced until after the Early Bird deadline has passed for everyone else.

People who buy Early Bird tickets are indeed buying [a pig in a poke](https://en.wikipedia.org/wiki/Pig_in_a_poke) — since
the program isn't announced until after the deadline, Early Birds don't know what sessions they'll be coming to see.
But I can assure you that 2019 is going to be a _great_ year for CppCon's program. There is literally _no chance_ that
Early Birds will be unpleasantly surprised by the program when it's announced.

The quality of previous years' programs will be a good guide to this year's.
See the schedule for [2018](https://cppcon2018.sched.com), [2017](https://cppcon2017.sched.com),
[2016](https://cppcon2016.sched.com), [2015](https://cppcon2015.sched.com), [2014](https://cppcon2014.sched.com).

-----

P.P.S. — Would you or your employer like to hire me to deliver "The STL From Scratch," "Intro to C++," or any other
professional training course in my repertoire _without_ flying everyone to Colorado first?
Contact me via the [email link](mailto:arthur.j.odwyer@gmail.com) below!
