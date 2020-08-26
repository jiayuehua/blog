---
layout: post
title: "`for (auto&& elt : range)` Always Works"
date: 2018-12-15 00:01:00 +0000
tags:
  c++-style
---

In my previous post [_Contra CTAD_](/blog/2018/12/09/wctad) (2018-12-09),
I mentioned that I like things that work 100% of the time, and that I dislike
— and strongly recommend against ever using — things that work 99% of the time
and then break on the 100th time. In response, Reddit commenter AlexAlabuzhev
[wrote](https://www.reddit.com/r/cpp/comments/a4uczw/contra_ctad/ebhouqd/):
"There are no features that work 100% of the time."

When it comes to C++, that statement is truer than I wish it were. This is the language
that [couldn't even get `vector` right](https://en.cppreference.com/w/cpp/container/vector_bool)!
But there's still hope for the working programmer. There *are* many C++ features
and C++ idioms that work 100% of the time. Typically these are the ones that turn
into teachable guidelines.

Here's one that keeps coming up over and over on Slack: the idiomatic way to loop over
a range in C++11.

    for (auto&& elt : range) {
        do_something_with(elt);
    }

Use this style of loop! Use `for (auto&& elt : range)`. It Always Works.
Never use anything *but* the thing that Always Works, unless you are willing to
deal with the possibility of the-thing-you-use Not Working.

Here are some ways I've seen people ignore the rule and come to grief, recently:

----

[This ranges-v3 code:](https://godbolt.org/z/Iyft--)

    T my_generator();
    void examine(T&);

    void test() {
        for (auto obj : ranges::view::generate(my_generator) | ranges::view::take(3)) {
            examine(obj);
        }
    }

With `auto`, this code is doing one more move-construction than it needs to. With `auto&&`,
it's... well, the codegen honestly still looks pretty inefficient, but it's one move/destroy cycle
more efficient than it *had* been with `auto`.

----

[This `unique_ptr` code:](https://godbolt.org/z/wSFe-l)

    using Map = std::map<std::string, std::unique_ptr<M>>;

    void transfer(Map& params) {
        Map mine;
        for (const auto& p : params) {
            mine.emplace(p.first, std::move(p.second));
        }
    }

The author of this code was rewarded with a spew of error messages, starting here:

    /opt/compiler-explorer/gcc-8.2.0/include/c++/8.2.0/ext/new_allocator.h:136:23:
    error: no matching constructor for initialization of 'std::pair<const std::
    __cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >,
    std::unique_ptr<M, std::default_delete<M> > >'
        { ::new((void *)__p) _Up(std::forward<_Args>(__args)...); }
                             ^   ~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /opt/compiler-explorer/gcc-8.2.0/include/c++/8.2.0/bits/alloc_traits.h:475:8:
    note: in instantiation of function template specialization '__gnu_cxx::
    new_allocator<std::_Rb_tree_node<std::pair<const std::__cxx11::basic_string<
    char, std::char_traits<char>, std::allocator<char> >, std::unique_ptr<M,
    std::default_delete<M> > > > >::construct<std::pair<const std::__cxx11::
    basic_string<char, std::char_traits<char>, std::allocator<char> >,
    std::unique_ptr<M, std::default_delete<M> > >, const std::__cxx11::
    basic_string<char, std::char_traits<char>, std::allocator<char> > &,
    const std::unique_ptr<M, std::default_delete<M> > >' requested here
        { __a.construct(__p, std::forward<_Args>(__args)...); }
              ^

Did you spot the bug? ...or did you just see the reddish flag of `const auto& p : params`
and decide to change it to the thing that Always Works, `auto&&`?

    void transfer(Map& params) {
        Map mine;
        for (auto&& p : params) {
            mine.emplace(p.first, std::move(p.second));
        }
    }

Now `p`'s type is deduced as `Map::value_type&`, rather than `const Map::value_type&`,
and so the `std::move` actually does something. So, from a certain point of view, the
"bug" was that `const auto&` had one extra `const` qualifier on it... but from another
point of view, all that's needed is to identify a for-loop that doesn't follow the
`for (auto&& elt : range)` pattern and [*fix it*](https://www.reddit.com/r/LiveFromNewYork/comments/6hrc9w/).
If you write things with `for (auto&& elt : range)` from the start, your code will
start out with fewer bugs.

----

The classic `auto&&` use-case, of course, [brings us full circle to `vector<bool>`](https://godbolt.org/z/_PRmXo):

    template<class T>
    void fill(std::vector<T>& vec, T value) {
        for (auto& elt : vec) {
            elt = value;
        }
    }

We know even before we compile this code that something is fishy, because we see `auto&`
where we expect always to see `auto&&`. So when we compile this code and hit a compiler error...

    int main()
    {
        std::vector<bool> v;
        fill(v, true);
    }

    error: non-const lvalue reference to type 'std::_Bit_reference' cannot bind
    to a temporary of type 'std::_Bit_iterator::reference' (aka 'std::_Bit_reference')
        for (auto& elt : vec) {
                   ^   ~

...we know immediately that the way to fix it is to go back to using `for (auto&& elt : range)`.

So,

> Use `for (auto&& elt : range)`. It Always Works.

(For extremely detailed information on *how* it Always Works, consult Stephan T. Lavavej's
proposal [N3853 "Range-Based For-Loops: The Next Generation"](http://open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3853.htm)
(January 2014). This paper proposed that `for (elt : range)` should be accepted as a shorthand
syntax (`auto&&` being inserted implicitly by the compiler); but it was rejected over quite valid
concerns about what should happen if some (perhaps global) variable `elt` is already in scope.)

See also:

* ["`for (auto&& elt : range)` Still Always Works"](/blog/2018/12/27/autorefref-still-always-works) (2018-12-27)

* ["`-Wrange-loop-bind-reference` and `auto&&`"](/blog/2020/08/26/wrange-loop-analysis) (2020-08-26)
