---
layout: post
title: 'How `hana::type<T>` "disables ADL"'
date: 2019-04-09 00:01:00 +0000
tags:
  argument-dependent-lookup
  standard-library-trivia
---

Today I learned another quirk of C++'s argument-dependent lookup.
The standard [says](http://eel.is/c++draft/basic.lookup.argdep#2.2):

> If `T` is a class type (including unions), its associated entities are:
> the class itself; the class of which it is a member, if any; and its direct and indirect base classes.
> [...]
> Furthermore, if `T` is a class template specialization, its associated namespaces and entities also
> include: the namespaces and entities associated with the types of the template arguments provided
> for template type parameters (excluding template template parameters);
> the templates used as template template arguments;
> the namespaces of which any template template arguments are members;
> and the classes of which any member templates used as template template arguments are members.

That is, the rules of ADL are not generally recursive. The relation "is-an-associated-entity-of"
is *non-transitive*.

    struct A {
        struct B {
            struct C {};
        };
    };

`A` is-an-associated-entity-of `A::B`, and `A::B` is-an-associated-entity-of `A::B::C`,
but `A` is *not* an associated entity of `A::B::C`.
Similarly,

    struct D {};

    template<class>
    struct E {
        struct F {};
    };

`D` is-an-associated-entity-of `E<D>`, and `E<D>` is-an-associated-entity-of `E<D>::F`,
but `D` is *not* an associated entity of `E<D>::F`.

The only recursive piece of the process is when we look at template type arguments,
and the template type arguments of *those* types, and the template type arguments of
*those* types... ad infinitum. `J` *is* an associated entity of `G<H<I<J>>>`.

[See all these examples in Godbolt Compiler Explorer.](https://godbolt.org/z/Zb3q9n)

----

[Boost.Hana](https://www.boost.org/doc/libs/1_68_0/libs/hana/doc/html/structboost_1_1hana_1_1type.html)
actually exploits this "hole" in ADL to its advantage.
([Relevant XKCD.](https://xkcd.com/1172/)) Consider Hana's `type<T>` metaclass.
It's just a simple tag type that represents the idea of "type `T`" without actually
being a `T`. If you or I or any mortal were implementing `type<T>`, we'd do it like this:

    template<class T> struct basic_type {};

There. Done. Now I can go ahead and use `basic_type<T>` in code like this:

    namespace unexpected {
        struct SomeType;
    }
    namespace A = unexpected;

    namespace expected {
        void helper(const basic_type<A::SomeType>&);

        void test() {
            helper( basic_type<A::SomeType>{} );
        }
    }

This works... until the maintainer of `namespace unexpected` decides to throw a wrench into
our code!

    namespace unexpected {
        struct SomeType;
        template<class T> void helper(T&&);
    }

Because `unexpected` is an associated namespace of `basic_type<unexpected::SomeType>`,
argument-dependent lookup will find the `helper` template; and overload resolution will
decide that `T&& [with T=basic_type<A::SomeType>]` is a better match for our rvalue argument than
`const basic_type<A::SomeType>&` is. So we'll end up getting hijacked by `unexpected::helper`.
This is obviously Not A Good Thing for a metaprogramming tool like `hana::type<T>`.

Therefore, Boost.Hana's `type<T>` _disables ADL_.

----

Here's how they do it.

    template<class T>
    struct Enhanced {
        struct type {};
    };

    template<class T>
    using enhanced_type = typename Enhanced<T>::type;

Now if we try the same code as before...

    namespace unexpected {
        struct SomeType;
        template<class T> void helper(T&&);
    }
    namespace A = unexpected;

    namespace expected {
        void helper(const enhanced_type<A::SomeType>&);

        void test() {
            helper( enhanced_type<A::SomeType>{} );
        }
    }

Now the argument to `helper` is of type `Enhanced<unexpected::SomeType>::type`.
According to the rules of ADL quoted at the top of this blog post, the associated namespaces
of `Enhanced<unexpected::SomeType>::type` are simply the namespaces of its associated types,
and those associated types are itself and `Enhanced<unexpected::SomeType>`. Associated-ness
is not transitive, so we *don't* continue onward to consider the associated types of
`Enhanced<unexpected::SomeType>` (which would have included something from `namespace unexpected`).

Therefore ADL on this call considers only what it can find in `namespace expected` (and its parent namespaces);
it does not search `namespace unexpected` because that's no longer an associated namespace.
There's only one viable overload of `helper` to consider, and it's the one we intended.
We have successfully "disabled" ADL's desire to interfere!

[Here](https://godbolt.org/z/c8jPN4)
is the whole example worked out on Godbolt; and [here](https://godbolt.org/z/it6V5V)
it is with the actual Boost.Hana library types.
