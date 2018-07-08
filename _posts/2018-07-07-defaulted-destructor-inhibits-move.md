---
layout: post
title: 'Defaulted destructor inhibits move'
date: 2018-07-07 00:01:00 +0000
tags:
  pitfalls
---

[Today I learned:](https://wandbox.org/permlink/ai9qGXYxLGIFjRD1)
Explicitly defaulting a class's destructor causes the compiler not to
implicitly define any move constructor at all. Thus:

    struct Oops {
        std::shared_ptr<int> p;
        ~Oops() = default;
    };
    static_assert(std::is_copy_constructible_v<Oops>);
    static_assert(std::is_move_constructible_v<Oops>);

Struct `Oops` is "move-constructible" in the sense that

    Oops b = std::move(a);

will compile. But what it actually *does* is make a copy!
