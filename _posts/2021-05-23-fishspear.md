---
layout: post
title: 'The Fishspear priority queue algorithm'
date: 2021-05-23 00:01:00 +0000
tags:
  data-structures
  knuth
  stl-classic
---

I continue participating in [Zartaj Majeed's book club](https://www.meetup.com/theartofcomputerprogramming/)
on Knuth's _The Art of Computer Programming_, previously mentioned in
["Bubblesort, rocksort, cocktail-shaker sort"](/blog/2020/12/13/bubblesort-rocksort-shakersort/) (2020-12-13).
The past couple of weeks, we've been looking at section 5.2.3, "Sorting by selection," in which the problem
of repeatedly selecting the max element leads Knuth to investigate priority queues.

So far, Knuth discusses basically four kinds of priority-queue data structures: the sorted list, the classic
random-access-array-based heap, the unbalanced binary heap (specifically the [leftist tree](https://en.wikipedia.org/wiki/Leftist_tree),
although in the third edition he describes leftist trees as "already obsolete"),
and the balanced binary search tree (such as an AVL or red-black tree; he defers this to section 6.2.3).
But he gives (on page 152) pointers to many other ideas from the literature:
[stratified trees](http://bioinfo.ict.ac.cn/~dbu/AlgorithmCourses/Lectures/Lec6-Boas-Tree-Boas1977.pdf),
[binomial queues](https://www2.seas.gwu.edu/~simhaweb/champalg/mst/papers/VuilleminBinomialHeap.pdf),
[pagodas](https://en.wikipedia.org/wiki/Pagoda_(data_structure)),
[pairing heaps](https://www.cs.cmu.edu/~sleator/papers/Pairing-Heaps.htm),
[skew heaps](https://www.cs.cmu.edu/~sleator/papers/Adjusting-Heaps.htm),
[Fibonacci heaps](https://www.cs.princeton.edu/courses/archive/fall03/cs528/handouts/fibonacci%20heaps.pdf),
[calendar queues](https://eecs.ceas.uc.edu/~wilseypa/classes/ece975/sp2010/papers/brown-88.pdf),
relaxed heaps,
fishspear,
hot queues, "etc."

I was intrigued by the name of the Fishspear data structure, so I went looking for that paper
(M. J. Fischer and M. S. Paterson, _JACM_ <b>41</b> (1994), 3–30) and [found it](http://wrap.warwick.ac.uk/60910/6/WRAP_cs-rr-221.pdf)
— or something close enough to it, anyway. It turns out to be a data structure that looks like this:

![A k-barbed fishspear.](/blog/images/2021-05-23-fishspear.jpg)

Each of those labeled segments is a list of data elements, sorted so that priority increases to the left.
List $$U$$ is referred to as the "sharp"; lists $$V_i$$ are referred to as the "barbs." (Lists $$W_i$$ have
no special name, but might be considered the "shaft" of the fishspear.) The Fishspear algorithm describes
how — and more importantly, when! — to splice an element from the head of $$W_k$$ and/or $$V_k$$ onto the tail of $$U$$,
when to start a new barb or merge two barbs together, and so on.

The paper actually expresses the algorithm as a sort of coroutine procedure `S`, where sometimes `S`
handles an "insert" request by recursively invoking `S` again. When I translated this into a C++
`class Fishspear` with non-coroutine methods, the activation stack turned into a `vector` holding the
state of `S` (which was really just one integer variable anyway).

As you might have guessed from the diagram, Fishspear's interesting property is that it's
based on "sequential storage" (lists) rather than "random-access storage" (arrays). When it
needs to rearrange data, it does so by _merging_ lists, rather than random-access shuffling
the elements of arrays. This is super cache-unfriendly for an in-memory C++ implementation,
but it does have three potential upsides:

- Fishspear in general might be well suited to _non-RAM_ storage, such as a priority queue
  stored on tape. (The authors mention this in the paper.)

- By linking elements instead of keeping them contiguous, we lose RAM-friendliness
  but we gain the ability to keep newly inserted items right near the top of the
  data structure instead of having to insert them all the way at the bottom.
  When the shape of the workload makes it likely that "a newly inserted element
  will very soon be deleted," a classic heap will still do O(lg n) comparisons
  on that element to get it into its proper place; Fishspear will do O(1).

- My C++ version of Fishspear never moves or copies elements, so it can be used with
  immutable types, unlike `std::priority_queue`.

I've put my C++ implementation on GitHub here: [github.com/Quuxplusone/Fishspear](https://github.com/Quuxplusone/Fishspear).
It comes with a "bench.cpp" written specifically to demonstrate the second and third bullet points above.
Notice that `Heap` (i.e. `std::priority_queue`) wins massively on cache-friendliness, but `Fishspear`
doesn't move elements in memory and (on this specific workload, with a large queue and on-average-short-lived
elements) does vastly fewer comparisons.

    $ g++ -W -Wall bench.cpp -o unoptimized
    $ ./unoptimized
    200000 operations (99938 push, 100062 pop) on a queue of initial size 10000
    Heap:      3438552 comparisons, 279710 moves, 2504937 assignments, 119 milliseconds
    Fishspear: 714652 comparisons, 0 moves, 0 assignments, 270 milliseconds
    $ g++ -W -Wall -O2 bench.cpp -o optimized
    $ ./optimized
    200000 operations (99938 push, 100062 pop) on a queue of initial size 10000
    Heap:      3438552 comparisons, 279710 moves, 2504937 assignments, 11 milliseconds
    Fishspear: 714652 comparisons, 0 moves, 0 assignments, 28 milliseconds

----

* [Get the code!](https://github.com/Quuxplusone/Fishspear)
