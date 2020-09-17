---
layout: post
title: "Concepts can't do quantifiers"
date: 2020-08-10 00:01:00 +0000
tags:
  concepts
excerpt: |
  People frequently ask how to do things like this in C++20:

  * Make a concept `Renderer` that is satisfied if and only if `t.render(u)` is valid for all `Printable` types `U`.
      (From [/r/cpp](https://www.reddit.com/r/cpp/comments/i6elbg/concepts_question_constraint_type_to_have_a/).)

  * Make a concept `ValidSize` that is satisfied if and only if `t` is convertible to some integral type `U`.
      (From [the standard](http://eel.is/c++draft/alg.modifying.operations#alg.copy-11).)

  The key words above are "all" and "some." In mathematics, these are known as the _universal_ and _existential_ quantifiers.
  C++20 Concepts do not support either quantifier.
---

People frequently ask how to do things like this in C++20:

* Make a concept `Renderer` that is satisfied if and only if `t.render(u)` is valid for all `Printable` types `U`.
    (From [/r/cpp](https://www.reddit.com/r/cpp/comments/i6elbg/concepts_question_constraint_type_to_have_a/).)

* Make a concept `ValidSize` that is satisfied if and only if `t` is convertible to some integral type `U`.
    (From [the standard](http://eel.is/c++draft/alg.modifying.operations#alg.copy-11).)

The key words above are "all" and "some." In mathematics, these are known as the _universal_ and _existential_ quantifiers:

* `Renderer<T>` ⇔ ∀`U`∈`Printable`: `t.render(u)` is well-formed

* `ValidSize<T>` ⇔ ∃`U`∈`Integral`: `U u = t;` is well-formed

C++20 Concepts do not support either quantifier.


## Universal quantification

Consider this code:

    struct Apple {};

    template<class T>
    concept Pommivorous = requires(T t, Apple a) {
        { t.eat(a) };
    };

    struct Horse {
        void eat(Apple);
    };
    static_assert(Pommivorous<Horse>);

    struct Goat {
        template<class T>
        void eat(T);
    };
    static_assert(Pommivorous<Goat>);

`Pommivorous` says pretty much everything there is to say about `Horse`.
But `Goat` is more interesting. A `Goat` can eat literally anything. Can we make a concept `Omnivorous`
that is satisfied by classes like `Goat` that can eat _anything_ (but not by classes like `Horse`)?

No, we cannot, because the notion of "eat _anything_" involves a universal quantifier!
The best we can do is fake it, by doing something like this:

    template<class T>
    concept Omnivorous = requires (T t, Apple a,
            Banana b, int c, std::string d) {
        { t.eat(a) };
        { t.eat(b) };
        { t.eat(c) };
        { t.eat(d) };
    };

Any animal that can eat apples, bananas, ints, and strings is _probably_ omnivorous, right? Sure.
But we haven't actually _expressed_ a universal quantifier in this code. We've just listed off
a finite set of special cases.
The C++ compiler fundamentally cannot deal with the abstract idea of "any class at all." C++ can deal
only in concrete examples, and only a finite number of them. It can't help us test a proposition
for _all possible_ classes.

By the way, the same is true for any infinite or open set of types. We can't make a concept `Omnivorous`
that means "eats everything ever," and equally, we can't make a concept that means
"eats every type that satisfies `Vegetable`," nor can we make one that means "eats all pointer types."
For a closed set, such as "eats all integral types," we can do it, but only by listing out and testing
every member of the set, one by one. (C++20 has exactly 16 integral types.)


### But we can test for "all possible values"?

"Wait a minute," you might say. "Consider _this_ code..."

    template<class T>
    concept Munchicus = requires(T t, int i) {
        { t.eat(i) };
    };

    struct Digitus {
        void eat(int);
    };
    static_assert(Munchicus<Digitus>);

"Doesn't this code correctly convey the meaning that a `Digitus` can eat any `int` value?
That is, `t.eat(1)`, `t.eat(2)`, `t.eat(3)`,... These are _all_ well-formed, according to concept
`Munchicus`. So how can you say that C++ can't deal with universal quantification?"

That's a good point! Leaving aside some pedantic quibbles,<sup>[[1]](#pedantic-quibbles)</sup> what you say is true:
C++ _can_ deal with universal quantification in some areas. Specifically, C++ believes that
what's true for one int (say, `i`) is going to hold true for _all_ ints (say, `1`, `2`, `3`,...).
The whole idea of _type-checking_ an expression is based on this fundamental intuition that
one value of type `int` behaves pretty much the same as any other. In fact, this is pretty much
what we mean when we talk about "type" in _any_ programming language!
We can let the expression `i` stand in for "`1`, or `2`, or `3`,..." precisely _because_ all these
expressions share the same bundle of behaviors, a.k.a. the same type.

C++'s static type system is essentially a vocabulary for talking about the ways in which
one int "is like" any other int.

C++ has no vocabulary for talking about the ways in which one whole type "is like" any other
whole type.
Type-checking permits (in fact, requires) us to make sure our value-manipulating code preserves
certain properties _universally quantified_ over all possible values of a given type. But it's
difficult even to imagine a mechanism that would ensure our type-manipulating code preserves
certain properties _universally quantified_ over all possible _types!_

> One possible analogue of type-checking is known as "definition checking." See
> ["Concept definition-checking and its woes"](/blog/2019/07/22/definition-checking-with-if-constexpr/) (2019-07-22).

To put it another way: The part of any programming language that _can_ deal with universal quantifiers
is the part that we refer to as the "type system." Concepts lie essentially outside the bounds
of C++'s type system. One easy way to tell: We don't refer to concepts as "types." :)


### Pedantic quibbles

The pedantic quibbles mentioned above include that value categories are tricky:

    struct Timidus {
        void eat(int&);
    };
    static_assert(Munchicus<Timidus>);  // yet t.eat(42) is ill-formed

and that the expression `0` doesn't always mean `0`:

    struct Assistus {
        void eat(long);
        void eat(void*);
    };
    static_assert(Munchicus<Assistus>);  // yet t.eat(0) is ill-formed

These specific quirks of C++ are unrelated to the point of this blog post, which is that
concepts can't quantify over types _in general_.


## Existential quantification

Consider this code:

    struct Apple {};
    struct Carrot {};

    struct Horse {
        void eat(Apple);
        void eat(Carrot);
    };

    struct Goat {
        template<class T>
        void eat(T);
    };

    struct Tree {
        void photosynthesize();
    };

Can we make a concept `Eater` that is satisfied by classes like `Goat` and `Horse`, but not by `Tree`?
Essentially, we want to match any class that can `eat` _something_ (but without specifying up front what
that _something_ is).

No, we cannot, because the notion of "eat _something_" involves an existential quantifier!
It's equivalent to saying: "Class `T` satisfies `Eater` if-and-only-if _there exists_ some class `U`
such that `T` eats `U`."

There's not even any half-decent way to fake the existential quantifier. It reminds me of
[that one Richard Dawkins quote](https://en.wikiquote.org/wiki/Richard_Dawkins):

> We are all atheists about most of the gods that humanity has ever believed in. Some of us just go one god further.

If someone claims to believe in _all_ the gods, you can ask them if they believe in ten or twenty specific gods
you happen to know about, and if they say yes to all of the above, then that's at least _some_ evidence that
their claim might be correct. But if somebody claims only to believe in _some_ god, you might ask them about twenty
specific gods — Zeus, Brahma, Odin, Marduk, Bes, Jehovah — and their answer might be "no" every single time.
Distinguishing a monotheist from an atheist is surprisingly difficult!

In practice, the best way to fake this up is to require the purported theist to tell you the name of (one of) their god(s).
In our `Eater` example, we might write:

    template<class T>
    concept Eater = requires(T t, T::food_type u) {
        { t.eat(u) };
    };

    struct Horse {
        using food_type = Apple;
        void eat(Apple);
        void eat(Carrot);
    };

    struct Fox {
        void eat(Chicken);
    };

Under this regime, `Horse` is considered an `Eater` because it exposes a member typedef `food_type` and
a member function `eat` that takes that type as a parameter. `Fox` has an `eat` method, but is
not considered an `Eater`, because it does not expose its `food_type`.

In this example, the fact that `Horse` can also eat `Carrot` is effectively irrelevant. But if you
imagine that `Apple` is `std::string` and `Carrot` is `const char*` (or `std::string_view`), you
can maybe see how such an overload set could be useful in practice.


### Footnote on SFINAE-friendliness

Both in the universal and in the existential case, beware of the fact that
not all failure modes are SFINAE-friendly. [Godbolt:](https://godbolt.org/z/rsWfco)

    struct Human {
        void eat(Apple);
        template<class T=void>
        auto eat(Glass) { T t; }
    };

Even though `Human` is capable of eating _something_ (namely `Apple`), it is problematic to test this
by feeding arbitrary items to the `Human`, because merely testing the well-formedness of `t.eat(Glass{})`
for `Human t` will trigger a hard error.

This caveat applies to both the existential quantifier and the universal quantifier; it is
dangerous to evaluate `Eats<Human, Glass>` regardless of whether you plan to `||` the result or `&&` it.


## Techniques for faking quantifiers

C++ Concepts don't support universal or existential quantifiers. There is no hack I can teach you that
will change this fundamental fact. But here are some tricks you can use to _change the question you're asking_
from one that Concepts can't solve, to one that it can.

If your problem is of the form "T holds relationship R with all types" or "T can be R'ed with any type,"
it likely involves a universal quantifier.
Try to recast it using phrases like "T holds relationship R with `int` (or any specific, concrete type)"
or "T can be R'ed with any type on this concrete list."

If your problem is of the form "T holds relationship R with some type" or
"there exists some type U such that T can be R'ed with U,"
it likely involves an existential quantifier.
Try to recast it in a form such as "T provides a member typedef `T::foo_type`, and T can be R'ed with `T::foo_type`."

The following two mathematical identities are sometimes known as
[De Morgan's laws for quantifiers](https://en.wikipedia.org/wiki/De_Morgan%27s_laws_for_quantifiers).

* $$\neg\exists x\in X:P(x)$$ is equivalent to $$\forall x\in X:\neg P(x)$$

* $$\neg\forall x\in X:Q(x)$$ is equivalent to $$\exists x\in X:\neg Q(x)$$

If your problem is of the form "T does not hold relationship R with any other type" or
"there is no type such that...,"
it likely involves a universal quantifier in disguise.
First use the mathematical identities above to rewrite it in positive form
(e.g. "T holds relationship not-R with all other types"),
and then recast it by applying the previous techniques (e.g. "T holds relationship not-R with `int`").
