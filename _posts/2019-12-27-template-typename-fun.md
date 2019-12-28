---
layout: post
title: 'Holiday fun with `template<class>` and `template<typename>`'
date: 2019-12-27 00:01:00 +0000
tags:
  class-template-argument-deduction
  implementation-divergence
  rant
  templates
---

Let's start simple.

    template<class T> struct S1;
    template<typename T> struct S1;

These declarations are synonymous.

    using X1 = S1<int>;

----

How about this one? ([Godbolt.](https://godbolt.org/z/kDGFh5))

    template<class T, class T::U> struct S1;
    template<typename T, typename T::U> struct S2;

In this case, `S1` and `S2` both take one type parameter (`T`) and one non-type template parameter
(unnamed). However, `S1` will give a hard error if `T`'s nested type `U` is not a class type,
whereas `S2` will permit `T`'s nested type `U` to be anything — enum, reference, whatever.

    struct Y1 { class U {}; };
    using X1 = S1<Y1, Y1::U{}>;  // new in C++2a

    struct Y2 { using U = int*; };
    using X2 = S1<Y2, nullptr>;

----

How about this one?

    template<class T::U> struct S1;
    template<typename T::U> struct S2;

In this case, `S1` is valid if and only if it's preceded by a declaration of `T::U`. For example,
`T` might be a namespace or a class type, and `U` must be a class type nested within `T`:

    struct T { class U {}; };
    template<class T::U> struct S1;
    using X1 = S1<T::U{}>;  // new in C++2a

The keyword `class` in front of `T::U` is the _class-key_ introducing an
[_elaborated-type-specifier_](http://eel.is/c++draft/dcl.type.elab#nt:elaborated-type-specifier).

`S2` is never valid, as far as I know, but GCC doesn't complain about it. (GCC essentially treats that
`typename` keyword as a synonym for `class`.)

However, if `T` is a template parameter, everything changes!

    template<typename T, typename T::U> struct S3;

    struct Y { using U = int; };
    using X3 = S3<Y, 42>;

(Hat tip to Jon Kalb for this snippet.) Here, `S3` is a perfectly valid template (all the way back to C++03);
it takes one type parameter formally named `T` and one unnamed non-type parameter of type `T::U`. The first
instance of the `typename` keyword is a
[_type-parameter-key_](http://eel.is/c++draft/temp.param#nt:type-parameter-key),
but the second instance of the `typename` keyword is part of a
[_typename-specifier_](http://eel.is/c++draft/temp.res#nt:typename-specifier) instead.

----

Let's throw CTAD into the mix! ([Godbolt.](https://godbolt.org/z/XtLExJ))

    template<class Policy>
    struct S {
        template<typename Policy::FixedString> // INCORRECT!
        struct N {};
    };

    struct Y {
        template<class T> struct FixedString {
            constexpr FixedString(T) {}
        };
    };
    using X1 = S<Y>::N<42>;

In keeping with CTAD's general "ignore all the rules" approach, it seems there is no way to
express that `S<Y>::N` wants to take a non-type template parameter of type `Y::FixedString<...auto...>`
(to borrow GCC's way of writing CTAD placeholders).

We might try to work around this deficiency in CTAD with a layer of indirection — a local alias template
([Godbolt](https://godbolt.org/z/Jc2ckp)) —

    template<class Policy>
    struct S {
        template<class T> using PFS = Policy::FixedString<T>;
        template<PFS> // Workaround?
        struct N {};
    };

— but GCC doesn't implement [P1814 "Wording for Class Template Argument Deduction for Alias Templates"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1814r0.html)
yet, so it's [full of ICEs](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93085)
in this area.

> GCC is the only vendor that implements CTAD-in-NTTPs so far. Recall that CTAD-in-NTTPs is the "killer app"
> for NTTPs of class type — formerly [P0732](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0732r0.pdf),
> now [P1907](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1907r1.html), a feature that
> [I consider unbaked and would rather not have in C++2a at all](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1837r0.html).
> For that matter, I wish CTAD had never gone into C++17, and I advise against using CTAD in any production codebase
> (["_Contra_ CTAD,"](/blog/2018/12/09/wctad/) 2018-12-07).

----

Observe that `class T` introduces a template type parameter whose formal name is `T`,
but `class T*` introduces a non-type template parameter with no formal name. [Thus:](https://godbolt.org/z/9chJPJ)

    class T {} t;

    template<class T> struct S1;
    template<class T*> struct S2;

    using X1 = S1<int>;
    using X2 = S2<&t>;

Notice that `class T*` also has the interesting side effect of adding a declaration of `class T` to the current
(very tiny) scope. Declaring new meanings for names that were already used as template parameter names in an outer
scope (that is, "shadowing" a template parameter with some other declaration) is ill-formed, and implementations give
a wide range of fun error messages if you try. Clang's is perhaps the clearest, but even so, I almost filed a bug
about it before deciding that it was correct to complain. [Godbolt:](https://godbolt.org/z/cXcxUG)

    class T {} t;
    class U {} u;

    template<class T>
    struct S {
        template<class T*, T*> struct N {};
    };

    using X = S<U>::N<&u, &u>;

This code is 100% ill-formed, because `class T*` causes a shadowing declaration of `T` in a scope where `T` already
refers to a template parameter. GCC and Clang give error messages; MSVC is utterly confused by the _class-key_;
and EDG emits a warning before proceeding to treat `class T*` as if the `class` keyword weren't there. (So EDG
successfully compiles the above code, even though ideally it shouldn't.)
