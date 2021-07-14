---
layout: post
title: 'Pitfalls and decision points in implementing `const_iterator`'
date: 2018-12-01 00:01:00 +0000
tags:
  antipatterns
  c++-style
  constructors
  library-design
  metaprogramming
  pitfalls
  stl-classic
---

This antipattern keeps coming up, so here's the blog post I can point people to.
Today we're talking about `const_iterator`. First of all — you know this —
`const_iterator` is different from `iterator const`, in exactly the same way that
`const int *` is different from `int *const`.

(I am ["west const" for life](/blog/2018/03/15/east-const-west-const/),
but even a west-conster can write the `const` on the east-hand side when it is pedagogically useful!)

    struct MyContainer {
        using iterator = MyIterator;

        iterator begin();              // GOOD!
        const iterator begin() const;  // WRONG!
    };

Remember that [const-qualifying the return type of a function that _returns by value_
is never, ever useful; and quite often is a pessimization.](https://stackoverflow.com/questions/8716330/purpose-of-returning-by-const-value)
See ["`const` is a contract"](/blog/2019/01/03/const-is-a-contract/) (2019-01-03).

Okay, so we all agree: we should be writing our containers like this —

    template<bool IsConst>
    struct MyIterator {
        // ...
    };
    using Iterator = MyIterator<false>;
    using ConstIterator = MyIterator<true>;

    struct MyContainer {
        using iterator = Iterator;
        using const_iterator = ConstIterator;

        iterator begin();              // GOOD!
        const_iterator begin() const;  // GOOD!
    };

The next pitfall, the one I actually want to talk about, is that now we have two different class types
(`MyIterator<false>` and `MyIterator<true>`), and so we have to think about how they interact with
each other. For example, we want this to compile, right?

    MyContainer c;
    MyContainer::const_iterator it = c.begin();

So we need to have an implicit conversion from `iterator` to `const_iterator`. And we don't want to
implement it with a [conversion operator](https://en.cppreference.com/w/cpp/language/cast_operator),
because that disables implicit move.
([EWG members, take note!](http://wg21.link/p1155)) So we break out the converting constructor.

    template<bool IsConst>
    class MyIterator {
        int *d_;
    public:
        // Always permit conversion from this class's own type...
        MyIterator(const MyIterator<IsConst>& rhs) = default;

        // ...and always permit conversion FROM the non-const iterator type.
        MyIterator(const MyIterator<false>& rhs) : d_(rhs.d_) {}
    };

This doesn't compile, because in the case that `IsConst == false`, we've just declared
the same constructor twice. But if we're too-clever-for-our-own-good, we simply drop
the defaulted copy constructor — it'll get implicitly generated anyway, right?

    template<bool IsConst>
    class MyIterator {
        int *d_;
    public:
        // Always (implicitly) permit conversion from this class's own type...

        // ...and always permit conversion FROM the non-const iterator type.
        MyIterator(const MyIterator<false>& rhs) : d_(rhs.d_) {}
    };
    using Iterator = MyIterator<false>;
    using ConstIterator = MyIterator<true>;

Now `ConstIterator` is constructible from either `ConstIterator` or `Iterator`, and
`Iterator` is constructible from only `Iterator`. Ship it, right?

----

In the case that `IsConst == false`, the constructor we've just
provided *is* the copy constructor. This was intentional; it's not the pitfall.
The pitfall is that, because we provided a curly-braced body for it, _it's no longer trivial._
So [we'll find that](https://godbolt.org/z/GnEdAg)

    static_assert(std::is_copy_constructible_v<ConstIterator>);
    static_assert(std::is_trivially_copy_constructible_v<ConstIterator>);
        // GOOD

    static_assert(std::is_copy_constructible_v<Iterator>);
    static_assert(not std::is_trivially_copy_constructible_v<Iterator>);
        // OOPS!

[The libc++ implementation of `vector<bool>::iterator`](https://github.com/llvm-mirror/libcxx/blob/f017e1e/include/__bit_reference#L1112-L1114)
currently falls into this exact trap. ([Godbolt.](https://godbolt.org/z/B93c1E))

----

So if we shouldn't write the too-clever code above, what should we write?
It turns out that all we have to do is be a tiny bit less clever. Thanks to Glen Fernandes
for pointing out that [this works](https://godbolt.org/z/15v7H6) —

    template<bool IsConst>
    class MyIterator {
        int *d_;
    public:
        template<bool WasConst, class = std::enable_if_t<IsConst || !WasConst>>
        MyIterator(const MyIterator<WasConst>& rhs) : d_(rhs.d_) {}  // OK
    };

This constructor template is too templatey to be recognized by the compiler as an attempt
to define the class's copy constructor. So the compiler implicitly generates a defaulted
(and trivial) copy constructor in both the `IsConst` and `!IsConst` cases. Our hand-written
constructor gets called only when `IsConst && !WasConst`. ...Which means it might be clearer
to replace the `||` with an `&&`.

We could (and for my money, *should*) add a declaration for the `=default`’ed copy constructor,
too.

Personally, I would be inclined to eliminate `WasConst` and then use my standard metaprogramming trick
to "lazify" the evaluation of the `enable_if` condition — so I would write

    template<bool IsConst>
    class MyIterator {
        int *d_;
    public:
        MyIterator(const MyIterator&) = default;  // REDUNDANT BUT GOOD STYLE

        template<bool IsConst_ = IsConst, class = std::enable_if_t<IsConst_>>
        MyIterator(const MyIterator<false>& rhs) : d_(rhs.d_) {}  // OK
    };

Read the next section to see one way in which my approach might be considered suboptimal;
and the section after that for a way in which it could be considered _more_ optimal.

----

You might wonder — iterators are cheap to copy (this one is trivial and register-sized),
so can we make our converting constructor pass-by-value instead of pass-by-const-reference?
It turns out that [all non-GCC compilers are unhappy with](https://godbolt.org/z/k9u_ko)

    template<bool IsConst>
    class MyIterator {
        int *d_;
    public:
        template<bool IsConst_ = IsConst, class = std::enable_if_t<IsConst_>>
        MyIterator(MyIterator<false> rhs) : d_(rhs.d_) {}  // BOGUS(?) ERROR
    };

but [everyone is happy with](https://godbolt.org/z/XAnkOn)

    template<bool IsConst>
    class MyIterator {
        int *d_;
    public:
        template<bool WasConst, class = std::enable_if_t<IsConst || !WasConst>>
        MyIterator(MyIterator<WasConst> rhs) : d_(rhs.d_) {}  // OK(?)
    };

----

Finally, if we're providing a converting constructor from `Iterator` to `ConstIterator`,
should we also provide a converting _assignment operator?_

    template<bool IsConst>
    class MyIterator {
        int *d_;
    public:
        MyIterator(const MyIterator&) = default;  // REDUNDANT BUT GOOD STYLE
        MyIterator& operator=(const MyIterator&) = default;  // REDUNDANT BUT GOOD STYLE

        template<bool WasConst, class = std::enable_if_t<IsConst && !WasConst>>
        MyIterator(const MyIterator<WasConst>& rhs) : d_(rhs.d_) {}

        template<bool WasConst, class = std::enable_if_t<IsConst && !WasConst>>
        MyIterator& operator=(const MyIterator<WasConst>& rhs) { d_ = rhs.d_; return *this; }
    };

Notice that ordinary assignments from `Iterator` to `ConstIterator` will compile just fine
without such an assignment operator. `ConstIterator::operator=` is just like any other
C++ function; you can call it with any argument that _implicitly converts_ to the type of
its formal parameter. And we just got done adding that implicit conversion in the form of
our converting constructor.

    Iterator it;
    ConstIterator cit;
    cit = it;  // implicitly convert `it` to ConstIterator, then use the copy assignment operator

However, implicit conversions in C++ are "limit 1 per customer." The following code
using [`std::reference_wrapper`](https://en.cppreference.com/w/cpp/utility/functional/ref)
will [*fail* to compile](https://godbolt.org/z/Re_BMD) —

    Iterator it;
    ConstIterator cit;
    cit = std::ref(it);

— unless we add an `operator=`. And here there is a *big difference* between

    template<bool WasConst, class = std::enable_if_t<IsConst && !WasConst>>
    MyIterator& operator=(const MyIterator<WasConst>& rhs) { ... }

    template<bool IsConst_ = IsConst, class = std::enable_if_t<IsConst_>>
    MyIterator& operator=(const MyIterator<false>& rhs) { ... }

The former requires deducing `WasConst`, which means it relies on template type deduction,
which means that the actual argument must match the formal parameter's type exactly. Which
means that the former `operator=` _will not help us_ if our goal is to make `cit = std::ref(it);`
compile. We _must_ use the latter version.

So, should we provide this converting `operator=`? In my opinion, the right thing to do here
is "do as the STL does." So [let's find out](https://godbolt.org/z/8ss9PL)
if the STL provides converting assignment operators:

    #include <functional>  // for reference_wrapper
    #include <list>

    std::list<int>::iterator it;
    std::list<int>::const_iterator cit = std::ref(it);  // OK?
    cit = std::ref(it);  // OK?

It turns out that libstdc++ and libc++ do _not_ provide converting assignment operators.
In fact, even their converting constructors rely on template type deduction, and so _neither_
of our `//OK?` lines work on libstdc++ or libc++!  On the other hand, MSVC's standard library
makes both of these lines work fine.

Conclusion: If you are implementing your own container iterators — or any other pair of types
with this "one-way implicit converting" behavior, such as the Networking TS's `const_buffers_type`
and `mutable_buffers_type` — then you should use one of the patterns above to implement
converting constructors without accidentally disabling trivial copyability. And you should add
explicit `static_assert`s to make sure that you never regress that functionality! But you should
follow the example set by the majority of STL vendors, and _not_ add converting assignment operators.
