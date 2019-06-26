---
layout: post
title: 'Trivially Relocatable FAQ'
date: 2018-10-04 00:01:00 +0000
tags:
  relocatability
---

I presented P1144 "Object relocation in terms of move plus destroy" at the SG14 working meeting
on 2018-09-26. Here are three questions that came up during the Q&A and which have come up before;
I thought I'd put them into a little FAQ for quick reference.

----

> Isn't it a problem that this attribute is detectable by a type-trait?

The type trait does not detect the _attribute_. The type-trait detects the
_property_, which does not correspond one-to-one with the attribute itself.
P1144 does not propose any mechanism by which this or any other attribute
can be detected; that would be the proper domain of the Reflection study group.

    struct [[trivially_relocatable]] A { A(A&&); ~A(); };
    static_assert(std::is_trivially_relocatable_v<A>);

    struct B {};  // no attribute here!
    static_assert(std::is_trivially_relocatable_v<B>);

    struct C { A m; };  // no attribute here!
    static_assert(std::is_trivially_relocatable_v<C>);

The above example should clear up the interaction between the type-trait
and the attribute.

----

> What if I use the `[[trivially_relocatable]]` attribute on a class that is _currently_
> safe to memcpy, but then another developer makes a distant change that makes my class
> no longer safe to memcpy? I get undefined behavior; isn't this bad?

(This question led me to blog about [Pray-Mister-Babbage problems](/blog/2018/09/26/pray-mister-babbage/).)

The situation being described here is (I imagine) something like

    struct Animal {
        boost::container::vector<Leg> legs_;
    };

    struct [[trivially_relocatable]] Ark {
        std::pair<Animal, Animal> elephants;
        std::pair<Animal, Animal> kangaroos;
    };
    static_assert(std::is_trivially_relocatable_v<Ark>);

Here, three different people have been responsible for developing `boost::container::vector`,
`struct Animal`, and `struct Ark`. Two of those developers were unaware of `[[trivially_relocatable]]`,
and so we have

    static_assert(not std::is_trivially_relocatable_v<boost::container::vector<Leg>>);
    static_assert(not std::is_trivially_relocatable_v<Animal>);

Nevertheless, the developer of `struct Ark` wants to make sure that

    static_assert(std::is_trivially_relocatable_v<Ark>);

so he adds the attribute. This all works great, so far. But then the developer of `Animal`
decides to switch implementation strategies:

    struct Animal {
        boost::container::list<Leg> legs_;
    };

Now `struct Ark` is lying to the compiler! We get undefined behavior.

The foolproof solution to this problem is "[Don't do that.](https://www.youtube.com/watch?v=ri3aL8At44I&t=1m25s)"
If you aren't in control of your `Ark`'s innards, then you have no business putting a `[[trivially_relocatable]]`
sign out front. So one valid solution is

    struct Ark {  // don't use the attribute!
        std::pair<Animal, Animal> elephants;
        std::pair<Animal, Animal> kangaroos;
    };
    static_assert(implies(
        std::is_trivially_relocatable_v<Animal>,
        std::is_trivially_relocatable_v<Ark>
    ));

and then file a bug report against `struct Animal` complaining that `struct Animal` is not yet
marked as trivially relocatable even though it could be. And then the author of `struct Animal`
goes and files a bug against `boost::container::vector`, and then finally the recursion bottoms out
in the form of a Boost patch. The developer of `Ark` will then upgrade their Boost distribution,
and ta-da, `Ark` will have become trivially relocatable simply by following the Rule of Zero!

Suppose "file a bug upstream" is not an acceptable solution. Another possible engineering solution
would be to take all our upstream dependencies and wrap them up in wrappers that are
"extremely loud and incredibly narrow," like this:

    template<class T>
    struct [[trivially_relocatable]] ASSERT_TRIVIAL_RELOCATABILITY : T {
        using T::T;
    };

    struct Animal {
        boost::container::vector<Leg> legs_;
    };

    struct Ark {
        using A = ASSERT_TRIVIAL_RELOCATABILITY<Animal>;
        std::pair<A, A> elephants;
        std::pair<A, A> kangaroos;
    };
    static_assert(std::is_trivially_relocatable_v<Ark>);

Or if you control the definition of `struct Animal`, then you can do even better:

    struct Animal {
        ASSERT_TRIVIAL_RELOCATABILITY<boost::container::vector<Leg>> legs_;
    };
    static_assert(std::is_trivially_relocatable_v<Animal>);

    struct Ark {
        std::pair<Animal, Animal> elephants;
        std::pair<Animal, Animal> kangaroos;
    };
    static_assert(std::is_trivially_relocatable_v<Ark>);

The reason this version is "better" is that when your junior dev comes along, he'll see this:

    struct Animal {
        ASSERT_TRIVIAL_RELOCATABILITY<boost::container::vector<Leg>> legs_;
    };

and have to think *pretty hard* about what to change it to. If he changes it to

    struct Animal {
        boost::container::list<Leg> legs_;
    };

then some of your `static_assert`s will fail, but you won't get undefined behavior.
Or, if he changes it to

    struct Animal {
        ASSERT_TRIVIAL_RELOCATABILITY<boost::container::list<Leg>> legs_;
    };

then at least he'll be *loudly* lying to the compiler, and you can maybe catch that in
code review. The "spooky action" here is no longer happening at a distance; the lie is
encapsulated in a single line.

Certainly if you are worried about your developers misusing `[[trivially_relocatable]]`,
it would be perfectly appropriate to enforce a rule against *ever* explicitly using that
attribute in your codebase. Leave the attribute to libc++ and Boost and so on. In your
own codebase, simply follow the Rule of Zero; you'll still benefit from the feature.

----

> Instead of standardizing the attribute, could we just require that users specialize
> the `std::is_trivially_relocatable` type-trait themselves?

That is, we could still have the core-language rules for propagating the property of
trivial relocatability through Rule-of-Zero class types and lambdas; but instead of

    namespace my {
        struct [[trivially_relocatable]] Animal {
            boost::container::vector<Leg> legs_;
        };
    } // namespace my

we'd make people write

    #include <type_traits>  // for the primary template

    namespace my {
        struct Animal {
            boost::container::vector<Leg> legs_;
        };
    } // namespace my

    template<> struct std::is_trivially_relocatable<my::Animal> : std::true_type {};

There are two problems I see with this idea. One is concisely expressed by the snippets above:
specializing a type-trait is *hard*, requiring expert-level knowledge of template programming,
plus expert-level knowledge of namespaces, plus a subexpert-level tolerance for tedious typing.

The second problem is that this idea implies a *feedback loop* between the library
type-trait and the core-language propagation rules, which would be not only
[contrary to the spirit of C++](/blog/2018/04/15/built-in-library-types/),
but extremely confusing (and perhaps difficult) to implement.

    struct A { A(A&&); };
    struct B : A {};

    static_assert(not std::is_trivially_relocatable_v<B>, "????");

    template<> struct std::is_trivially_relocatable<A> : std::true_type {};

    static_assert(std::is_trivially_relocatable_v<B>, "????");

Philosophically, as usual, we're talking about the design of a customization point; and
[every customization point has two pieces](/blog/2018/03/19/customization-points-for-functions/).
In P1144's design, piece A (the "warrant") is named `[[trivially_relocatable]]` and piece B
(the "detection mechanism") is named `std::is_trivially_relocatable`.

This question is basically asking: Could we still have a sane design if we collapsed pieces A and B
into a *single* piece named `std::is_trivially_relocatable`? And I firmly believe that the answer
to that question is: "No, that would not produce a sane design."
