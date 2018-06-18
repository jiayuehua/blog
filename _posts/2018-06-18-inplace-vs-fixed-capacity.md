---
layout: post
title: '`inplace_foo` versus `fixed_capacity_foo`'
date: 2018-06-18 00:02:00 +0000
tags:
  library-design
  naming
  sg14
---

Someone on Slack just asked, "What is the proper name for my fixed-capacity string class?"
I've [thought about this question before](https://groups.google.com/a/isocpp.org/d/msg/sg14/jYIoCZfDulo/toLxMBq-CQAJ),
which is unsurprising for someone active in SG14 where non-allocating containers are all
the rage. In October 2017 I wrote:

> It occurs to me that we must distinguish between two kinds of "in-place-ness":
>
> (1) `inplace_function`, `inplace_unique_function`, `inplace_any`, etc.: Has a fixed-size buffer
> for type-erasure. Requires template parameters "`Size`" and "`Align`". Does not require any
> "overflow" behavior, because if the user attempts to store a too-big or too-aligned object
> into the buffer, we static-assert failure. Storing a too-big object is just as much a
> compile-time failure as storing a non-callable or non-copyable object.
>
> (2) `bounded_string`, `bounded_vector`, `bounded_concurrent_queue`, etc.: Has a fixed-size array
> of non-type-erased elements. Requires template parameters "`T`" and "`Size`"; the appropriate
> alignment is completely determined by `alignof(T)`. Requires the specification of some
> particular "overflow" behavior, because it is part of the contract of this type that
> it can gain or lose elements. For example, `bounded_vector::push_back` might throw an
> exception; `bounded_string::append` might truncate on overflow;
> `bounded_concurrent_queue::push_back` might block.
>
> I strongly prefer the term "inplace" for the former (type-erased) category.
> I strongly prefer that the former (type-erased) category be differentiated from the latter (non-type-erased) category.
> I tentatively prefer the term "bounded" for the latter category.
> I am not sure that the latter category is all one category; it might yet be several categories mashed together.

My opinions on the subject have not changed, except that I now more-than-tentatively prefer
the term "fixed-capacity" for the latter category. Thus:

    template<class Sig, size_t Cap, size_t Align>
    class inplace_function;

    template<size_t Cap, size_t Align>
    class inplace_any;

    template<size_t Cap /*,class OverflowPolicy?*/>
    class fixed_capacity_string;

    template<class T, size_t Cap /*,class OverflowPolicy?*/>
    class fixed_capacity_vector;

    template<class T, size_t Cap /*,class OverflowPolicy?*/>
    class fixed_capacity_queue;

In the `fixed_capacity_string` and `fixed_capacity_vector` cases, the only sensible
overflow policy IMHO is "undefined behavior," i.e., a runtime assertion failure.
In the case of `fixed_capacity_queue`, I could see the desire for ringbuffer-like
behavior — just quietly drop the element at the front of the queue and keep going.
However, that could reasonably be considered a new kind of container — say,
`fixed_capacity_ringbuffer` — instead of a parameterization of the "queue" container.

Notice that the Boost project's naming convention for the latter category is
not `fixed_capacity_foo` but rather `static_foo`, as in
[`static_string<Cap>`](https://www.boost.org/doc/libs/develop/libs/beast/doc/html/beast/ref/boost__beast__static_string.html)
and [`static_vector<T, Cap>`](https://www.boost.org/doc/libs/1_67_0/doc/html/boost/container/static_vector.html).
I should explain that I don't like that naming convention for two reasons:

- The defining characteristic of a fixed-capacity foo is that its overflow
  behavior is *dynamic*; you can add or remove elements at runtime; it
  *cannot* be static-asserted upon. So emphasizing "static" in this context
  sends precisely the wrong message.

- The word "static" is already overloaded about five different ways in C++,
  and we don't need to add any more. In particular, `static_foo f` looks
  a heck of a lot like `static foo f`, and I can imagine people getting
  very confused about the meaning of `thread_local static_foo f`.

`fixed_capacity_foo` suffers from neither of these disadvantages; its only
disadvantage is that it's verbose. But C++ could use a little more verbosity
sometimes. And
`template<class T> using fcv10 = fixed_capacity_vector<T, 10>`
is always available to the masochists.
