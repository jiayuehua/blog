---
layout: post
title: "ADL insanity"
date: 2019-04-08 00:01:00 +0000
tags:
  argument-dependent-lookup
---

[Consider the following program](https://godbolt.org/z/EZi87a)
(hat tip to Agustín Bergé):

    #include <stdio.h>

    namespace A {
        struct A {};
        void call(void (*f)()) {
            f();
        }
    }

    void f() {
        puts("Hello world");
    }

    int main() {
        call(f);
    }

Right now, this code doesn't compile. Clang will helpfully tell you:

    error: use of undeclared identifier 'call'; did you mean 'A::call'?
        call(f);
        ^~~~
        A::call

----

But we can [make it compile](https://godbolt.org/z/EZi87a) by adding a *completely unused overload* of `f`!

    #include <stdio.h>

    namespace A {
        struct A {};
        void call(void (*f)()) {
            f();
        }
    }

    void f() {
        puts("Hello world");
    }

    void f(A::A);  // UNUSED

    int main() {
        call(f);
    }

This program compiles just fine and prints `Hello world` on GCC, Clang, and ICC (but not MSVC).
And apparently it's right to do so!

From [[basic.lookup.argdep]/2](http://eel.is/c++draft/basic.lookup.argdep#2.sentence-7):

> [I]f the argument is the name or address of a set of overloaded functions and/or function templates,
> its associated entities and namespaces are the union of those associated with each of the members
> of the set, i.e., the entities and namespaces associated with its parameter types and return type.

And the standard continues:

> Additionally, if the aforementioned set of overloaded functions is named with a _template-id_,
> its associated entities and namespaces also include those of its type _template-arguments_ and
> its template _template-arguments_.

That is to say, GCC is technically correct to accept
(and Clang, ICC, and MSVC are technically wrong to reject)
[the following program](https://godbolt.org/z/755cVu):

    #include <stdio.h>

    namespace B {
        struct B {};
        void call(void (*f)()) {
            f();
        }
    }

    template<class T>
    void f() {
        puts("Hello world");
    }

    int main() {
        call(f<B::B>);
    }

----

In both of these cases, we have an "argument" with no actual type. `f` and `f<B::B>`
are _names of overload sets_, and an overload set doesn't have a type. In order to collapse
the overload set down to a single function, we need to know what kind of function pointer type
is acceptable to the best-matching overload of `call`... and _that_ means we need to build
a candidate set for `call`, which means we need to look up the name `call`. Which happens via
ADL. Which _normally_ requires that we know the types of our arguments!

So `f` and `f<B::B>` are "arguments without types," but they are *also* arguments which
nevertheless contribute to argument-dependent lookup!

I suspect that there is no C++ code in the wild that relies on this little-known and frankly insane
feature of ADL. It would be interesting to learn otherwise.
