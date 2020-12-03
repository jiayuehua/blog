---
layout: post
title: "How to use a private member from outside the class"
date: 2020-12-03 00:01:00 +0000
tags:
  access-control
  metaprogramming
  templates
excerpt: |
  Yesterday I asked:

  > Am I missing some subtle mechanism by which "unused" private members
  > might actually be referenced from other TUs?

  Indeed, I was! Check this out ([Godbolt](https://godbolt.org/z/bqM6hK)):
---

[Yesterday I asked:](/blog/2020/12/02/unused-private-member/)

> Am I missing some subtle mechanism by which "unused" private members
> might actually be referenced from other TUs?

Indeed, I was! Check this out ([Godbolt](https://godbolt.org/z/bqM6hK)):

    struct S {
    private:
        static int private_static_data;
    };
    int S::private_static_data = 42;  // Optimize this away?


    // Meanwhile, in some other TU...
    static int *f();

    template<int *x>
    struct F {
        friend int *f() { return x; }
    };
    template struct F<&S::private_static_data>;

    int steal_private_data() {
        return *f();
    }

According to [[temp.spec]/6](http://eel.is/c++draft/temp.spec#general-6),
the template arguments of explicit instantiation declarations are permitted
to name "private types or objects that would normally not be accessible" —
such as `S::private_static_data` in this case.

So we can instantiate `F<&S::private_static_data>`, even though we cannot
actually refer to that instantiation by name afterwards. This "leaks" the
private data of `S` into `F`. `F` leaks it out again via the global function
`f()`. And then anyone can call `f()` to obtain a pointer to `S`'s private
data.

Therefore, although the compiler of TU #1 can see that none of `S`'s members and
friends use `S::private_static_data`, it cannot conclude that
the definition of `S::private_static_data` is unused. Other object files
may indeed refer to that linker symbol. So the compiler _must_ generate code for it.
