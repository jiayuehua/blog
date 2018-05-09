---
layout: post
title: 'Hiding template parameters from casual users'
date: 2018-05-09 00:01:00 +0000
tags:
  c++-style
  templates
---

Related to [Bob Steagall's C++Now 2018 talk on improving the STL containers](https://cppnow2018.sched.com/event/EC7W/if-i-had-my-druthers-a-proposal-for-improving-the-containers-in-c2x)...
Bob suggested that it might have been better to split up the newbie-friendly `std::vector<T>` from the most customizable
and power-user-friendly `std::vector<T, A>`. And even more so: splitting up `std::unordered_map<K, V>` from the
power-user-friendly `std::unordered_map<K, V, H, E, A>`.

Notice that I can already write `std::vector<T>` in my non-power-user code. But there's a bit of a downside there
in terms of documentation (the defaulted `A` parameter has to be documented anyway) and compiler error messages
(they'll probably include the value of the `A` parameter, cluttering up my error output, which is probably the worst
part of C++ for newbies).

I can think of three different approaches to "hiding" a template type parameter such as `A`.

First, what the STL does: The only class is the expert-friendly class. It has defaulted
parameters, so that the "newbie-friendly" version just omits those parameters.

    template<class T, class A = std::allocator<T>>
    class vector {
        // ...
    };

    vector<int> v1;
    vector<int, OtherAlloc> v2;

Second, we could do something inspired by `std::pmr`: The expert-friendly class has no
defaulted parameters, but we provide a newbie-friendly *type alias*.

    template<class T, class A>
    class expert_vector {
        // ...
    };

    template<class T>
    using vector = expert_vector<T, std::allocator<T>>;

    vector<int> v1;
    expert_vector<int, OtherAlloc> v2;

Third, we could implement both "newbie-friendly" and "expert-friendly" versions
as totally different types. In this case we introduce an internal
`__basic_vector` to hold the common implementation bits.

    template<class T, class A>
    class __basic_vector {
        // ...
    };

    template<class T, class A>
    class expert_vector : private __basic_vector<T, A> {
        // ...
    };

    template<class T>
    class vector : private __basic_vector<T, std::allocator<T>> {
        // ...
    };

The third approach is the only one that really solves the "confusing error messages"
problem for newbies. But it's also the only one that *causes* an API-reusability problem:
our newbies with `vector<T>` objects can no longer pass those objects to expert-written
APIs that expect `expert_vector<T, A>` objects.

No particular conclusion here. I'm fairly sure I prefer the second approach,
because it doesn't rely on defaulted parameters (which are the devil) and it
doesn't cause API problems as the third approach does.
