---
layout: post
title: 'Constrained forwarding references considered sketchy as frick'
date: 2018-09-09 00:01:00 +0000
tags:
  concepts
  metaprogramming
  pitfalls
---

I'll just leave [this](https://godbolt.org/z/oSRvvp) here.

    template<class I> requires is_integral_v<I>
    void foo(I&& i) {
        puts("ONE");
    }

    template<class T> // unconstrained
    void foo(T&& t) {
        puts("TWO");
    }

The intent of this code (says the casual reader) is that `foo` takes
one parameter by forwarding reference; and if that parameter is integral,
it'll do one thing and otherwise it'll do the other thing. But watch:

    int main() {
        constexpr int zero = 0;
        foo(int(zero));
        foo(zero);
    }

This [prints](https://wandbox.org/permlink/yBKPKpCEwny8Go5L) "ONE TWO".

This just came up in the wild, so to speak,
[on the Code Review StackExchange](https://codereview.stackexchange.com/questions/203435/c-multithread-pool-class/).
