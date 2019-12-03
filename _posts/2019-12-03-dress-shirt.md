---
layout: post
title: 'Setup and teardown routines in non-flat class hierarchies'
date: 2019-12-03 00:01:00 +0000
tags:
  classical-polymorphism
  rant
---

Here's a C++ problem where I still don't know what's a good way to deal with it.
(But I asked on Slack the other day and got some good-sounding advice from Kevin Zheng.
See further down in this post.)
Consider a class hierarchy something like this. We have an abstract base class `Garment`,
a derived class `Shirt`, and a leaf class `DressShirt` that derives from `Shirt`.
To `don` a `DressShirt` means to first do everything you'd do to `don` the `Shirt` part of
the object, and then additionally apply the cufflinks.

    class Garment {
    public:
        virtual void don() = 0;
    };

    class Shirt : public Garment {
        Sleeves sleeves_;
    public:
        void don() override {
            sleeves_.insert_arms();
        }
    };

    class DressShirt final : public Shirt {
        Cufflinks cufflinks_;
    public:
        void don() override {
            this->Shirt::don();
            cufflinks_.apply();
        }
    };

This way of organizing things does work, but it fails to conform to the
Non-Virtual Interface idiom. (Previously on this blog:
["A C++ acronym glossary"](/blog/2019/08/02/the-tough-guide-to-cpp-acronyms/#nvi) (2019-08-02),
["CppCon 2019 talks are up"](/blog/2019/10/18/quotable-cppcon-talks/#jon-kalb-back-to-basics-object-o) (2019-10-18).)
The NVI version would look more like this ([Godbolt](https://godbolt.org/z/zeQW_z)):

    class Garment {
        virtual void do_don() = 0;
    public:
        void don() { this->do_don(); }
    };

    class Shirt : public Garment {
        Sleeves sleeves_;
        void do_don() override {
            sleeves_.insert_arms();
        }
    };

    class DressShirt final : public Shirt {
        Cufflinks cufflinks_;
        void do_don() override {
            this->Shirt::don();  // OOPS!
            cufflinks_.apply();
        }
    };

...Except that [this produces an infinite loop!](https://godbolt.org/z/zeQW_z)
The NVI idiom prevents anyone from deliberately calling a "sliced" version of
any virtual function via qualified call syntax. You can't say `myDressShirt.Shirt::do_don()`
because `do_don()` is private at every level. However, the NVI does not protect us
against someone *trying* to make a qualified call and accidentally getting themselves
into trouble!

In the above code, `this->Shirt::don()` is a silly (but valid) way to write `this->don()`;
which (because we're using the NVI idiom) means `this->do_don()`; which means
infinite recursion.

To express the statement that "to don a dress shirt, you first don the shirt part and
then apply the cufflinks," we would have to refactor our class hierarchy in some way.
For example:

    class Garment {
        virtual void do_don() = 0;
    public:
        void don() { this->do_don(); }
    };

    class Shirt : public Garment {
        Sleeves sleeves_;
    protected:                      // OK but yuck!
        void do_don() override {
            sleeves_.insert_arms();
        }
    };

    class DressShirt final : public Shirt {
        Cufflinks cufflinks_;
        void do_don() override {
            this->Shirt::do_don();  // OK
            cufflinks_.apply();
        }
    };

However, now we're using `protected` (which personally I consider a code smell,
by the way), and we've cracked open our nice safe NVI `don` method by calling
`Shirt::do_don` with qualified syntax — exactly the thing we were using NVI
to get away from!

Kevin Zheng suggested (something isomorphic to) the following design:

    class Garment {
        virtual void do_don() = 0;
    public:
        void don() { this->do_don(); }
    };

    class Shirt : public Garment {
        Sleeves sleeves_;
        void do_don() override {
            this->shirt_specific_don();
        }
    protected:
        void shirt_specific_don() {
            sleeves_.insert_arms();
        }
    };

    class DressShirt final : public Shirt {
        Cufflinks cufflinks_;
        void do_don() override {
            this->shirt_specific_don();
            cufflinks_.apply();
        }
    };

This design has the same essential shape as the previous one, except
that we are never using the verboten qualified call syntax. Instead of
`this->Shirt::do_don()`, we call `this->shirt_specific_don()`, whose name
clearly indicates its purpose.

...Ish. If there's some additional code required to don a `Garment`, then
it's unclear whether `shirt_specific_don()` should itself call
`garment_specific_don()`, or whether `DressShirt::do_don()` should call
both `garment_specific_don()` and `shirt_specific_don()` in that order.

> Okay, since `Garment` is the root class, we could just put our
> `Garment`-specific code directly into the non-virtual `Garment::don` method.
> But let's ignore that possibility, because it wouldn't apply to a
> `ClothGarment` class introduced between `Garment` and `Shirt`.

And we still haven't gotten rid of the requirement for `DressShirt` to
know about protected details of `Shirt`. In other words, `DressShirt`
must know *more* about `Shirt` than you'd normally expect to need to
know — more than its public API. Ideally, I'd like to be able to inherit
from `Shirt` just as easily as I can compose with it.

But maybe this is just another argument in favor of "Prefer composition
over inheritance." If we use composition, then we have something more
like this:

    class Garment {
        virtual void do_don() = 0;
    public:
        void don() { this->do_don(); }
    };

    class Shirt final : public Garment {
        Sleeves sleeves_;
        void do_don() override {
            sleeves_.insert_arms();
        }
    };

    class DressShirt final : public Garment {
        Shirt shirt_;
        Cufflinks cufflinks_;
        void do_don() override {
            shirt_.don();
            cufflinks_.apply();
        }
    };

That is, we can avoid this problem related to non-flat class hierarchies
by... _flattening our class hierarchy!_

Is this really the only elegant way out of our difficulty?
Am I missing some idiom or pattern that would make our original
`DressShirt` less wonky? If you know a good answer not mentioned
here, please let me know!
