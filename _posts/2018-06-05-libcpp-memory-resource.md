---
layout: post
title: '`<memory_resource>` for libc++'
date: 2018-06-05 00:01:00 +0000
tags:
  allocators
  library-design
  llvm
---

Since approximately the week before C++Now, I've been working on a hobby project
to get the C++17 `<memory_resource>` header implemented in libc++. Yes, it's kind
of silly and unfortunate that implementing a major standard feature of C++17 is
happening in _2018_ as a _hobby project_, but, so it goes. Here are my notes
on the subject.


## Semantics of `<memory_resource>`

There are approximately three standards governing `memory_resource`-like
things:

- [ISO/IEC TS 19568:2015 "Library Fundamentals v1"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4480.html),
  published 2015-09-30.
  Defines a header called `<experimental/memory_resource>`, containing
  `memory_resource`, `polymorphic_allocator`, the three
  concrete subclasses of `memory_resource`, `resource_adaptor`, and `erased_type`
  (all in namespace `std::experimental`).

- [ISO/IEC TS 19568:2017 "Library Fundamentals v2"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/n4617.pdf),
  published 2017-03-30.
  Defines a header called `<experimental/memory_resource>`, containing
  `memory_resource`, `polymorphic_allocator`, the three
  concrete subclasses of `memory_resource`, `resource_adaptor`, and `erased_type`
  (all in namespace `std::experimental`).

- [ISO/IEC 14882:2017 "Programming Language C++"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4659.pdf),
  published 2017-03-21.
  Defines a header called `<memory_resource>`, containing
  `memory_resource`, `polymorphic_allocator`, and the three
  concrete subclasses of `memory_resource` (in namespace `std`).

The links above go to the final public _drafts_ of each standard. The actually official
documents are available only if you pay ISO; but there are no significant differences between
the final drafts and the official documents as far as I know.

Technically, `erased_type` is defined in `<experimental/utility>`, not `<experimental/memory_resource>`,
but its only use as far as I know is in uses-allocator construction; it is pretty fundamentally tied up
with the LFTS allocator model, if not `pmr` per se.


## Status of `<memory_resource>` by vendor

