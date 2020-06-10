---
layout: post
title: 'Superconstructing super elider, round 2'
date: 2018-05-17 00:01:00 +0000
tags:
  blog-roundup
  copy-elision
  move-semantics
  today-i-learned
---

Remember my suggestion from ["The Superconstructing Super Elider"](/blog/2018/03/29/the-superconstructing-super-elider)
(2018-03-29) that maybe our containers ought to have a "directly-emplacing, never-moving" facility like this?

    template<class F>
    T& emplace_back_with_result_of(const F& factory_fn) {
        assert(size_ < Cap);
        T *p = new ((void*)buffer_[size_]) T(factory_fn());
        size_ += 1;
        return *p;
    }

Well, today I learned that [Andrzej Krzemieński knows how to get this effect without modifying the
container.](https://akrzemi1.wordpress.com/2018/05/16/rvalues-redefined/) The trick is that when we write

    new (voidptr) T(some_value)

we aren't *just* invoking `T`'s constructor; we might *also* be invoking some implicit conversion
on `some_value`. And we can overload the behavior of that implicit conversion! Thus:


    template<class F>
    class with_result_of_t {
        F&& fun;
    public:
        using T = decltype(std::declval<F&&>()());
        explicit with_result_of_t(F&& f) : fun(std::forward<F>(f)) {}
        operator T() { return fun(); }
    };

    template<class F>
    inline with_result_of_t<F> with_result_of(F&& f) {
        return with_result_of_t<F>(std::forward<F>(f));
    }

Notice the overloaded `operator T`.
[Now we can write](https://wandbox.org/permlink/qK78vKwBwrqTULMU):

    std::vector<Widget> vec;

    vec.emplace_back(Widget::make());  // A

    vec.emplace_back(with_result_of([]{ return Widget::make(); }));  // B

Line `A` produces an extra move-construction of `Widget`.
Line `B` constructs the return value of `Widget::make()` *directly* into
its final place in the vector; there is no extra movement at all.

This mechanism is super clever. But I must note that it is not 100% foolproof.
It works more or less because overload resolution finds `Widget(Widget)`
to be the best match for construction from `with_result_of_t<F>`.
If `Widget` were to have a constructor that were a *better match* for
`with_result_of_t<F>`, then the mechanism
[would fail](https://wandbox.org/permlink/3Zh2piwRxwKvDnhl):

    struct Widget {
        int value_;
        explicit Widget(int i) : value_(i) { puts("  construct from int"); }
        Widget(Widget&&) noexcept { puts("  move-ctor"); }
        ~Widget() { puts("  destructor"); };
        static Widget make() { return Widget(42); }

        template<class U> explicit Widget(U&&) : value_(0) { printf("  %s -- OOPS!\n", __PRETTY_FUNCTION__); }
    };

So this trick does have at least one sharp edge. But it's pretty cool that you
can get this effect if you need it, even without abandoning the standard containers!
