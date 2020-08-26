---
layout: post
title: "`for (auto&& elt : range)` Still Always Works"
date: 2018-12-27 00:01:00 +0000
tags:
  c++-style
  ranges
---

Just to drive home the point I made in my previous post
["`for (auto&& elt : range)` Always Works"](/blog/2018/12/15/autorefref-always-works) (2018-12-15),
here's an example from elsewhere in the C++ blogosphere.
[Eric Niebler writes](http://ericniebler.com/2018/12/05/standard-ranges/)
one of his for-loops like this:

    // Display the first 10 triples
    for (auto triple : triples | view::take(10)) {
        cout << '('
             << get<0>(triple) << ','
             << get<1>(triple) << ','
             << get<2>(triple) << ')' << '\n';
    }

(This is _by far_ the most readable snippet of his Pythagorean-triples program.
And yes, there's a `using namespace std;` in there to make it all stick together.)

Since Eric pretty much _wrote_ Ranges, I thought there might be a subtle reason he went with the
un-idiomatic `for (auto triple : range)` instead of the "Always Works" version `for (auto&& triple : range)`.
It would be a tough blow for my guideline if `auto&&` were somehow incompatible with Ranges!

So I went to Godbolt and tried out the same program with `auto` and with `auto&&`.
[The result?](https://godbolt.org/z/kiyHg5) With `auto&&`, the compiled program is two instructions
shorter. (Look around line 138 in the assembly listing for the major difference: we save a copy
in the inner loop.)

So,

> Use `for (auto&& elt : range)`. It Always Works.

(And also probably "Don't use Ranges," at least not on Godbolt Compiler Explorer. If you do, you'll
need to know about the "Clear cache & recompile" button in the lower left corner of the compiler pane.
I had to click it about a dozen times before Eric's toy program managed to sneak in under CE's
eight-second limit on compilation time.)

See also:

* ["`-Wrange-loop-bind-reference` and `auto&&`"](/blog/2020/08/26/wrange-loop-analysis) (2020-08-26)
