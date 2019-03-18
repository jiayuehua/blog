---
layout: post
title: "Should _foo_ be move-only?"
date: 2019-01-02 00:01:00 +0000
tags:
  concepts
  library-design
  move-semantics
  type-erasure
---

Earlier today Vinnie Falco asked if I had an opinion on whether the Networking TS's
`CompletionCondition` concept should support "move-only" types. I said I don't even
know what `CompletionCondition` is, so how could I have an opinion on that? But then
it turned out that I did have an opinion.

The question "should my concept support move-only models?" is basically the same question
as "should my type-erased wrapper be able to wrap a move-only object?"
We're familiar with the latter from `std::function` versus
[`unique_function`](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0288r1.pdf).

There are two things to think about here.

First: If your thingie (concept or type-erased wrapper) needs to be copyable, or will be stored
inside something that needs to be copyable, then clearly your thingie itself must be copyable.
Example:

> Should the `Comparator` parameter to a `std::set` be copyable?
> Well, the `set` object itself HAS-A `Comparator`, and the `set` is copyable.
> When you copy a `set`, you copy its `Comparator`.
> So `Comparator` must be copyable; so it _must not_ support move-only models.

(Or else `set` must be only _conditionally_ copyable — that's the other way out of the dilemma.
But, like any [dilemma](https://www.etymonline.com/word/dilemma), there are only those two ways out.)

Second: So you've decided that your thingie doesn't _need_ to be copyable. Should you still require
copyability?

> Well, what part of the code gets simpler if you're allowed to copy them?
> Think about what if my completion condition holds within itself a `std::string` member.
> I wouldn't want you to copy that member more times than necessary, right?
> So is the number of "necessary" copies equal to zero?
> Then it needn't be copyable.

That is, you should require copyability only if that requirement is going to make your life
easier somehow. And then, even as you're enjoying your easy life, you should still make sure
to use move semantics wherever possible, so that you don't end up wasting
resources by expensively copying objects. If, at the end of your implementation, you realize
that you only ever _moved_ your thingies around — you never needed to copy them — well,
then copyability is not needed by your algorithm. You probably shouldn't require
something that you don't need.

Q.E.D.

Go on, ask me [if `Iterator` should be default-constructible...](/blog/2018/05/10/regular-should-not-imply-default-constructible/#the-stl-doesn-t-ever-need-defaul)
