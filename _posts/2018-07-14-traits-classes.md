---
layout: post
title: 'What does the Standard mean by "`_traits`"?'
date: 2018-07-14 00:01:00 +0000
tags:
  allocators
  library-design
  metaprogramming
  rant
  templates
---

This is the first sequel to my post several months ago on
[customization point design for library functions](/blog/2018/03/19/customization-points-for-functions).
That post was all about _function_ customization points. This post is all about _type-based_ customization
points — a.k.a. traits classes — or at least what the standard library means when *it* says "traits class."

If you're just looking for a tutorial introduction to traits classes, without all the nitpicky wiffling
I'm about to do in this blog post, I highly recommend that you stop reading and go watch
my two-hour CppCon 2016 session on ["Template Normal Programming"](https://www.youtube.com/watch?v=vwrXHznaYLA)
([Part 1](https://www.youtube.com/watch?v=vwrXHznaYLA), [Part 2](https://www.youtube.com/watch?v=VIz6xBvwYd8));
and/or, Vinnie Falco's CppCon 2017 session ["Make Classes Great Again"](https://www.youtube.com/watch?v=WsUnnYEKPnI).
This post (as with [much of my content](https://amzn.to/2Joe5C2)) is intended mostly for people who
already think they know this stuff backward and forward.

----

Unfortunately, the C++ standard library overloads the notion "traits class" to mean at least two and
probably three or more different things. One kind of "traits class" looks like this:

    template<class T>
    struct char_traits;

    template<class T, class Traits = char_traits<T>, class Alloc = allocator<T>>
    class basic_string {
        // uses Traits::compare, etc.
    };

`regex_traits` also falls into this category. This kind of "traits class" is fundamentally misnamed.
The programmer uses `std::char_traits<T>` in exactly the same way as, say, `std::allocator<T>`: it
serves as a convenient _archetype_ or _reference implementation_ or _default implementation_ of a
certain concept. (For example, `std::allocator<T>` is a reference implementation of the `Allocator`
concept.)  The library will use it as the _default_ for a template parameter; the programmer will
usually be unaware of its existence.

This kind of class — `char_traits<T>`, `regex_traits<T>`, `allocator<T>`, `less<T>` — is *not* a
customization point. If your code specializes `char_traits<X>` for some `X`, you are definitely
doing it wrong.

    // Goofus writes:
    template<>
    struct std::char_traits<CaseInsensitiveChar> {
        // ...
    };
    using CaseInsensitiveString = std::basic_string<CaseInsensitiveChar>;

    // Gallant writes:
    struct CaseInsensitiveTraits {
        // ...
    };
    using CaseInsensitiveString = std::basic_string<char, CaseInsensitiveTraits>;

----

Another kind of "traits class" looks like this:

    template<class A>
    struct allocator_traits;

    template<class T, class Alloc = allocator<T>>
    class vector {
        // uses allocator_traits<Alloc>::allocate, etc.
    };

Here, the name of the traits class is *well-known*; rather than parameterizing `vector`
on the name of the traits class (as in our previous example), the library writer actually
hard-codes the name `allocator_traits` directly into the implementation of `vector`.
(`iterator_traits` and `pointer_traits` also fall into this category; and coincidentally,
the implementation of `vector` hard-codes both of those names as well.)

This kind of class — `allocator_traits<T>`, `iterator_traits<T>`, `pointer_traits<T>` — is
*dangerous to treat as* a customization point. If your code specializes `allocator_traits<X>`
for some `X`, you are *probably* doing it wrong, and you should stop immediately.
Here's an example of what I'm calling dangerous:

    template<class T>
    struct MyAlloc : std::allocator<T> {
        using std::allocator<T>::allocator;
        template<class U> struct rebind {
            using other = MyAlloc<U>;
        };
    };

    // Goofus writes:
    template<class T>
    struct std::allocator_traits<MyAlloc<T>> {
        using A = MyAlloc<T>;
        using allocator_type = A;
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using void_pointer = void*;
        using const_void_pointer = const void*;
        using difference_type = std::ptrdiff_t;
        using size_type = std::size_t;
        using propagate_on_container_copy_assignment = std::true_type;
        using propagate_on_container_move_assignment = std::true_type;
        using propagate_on_container_swap = std::true_type;
        using is_always_equal = std::true_type;
        template<class U> using rebind_alloc = MyAlloc<U>;
        template<class U> using rebind_traits = std::allocator_traits<MyAlloc<U>>;
        static T *allocate(A& a, size_t n) { return a.allocate(n); }
        static void deallocate(A& a, T *p, size_t n) { return a.deallocate(p, n); }
        template<class U, class... Args> static void construct(A&, U *p, Args&&... args) { ::new ((vo$
        template<class U> static void destroy(A&, U *p) { p->~U(); }
        static A select_on_container_copy_construction(const A& a) { return a; }
        static size_t max_size(const A&) { return size_t(-1); }
    };

    int main() {
        std::vector<int, MyAlloc<int>> vec;
        vec.push_back(1);
    }

We might say that `allocator_traits` is something like a Boost `iterator_facade`, in that it takes a user-provided
type (such as `std::allocator<T>` or `MyAlloc<T>`) which implements a small set of primitive methods, and
fleshes it out into the "full" set of operations by providing sensible defaults for the missing methods.
(For example, if `A::construct` doesn't exist, `allocator_traits<A>::construct` will just call placement `new`.)
For `allocator_traits`, the "full set of operations" in C++17 has cardinality 20, and there's no guarantee that
it won't go up again in C++2a.

In fact, as of this writing, Goofus's code *will not compile* under libc++, because libc++ (rightly) does not anticipate
that any programmer would be so foolish as to write an explicit specialization of `std::allocator_traits<Foo>` with
all its 20 members. Therefore libc++ feels little compunction in [secretly upping that number
to 23](https://github.com/llvm-mirror/libcxx/blob/54238057d6/include/memory#L1609-L1617):

    /usr/include/c++/v1/vector:889:21: error:
          no member named '__construct_backward' in 'std::__1::allocator_traits<MyAlloc<int> >'
        __alloc_traits::__construct_backward(this->__alloc(), this->__begin_, this->__end_, __...
                        ^
    /usr/include/c++/v1/vector:1575:5: note:
          in instantiation of member function 'std::__1::vector<int, MyAlloc<int>
          >::__swap_out_circular_buffer' requested here
        __swap_out_circular_buffer(__v);
        ^

libc++ (rightly) treats `allocator_traits` as its own personal playground, where it can stash helper methods
such as this `__construct_backward`. (Full disclosure: counterintuitively, I do have
[a patch out](https://reviews.llvm.org/D49317) which aims to change that.)

As a rule of thumb, we might say that if a "traits class" has more than about 3 members in it, you're probably
not supposed to be customizing it yourself.

Another good rule of thumb to follow is: *Don't specialize any template that comes from `namespace std`.*
There are very *very* few cases where this rule will steer you wrong. (One such case is
[`std::hash`](https://en.cppreference.com/w/cpp/utility/hash). Notice that `std::hash` has exactly 1 member.)

----

The [Coroutines TS](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4760.pdf)
(currently, and controversially, proposed for inclusion in C++2a; I don't think it's ready at all)
adds to the mix a new "traits class" called `std::coroutine_traits<T>`, with *just one* member:

    // Goofus writes:
    template<class... Args>
    struct std::coroutine_traits<MyCoro, Args...> {
        using promise_type = typename MyCoro::promise_type;
    };

(The above definition would be redundant with the "sensible default" already provided by the primary template,
but you get the idea.)

On the one hand, `coroutine_traits` has only one member, so we might want to think of it like `std::hash` —
as a customization point. On the other hand, by looking at the primary template, we can see that `coroutine_traits`
fills the same ecological niche as `allocator_traits`: it has a *well-known name*, and it provides a *façade*
in front of the user-provided type `C`. There is nothing we can do by specializing `coroutine_traits<MyCoro>` that
we couldn't do more easily by putting our code directly into `MyCoro`.

And if Goofus does specialize `coroutine_traits<MyCoro>`, then he runs the risk of breakage —
either today (as in our libc++ example) or tomorrow (when C++2b adds additional members to the
primary template of `coroutine_traits`, and Goofus's C++2a-era specialization doesn't have those members).

By the way, you might be wondering what's the purpose of the `Args...` in that primary template. I'll tell you:
[I don't know.](https://www.youtube.com/watch?v=kDtabTufxao&t=2m13s)

----

Readers of my previous post on [customization point design for library functions](/blog/2018/03/19/customization-points-for-functions)
(including yourself... right?) will recall that any customization point has two pieces:

* A, the piece the user is required to specialize; and

* B, the piece the user is required to _invoke_ (and therefore must _not_ specialize).

In our first set of examples, the so-called "traits class" (e.g. `char_traits`) plays no role in the customization dance.
The part of "A" is played by some-class-modeling-CharTraits; the part of "B" is played by `std::basic_string`.
Or the part of "A" is played by some-class-modeling-Allocator; the part of "B" is played by `std::vector`.
Or the part of "A" is played by some-class-modeling-Comparator; the part of "B" is played by `std::set`.
The so-called "traits class" (respectively: `char_traits<T>`, `allocator<T>`, `less<T>`) is really just provided by
the standard library as a helpful default reference implementation, or archetype, of what-you-have-to-do-to-model the
relevant concept.

In our second set of examples, the so-called "traits class" (e.g. `allocator_traits`) plays the part of "B", where the
"user" who invokes `allocator_traits` is the library itself (e.g. `vector`). The part of "A" is played by the user's
allocator type itself — that's where all the customization actually goes. For example, to customize the behavior of
the "allocate" operation, the user customizes `MyAlloc::allocate` (that's "A"); but when someone like `vector` wants
to invoke the "allocate" operation, they do it via `allocator_traits<A>::allocate` (that's "B").

----

The C++ standard library has evolved over decades, and some of its names and idioms have not aged well — either because
they weren't perfect to begin with, or because their purposes have been forgotten. For example, `pointer_traits` used
to be perfectly analogous to `allocator_traits` — unspecializable — but some of the additions in C++20 were apparently
done under the mistaken impression that users were expected to explicitly specialize `pointer_traits`. Look particularly
at the incompatibility in API design between C++17's [`pointer_traits<P>::pointer_to`](https://en.cppreference.com/w/cpp/memory/pointer_traits/pointer_to)
(playing role "B" while `P::pointer_to` plays "A") and C++20's [`pointer_traits<P>::to_address`](https://en.cppreference.com/w/cpp/memory/pointer_traits/to_address)
(playing role "A" while [`std::to_address`](https://en.cppreference.com/w/cpp/memory/to_address) plays "B").
The same entity — `pointer_traits` — can't possibly fulfill both role "A" and role "B" at the same time; and yet
that's what's currently attempted by the C++20 Working Draft.

More on this topic later, perhaps. I'd certainly like to see some thorough investigation into the notion of
"traits classes" before C++20 goes out the door.

----

P.S. — Which kind of "traits class" is `numeric_limits<T>`? (It has
[34 members](https://en.cppreference.com/w/cpp/types/numeric_limits#Member_constants).
Its specification changed significantly between C++03 and C++11, but has not changed significantly since then.)

P.P.S. — Which kind of "traits class" is the C++20 Working Draft's
[`std::chrono::zoned_traits`](https://en.cppreference.com/w/cpp/chrono/zoned_traits)?

----

P.P.P.S. — In ["The Best Type Traits C++ Doesn't Have"](https://www.youtube.com/watch?v=MWBfmmg8-Yo),
I describe a class template called `tombstone_traits`. In the presentation, I fit it snugly into our second category,
alongside `allocator_traits` and `iterator_traits`: as a *well-known name* that can have only one specialization
per type `T`.

    template<class T>
    struct tombstone_traits;

    template<class T>
    class optional {
        // uses tombstone_traits<T>::spare_representations, etc.
    };


Not long after that presentation, Nicole Mazzuca convinced me that it would actually be much more
natural to move it into our first category, alongside `allocator` and `char_traits`: as a *convenient default*.

    template<class T>
    struct default_tombstone_traits;

    template<class T, class Traits = default_tombstone_traits<T>>
    class optional {
        // uses Traits::spare_representations, etc.
    };

This latter design is more useful and flexible than the former design. In particular, it allows something like

    using general_purpose_optional_int = optional<int>;
    using special_purpose_optional_int = optional<int, UseMinusOneAsTombstone>;

Notice that the C++ allocator model has both a *convenient default* (`allocator`) and a *well-known façade*
(`allocator_traits`), each with its own well-defined purpose.
