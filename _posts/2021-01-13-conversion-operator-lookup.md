---
layout: post
title: "Fun with conversion-operator name lookup"
date: 2021-01-13 00:01:00 +0000
tags:
  implementation-divergence
  name-lookup
---

As of this writing (but perhaps not for very much longer!) the four mainstream compilers
on Godbolt Compiler Explorer give four different answers for
[this simple C++ program](https://godbolt.org/z/jo3dc4):

    struct A {
        using T = T1;
        using U = U1;
        operator U1 T1::*();
        operator U1 T2::*();
        operator U2 T1::*();
        operator U2 T2::*();
    };

    inline auto which(U1 T1::*) { return "gcc"; }
    inline auto which(U1 T2::*) { return "icc"; }
    inline auto which(U2 T1::*) { return "msvc"; }
    inline auto which(U2 T2::*) { return "clang"; }

    int main() {
        A a;
        using T = T2;
        using U = U2;
        puts(which(a.operator U T::*()));
    }

The question is whether `U` should be looked up in the scope of `test` or in the scope of `A`;
and the same question for `T`.

According to the current draft standard, it sounds like the conforming answer is
"they should _both_ be looked up in the scope of `A`"; i.e., GCC's answer is correct
and the others are wrong in three different ways. [[basic.lookup.unqual]/5](http://eel.is/c++draft/basic.lookup.unqual#5):

> An unqualified name that is a component name of a _type-specifier_ or _ptr-operator_
> of a _conversion-type-id_ is looked up in the same fashion as the _conversion-function-id_
> in which it appears. If that lookup finds nothing, it undergoes unqualified name lookup;
> in each case, only names that denote types or templates whose specializations are types are considered.

I'm never a fan of lookups that don't consider certain _kinds_ of names; I'm sure there's more divergence
to be discovered in this area. Anyway, in the type name `U T::*`, `U` is the _type-specifier_
and `T::*` is the _ptr-operator_, and the whole type is pronounced "pointer to a data member of `T`,
where that data member itself is of type `U`."
(More concisely: "pointer to data member (of type `U`) of `T`," or "pointer to a `U` member of `T`.")

See also:

* ["Implementation divergence with `friend`"](/blog/2019/09/10/friend-access-inconsistencies/) (2019-09-10)
