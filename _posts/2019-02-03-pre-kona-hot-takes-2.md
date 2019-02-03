---
layout: post
title: "Hot takes on P1452, P1470, P0408, and P0843"
date: 2019-02-03 00:01:00 +0000
tags:
  concepts
  kona-2019
  library-design
  rant
  wg21
---

The pre-Kona WG21 mailing has been out for a little while. Tonight I looked at a few papers;
here's my hot takes. (This is round 2. Round 1 is [here](/blog/2019/01/28/pre-kona-mailing-hot-takes-1).)

----

[P1452R0 "On the non-uniform semantics of <i>return-type-requirement</i>s"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1452r0.html):
Points out that in the current C++2a working draft,

    template<class T>
    concept Fooable = requires(T t) {
        { t+1 } -> int;
    };

means "`t+1` should have a type convertible to `int`," but,

    template<class T> concept Int = std::is_same_v<T, int>;

    template<class T>
    concept Fooable = requires(T t) {
        { t+1 } -> Int;
    };

means "`t+1` should have a type _which is exactly_ `Int`, i.e., `int`."
Meanwhile, [P1141 "Yet another approach for constrained declarations,"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1141r2.html#wordingdcl)
which was adopted at the San Diego meeting in late 2018, has introduced the idea of "placeholder types with type-constraints,"
as in the abbreviated function template `void f(Int auto x)`. P1141 specifically *does not* propose that constrained placeholder
types should be permitted in _trailing-return-types_, so in C++2a right now we have

    auto f(Int auto x) { return x; } // OK

    Int auto f(Int auto x) { return x; } // FORBIDDEN

But if we did gain the ability to use constrained placeholder types everywhere
we can currently use unconstrained placeholder types, then we'd naturally expect
to be able to write

    template<class T>
    concept Fooable = requires(T t) {
        { t+1 } -> Int auto;
    };

which would mean "`t+1` should have a type which _when decayed_ is exactly `Int`, i.e., `int`,"
because that's what `-> Int auto` would mean as an actual _trailing-return-type_.

So we have these three (two current, one hypothetical) meanings for `->` in a `requires`-expression.
P1452 says, let's just rip the `->` syntax out of C++2a for now, and bring it back when we have more
of an idea what it's supposed to mean. Anyone who needs the "exactly models" behavior can write

    template<class T>
    concept Fooable = requires(T t) {
        requires Int<decltype(t+1)>;
    };

while anyone who needs the "is convertible to" or "when decayed, models" behavior can write

    template<class T>
    concept Fooable = requires(T t) {
        { [](Int auto){}(t+1) };
    };

Since I don't like putting things into the C++ Standard without knowing what they mean, I support P1452.

----

[P1470R0 "Against a standard concurrent hashmap"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1470r0.pdf):
This was submitted as a rebuttal to [P0652 "Concurrent associative data structure with unsynchronized
view"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0652r2.html), but the first four pages could
have been submitted under the title "What belongs in the Standard Library?" and should be required reading for
everyone involved in WG21.

David Goldblatt's paper lays out five "unscientific, non-exhaustive" categories of things that
have gone into the Standard:

> To try to expand on this a little, I’ll unscientifically and non-exhaustively
> categorize various types of things that get (or could get) standardized.
>
> Hooks into the core language or runtime [such as `initializer_list` and `exception_ptr` ...]
> These should be included because their functionality is important to end-users, and
> wouldn’t be accessible to end-users otherwise.
>
> Vocabulary [such as `swap`, `string`, allocators, `optional`, `Lockable`, ...]
> These should be included because they enable library composition.
> Lots of libraries need some notion of a dynamic array with amortized
> constant time appends. But if we don’t pick a single name for it,
> user code won’t be able to take the result of one such library and pass it to another.
>
> Portability [such as `<filesystem>`, `mutex`, clocks, ...]
> Sometimes important functionality is exposed by a wide variety of
> different implementations in a conceptually similar way that results
> in API differences. Standardizing these abstractions lets users write
> code without worrying about platform specifics. [...]
>
> Simple APIs for simple, common problems [such as `sort` ...]
> These allow a nice “out of the box” experience for the language.
> It’s irritating and unfortunate if students or newcomers need to
> find and install a set of utility libraries just to do basic CS 101 tasks.
>
> “Batteries included” APIs [such as `<random>`, `<regex>`, locales ...]
> This is the riskiest category of things to standardize; there are
> hard-to-reason-about tradeoffs involved. If we get it wrong, will
> users make mistakes? How important is the problem? What are the user’s
> best alternatives? Do we have sufficient domain expertise? Indeed,
> many of the APIs commonly considered mistakes fall into this category.
> I’m thinking of locales, regexes, and iostreams manipulators, which
> many regard as poorly performing, hard to use, and, at least in
> regex’s case, buggy.

I'm almost positive he's right about the narrow case of P0652's `concurrent_unordered_map`
— I can't imagine how that would ever fall into WG21's purview — but I think the list
of categories above is much more important than _just_ an entry in the hashmap debate.
This is good stuff, and I hope it gets wider exposure.

----

[P0408R6 "Efficient Access to `basic_stringbuf`'s Buffer"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0408r6.pdf):
This paper has been bouncing around LEWG and LWG for a long time and really just needs to
happen already. (This is the paper that would make `std::move(oss).str()` on an `ostringstream` actually
Do The Right Thing.)

On this reading, I noticed that P0408 also includes a drive-by fix for `basic_stringbuf::swap`,
which is [somehow, bizarrely, `noexcept(false)`](https://en.cppreference.com/w/cpp/io/basic_stringbuf/swap)
— I guess `basic_stringbuf` missed the [P0884](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0884r0.pdf) memo!
I feel like getting a proper noexcept specification for `swap` ought to be an almost editorial LWG issue;
it seems weird that it's lumped in with the rest of this inexplicably slow-moving paper.

Ship it already!

----

[P0843R3 "`static_vector`"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0843r3.html):
This is the container [properly known as `fixed_capacity_vector`](/blog/2018/06/18/inplace-vs-fixed-capacity).
From the paper I get the impression that LWG has been pushing hard for some weird design decisions,
and the paper author has been correctly pushing back. He resisted advice to add a hard dependency
on `abort()` (instead, if you `push_back` on a full buffer, you get undefined behavior — which can be
expressed as a "contract violation" [as long as Contracts stay in the language](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1426r0.md)).
He resisted advice to merge `<static_vector>` into the existing `<vector>` header (which is already
[fairly expensive](http://virtuallyrandom.com/c-headers-are-expensive/); even today, non-Microsoft vendors
[resist putting `std::pmr::vector` into `<vector>`](https://godbolt.org/z/yUZ-YA) in order to save on
compilation times).

I'm still weakly against this paper as a whole, unless the name `static_vector` can be changed
— for reasons detailed in [my previous blog post on naming](/blog/2018/06/18/inplace-vs-fixed-capacity).
But kudos to the author for having gotten everything _but_ the name right! :)

I notice a few minor typos in the wording. Also one surprising omission: P0843R3's `static_vector` is guaranteed to
preserve trivial destructibility (that is, `static_vector<T>` is trivially destructible iff `T` is trivially
destructible), but it is not guaranteed to preserve trivial move-constructibility. I believe it should.
