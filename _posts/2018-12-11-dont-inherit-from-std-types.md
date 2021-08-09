---
layout: post
title: "Don't inherit from standard types"
date: 2018-12-11 00:01:00 +0000
tags:
  c++-style
  class-template-argument-deduction
  library-design
  pitfalls
  sd-8-adjacent
  slogans
---

[Puzzle of the day:](https://wandbox.org/permlink/92O8YK10Ebh5EUoA)
Here we have two different class types, both of which conceptually
wrap up a sequence of integers into something that can be "decremented"
(popping the last element) and tested for emptiness via `operator bool`.
The only difference between them is that one is expressed via composition
and the other via inheritance.

    struct Composition {
        std::vector<int> m_;
    public:
        Composition(std::initializer_list<int> il) : m_(il) {}
        operator bool() const { return !m_.empty(); }
        void operator--() { m_.pop_back(); }
    };

    struct Inheritance : std::vector<int> {
    public:
        Inheritance(std::initializer_list<int> il) : vector(il) {}
        operator bool() const { return !this->empty(); }
        void operator--() { this->pop_back(); }
    };

Now we feed them to this function:

    template<class T>
    int countdown_to_zero(T t)
    {
        std::vector results = { t };
        for ( ; t; --t)
            results.push_back(t);
        return results.size();
    }

    int main() {
        auto x = Composition{ 1, 2, 3 };
        auto y = Inheritance{ 1, 2, 3 };
        std::cout << countdown_to_zero(x) << "\n";
        std::cout << countdown_to_zero(y) << "\n";
    }

And we feed this to the compiler, and we run the resulting program, [and we get:](https://wandbox.org/permlink/92O8YK10Ebh5EUoA)

    4
    6

The puzzle is: *Why the difference?*

----

Suppose I told you that the keyword `struct` was relevant...

(Recall that members and inheritance relationships are `private`-by-default for classes declared
with the `class` keyword, but `public`-by-default for classes declared with the `struct` keyword.)

Change the definitions to

    class Composition {

    class Inheritance : std::vector<int> {

and you'll see that [you now get a compiler error]().

    prog.cpp:23:29: error: cannot cast 'const Inheritance' to its
    private base class 'const std::__1::vector<int, std::__1::allocator<int> >'
        std::vector results = { t };
                                ^

That's right — I snuck in some [CTAD](/blog/2018/12/09/wctad/)
when you weren't looking! (Or, if you *were* watching for uses of CTAD, this probably wasn't
much of a puzzle, eh?)

In the `Composition` case, what we have here is a `vector<Composition>`. When we `push_back`
onto it, we're pushing back `Composition` objects.

In the `Inheritance` case, what we have here is a `vector<int>`. When we `push_back` onto it,
we're pushing back `int` objects, which we get by calling `Inheritance`'s implicit `operator bool`.
And when we try to hide our inheritance relationship by making it `private`, we end up
hiding the relationship from `countdown_to_zero` — but we can't hide it from CTAD!

----

Any time we see a C++ "puzzle" — after we finish enjoying its puzzling aspect — we should pause
and think: *Real production codebases should never be puzzling.* What guidelines can we follow
in our daily work, to make sure that we don't accidentally leave a puzzle for our coworkers?
What simple guidelines, if followed punctiliously in this case, would have led to *no puzzle at all?*

This puzzle becomes puzzling by breaking two of my simple guidelines.

1. Never, ever use CTAD. Not even by accident! (I hope to see [`-Wctad`](/blog/2018/12/09/wctad/#as-of-a-few-days-ago-i-have-subm) in Clang soon.)

2. Never, ever inherit from any `std::` type. Not even privately!

(Except of course for the standard types you're *supposed* to inherit from. `std::iterator` has
rightly been deprecated, but `std::enable_shared_from_this<T>` is certainly an exception to
this general rule. Another exception to the rule — fittingly! — is `std::exception`, which should
be at the root of all your exception hierarchies.)

I recently said in a mailing-list discussion, and I stick by it:

> Standard types [are like boxes of chocolates](https://movies.stackexchange.com/questions/66827/what-is-the-meaning-and-significance-of-the-quote-life-is-like-a-box-of-chocola):
> You never know what you're going to get.

If you think you *do* know "what you get" when your type `T` inherits from `std::vector`: quick, does your type `T`
have a member function `==`, and what does it do? Does it have `emplace_back`, and if so, what is `emplace_back`'s
return type? Does it have CTAD deduction guides? And so on. It's not that these questions don't *have* answers;
it's that *you* don't know the answers (and neither do your coworkers). And even if you know everything you get with
`std::vector` this year — which means you had no trouble with the puzzler, right? — well, in C++2a everything's
going to change again, and you'll have to go re-audit your code to make sure it still does the right thing.

Rather than audit your code every time the definition of `std::vector` changes, I confidently assert
that it's *vastly* more reliable to simply *not inherit from `std::vector`* in the first place.
And this goes for all the types you didn't write yourself: not just `vector`, not just "standard containers,"
not even "standard types." If you (or your project, or your company) didn't write `class Foo`, then `class Foo`
should not be granted control over the API of your own class. And that's what you're doing when you inherit
from a base class: you're granting that class control over your API.

I'm sure we'll revisit this topic again.
