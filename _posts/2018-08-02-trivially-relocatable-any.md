---
layout: post
title: 'Case study: Making `std::any` trivially relocatable'
date: 2018-08-02 00:01:00 +0000
tags:
  library-design
  relocatability
---

Many library types are not trivially relocatable even though one's initial impression
is "sure, why wouldn't they be?" [For example, `std::list`](https://github.com/facebook/folly/issues/889).
Another example: `std::any`. In my libc++ branch, I make `std::any` trivially relocatable
under a preprocessor `#ifdef`. [Here's the code change:](https://github.com/Quuxplusone/libcxx/blob/83c5b7b/include/any#L137-L147)

    template <class _Tp>
    using _IsSmallObject = integral_constant<bool,
        sizeof(_Tp) <= sizeof(_Buffer)
        && alignment_of<_Buffer>::value
             % alignment_of<_Tp>::value == 0
    #ifdef _LIBCPP_TRIVIALLY_RELOCATABLE_ANY
        && is_trivially_relocatable<_Tp>::value
    #else
        && is_nothrow_move_constructible<_Tp>::value
    #endif
    >;

The reason I put this under an `#ifdef` in my branch is that it actually changes the semantics of `std::any`.
Most of my library changes are non-ABI-breaking — I'm just taking the *existing fact* that certain types
(such as `std::vector`) *are* trivially relocatable, and exposing that fact to the compiler in a machine-readable way
(which is the action I call "warranting"). This requires a little bit of refactoring but does not affect the struct
layout or behavior of those types.

But with `std::any`, making the above change to `_IsSmallObject` actually does break ABI!

----

First, consider this program, which demonstrates a way in which `-D_LIBCPP_TRIVIALLY_RELOCATABLE_ANY` does *not* break ABI.

    // tu1.cpp
    #include <any>
    struct [[trivially_relocatable]] FooBar { FooBar(); FooBar(const FooBar&); };
    std::any TU1() { return FooBar{}; }

    // tu2.cpp
    #include <any>
    std::any TU1();
    struct [[trivially_relocatable]] FooBar { FooBar(); FooBar(const FooBar&); };
    FooBar TU2() { return std::any_cast<FooBar>(TU1()); }

Suppose we compile `tu1.cpp` with `-D_LIBCPP_TRIVIALLY_RELOCATABLE_ANY`, and we compile `tu2.cpp` without it.
Then our `FooBar` object *will* be stored in the SSO buffer. But this is fine because `TU2` does not expect to
know where it's stored; it just asks the `any` object's "vtable" where it is. No problem.

Suppose we compile `tu1.cpp` without `-D_LIBCPP_TRIVIALLY_RELOCATABLE_ANY`, and we compile `tu2.cpp` with it.
Then our `FooBar` object *will not* be stored in the SSO buffer. But this is fine because `TU2` does not expect to
know where it's stored; it just asks the `any` object's "vtable" where it is. No problem.

----

However!

Consider this program, which demonstrates a way in which `-D_LIBCPP_TRIVIALLY_RELOCATABLE_ANY` *does* break ABI.

    // tu1.cpp
    #include <any>
    #include <list>
    std::any TU1() { return std::list<int>{1,2,3}; }

    // tu2.cpp
    #include <any>
    #include <list>
    #include <vector>
    std::any TU1();
    std::list<int> TU2() {
        std::vector<std::any> vec;
        vec.push_back(TU1());
        vec.push_back(42);  // suppose this causes reallocation
        return std::any_cast<std::list<int>>(vec[0]);
    }

Suppose we compile `tu1.cpp` with `-D_LIBCPP_TRIVIALLY_RELOCATABLE_ANY`, and we compile `tu2.cpp` without it.
Then our `std::list` object *will not* be stored in the SSO buffer. But this is fine because `TU2` does not expect to
know where it's stored. When it comes time to reallocate `vector`'s underlying buffer, it relocates each `any`
object in the buffer by calling `any`'s move-constructor and then its destructor. And `any`'s move-constructor
doesn't expect to know where the `std::list<int>` object is stored; it just asks the `any` object's "vtable"
where it is. No problem.

Suppose we compile `tu1.cpp` without `-D_LIBCPP_TRIVIALLY_RELOCATABLE_ANY`, and we compile `tu2.cpp` with it.
Then our `std::list` object *will* be stored in the SSO buffer. *Now* when it comes time to reallocate `vector`'s
underlying buffer, it relocates each `any` object in the buffer by calling `memcpy` (because in `tu2.cpp`, we
have warranted that `std::any` is trivially relocatable). So it'll `memcpy` the whole object, including the SSO buffer
containing a `std::list` object — and it'll break that `std::list` object! When we go to call the copy-constructor
on that broken `list` object, on the final line of `TU2()`, we'll get an infinite loop or segfault.

So if you compile part of your program with `-D_LIBCPP_TRIVIALLY_RELOCATABLE_ANY` and part without, you run the risk
of segfaults or worse. That's because `-D_LIBCPP_TRIVIALLY_RELOCATABLE_ANY` changes the *actual run-time behavior*
of `std::any`. We're not just *warranting* an existing fact in a way that the compiler can understand; we're actually
*changing* the facts.

----

My proposal P1144 "Object relocation in terms of move plus destroy" specifically does not propose that any standard
library types should (or should not) become trivially relocatable — I don't want to require any vendors to *change*
their facts. I just want them to *warrant* the existing facts on their implementation.
