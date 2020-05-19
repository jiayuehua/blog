---
layout: post
title: "Covariance and contravariance in C++"
date: 2019-01-20 00:02:00 +0000
tags:
  concepts
  implementation-divergence
  metaprogramming
  templates
  variadic-templates
---

Today I'd like to try to explain _covariance and contravariance_, and the many places we
see that notion pop up in C++.


## Covariance and contravariance, by example

Consider the following veterinary analogy.
We have the notion of an animal. All animals can make noise.
Cats are animals. Horses are animals that you can ride.

    struct Animal {
        virtual void make_noise() = 0;
        virtual ~Animal() = default;
    };

    struct Cat : Animal {
        void make_noise() override { puts("meow"); }
    };

    struct Horse : Animal {
        void make_noise() override { puts("neigh"); }
        virtual void ride() { puts("okay sure"); }
    };

Crucially, it is _part of the static type system_ that you can't
ride cats. It's not just that you try and it throws an exception, or
something; cats literally don't have a `ride` method. Okay?

Now we add a _producer_ of animals; let's say, an animal breeder.
(I was going to say "a mama animal," but then we'd have to decide
whether to express that a `MamaCat` was both a `MamaAnimal` and a `Cat`,
and [that way lies madness](/blog/2018/11/26/remember-the-ifstream).)

    struct AnimalBreeder {
        virtual Animal *produce() = 0;
    };

    struct CatBreeder : AnimalBreeder {
        Animal *produce() override { return new Cat; }
    };

    struct HorseBreeder : AnimalBreeder {
        Animal *produce() override { return new Horse; }
    };

