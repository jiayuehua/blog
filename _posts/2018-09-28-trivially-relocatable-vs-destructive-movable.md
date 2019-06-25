---
layout: post
title: 'Trivially Relocatable versus Destructive Movable'
date: 2018-09-28 00:01:00 +0000
tags:
  conferences
  cppcon
  relocatability
---

I presented P1144 "Object relocation in terms of move plus destroy" at the SG14 working meeting
on 2018-09-26. I was also pleasantly surprised by the number of shout-outs the idea received at
CppCon in general — including [in Mark Elendt's keynote](https://youtu.be/2YXwg0n9e7E?t=37m45s).
(The specific shout-out to "`std::trivially_relocatable` in the pipeline" is at 42m00s.
I did talk with Mark later on and point out that "in the pipeline" is an extremely generous
way to describe the situation!)

This evening I had a very productive conversation with Pablo Halpern, in which I learned that
my P1144 is not actually as close to Pablo's
[N4158 "Destructive Move rev. 1"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4158.pdf) (October 2014)
as I had been thinking and saying — and I believe Pablo learned that he had been similarly
misinterpreting P1144! Our papers actually have a significant theoretical difference.

The major major difference between N4158 and P1144 comes down to this question:

> Can I define my own "relocation" operation, to be used by `vector::resize` and so on,
> which is not quite `memcpy` but also more efficient than move-plus-destroy?

My P1144 says, definitively, _no_.

Pablo Halpern's N4158 says, definitively, _yes_.

N4158 proposes that there should be an _ADL customization point_
which Pablo named `uninitialized_destructive_move(T *from, T *to)` (but whose natural name
in P1144-land would be simply `relocate`). Any user could overload `uninitialized_destructive_move`
for their own type. Pablo's N4158 strongly implies that `vector::resize` et cetera
would be respecified to call this customization point.

For example, consider a class using something like the PIMPL idiom, with the class invariants that
`pimpl` is _never_ null and `pimpl->me` is always `this`, even for a moved-from object:

    namespace My {

    struct BlobImpl {
        Blob *me;
        explicit BlobImpl(Blob *me) : me(me) {}
    };

    struct Blob {
        std::unique_ptr<BlobImpl> pimpl;

        explicit Blob() {
            pimpl = std::make_unique<BlobImpl>(this);
        }
        Blob(Blob&& rhs) : pimpl(std::move(rhs.pimpl)) {
            pimpl->me = this;
            rhs.pimpl = std::make_unique<BlobImpl>(&rhs);
        }
        Blob(const Blob&);  // expensive
        ~Blob() = default;
    };

    } // namespace My

Pablo's N4158 would have allowed the user to provide an ADL overload of
`uninitialized_destructive_move`, like this:

    struct Blob {
        std::unique_ptr<BlobImpl> pimpl;

        explicit Blob() {
            pimpl = std::make_unique<BlobImpl>(this);
        }
        Blob(Blob&& rhs) : pimpl(std::move(rhs.pimpl)) {
            pimpl->me = this;
            rhs.pimpl = std::make_unique<BlobImpl>(&rhs);
        }
        Blob(const Blob&);  // expensive
        ~Blob() = default;

        friend void uninitialized_destructive_move(Blob *from, Blob *to) noexcept {
            ::new (to) Blob(from, Blob::DestructiveMoveTag{});
        }
    private:
        struct DestructiveMoveTag {};
        explicit Blob(Blob *rhs, DestructiveMoveTag) noexcept :
            pimpl(std::move(rhs->pimpl))
        {
            pimpl->me = this;
            rhs->~Blob();
        }
    };

Under N4158, _trivial_ destructive-movability was an orthogonal concern; in the absence of
attributes, Pablo proposed that maybe the user could just specialize
`std::is_trivially_destructive_movable` themselves. So then the library would provide
a fallback `std::uninitialized_destructive_move` just like it provides a fallback `std::swap` today:

    namespace std {

    template<class T> void uninitialized_destructive_move(T *from, T *to) {
        if constexpr (std::is_trivially_destructive_movable_v<T>) {
            // magic, tantamount to memcpy
        } else {
            ::new (to) T(std::move(*from));
            from->~T();
        }
    }

    } // namespace std

And `vector::resize` would do something like this:

    if constexpr (std::is_nothrow_destructive_movable_v<T>) {
        for (size_t i = 0; i < size_; ++i) {
            using std::uninitialized_destructive_move;
            uninitialized_destructive_move(olddata_ + i, newdata_ + i);
        }
    } else ...

Notice all the details above: the fact that `vector` can use
`uninitialized_destructive_move` *only* when it is nothrow; the fact that it still has to
be used in a loop; my complete ignoring of allocator-awareness concerns (which in fairness
are not hard to solve); and the low but non-zero potential for foot-shooting.
Bottom line, this sounds utterly complicated!

However, N4158's customization-point approach does have a tangible benefit in one case.
Observe that

    static_assert(not std::is_nothrow_default_constructible_v<Blob>);
    static_assert(not std::is_nothrow_move_constructible_v<Blob>);  // !!
    static_assert(std::is_nothrow_destructible_v<Blob>);
    static_assert(std::is_nothrow_destructive_movable_v<Blob>);  // N4158

The line marked `// !!` implies that by the definition in previous drafts of P1144,

    static_assert(not std::is_nothrow_relocatable_v<Blob>);

But in fact the human programmer _does_ know how to relocate a `Blob` in a noexcept manner!
P1144 doesn't provide any mechanism to share that information with `vector::resize`.

Under P1144, `vector::resize` will merely see that `Blob` is non-nothrow-move-constructible
and fall back to the expensive copy constructor.

Under Pablo's N4158, `vector::resize` would have used the fast nothrow ADL overload of
`uninitialized_destructive_move` rather than the expensive copy constructor.

I believe that this particular part of N4158 is orthogonal to P1144's concept of "trivial" relocatability,
and theoretically could be added later (as a pure library extension) if the committee wanted it.
I'm even ambivalent now as to whether I should remove the trait `is_nothrow_relocatable_v` from my proposal.

But one thing I know is that calling an ADL `uninitialized_destructive_move` (or an ADL `relocate`)
in a loop will never give optimal codegen for the trivial case.
The implementor of `std::vector` or `folly::FBVector` _must_ be given permission to use `memcpy`
on a whole range of trivially relocatable objects — no loop, no constructor calls, just a single
simple call to the library `memcpy`. As Mark Elendt's keynote session quite visibly showed,
that's what they're already doing today.
