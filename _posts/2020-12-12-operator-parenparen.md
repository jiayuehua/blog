---
layout: post
title: "Tips re `operator()`"
date: 2020-12-12 00:01:00 +0000
tags:
  c++-style
  lambdas
  metaprogramming
  rant
---

Several times this week I've seen people talking about C++20's new generic lambdas
with explicit template parameters. Time for a blog post so I can link to it...

> A note on terminology: Several experts — notably Jon Kalb but also others — make a big deal
> about how the name of the grammar production is <b>lambda-expression</b>,
> that the resulting object is a <b>closure object</b>, and that it is a grievous vulgarism
> to refer to either one by the bare word <b>lambda</b>. On the other hand, my view
> is that "lambda" is totally fine, and that if we can avoid using two different words
> ("closure" and "lambda") to refer to the same idea, that'd be great, thanks.
> In situations where the difference matters, I happily say "lambda expression,"
> "lambda object," or "lambda type" to disambiguate; but generally I
> just say "lambda," and I claim there's nothing wrong with that.

Okay. C++14 added "generic lambdas," which are spelled like this—

    auto lam = [](auto x, auto y) { return x + y; };

—and get lowered by the compiler to look something like this (modulo
some minor details):

    struct Unnameable {
        template<class T, class U>
        constexpr auto operator()(T x, U y) const { return x + y; }
    };
    Unnameable lam = Unnameable();

When you call `lam("abc", 1)`, you're instantiating its templated `operator()`
with some particular pair of types. (In that specific case: `const char *` and `int`.)


## The lambda type itself is not a template!

You cannot write `lam<int, int>(1, 2)`. Angle-brackets are permitted
only after the name of a template. `lam` is not a template; it's a variable.
Also, like any variable, it has a single static type — its type is
exactly `Unnameable`. There are no class templates lurking here.

The templated entity here is the lambda type's `operator()`: it is a member function template,
a template for stamping out member functions.
So you can, physically, write:

    int r = lam.operator()<int, int>(1, 2);

This means "take `lam`, instantiate its `operator()` with `<int, int>`, and then
call that member function with the arguments `(1, 2)`." _But please don't write this!_


## Always use "natural" syntax to call overloaded operators

Consider this code:

    struct Base {
        virtual void operator()(int) const = 0;
    };

    Base *p = ...;
    return p->operator()(arg);

I consider this bad for two reasons.

Number one, if you're doing classical polymorphism,
_please_ don't name your virtual functions after punctuation! Please don't have a virtual
`operator()` or a virtual `operator=` or a virtual `operator<<` or a virtual `operator==`!
Instead, use human-readable method names, such as `call` and `print` and `equals`.
(Classically polymorphic types shouldn't use `operator=` at all, so it doesn't need a name;
but consider providing a virtual function named `clone` in lieu of a copy constructor.)
Then provide a single base-class implementation of `operator<<` that calls `this->print()`,
and recognize that you're providing `operator<<` only for the purpose of interoperating
with specific generic algorithms.
(See ["Inheritance is for sharing an interface (and so is overloading)"](/blog/2020/10/09/when-to-derive-and-overload/)
(2020-10-09).)

Number two, please always use the natural ("infix") syntax to call overloaded operators!
Even if `operator()` weren't virtual here, I wouldn't want to see `p->operator()(arg)`.

    Base *p = ...;

    return p->operator()(arg);  // NO! BAD!

    return (*p)(arg);  // Good: call *p like a function

    return p->call(arg);  // Good: call *p's 'call' method

If `(*p)(arg)` looks too confusing and implicit to you — if you don't really want `*p`
to be "callable like a function" — then you certainly shouldn't be overloading its
`operator()`, because _that is the entire purpose of `operator()`._ Vice versa, if you _do_
want `*p` to be callable like a function, you should call it like a function!
That this is lowered into a call to `p->operator()` is
an implementation detail that shouldn't leak out into your higher-level code.


