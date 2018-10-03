---
layout: post
title: '"Perfect backwarding"'
date: 2018-09-25 00:01:00 +0000
tags:
  copy-elision
  metaprogramming
---

Tonight at CppCon, Ezra Chung gave a lightning talk on what he called
"perfect forwarding and perfect backwarding." Perfect forwarding, as we
all know, is when you receive an argument from _above_ and pass it downward
with the same value category; Ezra's "perfect backwarding," then, is when
you receive a return value from _below_ and pass it _upward_ with the same
value category.

[Here are](https://godbolt.org/z/4JpcW7) our test cases for perfect forwarding,
[and also](https://godbolt.org/z/wsE6Co) for perfect backwarding:

    void taker(int&);
    void taker(const int&);
    void taker(int&&);
    void taker(const int&&);

    template<class T>
    void perfect_forward(T&& t) {
        taker(std::forward<T>(t));
    }

    int giver(index_constant<0>);
    int& giver(index_constant<1>);
    const int& giver(index_constant<2>);
    int&& giver(index_constant<3>);
    const int&& giver(index_constant<4>);

    template<int N>
    decltype(auto) perfect_backward() {
        return giver(index_constant<N>{});
    }

We already kind of see our test cases breaking down, in that
`perfect_backward<0>()` returns a different type from `perfect_backward<3>()`,
but both of those types eagerly turn into `int&&` when passed as function arguments.
(Perfect forwarding cannot perfectly forward prvalues!)

But where Ezra's code really gets tricky is if you want to _name_ the return value
(possibly so you can do something else with it before returning it).

    template<int N>
    decltype(auto) imperfect_backward_alpha() {
        decltype(auto) result = giver(index_constant<N>{});
        printf("%d\n", result);
        return std::forward<decltype(result)>(result);
    }

This `imperfect_backward_alpha` works in all cases _except_ when `result`'s type is a
non-reference type. In that case, `std::forward<int>(result)` has type `int&&`, and so
we return a soon-to-be-dangling reference. (GCC [catches](https://godbolt.org/z/WBgL73)
this bug — hooray!)

In his lightning talk, Ezra suggested

    template<int N>
    decltype(auto) imperfect_backward_beta() {
        decltype(auto) result = giver(index_constant<N>{});
        printf("%d\n", result);
        return decltype(result)(result);
    }

This [works](https://godbolt.org/z/hnzDHa), for `int`, but it has bad behavior for
`std::string`. In the non-reference-type case there, we end up with the moral
equivalent of

    std::string result = giver(index_constant<0>{});
    return std::string(result);

which of course not only inhibits copy elision but also makes a *copy* of `result`.

What we really want to do, to get "perfect backwarding" of all
return values — even prvalue return values — is more like this:

    template<int N>
    decltype(auto) perfect_backward() {
        decltype(auto) result = giver(index_constant<N>{});
        printf("%d\n", result);
        if constexpr (std::is_reference_v<decltype(result)>) {
            return decltype(result)(result);
        } else {
            return result;
        }
    }

I don't think there's any shorter way to write this in C++17.

----

But it occurs to me that we can eliminate all this wacky metaprogramming by adopting a minor
tweak to David Stone's [P0527 "Implicit move from rvalue references in return statements."](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0527r1.html)
All we'd need to do is make an additional (orthogonal) change to the Standard's current wording
so that _this example also_ did an implicit move:

    std::string&& example() {
        std::string&& result = something();
        return result;
    }

Right now, that's ill-formed: the expression `result` is an lvalue expression
(because `result` is a named variable), and so `return result` is attempting to
bind the lvalue `result` to an rvalue reference `std::string&&`, and that doesn't
work (yes, even though `decltype(result)` is `std::string&&`).

If we could somehow change the Standard so that our `example()` was legal C++,
then Ezra's "perfect backwarding" problem would have [a simple solution](https://godbolt.org/z/5lsSlT):

    template<int N>
    auto perfect_backward_hypothetical()
        -> decltype(giver(index_constant<N>{}))
    {
        decltype(auto) result = giver(index_constant<N>{});
        printf("%d\n", result);
        return result;
    }

I suspect there's some reason this change would be too scary to ever actually implement in C++,
but I can't think of the reason off the top of my head.

(Perfect _forwarding_ of prvalue expressions is left as an exercise for the reader.)
