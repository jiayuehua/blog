---
layout: post
title: "Four versions of Eric's Famous Pythagorean Triples Code"
date: 2019-03-06 00:02:00 +0000
tags:
  coroutines
  metaprogramming
  ranges
  sufficiently-smart-compiler
---

Back in December, Eric Niebler made some waves in the C++ twitterverse with a post titled
["Standard Ranges"](http://ericniebler.com/2018/12/05/standard-ranges/) (2018-12-05).

[Here is Eric's code](https://godbolt.org/z/BHODEL) ([backup](/blog/code/2019-03-06-triples-ranges.cc)), modified to use the range-v3 library,
and with all the concepts/constraints removed since they use syntax that — while valid C++2a —
has never yet been implemented in either GCC or Clang. The constraints in his original code were there
just for documentation; they weren't being used for any functional reason, such as to control overload
resolution. Removing the constraints means the compiler doesn't have to check them,
so if anything it should compile *faster.*

This code is 74 lines long (or 66, if you remove the `using`-directives I added). It takes 10.5 seconds
to compile on Godbolt Compiler Explorer, and produces 715 lines of machine code.
You can [run it on Wandbox](https://wandbox.org/permlink/A463wD3MB6VIwZnO) and see that it produces
the correct answer.

----

[Here is a Coroutines version of the same code](https://godbolt.org/z/icY6RZ) ([backup](/blog/code/2019-03-06-triples-coro.cc)).

This code is 53 lines long. It takes 2 seconds to compile on Godbolt Compiler Explorer, and
produces 437 lines of machine code.
You can [run it on Wandbox](https://wandbox.org/permlink/uZI03C6UkLP3GMl4) and see that it produces
the correct answer.

----

[Here is a C++17 callback-based version of the same code](https://godbolt.org/z/lHrSN2) ([backup](/blog/code/2019-03-06-triples-callback.cc))

This code is 50 lines long. It takes 2 seconds to compile on Godbolt Compiler Explorer, and
produces 93 lines of machine code.
You can [run it on Wandbox](https://wandbox.org/permlink/eY20LSEEuE8VEn6f) and see that it produces
the correct answer.

----

[Here is a C++17 "vanilla" version of the same code](https://godbolt.org/z/IH14zJ) ([backup](/blog/code/2019-03-06-triples-vanilla.cc)).

This code is 16 lines long. It takes 0.2 seconds to compile on Godbolt Compiler Explorer, and
produces 59 lines of machine code.
You can [run it on Wandbox](https://wandbox.org/permlink/m02zKulIgyi488Td) and see that it produces
the correct answer.

----

Notice that all of these solutions to the Pythagorean Triples problem have the same essential form:
three nested `for` loops, with an `if` at the innermost level to do the filtering-out, and then a
stateful "counter" that goes from 1 to 10 and then bails out of the entire looping structure.

The callback-based and Ranges versions abstract out the "counter" into a filter named `take` that
fits into a general "pipeline" approach. My Coroutines approach promotes the counter to the top-level `for` loop,
and stuffs all the other logic into the generator (which is very similar to what Eric's Ranges version does).
My "vanilla" version treats the counter as a filter (i.e., an `if`) that naturally fits right next to the
other filter that checks for the Pythagorean-ness of each triple.

Each approach has its similarities and differences with the others, and its corresponding upsides and downsides.
All I would recommend is that whatever job you have to do, you should try to use the right tool for it. :)
