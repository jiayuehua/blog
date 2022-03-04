---
layout: post
title: 'Perfect forwarding call wrappers need `=delete`'
date: 2021-07-30 00:01:00 +0000
tags:
  metaprogramming
  pitfalls
---

(Hat tip to Louis Dionne and Tim Song for bringing this up
in a libc++ review, and then explaining it enough that I
think I got it.)

The C++20 standard, in section [[func.require](https://eel.is/c++draft/func.require#4)],
defines a new kind of library type called a _perfect forwarding call wrapper_.
A perfect forwarding call wrapper holds a data member
of some callable type `F`, and then has a templated `operator()(Args...)`
that perfectly forwards its arguments along to that `F`. Kind of like this:

    template<class F>
    struct PFCW {
        F f;

        template<class... Args>
        constexpr auto operator()(Args&&... args)
            noexcept(noexcept( f(static_cast<Args&&>(args)...) ))
            -> decltype(       f(static_cast<Args&&>(args)...) ) {
            return             f(static_cast<Args&&>(args)...) ;
        }
    };

(Remember: [You must type it three times.](https://www.youtube.com/watch?v=I3T4lePH-yA))

Notice that we're using `static_cast<Args&&>` (which is just a more compiler-friendly
synonym for `std::forward<Args>`) to perfectly forward the `args...` with their
original value categories. But in the above snippet, I failed to forward the
value category of the `PFCW` object itself! So if we stopped
here, you'd see the following misbehavior:

    const auto f = [](int x) { return x+1; };
    const auto pf = PFCW{f};
    assert(f(1) == 2);   // OK
    assert(pf(1) == 2);  // oops! does not compile

A real perfect forwarding call wrapper needs to
forward the value category of the called object itself,
by providing four overloads of `operator()`. For the sake of brevity,
I'll ignore rvalues and write just two overloads:

    template<class F>
    struct PFCW {
        F f;

        template<class... Args>
        constexpr auto operator()(Args&&... args)
            noexcept(noexcept( f(static_cast<Args&&>(args)...) ))
            -> decltype(       f(static_cast<Args&&>(args)...) ) {
            return             f(static_cast<Args&&>(args)...) ;
        }

        template<class... Args>
        constexpr auto operator()(Args&&... args) const
            noexcept(noexcept( f(static_cast<Args&&>(args)...) ))
            -> decltype(       f(static_cast<Args&&>(args)...) ) {
            return             f(static_cast<Args&&>(args)...) ;
        }
    };

This fixes our const lambda example. In fact, this version
can handle calls to mildly evil class types such as

    struct Naughty {
        int operator()(int x) { return x+1; }
        int operator()(int x) const { return x+2; }
    };

    Naughty n;
    auto pn = PFCW{n};
    assert(n(1) == 2);
    assert(pn(1) == 2);
    assert(std::as_const(n)(1) == 3);
    assert(std::as_const(pn)(1) == 3);

Another part of being a perfect forwarding call wrapper is that `PFCW<X>`
should be callable in situations where (and _only_ where) `X` itself
would be callable.
That's why we used all those `decltype` return types: to trigger SFINAE.
It seems to be working pretty well:

    struct X {
        int operator()(int);
    };
    static_assert(std::invocable<X, int>);
    static_assert(!std::invocable<const X, int>);
    static_assert(std::invocable<PFCW<X>, int>);
    static_assert(!std::invocable<const PFCW<X>, int>);

But here's the punch line (again, thanks to Tim Song for this):
Even this version can be tricked! [Godbolt](https://godbolt.org/z/dTY7P5jn3):

    struct Evil {
        int operator()(int) = delete;
        int operator()(int) const;
    };
    static_assert(!std::invocable<Evil, int>);       // as expected
    static_assert(std::invocable<PFCW<Evil>, int>);  // Oops!

    Evil e;
    auto pe = PFCW{e};
    // e(1);  // would not compile
    pe(1);    // OK -- oops!

When we call `e(1)`, the compiler looks at both candidates:
`operator()(int)` and `operator()(int) const`. The one without `const`
is the better match, so it is selected... but it was explicitly deleted
(which, as we know, signifies "The type designer knows what you're trying to
do, and what you're trying to do is wrong"). And so `e(1)` does not compile!

When we call `pe(1)`, the compiler looks at all the ways it might form
a call operator on `pe`. Every way is going to involve a template instantiation.
From the `operator()(Args&&...)` template we get... um... substitution
failure, so never mind that one. Keep going. From the `operator()(Args&&...) const`
template, though, we get a viable candidate. This is our only viable candidate,
so it's our best match. `pe(1)` compiles.

In order to make `PFCW` a real, standards-conforming perfect forwarding call wrapper,
we must fix it so that `pe(1)` won't compile. The way to do that is to
double the number of `operator()` overloads yet again. It's not enough to
provide overloads that _exist_ only when `f(args...)` _is_ callable; we must
also provide overloads that are _explicitly deleted_ when `f(args...)` is
_not_ callable. Our final perfect-forwarding call wrapper (still ignoring
rvalue-callable things) looks like this:

    template<class F>
    struct PFCW {       // Finally, the non-buggy version!
        F f;

        template<class... Args>
        constexpr auto operator()(Args&&... args)
            noexcept(noexcept( f(static_cast<Args&&>(args)...) ))
            -> decltype(       f(static_cast<Args&&>(args)...) ) {
            return             f(static_cast<Args&&>(args)...) ;
        }

        template<class... Args>
        constexpr auto operator()(Args&&... args) const
            noexcept(noexcept( f(static_cast<Args&&>(args)...) ))
            -> decltype(       f(static_cast<Args&&>(args)...) ) {
            return             f(static_cast<Args&&>(args)...) ;
        }

        template<class... Args>
            requires (!std::invocable<F&, Args&&...>)
        void operator()(Args&&... args) = delete;

        template<class... Args>
            requires (!std::invocable<const F&, Args&&...>)
        void operator()(Args&&... args) const = delete;
    };

A complete example, with all eight overloads (four deleted and four non-deleted),
is [here](https://godbolt.org/z/nh3f667an).

----

Finally, for the record, note that a standard _perfect forwarding call wrapper_
can do more than just call `f(args...)`. The Standard specifies many call wrappers,
each associated with a certain _call pattern_ in terms of `f` and `args...`.
For example, [`std::not_fn`](https://eel.is/c++draft/func.not.fn#4)
returns a perfect forwarding call wrapper with the call pattern `!std::invoke(f, args...)`;
[`std::bind_front`](http://eel.is/c++draft/func.bind.front#4)
returns a perfect forwarding call wrapper with the call pattern
`std::invoke(f, some, fixed, arguments, args...)`.
You can imagine abstracting this out into a `PFCW<Pattern, F>` where the pattern
is provided as a policy parameter; and in fact this is (more or less)
[what libc++ does](https://github.com/llvm/llvm-project/blob/050b064f15ee56/libcxx/include/__functional/perfect_forward.h#L80-L81).
