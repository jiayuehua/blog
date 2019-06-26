---
layout: post
title: 'Comparison categories for narrow-contract comparators'
date: 2018-08-07 00:01:00 +0000
tags:
  concepts
  contracts
  operator-spaceship
  undefined-behavior
---

WG21's Library Working Group is studying the wording for Marshall Clow's
[P0805R2 "Comparing Containers."](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0805r2.html)
The basic idea is that since C++ permits us to write

    float f = 3.14;
    int i = 42;
    assert(f < i);

then C++2a should permit us to write

    std::vector<float> vf = { 3.14 };
    std::vector<int> vi = { 42 };
    assert(vf < vi);

Notice that P0805 does not interact at all with the brand-new C++2a feature of "operator spaceship,"
`<=>`. (And this limited scope is good, IMHO.) But if, sometime before the C++2a ship date, we *want*
to explore those ramifications... well, C++2a permits us to write

    float f = 3.14;
    float g = 3.15;
    static_assert(std::is_same_v<decltype(f <=> g),
                                 std::partial_ordering>);

and so C++2a should also permit us to write

    std::vector<float> vf = { 3.14 };
    std::vector<float> vg = { 3.15 };
    static_assert(std::is_same_v<decltype(vf <=> vg),
                                 std::partial_ordering>);

But to do this (and notice that this is only the *homogeneous* case!) we'd have to relax
[the constraints](http://eel.is/c++draft/container.requirements#tab:containers.optional.operations) on
`vector`'s `operator<` and permit it to be instantiated with types whose `operator<`
does not provide a "[total ordering](https://en.wikipedia.org/wiki/Total_order) relationship."
That is, *technically* right now `vf < vg` has undefined behavior; but it doesn't matter because it
does the right thing in most cases (and in those cases where it doesn't do the right thing — i.e.,
in cases involving NaN — the programmer simply doesn't try to use it).

But in the operator-spaceship world of C++2a, this undefined behavior is suddenly brought to the
surface. We now have a standard way to *detect* whether a type's `operator<` provides a total
ordering relationship: we just examine `decltype(declval<T>() <=> declval<T>())` and compare
it to `std::strong_ordering`. So theoretically `vector<float>` could now `static_assert`, if it
wanted to. (Of course vendors won't really do that, but they *could*.)

----

How does this relate to [the Lakos Rule](/blog/2018/04/25/the-lakos-rule)?

Before C++11, we had no way to *warrant* to the compiler that a given function never threw exceptions.
In C++11, we gained that ability. We can mark

    int alpha(int i) noexcept(true) {
        return i;
    }

    int beta(int i) noexcept(false) {
        return i ? 1/i : throw "oops";
    }

But what do we do with

    int gamma(int i) noexcept(????) {
        return 1/i;
    }

It never *throws*, per se, but it does sometimes have *undefined behavior*.
[The Lakos Rule tells us](/blog/2018/04/25/the-lakos-rule) that this kind of narrow-contract function
should *not* be marked noexcept (that is, it should be `noexcept(false)`).

Stepanov's
[''Fundamentals of Generic Programming'' (1998)](http://stepanovpapers.com/DeSt98.pdf)
raises in my mind the possibility of a similar rule for comparators. Consider:

    std::strong_ordering one(int i, int j)
    {
        // Comparing numbers with different signs is perfectly fine.
        return i <=> j;
    }

    std::partial_ordering two(int i, int j)
    {
        // Comparing numbers with different signs is
        // well-defined, and yields "unordered."
        if ((i < 0) != (j < 0))
            return std::partial_ordering::unordered;
        return i <=> j;
    }

    auto three(int i, int j) -> ????
    {
        // Comparing numbers with different signs is a throwing offense.
        if ((i < 0) != (j < 0))
            throw "oops";
        return i <=> j;
    }

    auto four(int i, int j) -> ????
    {
        // Comparing numbers with different signs is undefined behavior.
        if ((i < 0) != (j < 0))
            return (1/0) <=> 0;
        return i <=> j;
    }

Somewhere between `three` and `four`, a "Lakos-ish" rule for comparison categories
would suggest that maybe this kind of comparator should no longer be considered
a "total order" from the point of view of the compiler (or the programmer, for that
matter). Should we mark `four` as returning `std::strong_ordering`, or would it in
some sense be more honest to mark it as `std::partial_ordering`?

"Of course not! It never returns `unordered`, so its range of possible return values
is exactly the range of `std::strong_ordering`! Claiming it returns `partial_ordering`
is basically lying to the reader."

Sure, agreed. But then, function `gamma` above never throws an exception! Claiming that it
might throw an exception is basically lying to the reader.

"But `gamma` might check its preconditions and throw in the UB case!"

But `four` might check its preconditions and return `unordered` in the UB case!

----

Is this exercise purely academic?  No. Consider:

    auto five(int *p, int *q) -> ????
    {
        // Comparing unrelated pointers yields an unspecified result.
        return p <=> q;
    }

    auto six(std::deque<int>::iterator p, std::deque<int>::iterator q) -> ????
    {
        // Comparing iterators into different deques has undefined behavior.
        return p < q ? std::strong_ordering::less :
               p > q ? std::strong_ordering::greater :
                       std::strong_ordering::equal;
    }

([Source for "unspecified result."](http://eel.is/c++draft/expr.spaceship#8)
[Source for "undefined behavior"](http://eel.is/c++draft/random.access.iterators#tab:iterator.random.access.requirements) —
but notice that the table claims "`<` is a total ordering relation" even as an earlier row
asserts that not all values are comparable via `<`.)

C++2a has decided that `five` should yield `std::strong_ordering` — comparison of pointers
is considered "strong" even though it is not technically a *total* order (many pointer values
are not ordered, or not comparable, with others).

C++2a has not yet had to deal with `six`, because no standard library types yet support
`operator<=>` (except for `std::vector<T>::iterator` on implementations where that's a typedef
for `T*`).

----

Personally, I find the Lakos Rule for `noexcept`
[easy to explain](/blog/2018/04/25/the-lakos-rule/#each-library-function-having-a-w)
but [annoying in practice](/blog/2018/04/25/the-lakos-rule/#personally-i-am-not-a-fan-of-thi);
I would like to see `noexcept` used more liberally.

However, today I think I do want a "Lakos-ish Rule" for comparisons; I would like to see
`strong_ordering` used more conservatively. Well, really I just want people to *stop designing types without total orders!*

But there's clearly a _reductio ad absurdum_ for "Lakos-ish Rules":

    int seven(int i) {
        return 1 / i;
    }

The absurdist might claim that `int` is the wrong return type for function `seven`, because it returns an `int`
only in the *well-defined* case. In the UB case, "it might return anything." Clearly, then, any function with a narrow
contract should not only be marked `noexcept(false)`, but also have `std::any` as its return type!

And the counterargument reduces to absurdity my position on comparisons: the absurdist doesn't literally want
everyone to write narrow-contract functions that return `std::any`; really they just want people
to *stop designing functions with narrow contracts!*
