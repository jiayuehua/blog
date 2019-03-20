---
layout: post
title: "How my papers did at Kona"
date: 2019-03-20 00:01:00 +0000
tags:
  kona-2019
  operator-spaceship
  relocatability
  wg21
---

As of a couple days ago, [the post-Kona mailing is out!](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/#mailing2019-03)

I presented three proposals at the WG21 meeting in Kona this February. Here's their status post-Kona:

## [P1144R3 "Object relocation in terms of move plus destroy"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1144r3.html)

A.k.a. "trivially relocatable."

I presented this paper to EWGI and there was some discussion. No straw polls were taken. EWGI wants to see it again.

Based on some hallway discussions with Jens Maurer and Richard Smith, I am no longer so worried about
the core-language object-lifetime issues with P1144 (a.k.a.
["it works but it's undefined behavior"](https://www.youtube.com/watch?v=8u5Qi4FgTP8&t=38m35s)).
The conclusion of these discussions was that it suffices to define a "magic" library function, which P1144R3
calls `std::relocate_at(src, dst)`.
[Unlike Pablo's N4158](/blog/2018/09/28/trivially-relocatable-vs-destructive-movable/), this magic library function
is *not* supposed to be a customization point! It's more like a cross between `memcpy` and `std::launder`
(and Richard's [`std::bless`](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0593r3.html)
that was just adopted in Kona), but it's not literally expressible as any combination of those three.
Anyway, we just expose that magic `relocate_at` library function, and then vendors or end-users can use that
function to do their relocations of trivially relocatable objects.

In practice, `relocate_at` could be implemented as a very thin wrapper around an *out-of-line* `memcpy`. We put
the definition in some other translation unit so that the compiler cannot look into it and see that it's "just"
copying bytes.

> Similarly, [P0593R3 `std::bless`](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0593r3.html)
> can be implemented as a very thin wrapper around an out-of-line no-op: the compiler cannot look into it
> and see that it doesn't placement-new anything, so the compiler must assume that it *might* placement-new
> something into the address it returns, which means that even if there were no object at that address before
> the call to `bless`, the compiler must assume that there might be an object there after the call.

And of course vendors of C++ implementations are also permitted to do anything they want under the as-if rule;
which means that in practice I do not expect any STL vendors to *actually* make calls to `std::relocate_at`. They
will probably make calls to `memcpy`, and coordinate with their respective compiler vendors to ensure that `memcpy`
Does The Right Thing for trivially relocatable types.

The other big change to P1144R3 compared to P1144R2 is that I have decided that I really want to be able to optimize
`vector<string>::insert` and `vector<string>::erase`. When we insert or erase in a vector, we have to shift all the
subsequent elements left or right. For trivially relocatable types,
[this ought to be done with a single `memmove`](https://godbolt.org/z/cZCyHU). However, the operation being replaced
by the `memmove` in this case is not a bunch of paired constructs and destroys; it's a "domino chain" of a
bunch of move-assignments that finishes up with a single destroy. We replace that chain of assignments
with a single destroy up front, followed by a `memmove`.

In order to give `vector` the freedom to do this optimization, we need to ensure that a
trivially relocatable type's move-assignment operator never does anything crazy. Therefore, P1144R3 proposes that
a type without the `[[trivially_relocatable]]` attribute should be considered trivially relocatable only if it
defaults _all_ of its special member functions, including the assignment operators and the copy constructor in addition
to the move constructor and destructor. If you define _any_ of these special member functions, and you want your type
to be trivially relocatable, you'll have to use the attribute. However, it remains true that I don't expect much
user code ever to use the attribute.
[The Rule of Zero](https://web.archive.org/web/20130607234833/http://flamingdangerzone.com/cxx11/2012/08/15/rule-of-zero.html)
still applies. Use Rule of Zero, get trivial relocatability for free (whenever possible).

## [P1155R2 "More implicit moves."](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1155r2.html)

A.k.a. "fixing `return x;` to Do The Right Thing all the time."

I presented this paper in EWG, and it was overwhelmingly
[approved for C++2a](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1018r3.html). Exciting news!

There is no new revision of P1155 in the post-Kona mailing. The wording in P1155R2 is the wording that's going to be considered
by CWG at the next meeting.

My [further proposal to handle assignment operators specially](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1155r2.html#further)
was discussed by EWG in the same discussion, but the room's consensus was generally against trying to move that piece forward.
Catching pessimizations like [`return x /= y;`](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85671) will stay the domain
of opt-in compiler diagnostics and human eyeballs.

## [P1154R2 "Type traits for structural comparison."](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1154r2.html)

A.k.a. "`has_strong_structural_equality_v<T>`."

I was absent for the discussion of this paper in LEWG. (Thanks to Jeff Snyder for presenting and for telling me how it went.)
The paper arrived in Kona with seven type-traits — one for each possible comparison category, plus one — and left LEWG chopped down to
just the single trait `has_strong_structural_equality_v<T>`. Thanks to Barry Revzin's pretty amazingly productive work on
`operator<=>`, the notions of "structural comparison" and comparison category are no longer orthogonal, and in fact it's possible
to make a type with strong structural equality without giving it an `operator<=>` at all. So there's just the one trait now,
and it's moving along to LWG wording review as part of Barry's latest omnibus paper
[P1614R0 "The Mothership Has Landed: Adding `<=>` to the Library."](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1614r0.html)

I did put a new revision of P1154 in the post-Kona mailing, but I believe it's already obsolete. LWG will consider the
identical wording that is now part of P1614R0.
