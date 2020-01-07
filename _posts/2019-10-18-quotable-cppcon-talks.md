---
layout: post
title: "CppCon 2019 talks are up"
date: 2019-10-18 00:01:00 +0000
tags:
  cppcon
  slogans
---

I've been watching a lot of CppCon talks on YouTube over the past couple of weeks.
I heard they're going online at the rate of about seven per day.

Incidentally, all of the Back to Basics track is uploaded now! I have updated my
original post ["Back to Basics at CppCon 2019"](/blog/2019/09/12/cppcon-2019-b2b-track/)
(2019-09-12) to include direct links to each of the Back to Basics videos on YouTube.

And all of my talks are captioned (by me)! This includes my talks on
[RAII](https://www.youtube.com/watch?v=7Qgd9B1KuMQ),
[smart pointers](https://www.youtube.com/watch?v=xGDLkt-jBJ4),
[lambdas](https://www.youtube.com/watch?v=3jCOwajNch0),
[type erasure](https://www.youtube.com/watch?v=tbUCHifyT24), and
[the ease of breaking container invariants in C++](https://www.youtube.com/watch?v=b9ZYM0d6htg).

Here are some quotable highlights from some of the videos I've watched so far.

----

Louis Dionne, ["The C++ ABI from the Ground Up"](https://youtube.com/watch?v=DZ93lP1I7wU&t=2m04s):

> I like to think of ABI as being like API, but for machine code.

By the way, "ABI" and "API" are now both in my
[C++ acronym glossary](/blog/2019/08/02/the-tough-guide-to-cpp-acronyms/#abi-api) (2019-08-02).

----

Andrzej Krzemieński, ["Error Handling is Cancelling Operations"](https://www.youtube.com/watch?v=zte8IxkHqc4&t=19m02s):

> We do not throw to say that something failed.
> We throw to say that something must be canceled.

That is, the point of throwing an exception is to abort subsequent control flow that might depend
on our not-having-errored. The point is not simply to report _that_ we errored.

I'm not sure I agree with Andrzej's thesis — I mean, at least it seems to be subjective and thus
dependent on the programmer's powers of imagination. ("I can't imagine that anyone would do an operation
conditional on my success in closing this socket; therefore I won't throw to report failure in closing
this socket.") However, it is _definitely_ worth thinking about.

----

Jon Kalb, ["Back to Basics: Object-Oriented Programming"](https://www.youtube.com/watch?v=32tDTD9UJCE&t=26m07s):

> Public virtual functions have two responsibilities.
> They have to define the calling interface (for callers),
> but also the inheritance interface (for derivers).

This violates the separation of concerns ("Give one entity one cohesive responsibility").
The solution, as Jon says, is the
[Non-Virtual Interface idiom](/blog/2019/08/02/the-tough-guide-to-cpp-acronyms/#nvi).

Jon also [points out](https://www.youtube.com/watch?v=32tDTD9UJCE&t=38m29s), correctly,
that you should never "override" a defaulted function parameter in a derived class.
He fails to mention that defaulted function parameters are the devil. Also, notice that
if you follow the NVI idiom, you will never call a virtual function with a defaulted parameter,
because in NVI the base class controls all the direct call-sites of the virtual function.
So again NVI protects us from all sorts of pitfalls.
