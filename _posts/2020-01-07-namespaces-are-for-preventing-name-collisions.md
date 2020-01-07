---
layout: post
title: 'Namespaces are for preventing name collisions'
date: 2020-01-07 00:01:00 +0000
tags:
  c++-learner-track
  c++-style
  modules
  slogans
---

From ["Stop Writing Classes"](https://www.youtube.com/watch?v=o9pEzgHorH0&t=9m29s) (Jack Diederich, PyCon 2012):

> Namespaces are for preventing name collisions, not for creating taxonomies.

He gives the pathological example of

    d = MuffinMail.MuffinHash.MuffinHash(foo=3)

(the `MuffinHash` class, which belongs to the `MuffinHash` module of the `MuffinMail` package).
At best that should be `MuffinMail.Hash(foo=3)`. In his particular case actually the `MuffinHash`
class should never have existed, and it should have been a plain old dict `{ 'foo': 3 }`.

It's a good talk, most of its points exactly as applicable to C++ as they are to Python.

Inevitable nit: In C++, our standard library primitives aren't nearly as fit-for-purpose nor as stable as Python's.
Where Jack prefers Python's `dict` over `MuffinMail.Hash`, I'd say that in C++ it _is_ good practice to introduce
`muffinmail::hash` at least as a typedef, and ideally as a full-blown class, in preference to peppering
our `muffinmail` code with `std::map<std::string, int>`. In C++, it's a bad idea to harness yourself to
the API of a standard library type — an API _you_ can't change to suit your needs, but which _someone else_
will change every three years in ways that may make it less fit for your particular purpose. Better to own your
own API surface area.
See also ["Don't inherit from standard types"](/blog/2018/12/11/dont-inherit-from-std-types/) (2018-12-11).
