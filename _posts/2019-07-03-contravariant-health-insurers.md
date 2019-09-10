---
layout: post
title: 'P1616R0 and health insurance'
date: 2019-07-03 00:01:00 +0000
tags:
  concepts
  implementation-divergence
  metaprogramming
  templates
  wg21
---

I've been discussing my post ["Covariance and contravariance in C++"](/blog/2019/01/20/covariance-and-contravariance)
(2019-01-20) with Roland Bock, who is one of the authors of
[P1616R0 "Using unconstrained template template parameters with constrained templates"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1616r0.html)
(March 2019, in the pre-Cologne mailing). Our discussion led to the following example, which I reproduce
here verbatim.

([Godbolt link.](https://concepts.godbolt.org/z/hgHakr))


    template<class T> concept Animal = true;
    template<class T> concept Cat = Animal<T> && sizeof(T)==2;
    template<class T> concept Horse = Animal<T> && sizeof(T)==4;

Roland has a sick cat.
Roland requires a doctor.
Roland requires that the doctor accept cats.

    template<template<Cat> class RolandsDoctor>
    struct Roland {
        using mySickCat = short;
        RolandsDoctor<mySickCat> treatmentPlan;
    };

Roland can accept a CatDoctor.

    template<Cat> struct CatDoctor {};
    template struct Roland<CatDoctor>;

Roland refuses to accept Hackenbush the horse doctor.

    template<Horse> struct HorseDoctor {};
    template struct Roland<HorseDoctor>;  // ERROR!

Roland gladly accepts Dolittle the animal doctor.

    template<Animal> struct AnimalDoctor {};
    template struct Roland<AnimalDoctor>;


Now we introduce a layer of indirection: a vendor of pet insurance.
A health insurer's job is to match up doctors with pet owners.

Here's our legacy health insurer.

    template<
        template<class> class Doctor,
        template<template<class> class> class PetOwner
    >
    struct HealthInsurerX {
        PetOwner<Doctor> referral;
    };

We get a compiler error, even though intuitively we want this to be OK.
`HealthInsurerX` accepts a doctor and a patient.
`HealthInsurerX` requires a doctor who accepts _anything_, whereas Dolittle accepts only animals.
Therefore Dolittle is not accepted by our insurer, even though Dolittle himself would happily accept our cat.
This could be a problem for our legacy code.

    template struct HealthInsurerX<AnimalDoctor, Roland>;
        // CLANG ERROR!

GCC is happy with this code, but Clang complains:

    template template argument 'AnimalDoctor' must not be
    more constrained than template template parameter 'Doctor'

The problem with the last example was that no doctor really treats _anything_. They treat animals!
Let's constrain the template parameters.

    template<
        template<Animal> class Doctor,
        template<template<Animal> class> class PetOwner
    >
    struct HealthInsurerY {
        PetOwner<Doctor> referral;
    };

Now the insurer is satisfied and will happily accept Dolittle into its health plan.

    template struct HealthInsurerY<AnimalDoctor, Roland>;

But the insurer is not satisfied with a plain old `CatDoctor`!
The insurer accepts only doctors who accept _any_ animal.
A doctor who accepts only cats will not be acceptable to the insurer.
This could be a problem for our non-legacy code!

    template struct HealthInsurerY<CatDoctor, Roland>;
        // ERROR!

Clang complains, and this time GCC complains, too.

    expected 'template<class> class requires Animal< <template-parameter-2-1> > Doctor'
    but got 'template<class> requires Cat< <template-parameter-1-1> > struct CatDoctor'

(For the record, I see no rhyme or reason to GCC's behavior. At least Clang
is being consistent here.)

Finally, let's try it with unconstrained variadic templates.
Both Clang and GCC agree that variadic templates turn off the constraint-checking behavior.
I don't know if this is actually motivated by any formal wording, or if it's just a convenient
bug that's shared by both implementations.

    template<
        template<class...> class Doctor,
        template<template<class...> class> class PetOwner
    >
    struct HealthInsurerZ {
        PetOwner<Doctor> referral;
    };

The insurer sends Roland to see Doctor Dolittle.
Notice that on paper, this insurer accepts only doctors who accept — not just "anything," but —
"any number of anythings all at once!" This is so insanely unrealistic that the insurer
simply throws all the paperwork in the trash. Dolittle gets a pass, this time.

    template struct HealthInsurerZ<AnimalDoctor, Roland>;

However, with all the paperwork in the trash, the insurer will just as happily send Roland to see Doctor Hackenbush.
This doesn't work, but nobody figures it out until Roland himself refuses to accept the horse doctor.

    template struct HealthInsurerZ<HorseDoctor, Roland>;
        // ERROR!

Both GCC and Clang both complain, but only at "Roland" time, not at "HealthInsurerZ" time.

    error: template template argument 'HorseDoctor' must not be
    more constrained than template template parameter 'RolandsDoctor'
        PetOwner<Doctor> referral;
                 ^~~~~~
    note: in instantiation of template class
    'HealthInsurer<HorseDoctor, Roland>' requested here
        template struct HealthInsurerZ<HorseDoctor, Roland>;
                        ^
