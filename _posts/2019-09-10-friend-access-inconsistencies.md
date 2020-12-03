---
layout: post
title: "Implementation divergence with `friend`"
date: 2019-09-10 00:01:00 +0000
tags:
  access-control
  classical-polymorphism
  implementation-divergence
  metaprogramming
---

The other day I was pondering a potential new exercise for
[my "STL from Scratch" class](https://cppcon.org/class-2019-stl-from-scratch/): write an `iterator_facade`
similar to [Boost's](https://www.boost.org/doc/libs/1_68_0/libs/iterator/doc/iterator_facade.html),
but as a mixin rather than a CRTP base class. I'll skip the details, but the point is that I ended up
thinking about how Boost's old
[`friend class iterator_core_access;`](https://www.boost.org/doc/libs/1_68_0/libs/iterator/doc/iterator_facade.html#iterator-core-access)
idiom would translate into a world full of SFINAE.

Recall that when you say `friend class Foo;`, what you mean is "Please allow `class Foo` to
access all of my private members." But in C++, _classes_ don't access members —
_expressions_ and _type-expressions_ access members! Sometimes an expression might clearly "belong"
to a member of `class Foo`, but other times it might not.
I realized I had no solid intuition about what `friend` should actually do in these corner cases,
so I took my questions to Godbolt.

Consider these situations:

    class Shy {
        friend struct A;
        friend struct B;
        friend struct C;
        friend struct D;
        static inline int m = 42;
    };

    struct A {
        decltype(Shy::m) m;   // OK

        decltype(Shy::m) f() {
            return Shy::m;    // OK
        }

        int g();
    };
    decltype(Shy::m) A::g() { // OK
        return Shy::m;        // OK
    }

Notice that with `A::g`, the compiler sees a mention of `decltype(Shy::m)`
_before_ it sees that it's parsing a member of `struct A`.
However, all vendors agree that this is fine.

Let's go harder!

    struct B {
        int f(int x);
    };
    int B::f(int x = Shy::m) {
        return x;
    }
    int nonfriend() {
        return B().f();
    }

Default function arguments are always _evaluated_ in the context of the caller, but their
access-control is always dealt with in their original context. So giving function parameter `x`
a default value of `Shy::m` is OK here.


## Divergence on `friend` with enums

Let's go harder!

    struct C {
        struct S;
        enum E : int;
    };
    struct C::S {
        int f(int x);
    };
    decltype(Shy::m) C::S::f(decltype(Shy::m) x = Shy::m) {
        return x;
    }
    enum C::E : decltype(Shy::m) {  // XX
        RED = sizeof(Shy::m),  // YY
    };

When `Shy` befriends `C`, it's saying that all members of class `C` have access to all members of `Shy`.
"All members" includes member types as well! So for example a member function of nested class `C::S`
should be able to refer to `Shy::m` in all the same ways that a member function of `C` itself could.
Similarly, we expect that member type `C::E` should have access to `Shy::m`.

On the `enum` member, we get some fun implementation divergence!

* Clang: This is all perfectly OK, thank you.

* ICC: Line `XX` is an error, but line `YY` is OK.

* GCC and MSVC: Lines `XX` and `YY` are both errors.

You can verify that the errors are all due to access control by changing `class Shy` to
`struct Shy` — every vendor's errors vanish when you do that.


## Divergence on `friend` with templates

Let's go harder! Let's do some templates.

    struct D {
        template<int VALUE>
        int f();
    };

    template<decltype(Shy::m) VALUE>
    int D::f() {
        return VALUE;
    }

Here again we have some implementation divergence.
GCC believes that `template<decltype(Shy::m) VALUE>` constitutes a different
_template-head_ from `template<int VALUE>` when `Shy::m` is inaccessible.
Everyone else is happy with this code.

What about metaprogramming? Can we successfully SFINAE on the accessibility of a member?
[Godbolt:](https://godbolt.org/z/FYzvSc)

    class Shy {
        friend struct A;
        static inline int m = 42;
    };
    class Unfriendly {
        static inline int m = 42;
    };

    struct A {
        template<class T, class = void>
        struct HasM;
    };
    template<class, class>
    struct A::HasM {
        static constexpr bool value = false;
    };
    template<class T>
    struct A::HasM<T, decltype(T::m, void())> {
        static constexpr bool value = true;
    };

    static_assert(!A::HasM<Unfriendly>::value, "oops Unfriendly should have inaccessible m");
    static_assert(A::HasM<Shy>::value, "oops Shy should have accessible m");
    static_assert(!A::HasM<int>::value, "oops int should have no m");

Sadly, we have a lot of implementation divergence here.

- Clang and ICC: This is all totally OK.

- MSVC: The mention of `T::m` in the partial specialization doesn't
    respect `friend` status; so `A::HasM<Shy>::value` is `false`.

- GCC: The mention of `T::m` in the partial specialization doesn't
    respect `friend` status, and, furthermore, is a hard error
    when `T::m` is inaccessible.

Can you come up with a portable way for `A::HasM` to SFINAE on the
accessibility of member `T::m`? If so, drop me a line!


## Divergence on template friends

How about if we actually befriend the template itself — can we then refer to
private members in the friend's own _template-head_? [Godbolt:](https://godbolt.org/z/CiP9IQ)

    class Shy {
        template<int> friend struct A;
        template<class> friend struct B;
        static inline int m = 42;
    };

    template<decltype(Shy::m)>
    struct A {};

    template<class = decltype(Shy::m)>
    struct B {};

- Clang and MSVC: This is all totally OK.

- GCC and ICC: Neither `A` nor `B` is well-formed, because `Shy::m` is inaccessible.


## Lagniappe: Divergence on default template arguments

Finally, here's an implementation divergence that turned out to have
nothing to do with `friend`.

    struct B {
        template<class>
        static int f();
    };
    template<class T = decltype(Shy::m)>
    int B::f() {
        return 0;
    }
    int main() {
        return B::f();
    }

This one gives us [four different vendor behaviors](https://godbolt.org/z/FQKJ69):

- Clang compiles it quietly and successfully.

- GCC compiles the template definition quietly, but then confusingly fails to deduce `T` in the call to `B::f()`.

- MSVC — the most helpful behavior — compiles the template definition under protest:

        warning C4544: 'T': default template argument ignored
        on this template declaration

- Intel ICC refuses even to compile the template definition.

        error #1081: a default template argument cannot be specified
        on the declaration of a member of a class template
        outside of its class

ICC's error message is clearly confused: `B::f()` is not a member of a class template,
it's a member of the non-template class `B`. However, MSVC and GCC seem to agree with
the very surprising dictum that defaulted template parameters (in the specific case of a
member template) should be ignored when they're seen anywhere except the template's _first_ declaration.

Anyway, this inconsistency has to do with some arcane aspect of templates, not of `friend`.
Changing `class Shy` to `struct Shy` doesn't alter any of the inconsistent behaviors here.
