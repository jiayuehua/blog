---
layout: post
title: '"Myths about /dev/urandom" and "What Is JavaScript Made Of?"'
date: 2020-02-09 00:01:00 +0000
tags:
  blog-roundup
  random-numbers
---

Today I read an excellent post: Thomas Hühn's ["Myths about /dev/urandom."](http://www.2uo.de/myths-about-urandom/)
Highly recommended reading; especially if, like me, you can never remember which of
`/dev/random` and `/dev/urandom` is the correct one to use.

Mnemonic: [The `u` in `/dev/urandom` stands for "unlimited,"](https://unix.stackexchange.com/questions/323610/what-does-the-letter-u-mean-in-dev-urandom)
or perhaps "un-blocking."

Even better mnemonic: The `u` in `/dev/urandom` stands for "Yo`u` should `u`se this one."
(Thomas Ptacek says: ["Use urandom."](https://sockpuppet.org/blog/2014/02/25/safely-generate-random-numbers/))

----

A few weeks ago I read another excellent post: Dan Abramov's
["What Is JavaScript Made Of?"](https://overreacted.io/what-is-javascript-made-of/) (December 2019).
I especially appreciate some of the little things he does, such as introducing
"equality" `===` before "loose equality" `==`, and his bit on `{}` which I'm
just going to quote here:

> We mentioned earlier that `2` is equal to `2` (in other words, `2 === 2`)
> because whenever we write `2`, we “summon” the same value.
> But whenever we write `{}`, we will always get a _different_ value!
> So `{}` is not equal to another `{}`. Try this in console: `{} === {}` (the result is `false`).
> When the computer meets `2` in our code, it always gives us the same `2` value.
> However, object literals are different: when the computer meets `{}`,
> it creates a new object, which is always a new value.

----

In dead-tree news, I've also received a copy of John Lakos' new book
[_Large-Scale C++, Volume 1: Process and Architecture_](https://amzn.to/2UC5u88)
(many thanks, Addison-Wesley!) and am slowly progressing through it.
Somehow I'm 200 pages in and I'm still on "chapter 1."
