---
layout: post
title: "An obvious guideline on function-call style"
date: 2020-07-23 00:01:00 +0000
tags:
  c++-style
---

Warning: This blog post is glaringly obvious!

The other day, in yet another discussion of how to get keyword parameters into C++,
I pointed to Titus Winters' excellent guideline about how to craft an overload set:

> Use overloaded functions only if a reader looking at a call site
> can get a good idea of what is happening without having to first
> figure out exactly which overload is being called.

This guideline comes verbatim from Titus's talk
["Modern C++ Design"](https://www.youtube.com/watch?v=xTdeZ4MxbKo&t=6m15s) (CppCon 2018),
which took it verbatim from [Google's C++ Style Guide](https://google.github.io/styleguide/cppguide.html#Function_Overloading),
which I suspect may have taken it from Titus. :)

The Google style guide refers back to this rule in several places, such as in
the sections on default function arguments and operator overloading.

It struck me that this guideline could probably be generalized as follows:

> Write your function calls so that the reader knows
> roughly what's going to be done with each argument
> and what's going to happen to its value.

----

This generalization encompasses a lot of other little style rules:

> Name functions with verb phrases.

`x.setLevel(y)` is better than `x.level(y)`. (What's going to be done to `x`?)
`repair(c)` is better than `garage(c)`. (What's going to be done to `c`?)

> Name functions after their purpose, and clearly.

`s.trim_in_place()` is better than `s.clean()`. (What's going to be done to `s`? Is this a mutating method, or a query?)

> Never pass by non-const `T&`. Pass out-parameters by pointer.

`getline(cin, &s)` is better than `getline(cin, s)`.
`trim(&s)` is better than `trim(s)`. (Will `s` keep its old value?)
Our default assumption is that parameters that look like they're being passed by value
_are_ being passed by value (or, as an optimization, by `const&`). If you write `y = trim(x)`,
and that causes `x`'s value to be modified, your reader _will_ be surprised.

> Use overloading only for functions that all do roughly "the same thing."

The STL notably violates this rule when it comes to the constructor overload set:
it's not obvious what `std::string("abc", 2)` and `std::string(2, 'a')` really have in common,
other than "creating a new string object" — which could be said of _any_ kind of function
with return type `string`. The syntax `string::n_copies_of(2, 'a')` would arguably have
been clearer than `string(2, 'a')`.
See ["Is your constructor an object-factory or a type-conversion?"](/blog/2018/06/21/factory-vs-conversion/) (2018-06-21).

> Avoid functions that take lots of parameters, especially lots of parameters
> of the same type one after another.

I refer to this specific failure mode as the "boolean parameter tarpit" in
["Default function arguments are the devil"](/blog/2020/04/18/default-function-arguments-are-the-devil/) (2020-04-18).

Instead of `x.print(msg, /*trailingNewline=*/false, /*wrapLines=*/true)`,
for almost all real-world use-cases, it ends up being a good idea to create completely distinct names
for the entry points you intend to support:
`x.print(msg)`, `x.printWithNewline(msg)`, `x.printWrapped(msg)`, `x.printWrappedWithNewline(msg)`.

In my experience, the boolean parameter tarpit is the main thing that makes people think
they want named parameters in C++.
People look at `x.print(msg, /*trailingNewline=*/false, /*wrapLines=*/true)` and immediately want to
rewrite it as something like `x.print(msg, trailingNewline: false, wrapLines: true)`.
Indeed, that _would_ help us adhere to the rule that function calls should clearly indicate what's
going to happen with each argument.
But as long as C++ doesn't support that syntax, I'd recommend using the solution above,
or even a simple integral bitmask like `x.print(msg, PO_TrailingNewline | PO_WrapLines)`.

----

So: I claim that this high-level guideline encompasses a whole lot of little style rules that
apply to function call syntax in C++ (and, in fact, in any language).

> Write your function calls so that the reader knows
> roughly what's going to be done with each argument
> and what's going to happen to its value.

Now, the enterprising reader will notice that really this is still a specific case,
and that the above guideline can be generalized as follows:

> Write your code so that the reader knows roughly what's going on.

Kernighan and Plauger's [_The Elements of Programming Style_](https://amzn.to/2X5W1H0)
— an excellent, excellent book, recommended reading for everyone at least once in their
career — starts out with two _extremely_ high-level versions of this rule:

> Write clearly — don't be too clever.

and

> Say what you mean, simply and directly.

That's good advice.
