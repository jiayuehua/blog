---
layout: post
title: "`view_interface` types are boolean-testable"
date: 2021-12-06 00:01:00 +0000
tags:
  c++-style
  ranges
  today-i-learned
---

Today I learned:
[`std::ranges::view_interface`](https://eel.is/c++draft/view.interface.general)
provides an `explicit operator bool`. When you have a view type from the Ranges STL
(such as `std::ranges::subrange`), you can branch on its boolean value: empty
ranges are falsey and everything else is truthy.

    namespace rv = std::views;

    std::vector<int> v = {1,2,3,4,5};
    auto isOdd = [](int x) { return (x % 2) != 0; };
    if (v | rv::filter(isOdd)) {
        puts("v contains at least one odd number");
    }

A common stumbling block for Ranges-learners (in which I still include myself)
is that the Ranges version of `remove_if` returns a `subrange` instead of
just an iterator... but the range it returns isn't the new range of valuable
elements, it's the range of trailing junk elements!

Since views are boolean-testable, and `subrange` is a view type,
testing the return value of `remove_if` tells you whether any elements
were removed:

    if (auto rr = std::ranges::remove_if(v, isOdd)) {
        puts("I just removed at least one odd number from v");
        v.erase(rr.begin(), rr.end());
    }

Using STL Classic, you'd have had to write a separate test `it != v.end()`
after the call to `remove_if`, like this:

    if (auto it = std::remove_if(v.begin(), v.end(), isOdd); it != v.end()) {
        puts("I just removed at least one odd number from v");
        v.erase(it, v.end());
    }

This is probably not the _primary_ reason that Ranges algorithms tend to return
ranges instead of iterators. In fact, honestly, please don't use this in your codebase!
But it's still kind of nifty to know about, and it might be a useful mnemonic
for the return values of certain Ranges algorithms (such as `remove_if` and `unique`).

> The `if (auto x=y; z)` syntax is new in C++17: see
> ["`while (auto x=y; z)`"](/blog/2020/10/28/while-with-initializer/) (2020-10-28).
> And note that in both snippets the second argument to `v.erase` is critically important:
> ["How to erase from an STL container"](/blog/2020/07/08/erase-if/) (2020-07-08).

This nifty trick relates only to views that inherit their `operator bool`
from `view_interface` — not to arbitrary ranges such as `vector<int>`,
and not even to arbitrary view types such as `string_view` or `span`.
But it does apply to `subrange`, and also to many types returned from range
adaptor pipelines. I suppose this is where I mention that thanks to C++20's
[P1252](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1252r2.pdf)
and [P1739](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1739r4.html),
even most range adaptors don't _always_ return `view_interface` types...

    if (auto r = "foo"sv | rv::reverse)               // OK, r is reverse_view<string_view>
    if (auto r = "foo"sv | rv::reverse | rv::reverse) // Error, r is string_view

    if (auto r = v | rv::drop(1))             // OK, r is drop_view<ref_view<vector<int>>>
    if (auto r = std::span(v) | rv::drop(1))  // Error, r is span<int>

But that's a story for another day.
