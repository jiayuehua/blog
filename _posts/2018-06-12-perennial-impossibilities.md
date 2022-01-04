---
layout: post
title: 'Perennial impossibilities of C++'
date: 2018-06-12 00:02:00 +0000
tags:
  constexpr
  language-design
  metaprogramming
---

These topics come up perennially in std-proposals, StackOverflow, and other places where
us C++ types hang out. I thought I'd collect them all in one place.


## Detect the first argument type of a function

We can easily write a type-based metafunction `first_arg_type_t<F>` such that for example
`first_arg_type_t<void(int,double)>` is `int` and `first_arg_type_t<void (A::*)(B&)>`
is `B&` (or, if you prefer, `A*`).

We can even [extend this to lambdas with a tiny bit of hackery](https://www.boost.org/doc/libs/develop/libs/callable_traits/doc/html/callable_traits/reference.html#callable_traits.reference.ref_args).

So people inevitably try to build APIs where they receive an arbitrary Callable and then
deduce what kind of arguments it wants to be passed.

    template<class F>
    void MyFuture::then(F&& f) {
        if constexpr (first_argument_type_is<F, MyFuture>) {
            return F(*this);
        } else {
            return F(this->value);
        }
    }

This is _just possible enough_ to get people in trouble.

Where it breaks down is when you have a generic lambda, or a function-like object with several
overloaded `operator()`s (such as separate `const` and non-`const` versions), or a function template
or overload set in general.

If (as in this case) you're trying to decide how to call the function, you also have to watch out
for cvref-qualification — what should you do if you wanted the first argument type to be `MyFuture`
but the user gave you a function that takes `const MyFuture&`?
And implicit conversions — what if you wanted a function taking `int` but the user gave you one taking `long`?

The solution is to *not do that*. In our snippet above, we should have created two member
functions, perhaps `then_f` and `then_v`, where the first unconditionally returns `F(*this)` and the
second unconditionally returns `F(this->value)`. Just let the user call the one they want!


## `string_view` versions of many utility functions

In C++11 we got `std::stoi(const std::string&)`, which is just a
[super convenient wrapper around `strtol`](https://github.com/llvm-mirror/libcxx/blob/41af64a/src/string.cpp#L90).

In C++17 we got `std::string_view`, which is a [super convenient replacement
for `const std::string&` as a function parameter](/blog/2018/03/27/string-view-is-a-borrow-type).

So where's my `std::svtoi(std::string_view)`?

Well, you'll get that exactly as soon as C provides a `memtol` function that can turn an arbitrary
non-null-terminated range of chars into an integer. Because nobody involved with C++ wants to
*reimplement* string-to-int conversion just in order to make it work with non-null-terminated
strings. (Except of course [when they do](https://en.cppreference.com/w/cpp/utility/from_chars).)

Similarly, you'll get `fopen(std::string_view)` just as soon as the POSIX `open` syscall
starts accepting non-null-terminated strings. Which is to say, never.


## Detect the constexprness of the current context

Today, we have a classic tradeoff: "Fast at runtime, or callable at compile-time: pick one."

Consider `sqrt`. This C++14 code is modeled on
[Alex Shtof's C++11 version](https://gist.github.com/alexshtf/eb5128b3e3e143187794).

    // Compiled with -ffast-math, this produces 95 bytes of code.
    // sqrtsd %xmm0, %xmm0 ; ret
    constexpr double constexpr_sqrt(double x)
    {
        if (0 <= x && x < std::numeric_limits<double>::infinity()) {
            double curr = x;
            double prev = 0;
            while (curr != prev) {
                prev = curr;
                curr = 0.5 * (curr + x / curr);
            }
            return curr;
        } else {
            return std::numeric_limits<double>::quiet_NaN();
        }
    }

    double nonconstexpr_sqrt(double x)
    {
        return __builtin_sqrt(x);
    }

The codegen for `constexpr_sqrt` (assuming you've got at least one call to it with a
non-compile-time-constant argument) is 23 instructions (and 4 branches) long, even with
`-O3 -ffast-math`. The codegen for `nonconstexpr_sqrt`, under the same conditions,
is two instructions long:

    sqrtsd %xmm0, %xmm0
    ret

So wouldn't it be cool if we could somehow branch on whether the constexpr version
was needed in the current context?

    constexpr double optionally_constexpr_sqrt(double x)
    {
        if constexpr (constexpr()) {
            if (0 <= x && x < std::numeric_limits<double>::infinity()) {
                double curr = x;
                double prev = 0;
                while (curr != prev) {
                    prev = curr;
                    curr = 0.5 * (curr + x / curr);
                }
                return curr;
            } else {
                return std::numeric_limits<double>::quiet_NaN();
            }
        } else {
            return __builtin_sqrt(x);
        }
    }

(That example uses Daveed Vandevoorde's original syntax from
[P0595R0](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0595r0.html),
which underwent severe changes in P0595R1.)

This is great, right? It's basically Perl's `wantarray` builtin, except that instead of
[telling you about the way your function's result is going to be used](https://www.perlmonks.org/?node_id=738558),
it... wait, never mind, that's *exactly* what it does.

So it seems like we can have some fun with this.

    constexpr auto silly()
    {
        if constexpr (constexpr()) {
            return (int*)nullptr;
        } else {
            return (double*)nullptr;
        }
    }

    void foo() {
        decltype(silly()) x = silly();
    }

The `silly()` inside the `decltype` is clearly a constexpr context — okay, actually the function
isn't evaluated at all, but let's call it a constexpr context — so we get `int *x = silly()`. But then
the right-hand side has no reason to prefer compile-time evaluation, so it gives back a `double*`,
and we get a compile-time error.

Alternatively,

    struct Evil {
        constexpr Evil(double*) {}
        Evil(int*) {}
    };
    Evil bar = silly();

This static-lifetime initializer *prefers* to be constexpr, if it can. So it evaluates `silly()`
in constexpr context and gets `int*`... which it can't use constexprly itself! So maybe it
promotes `bar`'s initialization to runtime; or maybe it backtracks and re-evaluates `silly()`
in non-constexpr context, gets a `double*`, and proceeds to initialize `bar` at compile-time after all.

[P0595R1](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0595r1.html) has some discussion
of this kind of issue, particularly focused on issues around template instantiations that might
be triggered by the backtracking, which P0595R1 calls "tentative constant evaluation."

(UPDATE: Barry Revzin points out that under P0595R1, the thing-formerly-known-as-`constexpr()`
actually looks only at its _immediately_ enclosing context; so all of my `if constexpr (constexpr())`
branches are actually asking about the constexprness of the `if constexpr` condition _itself_,
and therefore are all equivalent to `if constexpr (true)`. This seems to eliminate some of these
metaprogramming-related issues, although it does mean that both branches must be instantiable even
in the constexpr case.)

(UPDATE, 2022-01-04: In C++20 you can actually write `if (std::is_constant_evaluated()) {` to
get basically this effect — and in C++2b it's proposed that you'll be able to write `if consteval {`
directly in the core language. The C++20 feature sidesteps the `silly` and `Evil` issues by
forbidding your "compile-time" and "runtime" branches to return different types. I would say that
C++20 completely solves the problem of how to get different _behavior_ at compile-time versus run-time;
but only by firmly closing the door on how to get different _data representations_ at compile-time
versus run-time. Anyway, this item is pretty obsolete: I should rewrite or remove it at some point.)


## Strong typedefs

C++'s typedefs are notoriously weak. So weak, in fact, that we're not supposed to call them "typedefs"
anymore. Since C++11, we call them "type _aliases_." Thinking of type aliases as type _definitions_
can really get you into trouble.

    using AltitudeAboveMeanSeaLevel = int;
    void fly_at(AltitudeAboveMeanSeaLevel h);

    using HeightAboveGroundLevel = int;
    HeightAboveGroundLevel lowest_safe_altitude();

    static void cfit() {
        fly_at(lowest_safe_altitude());
    }

And the standard library isn't helping, with its zoo of synonyms for built-in types.
[Sergey Ignatchenko gives an example](http://ithare.com/c-on-using-int_t-as-overload-and-template-parameters/)
where `f` is overloaded for all four of `int8_t`, `int16_t`, `int32_t`, and `int64_t`,
and yet [a simple call to `f(0)` does not compile](https://godbolt.org/g/zkqCS8).

So, wouldn't it be cool if we had _strong type definitions_ in C++? Yes, it would.
The problem is that the API of a type is more than just its member functions
(or, in the case of primitive types, more than just its built-in operators).
Consider the following semi-realistic C++ class:

    struct Widget {
        int data;

        void swap(Widget&) noexcept;

        friend void foo();
        friend bool operator==(const Widget& a, const Widget& b) noexcept {
            return a.data == b.data;
        }
    };

    void swap(Widget& a, Widget& b) noexcept {
        return a.swap(b);
    }

    bool operator!= (const Widget& a, const Widget& b) noexcept {
        return !(a == b);
    }

    template<>
    struct std::hash<Widget> {
        size_t operator()(const Widget& w) const {
            return std::hash<int>()(w.data);
        }
    };

    void replace_if_possible(std::vector<Widget>& haystack, const Widget& needle);

    std::unique_ptr<Widget> make_widget();

    std::unordered_set<Widget> wset;

Now we suppose that C++ magically gains "strong typedefs," and we write:

    strong_typedef Gadget = Widget;
    Gadget g, h;
    std::vector<Gadget> gv;

Now go down the following list. Answer "yes" or "no" to each question.
You can stop at the first "no" if you want.

- A: Can we refer to `g.data`? (We'd better be able to, right?)
- B: Does `g.swap(h)` compile?
- C: What about `swap(g, h)`?
- D: Is `void foo()` a friend of `Gadget`?
- E: Does the template specialization `std::hash<Gadget>` exist?
- F: Does `g == h` compile?
- G: Does `g != h` compile?
- H: Is `std::unordered_set<Gadget>` instantiable?
- I: Does `replace_if_possible(gv, g)` compile?
- J: Does `auto p = make_gadget()` compile?
- K: Can we refer to `gset`?

If you answered "yes" to B, can you explain why the argument type
of `g.swap(...)` is apparently `Gadget&` and not its declared type `Widget&`?

If you said "yes" on C but "no" on G: why?

If you said "no" to either of E or F but "yes" to H: you're wrong.

If you said "yes" to G: Would your answer still be "yes" if `operator!=` had been
merely declared, not defined? What if its body was defined in some other .cpp
file entirely? What if that .cpp file didn't contain the `strong_typedef`
declaration?

If you said "yes" to J or K: Okay, so, ten out of ten for style, but...

And finally, if you said "no" to _all_ of B, C, D, E, F, G, and H:
you haven't really got a strong "typedef" anymore. It's more like
you just made a completely _new_ type, lacking the basic amenities such as
equality comparison, swappability, and hashability.

Any proposal for strong typedefs has got to somehow deal with the very messy
question of how to determine the "API" of a C++ type.
