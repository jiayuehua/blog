---
layout: post
title: "Fixing ADL, Round 2"
date: 2018-08-14 00:01:00 +0000
tags:
  argument-dependent-lookup
  llvm
  proposal
excerpt: |
  [Yesterday I blogged about](/blog/2018/08/13/fixing-adl-field-test/)
  a Clang patch I made that I claimed was an implementation of Herb Sutter's proposal
  [P0934R0 "A Modest Proposal: Fixing ADL"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0934r0.pdf) (February 2018).
  (You can find the patch [here](https://github.com/Quuxplusone/clang/commit/9ff89141fedd36af27820f5cc3a0edd8a1bef2db),
  or try it live [on Godbolt](https://godbolt.org/g/qvjmdS) as part of my experimental P1144 branch.)

  When the post hit Reddit, [commenter sphere991 pointed out](https://www.reddit.com/r/cpp/comments/977l4e/fieldtesting_herb_sutters_modest_proposal_to_fix/e472y1c/)
  that I had utterly missed about two-thirds of Herb's proposal!
---

[Yesterday I blogged about](/blog/2018/08/13/fixing-adl-field-test/)
a Clang patch I made that I claimed was an implementation of Herb Sutter's proposal
[P0934R0 "A Modest Proposal: Fixing ADL"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0934r0.pdf) (February 2018).
(You can find the patch [here](https://github.com/Quuxplusone/clang/commit/9ff89141fedd36af27820f5cc3a0edd8a1bef2db),
or try it live [on Godbolt](https://godbolt.org/g/qvjmdS) as part of my experimental P1144 branch.)

When the post hit Reddit, [commenter sphere991 pointed out](https://www.reddit.com/r/cpp/comments/977l4e/fieldtesting_herb_sutters_modest_proposal_to_fix/e472y1c/)
that I had utterly missed about two-thirds of Herb's proposal —

> Herb's proposal has two parts: narrowing the set of associated namespaces
> and narrowing the set of functions looked up.

Here's the "narrowing" wording from [P0934](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0934r0.pdf):

> When considering an associated namespace, the lookup is the same as the lookup performed
> when the associated namespace is used as a qualifier, except that:
>
> ... Any function that does not have a parameter type of (possibly cv-qualified) `T`
> or accessible base of `T`, pointer to `T` or accessible base of `T`, or reference
> to `T` or accessible base of `T`, in the parameter position of `T`, is ignored.

As far as I can tell, Herb's wording includes instantiated functions, so an ADL call
to `distance(list.begin(), list.end())` would still happily find `std::distance<std::list::iterator>`.
(There was some disagreement about this in the Reddit comments.)

My `-Wadl` patch does not implement this narrowed form of lookup.

----

But there is also a third part to P0934, missed by both myself and sphere991, which
*expands* the lookup set (compared to how it is today)!

P0934 replaces the phrase "lookup" with the phrase "ordinary unqualified lookup"
in two places, which seems to have the effect of circumventing C++'s current "name-hiding" mechanics
in ADL calls. [This test case](https://godbolt.org/g/JiK16z) illustrates the proposed change:

    namespace A {
        class A {};
        inline void h(A&) {
            puts("A::h");
        }
    }

    namespace B {
        inline void h(const A::A&) {
             puts("B::h");
        }

        void calls_Ah(A::A parm) {
            using B::h;  // supplementing
            h(parm);
        }

        void calls_Bh(A::A parm) {
            void h(const A::A&);  // hiding
            h(parm);
        }
    }

Right now, the declaration of `B::h` inside `calls_Bh` more or less "resets" the overload set available
to ADL calls from within `calls_Bh`, "hiding" the better-matching `A::h` from view.

Herb's paper claims (see the example involving `NS::g` on page 14) to effectively
eliminate this "hiding" mechanism for ADL calls, which means that both of the functions
above would end up calling `A::h` because it is the best match. If you deliberately
wanted the worse-matching `B::h`, you'd have to call it by its qualified name.

I don't claim to understand why this change would be good. I almost suspect that it would
be super bad because it would interfere with [lookups of class member functions](https://godbolt.org/g/k5PX45).
But I suspect I'm misunderstanding it. I'm certainly fuzzy enough on it that I
don't think I could implement it!
