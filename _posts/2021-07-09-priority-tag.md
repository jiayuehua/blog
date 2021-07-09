---
layout: post
title: '`priority_tag` for ad-hoc tag dispatch'
date: 2021-07-09 00:01:00 +0000
tags:
  metaprogramming
  pearls
  templates
---

I've mentioned the `priority_tag` trick a couple of times in previous posts;
it's time I did a post specifically on this trick.

This is all it is:

    template<int N> struct priority_tag : priority_tag<N-1> {};
    template<> struct priority_tag<0> {};

This gives us a class template which we can instantiate for arbitrary integers.
In that respect it's a lot like `std::integral_constant`. However, `priority_tag`
imposes an inheritance hierarchy. A `priority_tag<2>` <b>IS-A</b>
`priority_tag<1>`; a `priority_tag<1>` <b>IS-A</b> `priority_tag<0>`.

This is similar to the STL's iterator tag hierarchy:

    struct contiguous_iterator_tag : random_access_iterator_tag {};
    struct random_access_iterator_tag : bidirectional_iterator_tag {};
    struct bidirectional_iterator_tag : forward_iterator_tag {};
    ~~~

except that with `priority_tag<N>`, we don't bother to give the tag types
different names; we just give them numbers. To stretch an analogy, they're
[cattle, not pets](http://cloudscaling.com/blog/cloud-computing/the-history-of-pets-vs-cattle/).
We can use `priority_tag` in any context where we need a hierarchy of otherwise
meaningless and ephemeral tag types.

Overload resolution ranks argument conversions "shallow-to-deep":
converting the argument's type to one of its ancestor classes will be a better match
than converting it to one of that ancestor's ancestors
([over.ics.rank/4.4.4](https://timsong-cpp.github.io/cppwp/n4868/over.match.best#over.ics.rank-4.4.4)).
So, in the following snippet, overload resolution on line `D` will most prefer
the candidate on line `A` (if `A`'s return type is well-formed); otherwise it'll
prefer candidate `B` (if well-formed); otherwise it'll fall back on candidate `C`.
Candidate `B` is preferable to `C`, because conversion-to-parent
is preferred over conversion-to-parent's-parent. ([Godbolt.](https://godbolt.org/z/shT8xYYTv))

    template<class T>
    auto test(T *t, priority_tag<2>)  // A, exact match
        -> decltype(swap(*t, *t), std::true_type{});

    template<class T>
    auto test(T *t, priority_tag<1>)  // B, conversion to parent class
        -> decltype((*t).swap(*t), std::true_type{});

    template<class T>
    auto test(T *t, priority_tag<0>)  // C, conversion to grandparent
        -> std::false_type;

    template<class T>
    using HasSomeKindOfSwap = decltype(
        test((T*)nullptr, priority_tag<2>{})  // D
    );

It's possible to write this kind of SFINAE-based ordered overload set without `priority_tag`;
there are other ways that overloads get ranked. For example
([Godbolt](https://godbolt.org/z/shT8xYYTv)):

    template<class T>
    auto test(T *t, char)  // A, exact match
        -> decltype(swap(*t, *t), std::true_type{});

    template<class T>
    auto test(T *t, int)  // B, integral promotion
        -> decltype((*t).swap(*t), std::true_type{});

    template<class T>
    auto test(T *t, ...)  // C, ellipsis conversion
        -> std::false_type;

    template<class T>
    using HasSomeKindOfSwap = decltype(
        test((T*)nullptr, 'x')  // D
    );

This is totally fine and conforming C++, and even microscopically lighter-weight than
the previous snippet in terms of compile time. But it is _much less clear_, because
it relies on some pretty arcane rules about the rankings of conversion sequences.
Consider how you would deal with a request to add a fourth overload ranked
between B and C; and then a fifth overload ranked between A and B; and so on.
Each new overload would require its own little research project!
Whereas, with `priority_tag`, it's purely mechanical: You pick a new integer
(perhaps renumbering the old overloads to make room for it), and make sure
that at the call site (line `D`) you're passing `priority_tag<K>` for some `K`
at least as great as any of the priorities you've used so far. That's all.

`priority_tag`: "Cattle, not pets."

----

For other descriptions and uses of `priority_tag`, see:

- [dune/dune-common](https://github.com/dune-project/dune-common/blob/cb5d197/dune/common/typeutilities.hh#L47-L87) (November 2015)
- ["Some ruminations on tag dispatching"](https://pixorblog.wordpress.com/2016/05/14/some-ruminations-on-tag-dispatching/) (Vincent Picaud, May 2016)
- [ericniebler/range-v3](https://github.com/ericniebler/range-v3/blob/c6d7c74/include/range/v3/range_fwd.hpp#L292-L298) (December 2016)
- [Eric Niebler's `is_function` gist](https://gist.github.com/ericniebler/6612dd73d8eb1d7168a97f8f4927393d) (April 2017)
- ["A Soupçon of SFINAE"](https://www.youtube.com/watch?v=ybaE9qlhHvw&t=53m51s) (Arthur O'Dwyer, CppCon 2017), circa 53:50.
- [nlohmann/json](https://github.com/nlohmann/json/blob/6f55193/include/nlohmann/detail/meta/cpp_future.hpp#L63-L65) (January 2018)
- [tcbrindle/NanoRange](https://github.com/tcbrindle/NanoRange/blob/bf32251d65673fe170d602777c087786c529ead8/include/nanorange/detail/type_traits.hpp#L72-L78) (May 2018)
