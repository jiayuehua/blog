---
layout: post
title: "C++ Pub Quiz at CppCon 2019"
date: 2019-09-21 00:01:00 +0000
tags:
  conferences
  cppcon
---

Well, that's a wrap! [CppCon 2019](https://cppcon.org/cppcon-2019-program/) is pretty much
over for me; I've done [my pre-conference weekend class](https://cppcon.org/class-2019-stl-from-scratch/)
and [my four "Back to Basics" talks](/blog/2019/09/12/cppcon-2019-b2b-track/) and my
one non-B2B talk (based on [this blog post from February](/blog/2019/02/24/container-adaptor-invariants/))...
and also the perhaps-inaugural "C++ Pub Quiz."

What is C++ Pub Quiz, I hear you ask? Well, I got the idea after hearing that they do a
"pub quiz" at the ACCU conference. Turns out that
[their idea of "pub quiz" is vastly different from mine](http://jonjagger.blogspot.com/2017/05/accu-c-countdown-pub-quiz.html),
but I didn't find that out until later. Mine was a straightforward trivia night — five rounds,
ten questions per round, write down your team's answers on a sheet of paper and turn it in
at the end of the round. High score after five rounds wins the game. (I had prepared ten
rounds' worth of questions, so we played two games with a refueling break in the middle.)

It took all week for me to figure out if I should try to do this Open-Content-ish event.
But I'm very glad I did!
(We did have some snafus finding a suitable space. Pro tip: Late night after the closing keynote
is a good time to organize a casual event, but also a bad time to find empty rooms that
we're still technically allowed to use.)

Four teams participated in Pub Quiz. Thanks to (from left to right) Team Unknown Behavior,
Team Undefined Behavior, Julio Iglesias, and Team Jan!
Thanks also to Katrina Siegfried for helping organize and for taking the much higher-quality
original of this photo.

![Photo credit: Katrina Siegfried](/blog/images/2019-09-21-pub-quiz.jpg)

Our winning team of the night was
Team Undefined Behavior — which over the course of the night I learned contained both a
Princeton classmate of Raymond Chen's _and_ the Swedish national Mahjong champion!

A sample round of C++ Pub Quiz might look something like this:

1. In C++17, what is the type of the expression `L""`? [SAY: "capital ell, double-quote, double-quote."]

2. True or false: When `x` is of type `int`, the compiler may assume that `x+1-1` equals `x`.

3. What C++ feature did Sean Parent once call "the base class of evil"?

4. Which of the following is not an access control specifier in C++? `public`, `private`, `package`, `protected`.

5. There are two ways to write a type alias: old-school `typedef` syntax and modern `using` syntax. Using either of the two syntaxes, write a single declaration that defines `PA` as a type alias for “pointer to array of function pointers, each taking `int` and returning `bool`.”

6. The standard types `unique_ptr` and `shared_ptr` are commonly known as what kind of pointers?

7. The standard function objects for addition and subtraction are called `std::plus` and `std::minus`, respectively. What is the standard function object for multiplication?

8. The dawn of template metaprogramming was in 1994, when a German member of the ISO Committee wrote a program that printed the prime numbers as an infinite sequence of compiler error messages. Name the author of that first prime-printing metaprogram.

9. What is the base-10 representation of two to the thirty-first power?

10. Bjarne Stroustrup's 1985 book _The C++ Programming Language_ was consciously modeled on a 1978 book titled _The C Programming Language_ and popularly known as "K&R," after its two authors. For one point, name both authors of "K&R." First and last names, please. For one extra bonus point, give the middle initials of both authors.

Round 3 of each game was the brainstorm round. For example:

> The current C++ standard, C++17, defines 11 core language keywords beginning
> with the letter "C". Name as many of these keywords as you can. Your score
> for this round will be the number of keywords you name correctly, minus
> the number you name incorrectly (if any).

We even worked in a "music round" with surprisingly few technical difficulties.
I'd cue up a YouTube video of a conference talk at some pre-selected point,
play a snippet of the audio, and ask teams to write down — for one point each —
the presenter and the title of the talk.
([Example.](https://www.youtube.com/watch?v=vwrXHznaYLA&t=36m38s))
Team Undefined Behavior did _ridiculously_ well on this round, by the way!

In short, C++ Pub Quiz was a great success, and I think it just _might_ become a recurring
event at future CppCons.

----

If you run a local C++ user group — send me an email with 10 pub quiz questions of your own making,
and I'll send you the complete ten-round question set that I used at CppCon 2019.
