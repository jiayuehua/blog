---
layout: post
title: "Why do we require `requires requires`?"
date: 2019-01-15 00:02:00 +0000
tags:
  concepts
---

[Someone on StackOverflow just asked](https://stackoverflow.com/questions/54200988/why-do-we-require-requires-requires/)
about the silly-looking `requires requires` syntax that's used a few times
in the C++2a Working Draft and in the Ranges library implementation.
Since I had just written slides on this for
[my presentation at the NYC C++ meetup last week](https://www.meetup.com/nyccpp/events/256788175/),
and my explanation seemed well-received there, I gave an answer on SO. I'm
repeating it verbatim here, for visibility.
TLDR: `requires requires` is *not* grammatically insane;
but it *is* questionable style, and you should know how to eliminate it
from your code. We show all these things by analogy to `noexcept(noexcept(...))`.

----

We'll start with what you already know:
C++11 has "`noexcept`-clauses" and "`noexcept`-expressions." They do different things.

* A `noexcept`-clause says, "This function *should be noexcept when...* (some condition)."
    It goes on a function declaration, takes a boolean parameter, and causes a behavioral change
    in the declared function.

* A `noexcept`-expression says, "Compiler, _please tell me whether_ (some expression) is noexcept."
    It is itself a boolean expression. It has no "side effects" on the behavior of the program —
    it's just asking the compiler for the answer to a yes/no question. "Is this expression noexcept?"

We _can_ nest a `noexcept`-expression inside a `noexcept`-clause, but we typically consider
it bad style to do so.

    template<class T>
    void incr(T t) noexcept(noexcept(++t));  // NOT SO HOT

It's considered better style to encapsulate the `noexcept`-expression in a type-trait.

    template<class T> inline constexpr bool is_nothrow_incrable_v =
        noexcept(++std::declval<T&>());  // BETTER, PART 1

    template<class T>
    void incr(T t) noexcept(is_nothrow_incrable_v<T>);  // BETTER, PART 2

----

The C++2a Working Draft has "`requires`-clauses" and "`requires`-expressions." They do different things.

* A `requires`-clause says, "This function *should participate in overload resolution when...* (some condition)."
    It goes on a function declaration, takes a boolean parameter, and causes a behavioral change
    in the declared function.

* A `requires`-expression says, "Compiler, _please tell me whether_ (some set of expressions)
    is well-formed." It is itself a boolean expression. It has no "side effects" on the behavior
    of the program — it's just asking the compiler for the answer to a yes/no question.
    "Is this expression well-formed?"

We _can_ nest a `requires`-expression inside a `requires`-clause, but we typically consider
it bad style to do so.

    template<class T>
    void incr(T t) requires (requires(T t) { ++t; });  // NOT SO HOT

It's considered better style to encapsulate the `requires`-expression in a type-trait...

    template<class T> inline constexpr bool is_incrable_v =
        requires(T t) { ++t; };  // BETTER, PART 1

    template<class T>
    void incr(T t) requires is_incrable_v<T>;  // BETTER, PART 2

...or in a (C++2a Working Draft) concept.

    template<class T> concept Incrable =
        requires(T t) { ++t; };  // BETTER, PART 1

    template<class T>
    void incr(T t) requires Incrable<T>;  // BETTER, PART 2

----

See also: [A modest proposal for a GCC diagnostic](/blog/2018/04/09/long-long-long-is-too-long-for-gcc/).
