---
layout: post
title: "What is ADL?"
date: 2019-04-26 00:01:00 +0000
tags:
  argument-dependent-lookup
  c++-learner-track
  wg21-folkloristics
---

I was all set to write a blog post about hidden friends, and then I realized that it
was turning into a blog post largely ranting about ADL. So I figure I should rant about
ADL *first*, and then talk about hidden friends in Part 2.

First things first. ADL stands for "argument-dependent [name] lookup."
There. Now, on with the story!


## In the beginning Bjarne created namespaces

In C, there are no namespaces. If I write a function named `get_next` (and I don't mark it `static` to
the current translation unit), then you cannot write a function named `get_next` anywhere in *your*
part of the code — our two pieces of code won't link together, because we'll have multiple definitions
of the `get_next` function.

In C, we work around this by manually prefixing our function names: I'll agree to name all of my functions
`ajo_<whatever>`, and you'll call your functions `sak_<whatever>`, and then the linker will be happy because
`ajo_get_next` and `sak_get_next` are different names.

C++ took this existing convention and baked it into the language. In C++, I can place all of
my stuff under `namespace ajo`, and you can place all of your stuff under `namespace sak`. And then
the linker will be happy because `ajo::get_next` and `sak::get_next` are different names.
(That is, the name of the namespace becomes part of the entity's [name-mangling](https://godbolt.org/z/3fb_BP).)

This is really great for scalability, because it means *you* don't have to worry about what *I* write.
We can each develop whatever we want in our own namespaces. Any unqualified `get_next` I use in my own code
(inside `namespace ajo`) will naturally refer to `ajo::get_next`.

    // SakUtils.h
    namespace sak {
        struct bignum {};
        int get_next(int);
    }

    // AjoUtils.h
    namespace ajo {
        struct bignum {};
        int get_next(int);
    }

    // AjoExtra.cpp
    #include "AjoUtils.h"
    #include "SakUtils.h"
    namespace ajo {
        void foo(int& x) {
            bignum b;         // refers to ajo::bignum
            x = get_next(x);  // calls ajo::get_next
        }
    }

And then whenever I need to refer to an entity in a foreign namespace,
such as the `get_next` which is a member of namespace `sak`,
I simply namespace-qualify its name:

    namespace ajo {
        void bar(int& x) {
            sak::bignum b;         // refers to sak::bignum
            x = sak::get_next(x);  // calls sak::get_next
        }
    }

But what do I do when the function I want to call doesn't _have_ a name?


## ADL arises to resolve conflict between namespaces and operators

> For this section, C++ experts will have to put themselves in "alternate-history mode."
> Our code samples assume that ADL doesn't yet exist.

The problem with namespaces was recognized early on.
See, C++ had also added *operator overloading.*
So you could write things like this:

    // SakBigNum.h
    namespace sak {
        struct bignum {
            bignum operator++();
        };
        std::ostream& operator<<(std::ostream&, bignum);
    }

    // AjoBigNum.h
    namespace ajo {
        struct bignum {
            bignum operator++();
        };
        std::ostream& operator<<(std::ostream&, bignum);
    }

    // AjoExtra.cpp
    #include "AjoBigNum.h"
    #include "SakBigNum.h"
    namespace ajo {
        void foo(int& x) {
            bignum b;        // refers to ajo::bignum
            ++b;             // calls ajo::bignum::operator++()
            std::cout << b;  // calls ajo::operator<<(ostream&, bignum)
        }

        void bar(int& x) {
            sak::bignum b;   // refers to sak::bignum
            ++b;             // calls sak::bignum::operator++()
            std::cout << b;  // UH-OH!
        }
    }

Explicit namespace-qualification works fine for accessing `sak::bignum` and `sak::get_next`.
And we don't need any special rules to deal with the meaning of `++b`: it "obviously" should
call `b`'s member function `operator++()`.

But what about `operator<<`?

Sure, I could write the call above as

    sak::operator<<(std::cout, b);

But if that's the recommended solution, then I should just write `sak::print(b)` and stop
using overloaded operators altogether!

> Notice that this is not a problem for the "standard" stream insertion operators, such
> as the ones for primitive types. When you call `std::cout << 42`, you're not using ADL;
> you're just calling the member function
> [`std::ostream::operator<<(int)`](https://en.cppreference.com/w/cpp/io/basic_ostream/operator_ltlt).

"So we can blame iostreams for ADL?" Yeah, I won't stop you from blaming iostreams.
But, to be fair, the problem crops up anywhere you have an operator that
can't be a member because its arguments are in the "wrong" order.
For example, [`std::operator+(const char *, const std::string&)`](https://en.cppreference.com/w/cpp/string/basic_string/operator%2B).


## The solution is argument-dependent lookup

To solve the problem of `sak::operator<<`, the original C++98 standard grew a feature
known as "Koenig lookup." It was named after
[Andrew Koenig](https://en.wikipedia.org/wiki/Andrew_Koenig_(programmer)) —
although [he says he did not invent it](http://www.drdobbs.com/cpp/a-personal-note-about-argument-dependent/232901443).
Eventually, as the feature continued to evolve, Koenig's name was dissociated from it;
today it is known simply as "argument-dependent lookup" (ADL).

> For a glimpse into the wild and woolly pre-ADL world, see Bjarne Stroustrup's
> [P0262 "Name Space Management in C++ (revised)"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/1993/N0262.pdf) (1993),
> particularly Appendix D.
> A rationale for the feature can be found in Koenig's
> [N0645 "Reconciling overloaded operators with namespaces"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/1995/N0645.pdf) (January 1995).
> In [September 1996](http://www.open-std.org/Jtc1/sc22/wg21/docs/wp/html/sep96/basic.html#basic.lookup.koenig),
> the draft standard (for what ultimately became C++98) gained a section with the stable-name
> `[basic.lookup.koenig]`.
> By [October 2005](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2005/n1905.pdf),
> that section had been renamed to `[basic.lookup.argdep]`.

Under ADL, whenever we see an *unqualified* call to a
possibly overloaded operator — such as `std::cout << b` — we'll look up the
name of that operator not only in our current namespace (`namespace ajo`),
but also in all the namespaces associated with the types of the _arguments_ to
the operator (namely, `namespace std` and `namespace sak`). This allows lookup
on `std::cout << b` to find `sak::operator<<`, and get it into the candidate set,
whereupon overload resolution chooses `sak::operator<<(std::ostream&, sak::bignum)`
as the best-matching candidate for this particular set of arguments.

Koenig's original proposal applied only to overloaded operators. But then
in 1996 [it was decided](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/1996/N0952.asc)
"to extend Koenig lookup to function names" — that is, to extend ADL to cover
non-operator functions such as `swap` and `get_next`.

    // SakBigNum.h
    namespace sak {
        struct bignum {
            bignum operator++();
        };
        std::ostream& operator<<(std::ostream&, bignum);
        bignum get_next(bignum);
    }

    // AjoBigNum.h
    namespace ajo {
        struct bignum {
            bignum operator++();
        };
        std::ostream& operator<<(std::ostream&, bignum);
        bignum get_next(bignum);
    }

    // AjoExtra.cpp
    #include "AjoBigNum.h"
    #include "SakBigNum.h"
    namespace ajo {
        void foo(int& x) {
            bignum b;        // refers to ajo::bignum
            ++b;             // calls ajo::bignum::operator++()
            std::cout << b;  // calls ajo::operator<<(ostream&, bignum)
            get_next(b);     // calls ajo::get_next(bignum)
        }

        void bar(int& x) {
            sak::bignum b;   // refers to sak::bignum
            ++b;             // calls sak::bignum::operator++()
            std::cout << b;  // calls sak::operator<<(ostream&, bignum)
            get_next(b);     // calls sak::get_next(bignum)
        }
    }

This makes sense if you think of free functions (like `swap`) as being part of the
interface of a class. I can write `myApple.eat()` without redundant qualification;
it seems reasonable that I should also be able to write `eat(myApple)` instead of
having to type out `my::eat(myApple)`. Sometimes, free functionality — whether
it's spelled `operator<<` or `eat` or `swap` — is intrinsically entangled with the
class's own interface. ADL reinforces that entanglement, for better and worse.


## When does ADL kick in?

The compiler applies ADL whenever it's doing name lookup (building a candidate set)
for an _unqualified function call._

If the name of the thing-being-called has any `::`-qualification at all, then ADL
won't kick in. [Godbolt:](https://godbolt.org/z/62g_9j)

    namespace A {
        struct A { operator int(); };
        void f(A);
    }
    namespace B {
        void f(int);
        void test() {
            A::A a;
            f(a);     // ADL, calls A::f(A)
            B::f(a);  // no ADL, calls B::f(int)
        }
    }

Also, if the thing is not "a function call," then ADL won't kick in. (That is,
we don't try to apply _Argument_-Dependent Lookup to names that don't have
_arguments._) ADL is defined in terms of the _`unqualified-id`_ grammar production, which
means that ADL does not apply to a redundantly parenthesized call such as `(f)(a)`,
because `(f)` is a _`primary-expression`_, not an _`unqualified-id`_.

> Several other rules in C++ are defined in terms of
> the more nebulous English term "name." For example, `return (x);` still triggers
> copy elision when `(x)` is the "name" of a local variable; and `(f)(a)` will
> still treat `f` as the "name" of an overload set. However, because `(f)` is
> grammatically not an _`unqualified-id`_, that overload set will be constructed
> using regular unqualified lookup, not argument-dependent lookup.

[Godbolt:](https://godbolt.org/z/3Thme0)

    namespace A {
        struct A { operator int(); };
        void f(A);
    }
    namespace B {
        void f(int);
        void f(double);
        void test() {
            A::A a;
            void (*fp)(int) = f;  // OK, no ADL
            void (*gp)(A::A) = f; // ERROR, no ADL, fails to find A::f
            f(a);     // ADL, calls A::f(A)
            (&f)(a);  // no ADL, calls B::f(int)
            (f)(a);   // no ADL, calls B::f(int)
        }
    }

Finally, and perhaps most importantly, ADL won't kick in if the thing being
called is _not a function!_ That is, before we do ADL for a call to `f`, we'll
do an ordinary unqualified lookup of `f`, which means we'll look
in our current scope and all enclosing scopes. If this ordinary unqualified lookup
finds something called `f`, and that `f` is _not a function_ (or a function template),
then we'll just use that `f`; we won't let ADL drag in any other namespaces.
It's only if we find a function (or function template) named `f`,
or if we don't find anything at all, that we'll move on to argument-dependent lookup.
[Godbolt:](https://godbolt.org/z/YqzHur)

    namespace A {
        struct A { operator int(); };
        void f(A);
        void g(A);
        void h(A);
        int i(A);
        int j(A);
    }
    namespace B {
        void f(int);
        auto h = [](int) {};
        using i = int;
        void test() {
            A::A a;
            f(a);           // ADL, calls A::f(A)
            g(a);           // ADL, calls A::g(A)
            h(a);           // no ADL: lookup found B::h which is not a function
            int ia = i(a);  // no ADL: lookup found B::i which is not a function
            int j = j(a);   // no ADL, and ERROR! lookup found local variable j
        }
    }


## How does an ADL lookup behave?

The first thing to know is that ADL looks only at the *types* of the arguments!
(Assuming they have types at all. [There are a couple of poorly-supported exceptions
for untyped arguments.](/blog/2019/04/08/adl-insanity) In this post, we will ignore those exceptions.)
Every bit of information about the arguments, other than their types, is thrown away and never considered.

    namespace A {
        struct A {};
    }
    namespace B {
        using T = A::A;
    }
    namespace C {
        B::T c;
    }
    namespace C {
        void test() {
            f(C::c);  // HERE
        }
    }

Here we invoke `f` with a value of type `A::A`. _That's all that matters._
Sure, the value comes from evaluating a variable that was defined in namespace `C`,
but that doesn't matter. Sure, the variable was originally declared using a type alias
`B::T`, but that doesn't matter.
All that ADL cares about is that the function argument, _after_ evaluation,
_after_ looking through all the type aliases, is some value of type `A::A`.

Also, ADL considers only _function_ arguments, not _template_ arguments.
[Godbolt:](https://godbolt.org/z/qm7FOG)

    namespace A {
        struct A { operator int(); };
        struct X {};

        template<class T>
        void f(int);
    }
    namespace B {
        template<class T>
        void f();

        void test() {
            A::A a;
            f<A::X>();   // OK, ADL doesn't consider A::f, calls B::f
            f<A::X>(a);  // OK, ADL considers A::f because of A::A, calls A::f
            f<A::X>(42); // ERROR: ADL doesn't consider A::f
        }
    }

If the call has multiple function arguments, then ADL will consider all of them. (In no particular
order. Nothing in this algorithm will depend on the order.)

From the set of _argument types_ in the call, we break each type down further. Each argument type
produces zero or more _associated types_ and _associated namespaces_, via a complicated ad-hoc
process. For the simplest cases, you can think of it as essentially "write down the name of the
type as unambiguously as possible and then extract all the class-names and all the innermost namespace-names
from that string."

For example,

- An argument of type `int` (or any primitive type) doesn't give us any associated types.

- An argument of type `NS::SomeClass` (or `NS::SomeClass*` or `NS::SomeClass&`) gives us one associated type —
    `NS::SomeClass` — and one associated namespace — `NS`.

- An argument of type `NN::NS::SomeClass` gives us one associated type —
    `NN::NS::SomeClass` — and one associated namespace — `NN::NS`. Notice that it does _not_ produce
    `NN` as an associated namespace.

- An argument of type `SomeClass::NestedClassOrEnum` gives us _two_ associated types:
    `SomeClass::NestedClassOrEnum` itself, and the class `SomeClass` of which it is a member.

- An argument of type `NA::A (*)(NB::B, NC::C)` — that is, "pointer to function taking `B` and `C` and returning `A`" —
    gives us three associated types (`NA::A`, `NB::B`, and `NC::C`) and three associated namespaces
    (`NA`, `NB`, and `NC`).

- An argument of type `NS::SomeTemplate<NA::A, NB::B>` gives us three associated types (itself, `NA::A`, and `NB::B`)
    and three associated namespaces (`NS`, `NA`, and `NB`).

- An argument of type `NS::SomeClass::SomeNestedTemplate<NA::A>` gives us three associated types (itself, `NA::A`,
    and `NS::SomeClass`) and two associated namespaces (`NS` and `NA`). ([Godbolt.](https://godbolt.org/z/pAJZas))

- An argument of type `NA::A`, where `NA::A` inherits (even privately!) from `NB::B`, gives us two
    associated types (`NA::A` and `NB::B`) and two associated namespaces (`NA` and `NB`).

This list of rules is not exhaustive; and not every rule is applied recursively.
For example, although class `A::B::C` has associated type `A::B` and class `A::B` has associated type `A`,
that doesn't imply that `A::B::C` must have associated type `A` — in fact it doesn't! ([Godbolt.](https://godbolt.org/z/jfMQWY))
(See [this blog post](/blog/2019/04/09/adl-insanity-round-2/) for more on that case.)

Having created sets of associated namespaces and associated types for each argument,
we merge them all together (and add our current namespace and all its parents, too,
of course) and do a lookup for declarations of the name `f` in
*any* of these namespaces. Our overload resolution for this call will consider all the
function declarations that we found in any of those places.


### Wait, what does it mean to do a lookup "in an associated type"?

When ADL performs lookup in an associated class type, what it's considering are the (namespace-scope)
_friends_ of that class.
It won't consider the member functions of that class — not even the static member functions.
[Godbolt:](https://godbolt.org/z/TJRblO)

    namespace N {
        struct A {
            enum E { E0 };

            friend void f(E);
            static void g(E);
        };
    }

    namespace My {
        void f(int);
        void g(int);
        void test() {
            N::A::E e;
            f(e);  // ADL considers N::f (friend of N::A)
            g(e);  // ADL does not consider N::A::g
        }
    }

The friend functions that are found by ADL _might_ have been declared in the namespace enclosing the associated
type, or they might be declared nowhere else (the so-called "hidden friend" idiom, about which I hope to write
more later). However, when the associated type declares a friend function using explicit namespace-qualification
(as in `friend void NS::f(int)`), ADL will ignore that declaration. So even though it is technically possible
to befriend functions from other namespaces, those functions will not thereby become ADL candidates.
[Godbolt:](https://godbolt.org/z/pDq5FE)

    namespace Unrelated {
        void f(int);
    }

    namespace NN {
        void f(int);
        namespace NA {
            struct A {
                enum E : int { E0 };

                friend void f(int);
                friend void NN::f(int);
                friend void Unrelated::f(int);
            };
        }
    }

    namespace B {
        void test() {
            NN::NA::A::E e;
            f(e);  // OK: ADL considers NA::f which is an unqualified
                   // ("namespace-scope") friend of NA::A, but not
                   // the other two friends
        }
    }


### One last thing

I wrote:

> Having created sets of associated namespaces and associated types for each argument,
> we merge them all together (and add our current namespace and all its parents, too,
> of course) and do a lookup for declarations of the name `f` in
> <b>any</b> of these namespaces. Our overload resolution for this call will consider all the
> function declarations that we found in any of those places.

ADL will consider only _function declarations_ (and, as usual, function templates).
If our lookup in some associated namespace finds a non-function declaration of `f`,
we'll simply ignore that declaration. And remember: if our initial unqualified lookup
found a non-function, then we won't do ADL at all! [Godbolt:](https://godbolt.org/z/Kmj-Wa)

    namespace A {
        struct A { operator int() const; };
        auto f = [](A, int) {};
        void g(A, int);
        void h(A, int);
    }
    namespace B {
        struct B { operator int() const; };
        void f(int, B);
        using g = int;
        void h(int, B);
    }

    namespace C {
        void f(int, int);
        void g(int, int);
        auto h = [](int, int) {};
        void test() {
            A::A a;
            B::B b;
            f(a, b);  // OK: ADL ignores the non-function A::f
            g(a, b);  // OK: ADL ignores the non-function B::g
            h(a, b);  // OK: no ADL
        }
    }


## Conclusion

There you go — now you know (almost) everything there is to know about argument-dependent lookup!
The parts I consciously neglected in this blog post are:

- The exact rules by which associated classes and associated namespaces are produced

- The special cases for arguments-with-no-type ([see here](/blog/2019/04/08/adl-insanity))

- The role of `using`-directives and `using`-declarations

- The ways ADL is used behind the scenes by ranged-`for` in C++11 and structured binding in C++17

- Idioms that rely on ADL, such as [the `std::swap` two-step](/blog/2020/07/11/the-std-swap-two-step) (2020-07-11),
    hidden friends, and niebloids
    (I hope to write more on each of these in future posts, and will link them from here when I do)

For more information on on ADL, see these resources:

- ["Argument-dependent lookup" on cppreference](https://en.cppreference.com/w/cpp/language/adl)

- ["What is Argument-Dependent Lookup (aka ADL or Koenig Lookup)?" on StackOverflow](https://stackoverflow.com/questions/8111677/what-is-argument-dependent-lookup-aka-adl-or-koenig-lookup)
