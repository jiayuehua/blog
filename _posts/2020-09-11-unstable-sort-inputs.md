---
layout: post
title: 'For what inputs is `std::sort` unstable?'
date: 2020-09-11 00:01:00 +0000
tags:
  standard-library-trivia
---

A sorting algorithm is "stable" if, for two equivalent elements, it preserves their
original order relative to each other.
It might be useful to know a concrete example of input for which your library's
`std::sort` is unstable. Here are examples for libc++ and libstdc++ (as of September 2020).
[Godbolt.](https://godbolt.org/z/f8Pard)

    auto LessMod2 = [](int a, int b) {
        return (a % 2) < (b % 2);
    };

    // For libc++:
    std::vector<int> v1 = {
        0, 1, 0, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3,
    };
    std::sort(v1.begin(), v1.end(), LessMod2);
    // 0 2 0 1 1 ... 1 1 3

The above 31-element input sorts unstably for both libc++ and libstdc++. However,
libstdc++ also sorts the following 17-element input unstably:

    // For libstdc++:
    std::vector<int> v2 = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 3,
    };
    std::sort(v2.begin(), v2.end(), LessMod2);
    // 1 3 1 1 ... 1 1 1

As of September 2020, it appears that libc++ `std::sort` happens to be stable for all
ranges of size less than 31, and libstdc++ `std::sort` happens to be stable for all
ranges of size less than 17. (Do not rely on this little factoid in production!)

To be clear: There's nothing wrong with this. As the caller, if you want stable sorting,
you should use `std::stable_sort`, not `std::sort`.

As the vendor, if you don't intend to guarantee stability, it might be a _good_ idea
to break stability for small inputs, just to teach your users not to depend on that
property by accident or coincidence.

----

Notice that by definition, `std::stable_sort` never permutes a range that is already sorted.

    auto orig = x;
    if (std::is_sorted(x.begin(), x.end(), comp)) {
        std::stable_sort(x.begin(), x.end(), comp);
    }
    assert(x == orig);

I find it interesting that libstdc++'s `std::sort` _will_ permute a sorted range;
notice that the example input `v2` is trivially sorted, and yet libstdc++'s `std::sort`
swaps some of the elements. I'm not sure whether libc++'s `std::sort` will ever
permute a sorted range.
