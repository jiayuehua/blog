---
layout: post
title: "Book Review: Fedor Pikus' _The Art of Writing Efficient Programs_"
date: 2021-10-19 00:01:00 +0000
tags:
  c++-style
  litclub
excerpt: |
  The other day I received a pre-release review copy of Fedor Pikus's new book,
  [_The Art of Writing Efficient Programs_](https://amzn.to/3AZ35pz)
  ([Packt Publishing](https://www.packtpub.com/product/the-art-of-writing-efficient-programs/9781800208117), 2021).
  Here's my review of it. TLDR: it's very good! I don't say that about just
  any book; and for a review copy from Packt, this book _definitely_ exceeded my
  expectations.
---

![Cover image](/blog/images/2021-10-19-the-art-of-writing-efficient-programs.jpg){: .float-right}

The other day I received a pre-release review copy of Fedor Pikus's new book,
[_The Art of Writing Efficient Programs_](https://amzn.to/3AZ35pz)
([Packt Publishing](https://www.packtpub.com/product/the-art-of-writing-efficient-programs/9781800208117), 2021).
Here's my review of it. TLDR: it's very good! I don't say that about just
any book; and for a review copy from Packt, this book _definitely_ exceeded my
expectations.

> This is a very good book, and contains material that should be of interest even to
> people who don't work in super "high performance" environments.
>
> Fedor does well to begin with the fact that the meaning of "performance" is context-dependent,
> and the hard medicine that performance must be measured, not guessed at. The extended example
> on pages 21–28 is a bit hard to follow, but definitely illustrates how performance can sometimes
> be affected by microscopic, seemingly irrelevant changes. The worked examples on `perf`,
> `google-pprof`, and Google Benchmark are valuable.
>
> I'm glad that Fedor repeatedly focuses on the importance of communicating with the human reader
> of the code (for example, page 219 on the connotations of `AtomicCounter` versus `AtomicIndex`).
> There are other bits of good advice sprinkled throughout; for example, "prefer composition
> over inheritance" comes up on page 236 in the context of inheriting from `std::stack`,
> and "`const` means thread-safe" comes up on page 241.
>
> The book could almost just have been called "Concurrent Programming," because that's
> the focus of the hefty Part 2. There is quite a bit of material here on multi-threaded
> and even lock-free data structures, such as stacks, queues, and lists. Lock-free programming
> is hard; often it's better to stick with a simple mutex lock or redesign one's algorithm
> to remove the need for thread-safe data structures. And Fedor again gives good advice
> by repeatedly emphasizing this point (page 273), as well as "Implement the minimal
> necessary interface" (pages 258 and 281).
>
> The discussion of C++20 coroutines (pages 290–303) is extremely clear and correct.
> I noticed a few insignificant errata, such as referring to `co_return` as an "operator"
> instead of a "statement," but really this is a surprisingly good explanation of core
> coroutine support. And it cuts off at precisely the right point, refusing to speculate
> either on best-practice patterns or on comparative performance, given that the compiler
> support is still immature at best. The book ends with a good clear discussion of
> undefined behavior.
>
> As usual for Packt publications, the end-of-book index leaves a lot to be desired.
> [Although it's better than the index for my own Packt book four years earlier.]
>
> All in all, there's a lot of valuable material here. I recommend this book.

----

Full disclosure: This review was actively "solicited" by the publisher; but "paid" only
in the sense that I got a review copy of the book. If you buy Fedor's book from
[the Amazon link above](https://amzn.to/3AZ35pz), I get a few cents via Amazon Affiliates,
although I'm probably taking those few cents away from Fedor. :) Now, the way I'd
_really_ get paid is if you bought _my_ book,
[_Mastering the C++17 STL_](https://amzn.to/34rWnt9)
([Packt](https://www.packtpub.com/product/mastering-the-c-17-stl/9781787126824), 2017)!