## C++20 generic lambdas with explicit template parameters

C++20 allows us to write generic lambdas with explicit template parameters, like this:

    auto lam = []<class T>(T *x, T y) { *x = y; };

    int i = 1;
    int j = 2;
    lam(&i, j);
    assert(i == 2);

This is still plain old generic-lambda technology. `lam` is not a template;
`lam` is a lambda object of a concrete type with a templated `operator()`.
You _still_ cannot write

    lam<int>(&i, j);  // Error!

You still _can_ write

    lam.operator()<int>(&i, j);  // Bad style

but you still shouldn't, because it's bad style (see above).

----

Incidentally, I have seen multiple people try to write the above with a
superfluous `template` keyword:

    auto WRONG = []template <class T>(T *x, T y) { *x = y; }; // WRONG

Remember that the `template` keyword is only ever used at the beginning of a declaration,
or to help the parser disambiguate a dependent expression:

    template<class T>
    int declaration(T t) {
        auto u = t.template rebind<int>();
    }

Also remember that lambda-expressions consist of "one of every kind of brackets"
— square brackets, angle brackets (in C++20), parens, and curly braces. They don't contain
random keywords in between those brackets (except, sometimes, `mutable`).

----

If any of this was news to you, then you might be realizing that
"generic lambdas with explicit template parameters" aren't quite as awesome
a feature as you thought they were. (They're _not bad_. I've just seen a lot of people
expecting them to do things they don't actually do, kind of how we saw a lot of people
confusing lambdas with `std::function` when C++11 first came out.)

So what's the killer app for lambdas with explicit template parameters?
I think there are two:

(1) The `make_index_sequence` trick no longer requires a helper function.
Where a C++17 programmer would write

    template<class Tuple, size_t... Is>
    auto tuple_sum_impl(const Tuple& t, std::index_sequence<Is...>) {
        return (std::get<0>(t) + ... + std::get<Is+1>(t));
    }

    template<class Tuple>
    auto tuple_sum(const Tuple& t) {
        constexpr size_t N = std::tuple_size_v<Tuple>;
        return tuple_sum_impl(t, std::make_index_sequence<N-1>());
    }

a C++20 programmer can write:

    template<class Tuple>
    auto tuple_sum(const Tuple& t) {
        constexpr size_t N = std::tuple_size_v<Tuple>;
        auto impl = [&]<size_t... Is>(std::index_sequence<Is...>) {
            return (std::get<0>(t) + ... + std::get<Is+1>(t));
        };
        return impl(std::make_index_sequence<N-1>());
    }

(2) You can write lambdas whose `operator()`s are SFINAE-constrained in interesting
ways. For example, the lambda I showed above

    auto lam = []<class T>(T *x, T y) { *x = y; };

is callable with `(int*, int)` or `(double*, double)` but not with `(int*, double)`.
This might play into the "`std::overload`" trick with variant visitors.
Consider the following idiomatic way to print whatever's inside a `std::variant`:

    std::variant<int, double, std::string> v = ...;
    std::visit(
        [](const auto& x) { std::cout << x << "\n"; },
        v
    );

Now consider what happens if the variant might contain either a simple type
_or_ a `std::vector` of primitive types. A C++20 programmer can write
([Godbolt](https://godbolt.org/z/WdjzTd)):

    std::variant<
        int, double, std::string,
        std::vector<int>, std::vector<double>,
        std::vector<std::string>
    > v = ...;

    template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
    template<class... Ts> overload(Ts...) -> overload<Ts...>;

    std::visit(
        overload{
            [](const auto& x) { std::cout << x << "\n"; },
            []<class T>(const std::vector<T>&) { std::cout << "[vector]\n"; }
        },
        v
    );

A C++17 programmer could probably hack something together somehow,
but I don't think it would be nearly as clean-looking as this C++20
solution. Our overload set of lambdas there indicates very cleanly
and clearly that we want to do X if the argument is some kind of
`std::vector`, and Y otherwise.
