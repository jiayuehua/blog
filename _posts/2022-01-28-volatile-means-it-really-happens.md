---
layout: post
title: "`volatile` means it really happens"
date: 2022-01-28 00:01:00 +0000
tags:
  antipatterns
  c++-learner-track
  concurrency
  slogans
excerpt: |
  During my CppCon 2020 talk ["Back to Basics: Concurrency,"](https://youtu.be/F6Ipn7gCOsY)
  someone asked, "Do we ever need to use `volatile`?" To which I said, "No. Please don't
  use `volatile` for anything ever." This was perhaps a bit overstated, but at least
  _in the context of basic concurrency_ it was accurate. To describe the role of
  `volatile` in C and C++, I often use the following slogan:

  > Marking a variable as `volatile` means that reads and writes to that variable
  > <b>really happen.</b>
  >
  > If you don't know what this means, then you shouldn't use `volatile`.
---

<i>Thanks to Fatih Bakir for reviewing a draft of this post. All remaining errors
are mine, not his.</i>

----

During my CppCon 2020 talk ["Back to Basics: Concurrency,"](https://youtu.be/F6Ipn7gCOsY)
someone asked, "Do we ever need to use `volatile`?" To which I said, "No. Please don't
use `volatile` for anything ever." This was perhaps a bit overstated, but at least
_in the context of basic concurrency_ it was accurate. To describe the role of
`volatile` in C and C++, I often use the following slogan:

> Marking a variable as `volatile` means that reads and writes to that variable
> <b>really happen.</b>
>
> If you don't know what this means, then you shouldn't use `volatile`.

Consider the following snippet of C++, compiled on an average desktop system:

    volatile int g = 0;
    int test() {
        g = 1;
        g = 2;
        return g;
    }

According to our slogan above, because `g` is declared `volatile`, we can be
sure that the writes to `g` _really happen._ But what does "happen" really mean,
on a desktop system? For one thing, it means that the compiler can't just coalesce
those two writes into a single write — it can't throw out the "dead" write `g = 1`.
So it must emit the machine code for two writes to memory.

But what is "memory" in this context? We probably have three levels of cache between
the CPU and main memory — L1 cache, L2 cache, L3 cache. So a "write to memory" really
means loading a cache line into L1 and then writing our new value into that cache line.
Does that count as "really happening," for the purposes of `volatile int g`? Or does
a write only count as "really happening" if it makes it all the way back out to L2,
or L3, or main memory? What if we're on a multiprocessor system and some other CPU
has loaded a copy of that cache line at the same time — does
[cache coherency](https://en.wikipedia.org/wiki/Cache_coherency_protocols_(examples))
need to be involved here?

Vice versa, what if `volatile int g` were a local variable; could it be stored in a
register?— and what does it mean to "really write" to a register, in the presence of
hardware techniques like [register renaming](https://en.wikipedia.org/wiki/Register_renaming)?

This isn't just a software issue; it's a hardware issue. In order to know what it means
for a read or write to "really happen,"
[you must first invent the universe.](https://en.wikiquote.org/wiki/Carl_Sagan#Cosmos_(1980))

The `volatile` keyword probably isn't much use to you, unless you have deep knowledge
of the relevant pathway through your hardware platform. This is not the natural environment
in which most desktop programmers find themselves these days.

For an example where `volatile` _is_ useful, consider the following low-level C++ code.
I've shamelessly condensed it from
[this StackOverflow question about BeagleBone GPIO](https://stackoverflow.com/q/13124271/1424877):

    void blink_twice() {
        volatile int *off = reinterpret_cast<int*>(0x40225190);
        volatile int *on = reinterpret_cast<int*>(0x40225194);
        int flag = (1 << 22);

        *on = flag;
        sleep(1);
        *off = flag;
        sleep(1);
        *on = flag;
        sleep(1);
        *off = flag;
    }

This code is intended for use specifically on [a board](https://en.wikipedia.org/wiki/BeagleBoard)
([specs](http://meseec.ce.rit.edu/551-projects/fall2014/2-2.pdf))
where some ranges of memory are mapped to hardware activities such as turning on and off the
electrical current to an [LED](https://en.wikipedia.org/wiki/Light-emitting_diode).
Just like on a desktop system, when you ask to read or write at a given memory address,
you're speaking of _virtual_ addresses; it's the job of a thing called the
[MMU](https://en.wikipedia.org/wiki/Memory_management_unit) to receive every read or write
request coming out of the CPU and translate each virtual address into its corresponding
_physical_ address by consulting the [page table](https://en.wikipedia.org/wiki/Page_table).
But, on this particular system, when the MMU looks up address `0x40225194` in the page table,
it sees that that address is magic: it skips the L1 and L2 caches, and traffic to that
address on the bus isn't handled by RAM at all, but instead by a specific hardware peripheral
(a [GPIO](https://en.wikipedia.org/wiki/General-purpose_input/output) pin connected to our
LED light).

So, on this particular system, what it means to "really write" to address `0x40225194` is simply
to get the write out of the CPU and onto the MMU. From there, the MMU will reliably trigger
the hardware activity we actually care about — turning on our LED light.

Notice that even on this system, it's still unclear what it would mean to "really write" to
an address that _isn't_ magic to the MMU — say, `0xFFFF0042`. For example, if the CPU
issues two writes to address `0xFFFF0042` in quick succession, they will both hit L1 cache.
When that cache line is eventually written back to main memory, there will be only one
atomically visible change (and it'll be a whole 32-byte cache line at once). So, did
both writes "really" happen, or did only the second one happen?
Remember, `volatile` means nothing more than that our reads and writes will _really happen_.
If you can't explain what it would mean for a read or write at some address to really happen,
then you shouldn't bother qualifying that object with `volatile`.

Distinguish between [the efficient and the teleological cause](https://en.wikipedia.org/wiki/Four_causes);
between [the _is_ and the _ought_](https://en.wikipedia.org/wiki/Is%E2%80%93ought_problem).
It is not good enough to say, "I _need_ this write to 'really happen' in the sense that
I _need_ it to turn on that LED light; therefore I will use `volatile` here."
The hardware doesn't care about your _needs._ You must instead start from the efficient side:
"If I use `volatile` here, my compiler _will_ ensure that writes to this variable
are not coalesced; therefore each write _will_ make it to the MMU; the MMU _will_ observe
that the written address is in an uncached range and divert it to the GPIO pin controlling
my LED."

If you cannot explain the pathways involved at this level of detail — especially if your
explanations fail to survive their first encounter with caching or multiprocessing —
then `volatile` is not an appropriate tool for what you're trying to do.

> Marking a variable as `volatile` means that reads and writes to that variable
> <b>really happen.</b>
>
> If you don't know what this means, then you shouldn't use `volatile`.
