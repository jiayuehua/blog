---
layout: post
title: 'C++ quiz of the day'
date: 2018-07-08 00:01:00 +0000
tags:
  pitfalls
  relocatability
excerpt: |
  Is this code:

      struct VD2 {};
      struct VD2 final;

  - ill-formed, requiring a diagnostic from the compiler?

  - [ill-formed, no diagnostic required](https://stackoverflow.com/a/22180886/1424877)?

  - valid, and further inheritance from `VD2` is diagnosed as an error?

  - valid, and further inheritance from `VD2` is permitted with no diagnostic?

  The answer is below the break.
---

    struct VD2 {};
    struct VD2 final;

Is this code:

- ill-formed, requiring a diagnostic from the compiler?

- [ill-formed, no diagnostic required](https://stackoverflow.com/a/22180886/1424877)?

- valid, and further inheritance from `VD2` is diagnosed as an error?

- valid, and further inheritance from `VD2` is permitted with no diagnostic?

The answer is below the break.

-------

I kind of backed into this interesting question while coming up with the wording for
my soon-to-be-proposed attribute `[[trivially_relocatable]]`. The prior art that I'm
copy-and-pasting in my Clang patch is [`[[clang::trivial_abi]]`](/blog/2018/05/02/trivial-abi-101/),
but the closest prior art in the actual Standard seems to be `[[nodiscard]]`.
[Its wording](http://eel.is/c++draft/dcl.attr.nodiscard#1) is not as painstakingly
thorough as [the subsequent wording for `[[noreturn]]`](http://eel.is/c++draft/dcl.attr.noreturn#1),
though, so I'm mostly copying the latter.

> The _attribute-token_ `noreturn` specifies that a function does not return.
>
> It shall appear at most once in each _attribute-list_ and no _attribute-argument-clause_ shall be present.
>
> The attribute may be applied to the _declarator-id_ in a function declaration.
>
> The first declaration of a function shall specify the `noreturn` attribute if
> any declaration of that function specifies the `noreturn` attribute.
>
> If a function is declared with the `noreturn` attribute in one translation unit and
> the same function is declared without the `noreturn` attribute in another translation
> unit, the program is ill-formed, no diagnostic required.
>
> If a function `f` is called where `f` was previously declared with the `noreturn` attribute
> and `f` eventually returns, the behavior is undefined.

My wording copies essentially all of these conditions, making the obvious substitutions, so that
for example

    class [[trivially_relocatable]] Widget;
    class [[trivially_relocatable]] Widget { ... };

is permitted, as is

    class [[trivially_relocatable]] Widget;
    class Widget { ... };

but not

    class Widget;
    class [[trivially_relocatable]] Widget { ... };

and certainly not

    class Widget { ... };
    class [[trivially_relocatable]] Widget;

I was also investigating whether the set of "trivially relocatable by default"
class types should include a class with a virtual destructor if that virtual
destructor is both defaulted and `final` (or, more sensibly, if the class itself
is `final` — please don't put `final` on the destructor of a non-final class type!),
which I eventually decided was not worth the pain of specifying. But that's what
got me looking at the finer points of `final`.

Anyway, back to the quiz.

------

    struct VD2 {};
    struct VD2 final;

Recall that "`final`" is a [*contextual keyword*](https://en.cppreference.com/w/cpp/keyword) in C++11;
it carries special meaning only in certain contexts. In all other contexts it
acts as a plain old identifier.

This code defines a non-final struct type `VD2`, and then it defines a global
variable of type `VD2` that happens to be named `final`. Therefore the correct
answer is #4:

  - valid, and further inheritance from `VD2` is permitted with no diagnostic

I'm as surprised as you are!

For extra credit, identify the behavior of [this snippet](https://wandbox.org/permlink/khpNYng9yvNd1u8f) in C++:

    struct VD3 { int i; };
    struct VD3 final{ 42 };

For foreign-exchange student credit, identify the behavior of
[this snippet](https://wandbox.org/permlink/gE8ZGLqaEmpp6vvi) in C,
which comes via [Stefan Schulze Frielinghaus](https://stefansf.de/c-quiz/):

    struct VD4 final;
    struct VD4 {};
