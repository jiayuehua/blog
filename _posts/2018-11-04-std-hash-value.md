---
layout: post
title: 'WG21: Avoid creating ADL situations on functions with semi-common names'
date: 2018-11-04 00:01:00 +0000
tags:
  argument-dependent-lookup
  customization-points
  pitfalls
  proposal
  san-diego-2018
  wg21
---

This is a sequel and corollary to my previous blog post ["Avoid ADL calls to functions
with common names"](/blog/2018/06/17/std-size) (2018-06-17).

Recall that the problem in that case was some client code that did

    template<class T> int size(const T& x) {
        return x.size();
    }

    ...
    for (int i=0; i < size(cl.lst); ++i)
    ...

This worked fine in C++14, but gave compiler errors in C++17 —

    ./rug.cpp:585:18: error: call to 'size' is ambiguous
      for(int i=0; i<size(cl.lst); i++)
                     ^~~~
    /usr/include/c++/v1/iterator:1584:16: note: candidate
          function [with _Cont = std::__1::vector<hr::cell *, std::__1::allocator<hr::cell *> >]
    constexpr auto size(const _Cont& __c) -> decltype(__c.size()) { return __c.size(); }
                   ^
    ./hyper.h:400:23: note: candidate function [with T = std::__1::vector<hr::cell *, std::__1::allocator<hr::cell *> >]
    template<class T> int size(const T& x) {return x.size(); }
                          ^

— because the C++17 standard library added a new ADL-able function named `std::size`.
This means that any client code using `size` as the name of a free function will have
to be refactored for C++17. In my blog post I wrote:

> Unfortunately, C++ doesn't have a really great solution here. Future standards can add
> names into `namespace std`, which can then break your existing code's unqualified calls
> by causing unwanted ADL.
>
> The only answer I can really think of is — [*Don't do that, then.*](https://www.youtube.com/watch?v=ri3aL8At44I&t=1m25s)
> Just like in my `zip` example above: in our C++ programs, we need to maintain a good
> intuitive sense for when some unqualified name (like `begin`, or `size`, or `zip`)
> is *likely* to get stepped on in a future standard, and *avoid using that name today*,
> even if it has not *yet* been stepped on.
> This is arguably a very unfortunate state of affairs... but it's the state of affairs
> we've got.
>
> *Avoid making ADL calls to functions with common names.*


## The flip side

However, for C++2a, I'd like the C++ Standard Committee to consider that every bargain
has two sides. In exchange for client programmers' avoiding _common_ names such as
`begin`, `end`, `size`..., I'd like the Committee to give some consideration to avoiding
_uncommon_ names. Or rather, avoiding names that are in that nebulous gray zone between
"common and obvious" (say, `size`) and "rare and unique" (say, `do_bind_nth_regex_parameter`).
For names in this gray area (say, `hash_value`), the Committee should please try to avoid
creating _greedy function templates_ with these names.

In San Diego this coming week, the Committee will be discussing Walter Brown's
proposal [P0549R4 "Adjuncts for `std::hash`,"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0549r4.pdf)
which is based on work by Lisa Lippincott. P0549 proposes to add a whole zoo of helpers
to the already unwieldy [customization point](/blog/2018/03/19/customization-points-for-functions)
that is `std::hash`. In particular:

    template<class T> using hash_for = hash<remove_cvref_t<T>>;
    template<class T> using is_hashable = is_enabled_hash<hash_for<T>>;

    template<class T> requires is_hashable_v<T>
    size_t hash_value(T&& t)
      noexcept(noexcept(hash_for<T>{}(std::forward<T>(t))))
    {
        return hash_for<T>{}(std::forward<T>(t));
    }

As written, this creates an ADL situation for client code using the function name `hash_value`.
[Here's a piece of code](https://godbolt.org/z/dqhyN4) whose behavior silently changes as a result:

    #include <string>
    #include <type_traits>
    #include <utility>

    #ifdef P0549
    namespace std {
        template<class T>
        size_t hash_value(T&& t)
        {
            hash<remove_cv_t<remove_reference_t<T>>> H;
            return H(std::forward<T>(t));
        }
    }
    #endif

    namespace my {
        using Str = std::string;

        size_t hash_value(const Str& s) {
            return s[0];
        }

        int r() {
            Str s = "hello world";
            return hash_value(s);
        }
    }

Without `-DP0549`, the line `return hash_value(s)` is a call to `my::hash_value(const Str&)`,
which is exactly what the average programmer would expect (since the call is coming from inside
`namespace my`).

With `-DP0549`, the same line generates a call to `std::hash_value<std::string&>(std::string&)`,
which is likely to be very surprising, and also doesn't do the same thing. (In fact,
under `-DP0549`, this program has the surprising behavior that `hash_value(s) != hash_value(std::as_const(s))`!)

The problem here is that the proposed `std::hash_value` function template is a _greedy_ function
template. (Prior to C++2a Concepts, I might have called it an _unconstrained_ template; but in C++2a
that word has a technical meaning, and _technically_ speaking, the proposed `std::hash_value`
_is_ constrained.) But it's still extremely promiscuous in what it accepts, and because it accepts
by forwarding reference, it's generally able to form a better match (in the overload-resolution
sense) than whatever non-template function the client programmer might have written.

(Incidentally, [here is the same example using Walter's proposed `requires`-clause](https://godbolt.org/z/VEGNqP),
with several typos fixed, compiled using a Concepts-enabled compiler, just to prove that the
constrainedness of the function template is not relevant to the issue I'm talking about here.)


## A general solution for good libraries

Libraries that are trying to be "good" might consider that our ADL problem here comes from the fact
that we put the _generic_ algorithm `std::hash_value` into `namespace std` right alongside the
_concrete_ container type `std::string`. Our generic algorithm works on any type, by design; it is
not specially related to any particular concrete type. Therefore it has no reason to live in the
same namespace as any concrete type.

So for these libraries, we might make a general rule that _generic algorithms_ should live in their
own namespace, and _types_ should live in their own separate namespace(s). Then it would be more difficult
for ADL to confuse a new generic algorithm taking `T` and a user-defined function taking a concrete type.


## A specific solution for `hash_value`

The right answer for P0549R4 is actually to step back and consider the purpose of the proposed
`std::hash_value`. It's supposed to be a heterogeneous entry point that performs `std::hash<T>()(t)`
for any `T`. Where else in the STL do we see that pattern?— Since C++14, we see it in `std::less`.

    template<class = void>
    struct less;

    template<>
    struct less<void> {
        template<class T, class U>
        bool operator()(T&& t, U&& u) const {
            return std::forward<T>(t) < std::forward<U>(u);
        }
    };

This means that we can write for example `std::less<>(1, 2.0)` and get the right answer.
(Or write `std::less<>(-1, 0u)` and get the wrong answer... but let that pass.)

So the obvious API for a heterogeneous `std::hash` in C++2a would be `std::hash<>`:

    template<class = void>
    struct hash;

    template<>
    struct hash<void> {
        template<class T>
        size_t operator()(T&& t) const {
            return std::hash<std::remove_cvref_t<T>>{}(std::forward<T>(t));
        }
    };

(In both of these snippets, the `noexcept` and `requires` clauses have been left
as an exercise for the reader. They aren't relevant to the point.)


### So we don't need `hash_value`?

Yeah. Once you're able to hash any hashable object at all by writing e.g.

    template<class T>
    size_t hash_my_kv_pair(const std::pair<std::string, T>& kv) {
        return std::hash<>{}(kv.first) + std::hash<>{}(kv.second);
    }

you don't need any function named `hash_value` at all.

We could technically still provide one, just as a pure convenience function
for people who would rather write the six characters `_value` than the four characters
`<>{}`. But it's important to make it non-ADL-able, which means making it a callable object
(a.k.a. "customization point object" (CPO), even though it's not actually a customization point).

    namespace std {
        constexpr inline auto hash_value = [](auto&& t) {
            return std::hash<>{}(std::forward<T>(t));
        };
    }

CPOs, like all non-functions, do not suffer from ADL; so this would be an acceptable
definition of `std::hash_value` if we really wanted it for some reason. But in practice,
I wouldn't expect anyone to be unsatisfied with a heterogeneous `std::hash<>`.
