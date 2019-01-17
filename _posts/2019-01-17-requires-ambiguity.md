---
layout: post
title: "`requires`-expression grammar trivia"
date: 2019-01-17 00:01:00 +0000
tags:
  concepts
  language-design
  rant
---

A couple of footnotes to [my previous post
explaining `requires requires`](/blog/2019/01/15/requires-requires-is-like-noexcept-noexcept).

First: I confidently defend the rationale behind `noexcept(noexcept(...))` and `requires requires(...)`,
but that doesn't mean I like everything about the latter syntax. I believe that `requires`-*clauses*
are one of the best things about C++2a (at least, once someone sits down and figures out how they're going
to interact with name-mangling), but I believe `requires`-*expressions* are a complete garbage fire.

On that subject, here's a puzzle that some of my readers will have seen before, on Slack or
in [my CppCon talk](https://www.youtube.com/watch?v=CXn02MPkn8Y).
This is its blog debut. [Godbolt link:](https://concepts.godbolt.org/z/WeIAt-)

    template<class T>
    concept Negatable = requires(T t) {
        -t -> T;
    };

    static_assert(Negatable<char>);  // FAILS -- WHY?

The solution to this puzzle will be presented below.

----

Second: Reddit commenter Ameisen [writes:](https://www.reddit.com/r/cpp/comments/agd642/x/ee5zrve/)

> I feel like if the `requires` clause were given an expression, it should just
> treat it as an expression to evaluate...

which echoes Barry's original StackOverflow suggestion

> Why can't we just allow writing:
>
>     template<typename T>
>       requires (T x) { x + x; }
>         T add(T a, T b) { return a + b; }

The problem with this idea is that C++ is unparseable!
How do you know if you've been "given an expression" or not?
Consider

    template<class T> void f(T) requires requires(T (x)) { (void)x; };

With two `requires`es, the current C++2a working draft would parse this as
a function _declaration_ equivalent to

    template<class T>
    void f(T) requires (
        requires (T x) {
            { (void)x };
        }
    );

With one `requires`, it'd be parsed as a function _definition_ equivalent to

    template<class T>
    void f(T) requires T(x)
    {
        (void)x;
    }

    ;

(Here, both `x`s refer to some in-scope variable not shown in this snippet, such as
`constexpr int x = 42`. So `requires (T(x))` means "participates in overload
resolution when `x`, explicitly converted to `T`, is truthy."
[Compilers differ](https://github.com/saarraz/clang-concepts/issues/45)
on exactly what happens when `T(x)` is falsey, ill-formed, or non-constant.)

This example relies on several unnecessary quirks of the C++ grammar:

- A stray semicolon at file/namespace scope is simply ignored.

- A function parameter can redundantly be enclosed in parentheses.

- A _requirement-seq_ can contain _simple-requirements_ which lack the outer pair of curly braces
    (that is, `requires(T x) { (void)x; }` is treated as a valid `requires`-expression equivalent
    to `requires(T x) { { (void)x }; }`). This is crucial to my snippet because `{ (void)x; }` is
    a valid function body, whereas `{ { (void)x }; }` is not.

----

The third quirk above is a source of error for real programs. Did you figure out the
`Negatable` puzzler at the top of this post? Every time I present that puzzle, the very
first thing people say is, "Ooh, is this related to integer promotion?" — but no, it's
actually the missing curly braces!

Now, to be fair, Saar Raz's Clang branch actually gives a decent error message if you
try the `static_assert` exactly as written in this post:

    error: static_assert failed
    static_assert(Negatable<char>);
    ^             ~~~~~~~~~~~~~~~
    note: because 'char' does not satisfy 'Negatable'
    static_assert(Negatable<char>);
                  ^
    note: because '-t->T' would be invalid: member reference type 'char' is not a pointer
        -t -> T;
              ^

That's why I massaged the Godbolt example to use a SFINAE context, wherein the compiler never
feels any compunction to explain itself. (Or, for a similar effect, you can always use GCC. :))

Walter Brown and Casey Carter's [P1084 "Today's return-type-requirements are
Insufficient"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1084r2.pdf) (November 2018)
describes a previous suggestion that the "fat arrow" `=>` could
be introduced as a new token, in cases where the normal arrow `->` had had insufficient
or incorrect behavior (because its behavior _had_ been incorrect in most cases).
P1084's actual solution was to change the behavior of `->` so that it was more frequently correct.
However, I wonder if "fat arrow" should be introduced _anyway_, and mandated.

    template<class T>
    concept Negatable = requires(T t) {
        { -t } => T;    // unambiguous
        -t => T;        // unambiguous
        { -t } -> T;    // should be invalid: -> should never be the "convertible to" separator
        -t -> T;        // should be valid and unambiguous: -> should always be the arrow operator
    };

My _number one_ preference, though, is to see `requires`-expressions and `concept` definitions
completely removed from C++2a — or a commitment to postpone the release of C++2a until concepts are
ready for general use. Using `=>` instead of `->` would just be an incremental improvement: a
small band-aid on top of the garbage fire.

----

Even if C++ got rid of the "parenthesized parameter" ambiguity, new ambiguities are being
added to the language all the time. For example, Daveed Vandevoorde's
[P0632 "Down with `typename`!"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0634r3.html)
was recently adopted into the C++2a Working Draft. It introduces some new areas of ambiguity
into the grammar (cynic says: basically as [chaff](https://en.wikipedia.org/wiki/Chaff_(countermeasure))
to occupy some dev time for GCC and Clang while EDG catches up with implementing the
more important parts of C++2a).

    template<class T>
    void f(T) requires requires(X<T>::type && x) { (void)x; };

This would be unambiguously ill-formed C++2a code without P0632. With P0632,
`X<T>::type` would be treated as a type-expression because it happens to appear in a
position where a "parameter type" is expected. So, in C++2a as it stands right now,
P0632 having been adopted, this snippet would change meanings if one of the `requires`es were
removed.

    template<class T>
    void f(T) requires requires(X<T>::type && x) { (void)x; };

is a function declaration with the constraint `requires(typename X<T>::type&& x) { { (void)x }; }`.

    template<class T>
    void f(T) requires(X<T>::type && x) { (void)x; };

is a function declaration with the constraint `(X<T>::type && x)` and the function body
`{ (void)x; }`.

...well, okay, P0632 actually *doesn't* say that `typename` is optional in this particular
context, but I'm pretty sure that's a defect in the current draft (which I just reported
to the Core Working Group earlier today). When we have so many
disparate and fairly invasive features going into the same document from different directions,
it's easy for those features to fail to play together. And the Committee doesn't give itself
much "fudge time" to iron out these details, either — people are still confidently calling it
"C++20" — as in 2020 — even as we enter the year 2019, with no vendor having completed an
implementation of C++17 yet!
