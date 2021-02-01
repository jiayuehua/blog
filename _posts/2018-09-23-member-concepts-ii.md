---
layout: post
title: 'A simpler SUBSUMES() macro'
date: 2018-09-23 00:01:00 +0000
tags:
  concepts
  metaprogramming
  preprocessor
  rant
---

In [our previous installment](/blog/2018/09/17/i-can-haz-member-concepts/),
I called the following (not-yet-valid C++2a) idiom a "poor man's version"
of member concepts:

    struct integral_helper {
        template<class T>
        concept MemberConcept = Integral<T>;
    };

    template<class X>
    struct Foo {
        template<class T>
        void bar() requires X::template MemberConcept<T> {
            // ...
        }
    };

    Foo<integral_helper> foo;
    foo.bar(1);        // OK
    foo.bar("no");     // error

`integral_helper` is not valid C++2a because member concepts are not allowed.

We can actually get one step closer to reality (and one step "poorer")
using a new C++2a feature: generic lambdas with template parameter lists.
Consider that we can already "smuggle" _constraints_ via class templates.
[Here's a working C++2a example:](https://godbolt.org/z/wqpdc2)

    template<class T, template<class> class MC>
    concept Satisfies = requires {
        MC<T>{}();
    };

    template<class T>
    struct signed_helper {
        template<class U = T> requires Signed<U>
        void operator()() const;
    };

    template<
        template<class> class MC
    >
    struct Foo {
        template<Satisfies<MC> T>
        void bar(T t) {
            // ...
        }
    };

    Foo<signed_helper> foo;
    foo.bar(1);        // OK
    foo.bar("no");     // error

What we cannot do is smuggle _concepts_, with all of their subsumption behavior.
The constraint `requires Satisfies<T, signed_helper>` is not interchangeable with the
constraint `requires Signed<T>`, because the latter is subsumed by `requires Signed<T> && true`
whereas the former is unordered with respect to `requires Signed<T> && true`.

So we cannot use our `Satisfies` metafunction as a building block to implement a
`Subsumes` metafunction (as far as I know). But we can still improve our macro metaprogramming
from [last time](/blog/2018/09/17/i-can-haz-member-concepts/) by expressing our constraint
infrastructure as constraints on *lambdas* rather than constraints on out-of-line member functions.
Here's [some example code](https://godbolt.org/z/RmAZGk),
which will be working as soon as either Clang or GCC support generic lambdas with
template parameter lists.

    template<class T>
    concept AlwaysTrue =
        std::is_void_v<std::void_t<T>>;

    template<class T>
    concept EvenMoreTrue = AlwaysTrue<T> &&
        std::is_void_v<std::void_t<T, void>>;

    template<class... Ts>
    struct Overload : Ts... {
        explicit constexpr Overload(Ts... ts) :
            Ts(ts)... {}
        using Ts::operator()...;
    };

    template<class, class = int>
    struct HasUnambiguousCallOperator : std::false_type {};

    template<class MAB>
    struct HasUnambiguousCallOperator<MAB, decltype(std::declval<MAB>()(0))>
        : std::true_type {};

    template<class MAB>
    constexpr bool extract_value(MAB) {
        return HasUnambiguousCallOperator<MAB>::value;
    }

    #define SUBSUMES(A,B) \
        extract_value(Overload( \
            []<class T>(T) requires B<T> || AlwaysTrue<T> {}, \
            []<class T>(T) requires A<T> || EvenMoreTrue<T> { return 0; } \
        ))

    static_assert(SUBSUMES(Integral, Scalar));
    static_assert(not SUBSUMES(Scalar, Integral));

Recall that if we had member concepts, we could build this directly as a template
metafunction, without any macros.
