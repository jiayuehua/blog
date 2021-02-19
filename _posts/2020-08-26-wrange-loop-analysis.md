---
layout: post
title: '`-Wrange-loop-bind-reference` and `auto&&`'
date: 2020-08-26 00:01:00 +0000
tags:
  c++-style
  compiler-diagnostics
  lifetime-extension
  llvm
---

Remember my old posts
["`for (auto&& elt : range)` Always Works"](/blog/2018/12/15/autorefref-always-works) (2018-12-15) and
["`for (auto&& elt : range)` Still Always Works"](/blog/2018/12/27/autorefref-still-always-works) (2018-12-27)?
Well, I've got another example, although I think it's more of a weird compiler deficiency than anything else...

[Godbolt:](https://godbolt.org/z/6MKP66)

    std::vector<bool> vec;
    for (const auto& elt : vec) {
        (void)elt;
    }

Clang trunk has an off-by-default diagnostic called `-Wrange-loop-bind-reference`, under the
also-off-by-default `-Wrange-loop-analysis`. This diagnostic is *not* included in `-Wall`, nor `-Wextra`.
But, if you deliberately enable it, Clang will start complaining:

    warning: loop variable 'elt' binds to a temporary value produced
    by a range of type 'std::vector<bool>' [-Wrange-loop-bind-reference]
        for (const auto& elt : vec) {
                         ^
    note: use non-reference type 'std::_Bit_reference'
        for (const auto& elt : vec) {
             ^~~~~~~~~~~~~~~~~

Clang wants you to acknowledge the lifetime extension here, and then eliminate it.
(See ["Field-testing 'Down with lifetime extension!'"](/blog/2020/03/04/field-report-on-lifetime-extension) (2020-03-04).)
I think Clang is wrong, though. Writing `for (auto elt : vec)` would _look_ like the programmer intended
to make a mutable copy of a `bool` object, but in fact they'd have made a write-through-able copy
of a `_Bit_reference` object! [Godbolt:](https://godbolt.org/z/z35Ez5)

    std::vector<bool> vec = ...;
    int i = 0;
    for (auto fizz : vec) {           // uh-oh!
        fizz = fizz || (++i % 3);     // this writes through!
        std::cout << (fizz ? "Fizz" : "Buzz");
    }
    // now vec's elements have been modified

So replacing `const auto&` with `auto` is likely to be a cure worse than the disease.

However, Clang does _not_ emit its diagnostic if you use the "`auto&&` Always Works"
forwarding reference! If you write

    std::vector<bool> vec;
    for (auto&& elt : vec) {
        (void)elt;
    }

then you're still lifetime-extending a temporary object of type `std::_Bit_reference`;
but now you're binding it to an rvalue reference of type `std::_Bit_reference&&`
instead of a const lvalue reference.
For whatever reason, Clang trunk considers this not to be a diagnostic-worthy offense,
and `-Wrange-loop-analysis` remains silent, which is what we wanted. We get to
write code that reflects our intent (to take a reference, not make a copy), but
without Clang's unhelpful warning.

In short: Even in the face of weird spammy compiler diagnostics, it appears that
`auto&&` Always Works!

See also ["What is clang's 'range-loop-analysis' diagnostic about?"](https://stackoverflow.com/questions/50066139/what-is-clangs-range-loop-analysis-diagnostic-about)
(Stack Overflow, April 2018).
