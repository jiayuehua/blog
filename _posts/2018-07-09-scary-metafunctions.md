---
layout: post
title: 'SCARY metafunctions'
date: 2018-07-09 00:02:00 +0000
tags:
  compile-time-performance
  library-design
  metaprogramming
---

I just watched [Odin Holmes](https://odinthenerd.blogspot.com)'s half-session from C++Now 2017 titled
["Type-Based Template Metaprogramming is Not Dead."](https://www.youtube.com/watch?v=EtU4RDCCsiU)
Honestly, I found it mostly pretty hard to follow. (Note to presenters: The video
recording will pick up your mouse cursor, but not your laser pointer!) But there
was one "oooh" moment right around 15 minutes in. Context: compile-time performance.

Odin shows this classic code:

    template<bool B, class T, class F>
    struct conditional {
        using type = T;
    };

    template<class T, class F>
    struct conditional<false, T, F> {
        using type = F;
    };

    template<bool B, class T, class F>
    using conditional_t = typename conditional<B, T, F>::type;

And then he flips to the next slide:

    template<bool B>
    struct conditional {
        template<class T, class F> using f = T;
    };

    template<>
    struct conditional<false> {
        template<class T, class F> using f = F;
    };

    template<bool B, class T, class F>
    using conditional_t = typename conditional<B>::template f<T, F>;

Mind: blown.

The "classic" approach is so obviously inefficient by comparison; but
this is the first time I've seen Odin's approach presented. I wonder if
there's any chance of getting `std::conditional_t` defined this way in
the actual Standard.

----

The title of this post is a reference to [SCARY Iterators](http://www.open-std.org/jtc1/sc22/WG21/docs/papers/2009/n2911.pdf),
which apply the same "Don't Repeat Yourself" approach but in a less "meta" area of template programming.

> The acronym <b>SCARY</b> describes assignments and initializations that are <b>S</b>eemingly erroneous
> (appearing <b>C</b>onstrained by conflicting generic parameters), but <b>A</b>ctually work with
> the <b>R</b>ight implementation (unconstrained b<b>Y</b> the conflict due to minimized dependencies).