- As of this writing, libc++ has a header called [`<experimental/memory_resource>`](https://github.com/llvm-mirror/libcxx/blob/40a29e7/include/experimental/memory_resource),
  containing `memory_resource`, `polymorphic_allocator`, `resource_adaptor`, and `erased_type`.
  libc++ does not have the three concrete subclasses of `memory_resource`.
  libc++ does not have any header named `<memory_resource>`.

- As of this writing, libstdc++ has a header called [`<experimental/memory_resource>`](https://github.com/gcc-mirror/gcc/blob/8e8f643/libstdc%2B%2B-v3/include/experimental/memory_resource),
  containing `memory_resource`, `polymorphic_allocator`, `resource_adaptor`, and `erased_type`.
  libstdc++ does not have the three concrete subclasses of `memory_resource`.
  libstdc++ does not have any header named `<memory_resource>`.

- As of this writing, Visual Studio 2017 has no header named
  either `<experimental/memory_resource>` or `<memory_resource>`.
  (EDIT: I have been informed that the version on Godbolt is out of date,
  and in fact `<memory_resource>`
  [arrived in VS 2017 15.6](https://docs.microsoft.com/en-us/cpp/visual-cpp-language-conformance),
  released in March 2018.)

- [Boost.Container 1.67](https://www.boost.org/doc/libs/1_67_0/doc/html/container/polymorphic_memory_resources.html)
  supports all of the LFTSv2 version (as far as I can tell), in a collection of header files
  under [`<boost/container/pmr/xxxxx.hpp>`](https://github.com/boostorg/container/tree/2802a1f/include/boost/container/pmr).

As far as I can tell, Boost.Container 1.67 implements the resolution to [LWG 2969](https://cplusplus.github.io/LWG/issue2969)
(and the follow-up [LWG 3113](https://cplusplus.github.io/LWG/issue3113) filed by me).
As of 2018-05-28, libc++ also does.
As of this writing, libstdc++ does not.

Finally:

- Bloomberg's BDE library has the original implementation of what eventually became standardized
  as `pmr`; you can read about it [here](https://github.com/bloomberg/bde/wiki/BDE-Allocator-model).
  The equivalent of `monotonic_buffer_resource` is [`bdlma::BufferedSequentialAllocator`](https://github.com/bloomberg/bde/blob/master/groups/bdl/bdlma/bdlma_bufferedsequentialallocator.h);
  the equivalent of `unsynchronized_pool_resource` is [`bdlma::MultipoolAllocator`](https://github.com/bloomberg/bde/blob/master/groups/bdl/bdlma/bdlma_multipoolallocator.h);
  the equivalent of `synchronized_pool_resource` is [`bdlma::ConcurrentMultipoolAllocator`](https://github.com/bloomberg/bde/blob/master/groups/bdl/bdlma/bdlma_concurrentmultipoolallocator.h);
  the equivalent of `polymorphic_allocator<T>` is simply `bslma::Allocator*`.


## My series of libc++ patches

Originally, the goal of my hobby project was to provide a complete implementation of C++17
`<memory_resource>` without touching the existing `<experimental/memory_resource>` at all.
(That header is moribund; we have already seen libc++
[remove `<experimental/optional>`](https://github.com/llvm-mirror/libcxx/commit/95db3d2871d5678054d1ba3271f81baf60977c69#diff-16ce61d2b0f7572f5296c877ab65c542R11)
on the grounds that anyone who needs `optional` ought to be using libc++'s brand-new `<optional>`
implementation, which naturally is *not* available in C++14-and-earlier. Anyone "stuck" on C++14
had better switch to [Boost.Optional](https://www.boost.org/doc/libs/1_67_0/libs/optional/doc/html/index.html).)
But Eric Fiselier convinced me that the better approach was to make a series of incremental
patches to `<experimental/memory_resource>` until it fully implemented *not* LFTSv2 but actually
a superset of C++17 (notably including the resolution to [LWG 2969](https://cplusplus.github.io/LWG/issue2969)),
and only then, copy a subset of the new and improved `<experimental/memory_resource>` into a new
C++17-compatible `<memory_resource>`.

As of this writing (2018-06-05), I've got the following six patches lined up:

- [D46806 "Remove unused code from `__functional_base`"](https://reviews.llvm.org/D46806) (landed as a result of this blog post!)
- [D47109 "LWG 2969, do uses-allocator construction correctly"](https://reviews.llvm.org/D47109) (landed!)
- [D47344 "LWG 2843, stop quietly returning misaligned blocks"](https://reviews.llvm.org/D47344)
- [D47111 "Implement `monotonic_buffer_resource`"](https://reviews.llvm.org/D47111)
- [D47358 "Implement `{un,}synchronized_pool_resource`"](https://reviews.llvm.org/D47358)
- [D47360 "Copy `<experimental/memory_resource>` to `<memory_resource>`"](https://reviews.llvm.org/D47360)

D47111 and D47358 could use a lot more eyeballs than just mine and Eric's.


## Data size optimizations

One of Eric's pieces of feedback on my initial implementation of `{un,}synchronized_pool_resource`
was that the C++17 definition of `std::pmr::pool_options` is surprisingly wasteful of space.

    struct pool_options {
        size_t max_blocks_per_chunk = 0;
        size_t largest_required_pool_block = 0;
    };

Never mind that these two parameters are virtually irrelevant to the actual behavior of a pool resource
(you'd really want to know things like "what are all the different block sizes you're likely to see"
and "what is the maximum size/alignment supported by your upstream resource"). The significant thing
about them in this context is that they're quantities with _far fewer than 32 bits of entropy_, but
they're stored in fields _of size 64 bits_. So where a naïve implementation would have something like

    class unsynchronized_pool_resource : public memory_resource {
        pool_options __options_;
    public:
        explicit unsynchronized_pool_resource(pool_options __opts) :
            __options_(__opts)
        {}
        pool_options options() const { return __options_; }
    };

my implementation after one round of code review actually looked more like this:

    class unsynchronized_pool_resource : public memory_resource {
        int __max_blocks_per_chunk_;
        int __largest_required_pool_block_;
    public:
        explicit unsynchronized_pool_resource(pool_options __opts) :
            __max_blocks_per_chunk_(CLAMP(__opts.max_blocks_per_chunk)),
            __largest_required_pool_block_(CLAMP(__opts.largest_required_pool_block))
        {}
        pool_options options() const {
            return pool_options{
                __max_blocks_per_chunk_,
                __largest_required_pool_block_,
            };
        }
    };

It's more code, but it's a *significantly* smaller memory footprint.

Now, do we really care how big an `unsynchronized_pool_resource` is, given how
few of them we have in the program? Possibly not. But it's probably good practice
not to deliberately *waste* space, especially for `monotonic_buffer_resource`,
of which a program might contain dozens or even hundreds of simultaneous instances
spread out across the whole call stack.

For reference, here are the sizes of `memory_resource`'s concrete subclasses on
each of the major implementations listed above (assuming a typical 64-bit system,
and further assuming that `pthread_mutex_t` is 40 bytes):

|                 | `monotonic` | `unsync` | `sync` |
| --------------- | ----------- | -------- | ------ |
| Bloomberg BDE   | 320         | 28       | 60     |
| Boost.Container | 48          | 72       | 80     |
| mine (naïve)    | 64          | 56       | 104    |
| mine (original post)  | 56    | 40       | 88     |
| mine (current)  | 48          | 40       | 88     |

(UPDATE, 2019-05-31: In my initial post I just eyeballed the sizes of the BDL types,
and guessed them all amazingly incorrectly. I have since checked an actual BDL install
and report correct sizes in the table above. Also, in the 12 months since this post was made,
my `monotonic_buffer_resource` has shrunk to the size of Boost.Container's,
thanks to the favorable resolution of [LWG issue 3120](https://cplusplus.github.io/LWG/issue3120).
This is the subject of the next section.)


## How my `monotonic_buffer_resource` loses on data size

Boost.Container's `monotonic_buffer_resource` looks like this:

    class monotonic_buffer_resource {
        // vptr at offset 0
        struct block_slist {
            slist_node *next_;
            memory_resource& upstream_;
        } memory_blocks_;
        void *current_buffer_;
        size_t current_buffer_size_;
        size_t next_buffer_size_;
    };

Mine looks like this:

    class monotonic_buffer_resource {
        // vptr at offset 0
        struct initial_header {
            char *start_;
            char *cur_;
            char *end_;
        } initial_;
        struct chunk_header *chunks_;
        memory_resource *upstream_;
        size_t next_buffer_size_;
    };

The only real difference is that my version spends an extra 8 bytes to store the original pointer
to the "initial buffer" passed as an argument to the constructor. This means that when you
call `release()` on the resource, my version can roll back to the beginning of the buffer
and reuse it; whereas Boost.Container's version simply
[drops the original buffer on the floor](https://github.com/boostorg/container/blob/22f00f4/src/monotonic_buffer_resource.cpp#L92).

[We can observe](https://wandbox.org/permlink/uukKNnjdLEOoY6OX) that `release()`
causes Boost.Container to forget the initial buffer:

    int main() {
        using namespace boost::container::pmr;
        char data[1000];
        monotonic_buffer_resource mr(data, 1000);
        mr.release();
        mr.allocate(10);  // causes heap allocation
    }

In my version, calling `mr.release()` on the resource actually resets the
pointer to the beginning of the buffer, allowing you to reuse the same
resource (and the same buffer) multiple times. I think this feature is
worth the extra 8 bytes of memory footprint.

(EDIT: Ion Gaztañaga, the maintainer of Boost.Container, agrees with my analysis;
although he's not sure which one of us has the right behavior. He points out that
C++17 requires `mr.release()` to ["release all _allocated_ memory"](http://eel.is/c++draft/mem.res.monotonic.buffer.mem#1),
and doesn't say anything about what to do with the _non_-allocated memory from the original buffer.
If we read the standard's silence literally, as a requirement _not_ to reuse the
original buffer, then Boost.Container's behavior is conforming and my implementation's
behavior is non-conforming.)

(UPDATE, 2019-05-31: My `monotonic_buffer_resource` no longer stores an
explicit `next_buffer_size_`,
thanks to the favorable resolution of [LWG issue 3120](https://cplusplus.github.io/LWG/issue3120).
So I'm no longer spending an _extra_ 8 bytes of footprint; I'm just using 8 bytes
_differently_ from Boost.Container's version. They store `next_buffer_size_`;
I use those 8 bytes to store the original non-allocated buffer instead.)


## How my `synchronized_pool_resource` loses on data size

Boost.Container's `synchronized_pool_resource` looks like this:

    class synchronized_pool_resource {
        // vptr at offset 0
        void *opaque_sync_;
        unsynchronized_pool_resource unsync_;
    };

Mine looks like this:

    class synchronized_pool_resource {
        // vptr at offset 0
        std::mutex mutex_;
        unsynchronized_pool_resource unsync_;
    };

I don't actually understand what Boost.Container is doing with that `void*`.
Naïvely, it looks like they've got a race condition if two threads try to
call `do_allocate` at the same time:

    boost::container::pmr::synchronized_pool_resource mr;
    std::thread t1([&]{
        mr.allocate(1);
    });
    std::thread t2([&]{
        mr.allocate(1);
    });

This seems to result in two calls to `boost_cont_sync_create`, which is
fine, but then [both threads attempt to assign into `opaque_sync_`](https://github.com/boostorg/container/blob/develop/src/synchronized_pool_resource.cpp#L86)
with no synchronization as far as I can tell.

And, orthogonally, it looks like [`boost_cont_sync_create()` calls
`malloc`](https://github.com/boostorg/container/blob/9137957/src/dlmalloc_ext_2_8_6.c#L1414),
which seems a little bit sketchy for something that calls itself
a memory resource. :)

I'm sure I'm misunderstanding something about the design of Boost.Container's
`synchronized_pool_resource` here. But for now, I don't feel too bad about
the 40-byte overhead my version gets from `std::mutex`.

(EDIT: Ion Gaztañaga, the maintainer of Boost.Container, confirms that my analysis of
the data race is correct, and has made a note to fix it somehow in a later release.
_Mantra: When you roll your own multithreading code, you get bugs._ This applies not only to
beginners but also to experts.)


## Header weight

Notice that `synchronized_pool_resource` contains a `mutex`, which means that
`<memory_resource>` effectively must include `<mutex>`.

Simultaneously, `pmr` has infected pretty much every header in C++17, because for
example `<vector>` is required to include a definition of `std::pmr::vector<T>`,
which depends on `polymorphic_allocator<T>`.

Naïvely, this means that `<vector>` must include `<memory_resource>` must include
`<mutex>`; which is insane.

In practice, what we do is to split up `<memory_resource>` into two pieces:
`<__memory_resource_base>` has only the definitions for `polymorphic_allocator`
and `memory_resource`, and then `<memory_resource>` is the heavyweight one that
pulls in `<mutex>` and defines `class synchronized_pool_resource`.
Container headers such as `<vector>` (which don't care about `synchronized_pool_resource`)
can get away with including only the relatively lightweight `<__memory_resource_base>`.

Because of trivia related to uses-allocator construction,
`polymorphic_allocator` must know about `pair` and `tuple`, which means that
even `<__memory_resource_base>` must effectively include `<tuple>`. So again we
split up the heavy header, so that `<__memory_resource_base>` (which needs a
forward-declaration of `tuple` but doesn't care about the definition)
includes only the relatively lightweight `<__tuple>`.  (EDIT: Oops! As of this writing,
`polymorphic_allocator::construct` actually _does_ need the definition of `tuple`. This
could be fixed with some heavy surgery.)

Niall Douglas has some data on the weight of the headers in various STL
implementations [here](https://github.com/ned14/stl-header-heft). Notice that
as of this writing, both `<mutex>` and `<tuple>` are heavier than a lot of the
container headers; making the latter depend on the former would be a non-starter,
I hope.


## TL;DR

Please leave (relevant) comments on my libc++ code reviews!