Notice that a cat breeder IS-AN animal breeder. If you say to me,
"Quick, I need the phone number of someone who can produce me an animal!"
and I give you the number of a cat breeder, you will be happy.
(That's logic!)

Because a cat IS-AN animal, any breeder that produces cats by definition
IS-A breeder that produces animals. We say that the relationship between
`Animal` and `Cat` is _covariant with_ the relationship between
`AnimalBreeder` and `CatBreeder`.

----

Notice that a `CatBreeder` only ever produces `Cat`s, and a
`HorseBreeder` only ever produces `Horse`s. Conveniently, C++ permits us
to encode that extra snippet of information into our source code.

    struct AnimalBreeder {
        virtual Animal *produce() = 0;
    };

    struct CatBreeder : AnimalBreeder {
        Cat *produce() override { return new Cat; }
    };

    struct HorseBreeder : AnimalBreeder {
        Horse *produce() override { return new Horse; }
    };

This is a feature known as "covariant return types."

----

Now consider what happens if you _already have_ a `Cat`, and you come to
me and say, "Quick, I need the phone number of someone who can treat my
sick cat!" I'd try to find you an animal doctor.

    struct DoctorDolittle {
        virtual void treat(Animal *);
    };

    struct DoctorHackenbush {
        virtual void treat(Horse *);
    };

    struct DoctorKeat {
        virtual void treat(Cat *);
    };

(Footnote: My lovely wife, the namesake of `DoctorKeat`, would also have a
`treat(Dog *)` method _at least_. But for the purposes of this example,
we'll stick with cats.)

According to the static type system, Dr. Keat treats only cats, and
[Dr. Hackenbush](https://www.imdb.com/title/tt0028772/) treats only
horses. Whereas, of course, [Dr. Dolittle](https://www.imdb.com/title/tt0061584/)
can treat _anything!_

What does this mean for our class hierarchy? Well, consider `Horse`.
A horse can do anything a generic animal can do, _plus more_ (namely, be ridden),
so we made `Horse` inherit from `Animal`. `DoctorDolittle` can do anything
`DoctorHackenbush` can do, _plus more_, so logically `DoctorDolittle` should inherit
from `DoctorHackenbush`.

Taken literally, this would lead to a bit of a mess, because `DoctorDolittle`
would have to multiply inherit from `DoctorHackenbush` _and_ `DoctorKeat`
_and_ a bunch more animal-doctor classes that we don't even know about yet.
Which is crazy. So let's back off and just consider Dolittle and Hackenbush
in isolation.

    struct HorseDoctor {
        virtual void treat(Horse *);
    };

    struct AnimalDoctor : HorseDoctor {
        void treat(Animal *) override;  // hmm...
    };

This hierarchy is logically correct. If you come to me and say,
"I need the number of someone to treat my sick horse!" then you will be
happy with any `HorseDoctor`; but if you need a horse doctor to treat your
sick cat, you'll need a _more capable_ horse doctor. If it helps, consider
that an `AnimalDoctor` like Dr. Dolittle is also a `CatAndHorseDoctor`, which
sounds more like a proper subclass of `HorseDoctor`.

We say that the relationship between `Animal` and `Horse` is _contravariant with_
the relationship between `AnimalDoctor` and `HorseDoctor`: an `AnimalDoctor`
IS-A `HorseDoctor` precisely because a `Horse` IS-AN `Animal`.

Perhaps inconveniently, C++ does _not_ permit us to write the function marked
`hmm...` above. C++'s classical OOP system supports "covariant return types," but it
does not support "contravariant parameter types."

This concludes our explanation of covariance and contravariance in the classical
OOP system. Now let's look at other places the notion shows up in C++.


## In `std::function`

(Thanks to [Michał Dominiak](https://www.youtube.com/watch?v=nlxA7CWfy78) for giving
this example.)

The C++ core language may not support contravariant parameter types, but the
standard library does, in the form of `std::function` and its many implicit
conversions. [Example:](https://godbolt.org/z/7QEcCF)

    struct Animal {};
    struct Horse : Animal {};

    using AnimalDoctor = std::function<void(Animal*)>;
    using HorseDoctor = std::function<void(Horse*)>;

    auto hackenbush = [](Horse *) {};
    auto dolittle = [](Animal *) {};

    HorseDoctor a = hackenbush;  // OK
    HorseDoctor b = dolittle;  // OK
    AnimalDoctor c = hackenbush;  // ERROR
    AnimalDoctor d = dolittle;  // OK

We see that `dolittle` is an acceptable `AnimalDoctor` and thus also a `HorseDoctor`;
whereas `hackenbush` is nothing more than a `HorseDoctor`.

Similarly, `std::function` supports covariant return types. [Example:](https://godbolt.org/z/7QEcCF)

    using AnimalBreeder = std::function<Animal*(void)>;
    using HorseBreeder = std::function<Horse*(void)>;

    auto ben_ishak = []() -> Horse * { return new Horse; };
    auto moreau = []() -> Animal * { return /*some Animal*/; };

    HorseBreeder e = ben_ishak;  // OK
    HorseBreeder f = moreau;  // ERROR
    AnimalBreeder g = ben_ishak;  // OK
    AnimalBreeder h = moreau;  // OK

If you say to me, "Quick, I need the phone number of someone who can produce me an animal!"
and I give you the number of Dr. Moreau, you will be happy.
(That's logic!) But if you need specifically a _horse_, I should send you to a _more capable_
breeder, who is not only able to produce you an animal but _also_ to give you compile-time
assurance that it is actually a horse. I'd better not send you to Moreau.
With Moreau, you never know what you're going to get.


## In `const`-correctness

Covariance and contravariance are fully supported in both C and C++
for implicit conversions involving the `const` qualifier.
[Here's](https://godbolt.org/z/fhglgm) covariance.

(Remember that in C, when we want to _produce_ something, we use
an out-parameter — we "pass by pointer.")

    using Animal = int const *;
    using Horse = int *;

    const int tiger = 42;
    int stallion = 42;

    void moreau(Animal *r) { *r = &tiger; }
    void ben_ishak(Horse *r) { *r = &stallion; }

This snippet is logically correct! A horse is a _more capable_ animal.
In our classical OOP example, a horse was an animal you could `ride()`.
In this example, a horse is an animal whose target you can `++`.
(That is, a mutable int is a _more capable_ const int. Rust got it right!)

Now you come to me and say, "Quick, I need the phone number of someone
who can produce me a horse! It's a gift for my daughter."

    Horse giftbox;  // to be filled in by the producer
    ben_ishak(&giftbox);  // OK
    moreau(&giftbox);  // ERROR

I can send your empty gift box over to Ben Ishak, whom I trust to
fill it with a horse. I _can't_ send your box to Moreau; he'll probably
fill it with some other kind of animal. With Moreau, you never know what
you're going to get.

(Incidentally, it is super important that the compiler does enforce this
rule for us. If `moreau(&giftbox)` were permitted to compile,
then on Christmas morning your daughter would probably try to
`++*giftbox` and end up riding that tiger by mistake.)

That's covariance: the relationship between `int const *` and `int *`
is _covariant with_ the relationship between `void(int const **)`
and `void(int **)`. Now for contravariance:

    using Animal = int const *;
    using Horse = int *;

    void dolittle(Animal);
    void hackenbush(Horse);

If you come to me asking for someone to treat your cat (an `Animal`
that is not a `Horse`), I can send you to Dolittle but not to Hackenbush:

    Animal your_cat;
    dolittle(your_cat);    // OK
    hackenbush(your_cat);  // ERROR

That's contravariance: the relationship between `int const *` and `int *`
is _contravariant with_ the relationship between `void(int const *)`
and `void(int *)`.


## In non-type template parameters (C++17)

In C++17, non-type template parameters (NTTPs) demonstrate contravariance:

    #define Animal auto
    #define Horse int
    #define Cat int*

    template<Animal> struct dolittle {};
    template<Horse> struct hackenbush {};

    template<template<Cat> class>
    struct you {};

Now `you` come and say to me, "Quick, I need someone who can treat my sick cat!"
I can send you to Dr. Dolittle, but not to Dr. Hackenbush:

    template struct you<dolittle>;
    template struct you<hackenbush>;

([GCC, Clang, and ICC agree on this point.](https://godbolt.org/z/PUcltq)
MSVC believes it'd be fine to send you to Hackenbush.)

For the covariant case, we can use our "gift box" metaphor:

    static int tiger = 0;

    template<template<Horse> class Giftbox>
    struct ben_ishak { Giftbox<1> stallion; };

    template<template<Animal> class Giftbox>
    struct moreau { Giftbox<&tiger> surprise; };

    template<Horse> struct your_giftbox {};
    template struct ben_ishak<your_giftbox>;
    template struct moreau<your_giftbox>;

In this case, [not a single compiler detects](https://godbolt.org/z/p5Kcc0) the problem with
`moreau<your_giftbox>`: everyone freely instantiates `moreau` and lets Moreau place a `&tiger`
inside the `Giftbox`, which results in a hard error.

### In non-type template parameters with alias templates

By the way, we can replace `dolittle`, `hackenbush`, and `your_giftbox` with alias templates;
it [doesn't change](https://godbolt.org/z/gps6Iz) any compiler's behavior.
(MSVC still incorrectly accepts `you<hackenbush>`;
everyone still incorrectly accepts `moreau<your_giftbox>`.)

    template<Animal> using dolittle = int;
    template<Horse> using hackenbush = int;
    template<Horse> using your_giftbox = int;

In this case, every compiler does give a hard error at the point of instantiation of `Giftbox<&tiger>`,
which is refreshing — but nobody detects that `your_giftbox` should never have been given to `moreau`
in the first place.


## In constrained template type parameters (C++2a)

The C++2a Working Draft permits template type parameters to be constrained,
and even gives the programmer a shorthand syntax which (unfortunately? fortunately?)
makes the following example look almost identical to the NTTP example above.

    template<class T> concept Animal = true;
    template<class T> concept Horse = Animal<T> && sizeof(T)==4;
    template<class T> concept Cat = Animal<T> && sizeof(T)==2;

    template<Animal> struct dolittle {};
    template<Horse> struct hackenbush {};

    template<template<Cat> class>
    struct you {};

Now `you` come and say to me, "Quick, I need someone who can treat my sick cat!"
I can send you to Dr. Dolittle, but not to Dr. Hackenbush:

    template struct you<dolittle>;
    template struct you<hackenbush>;

([GCC and Clang agree](https://godbolt.org/z/rAlo02) on this point.)

For the covariant case, we can use our "gift box" metaphor:

    template<template<Horse> class Giftbox>
    struct ben_ishak { Giftbox<char[4]> stallion; };

    template<template<Animal> class Giftbox>
    struct moreau { Giftbox<char[42]> tiger; };

    template<Horse> struct your_giftbox {};
    template struct ben_ishak<your_giftbox>;
    template struct moreau<your_giftbox>;

[GCC and Clang agree](https://godbolt.org/z/g9DmV1) that `ben_ishak<your_giftbox>`
is acceptable and `moreau<your_giftbox>` is unacceptable.

### In constrained template type parameters with alias templates

By the way, we can replace `dolittle`, `hackenbush`, and `your_giftbox` with alias templates;
it [doesn't change](https://godbolt.org/z/Gjiafv) any compiler' behavior.

    template<Animal> using dolittle = int;
    template<Horse> using hackenbush = int;
    template<Horse> using your_giftbox = int;


## In variadic template parameters

A variadic parameter list (that could have any number of parameters) is like an
`Animal`; a parameter list constrained to take only two parameters is like a `Horse`.
(A horse IS-AN animal, but not all animals are horses.)

    #define Animal class...
    #define Horse class, class
    #define Cat class, class, class

    template<Animal> struct dolittle {};
    template<Horse> struct hackenbush {};

    template<template<Cat> class>
    struct you {};

Now `you` come and say to me, "Quick, I need someone who can treat my sick cat!"
I should be able to send you to Dr. Dolittle, but not to Dr. Hackenbush:

    template struct you<dolittle>;
    template struct you<hackenbush>;

(Here, Clang falters by [incorrectly rejecting](https://godbolt.org/z/nIehbQ)
`you<dolittle>`. GCC, ICC and MSVC accept it.
Everybody correctly rejects `you<hackenbush>`.)

For the covariant case, we can use our "gift box" metaphor:

    template<template<Horse> class Giftbox>
    struct ben_ishak { Giftbox<int, int> stallion; };

    template<template<Animal> class Giftbox>
    struct moreau { Giftbox<int, int, int> tiger; };

    template<Horse> struct your_giftbox {};
    template struct ben_ishak<your_giftbox>;
    template struct moreau<your_giftbox>;

In this case, [not a single compiler detects](https://godbolt.org/z/hr7sZP) the problem with
`moreau<your_giftbox>`: everyone freely gives `your_giftbox` to `moreau` and lets Moreau try
to place a tiger inside it, which results in a hard error.


### In variadic template parameters with alias templates

You might think this shouldn't be different (and you'd be right that it _shouldn't_), but
in fact we do see a change in MSVC's behavior...

    template<Animal> using dolittle = int;
    template<Horse> using hackenbush = int;
    template<Horse> using your_giftbox = int;

With `hackenbush` expressed as an alias template instead of a class template,
[MSVC incorrectly accepts](https://godbolt.org/z/tZ0L4H) `you<hackenbush>`!
Clang continues to incorrectly reject `you<dolittle>`.
And nobody notices Moreau taking the giftbox until it's too late.


## More?

What are some more places that covariance and contravariance show up in C++?
Think of other ways to smuggle "producers" (like `moreau` and `ben_ishak`) and
"consumers" (like `hackenbush` and `dolittle`) into the language... and if you think
of a good one, tell me about it!

The "implementation variance" on some of these variadic-template examples is
the subject of [CWG1430](http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#1430).

----

For a sequel to this post, see 
["P1616R0 and health insurance"](/blog/2019/07/03/contravariant-health-insurers) (2019-07-03).
