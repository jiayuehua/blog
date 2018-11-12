---
layout: post
title: '`[[trivially_relocatable]]` in San Diego: call for reader feedback'
date: 2018-11-11 00:01:00 +0000
tags:
  relocatability
  san-diego-2018
---

On the morning of Wednesday 2018-11-07, my paper [P1144R0 "Object relocation
in terms of move plus destroy"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1144r0.html)
was presented at the WG21 meeting in San Diego. It was presented in an early-morning
session of SG17, the Evolution Working Group Incubator (EWGI), which is one of two
"incubator" study groups newly formed for the San Diego meeting. (This means that it
did not get seen by EWG.)

My thanks to Corentin Jabot for championing the paper and attempting to get it heard!

One straw poll [was taken](http://wiki.edg.com/bin/view/Wg21sandiego2018/P1144R0):

> Should we commit additional committee time to solving the problem P1144R0 is trying to solve,
> knowing it will leave less time to other work?
>
> 8 3 0 0 0

(Read this result as "8 strongly in favor; 3 in favor; none neutral or against." There might have
been more than 11 people in the room, if some of them abstained due to lack of opinion; for
example, abstaining would be appropriate for someone who hadn't read the paper.)

EWGI did not give any other feedback on the paper.

This is where *you* come in!


## Please give me your feedback on the technical content of P1144R0!

The paper presents the following straw polls that could have been taken, but none of these
were permitted.

EWG1. We approve of the general idea that user-defined classes should be able to warrant their own trivial relocatability via a standard mechanism.

EWG2. We approve of the general idea that user-defined classes which follow the [Rule of Zero](https://web.archive.org/web/20130607234833/http://flamingdangerzone.com/cxx11/2012/08/15/rule-of-zero.html) should inherit the trivial relocatability of their bases and members.

EWG3. Nobody should be able to warrant the trivial relocatability of class `C` except for class `C` itself (i.e., we do not want to see a customization point analogous to `std::hash`).

EWG4. A class should be able to warrant its own trivial relocatability via the attribute `[[trivially_relocatable]]`, as proposed in this paper.

EWG5. A class should be able to warrant its own trivial relocatability via some attribute, but not necessarily under that exact name.

EWG6. A class should be able to warrant its own trivial relocatability as proposed in this paper, but we prefer to see a contextual keyword rather than an attribute.

EWG7. If a trait with the semantics of `is_trivially_relocatable<T>` is added to the `<type_traits>` header, the programmer should be permitted to specialize it for program-defined types (i.e., we want to see that trait itself become a customization point analogous to `std::hash`).

EWG8. Trivial relocatability should be assumed by default. Classes such as those in [Appendix C](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1144r0.html#non-trivial-samples) should indicate their non-trivial relocatability via an opt-in mechanism.

EWG9. To simplify [conditionally trivial relocation](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1144r0.html#sample-conditional), if an attribute with the semantics of `[[trivially_relocatable]]` is added, it should take a boolean argument.

LEWG10. The algorithm `uninitialized_relocate(first, last, d_first)` should be added to the `<memory>` header, as proposed in this paper.

LEWG11. The type trait `is_relocatable<T>` should be added to the `<type_traits>` header, as proposed in this paper.

LEWG12. If `is_relocatable<T>` is added, then we should also add `is_nothrow_relocatable<T>`, as proposed in this paper.

LEWG13. The type trait `is_trivially_relocatable<T>` should be added to the `<type_traits>` header, under that exact name, as proposed in this paper.

LEWG14. We approve of a trait with the semantics of `is_trivially_relocatable<T>`, but possibly under a different name. (For example, `is_bitwise_relocatable`.)

LEWG15. If `is_trivially_relocatable<T>` is added, under that exact name, then the type trait `is_trivially_swappable<T>` should also be added to the `<type_traits>` header.

If you feel comfortable responding to these statements in
public, I'd love to see your opinions (what Wikipedia calls
"[!votes](https://en.wikipedia.org/wiki/Wikipedia:Glossary#!vote)")
posted [in this std-proposals forum thread](https://groups.google.com/a/isocpp.org/d/msg/std-proposals/vfrDwX9wvoc/gTPYLQCwAgAJ).
If you don't want your !votes to be public, you can always email them to me privately.
I will tally the ballots and report the results in P1144R1, which will appear in the
post-San-Diego mailing (submission deadline: 2018-11-26).


## Please leave your comments on the Clang patch review!

[Clang patch D50119](https://reviews.llvm.org/D50119) is a complete and tested implementation
of `[[trivially_relocatable]]`. Nicolas Lesser has put in a lot of time reviewing the semantics,
but nobody with commit privileges has looked at it yet AFAIK. If you would like to see an
implementation of P1144 not just [available on Godbolt](https://p1144.godbolt.org/z/br8Ib6)
but actually shipping in Clang trunk, please ask your friendly neighborhood Clang committer to
look at [D50119](https://reviews.llvm.org/D50119)!

If you *are* a Clang committer, please review [D50119](https://reviews.llvm.org/D50119)!
