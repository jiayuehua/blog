---
layout: post
title: "C++ doesn't know how to do customization points that aren't operators"
date: 2018-08-23 00:01:00 +0000
tags:
  customization-points
  operator-spaceship
  rant
---

If you're a C++ programmer and you want to customize the behavior of your class for a certain
operation, you'll have one of two experiences:

- If the behavior you're trying to customize is spelled as punctuation: Easy peasy.
 You might even be able to `=default` it.

- If the behavior you're trying to customize is spelled as an English word: Horrible.

Examples of the first category: Copy construction. Assignment (`operator=`).
Addition/concatenation (`operator+`). Equality-comparison (`operator==`).
Destruction.

Examples of the second category: Swapping. Hashing. Providing a
["default order"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0181r1.html)
for the benefit of `std::map`, without exposing it to the world via `operator<`.
Three-way comparison. Stringifying. [Relocating](/blog/tags/#relocatability).

C++ just *cannot figure out* how to deal with the operations in category 2, except
to move them into category 1. Sometimes the efforts to move an operation into category 1
are quite nice; sometimes they're a mess. Here are some examples:


## Three-way comparison

This is the obvious one for C++2a geeks. We've known for a long time that three-way comparison
is a fundamental operation. I gave [a talk on it](https://www.youtube.com/watch?v=lKG1m2NkANM)
at CppCon 2014. Stepanov (or somebody) made sure to give `string` a method
[`a.compare(b)`](https://en.cppreference.com/w/cpp/string/basic_string/compare).

But `vector` doesn't have a three-way `compare`! Quick quiz: Why not?

Well, `vector<T>::compare` should return the result of lexicographically comparing the first
elements of the two vectors, then the second elements, and so on, until it finds a difference.
In C++17 we'd spell this something like

    template<class T>
    int vector<T>::compare(const vector<T>& rhs) const {
        for (size_t i = 0; i < size(); ++i) {
            if (i >= rhs.size())
                return +1;
            int c = (*this)[i].compare(rhs[i]);
            if (c != 0) return c;
        }
        return (size() < rhs.size()) ? -1 : 0;
    }

But this won't work unless `T` itself has a `compare` member function! And most class types don't
have such a method (for historical reasons)... but certainly no *primitive* type has such a
method. If you want this approach to work for `int`, you'll have to lift `a.compare(b)` into
a free function and provide overloads for `int`. Maybe do some ADL. (Or maybe not.
I highly recommend my previous blog post on
[customization point design for functions](/blog/2018/03/19/customization-points-for-functions/).)

    template<class T>
    int vector<T>::compare(const vector<T>& rhs) const {
        for (size_t i = 0; i < size(); ++i) {
            if (i >= rhs.size())
                return +1;
            int c = std::compare((*this)[i], rhs[i]);
            if (c != 0) return c;
        }
        return (size() < rhs.size()) ? -1 : 0;
    }

And then notice that if `std::compare` is implemented as a function, it will interact with ADL,
which might break existing code that uses ADL calls to `compare(x, y)`. So `std::compare` probably
ought to be a CPO (customization point object), as explained in the blog post above.

And... just *yuck*. This is *so much* code (and design work). So what's the solution in C++2a?
Simply extend the C++ parser to support `operator<=>`!  Now you can write

    template<class T>
    auto vector<T>::operator<=>(const vector<T>& rhs) const
        -> decltype(front() <=> front())
    {
        for (size_t i = 0; i < size(); ++i) {
            if (i >= rhs.size())
                return std::strong_ordering::greater;
            auto c = (*this)[i] <=> rhs[i];
            if (!std::is_eq(c)) return c;
        }
        return size() <=> rhs.size();
    }

Except that the actual STL probably can't write that, because most user types won't have implemented
`operator<=>`, and we'd probably like it to fall back to `operator<` in that case... but if it does
fall back, then how do we decide which ordering to use for our return type?
(UPDATE: We punt that problem onto the library function [`std::compare_3way`](https://en.cppreference.com/w/cpp/algorithm/compare_3way),
whose answer is that anything with `operator<` and no `operator<=>` must be
strongly ordered by definition.)

So switching from a named function `compare` to an infix operator `<=>` has solved *some* of our
usability problems and design decisions, but not *all* of them.


## Swapping

Swapping is probably the best-known named operation in C++. It uses basically the model that I
outlined above for `compare`: you need to provide a named member function `a.swap(b)` and then
also an ADL free function `swap(a, b)` that just calls the member function. (If the STL had
followed my [customization point design for functions](/blog/2018/03/19/customization-points-for-functions/),
they would have made `std::swap` a CPO, and made it look first for `a.swap(b)` before falling back
to the ADL free function. Then you'd only ever have to write the member function, not both versions.)

`swap` is so fundamental, and so kinda messed up in its current named-function state,
that there have been proposals to get rid of its name as well.
See Walter Brown's [N3746 "Proposing a C++1Y Swap Operator, v2"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3746.pdf)
(August 2013), where the suggested spelling was `operator:=:`. However, these proposals have not
taken flight the way `operator<=>` has.


## Hashing

You might think that given the pattern set by `a.compare(b)` and `a.swap(b)`, and the copious
precedent for `a.hash()` in third-party libraries, the STL would use `a.hash()` as the spelling
for the hashing operation. Nope! It uses `std::hash<T>{}(a)`.

Hashing is intimately related to equality-comparison: it must preserve the invariant that
`a == b` implies `a.hash() == b.hash()`.

As of C++2a, you'll be able to `=default` your equality-comparison by 
[jumping through a few hoops](http://eel.is/c++draft/class.rel.eq) —
hoops which *might* get a bit easier to jump through,
in the at-least-two-years between August 2018 and whenever C++2a is released.
But it is highly unlikely that you'll be able to `=default` your hashing operation!
So you'll be writing something like this:
 
    class Widget {
        int a, b, c;
    public:
        auto operator<=>(const Widget&) const = default;
        bool operator<(const Widget&) const = delete;
        bool operator<=(const Widget&) const = delete;
        bool operator>(const Widget&) const = delete;
        bool operator>=(const Widget&) const = delete;

        size_t hash() const {
            return size_t(a)+b+c; // whatever
        }
    };

    template<>
    struct std::hash<Widget> {
        size_t operator()(const Widget& w) const {
            return w.hash();
        }
    }

The infelicity here is that if you add a new field `d` to your class,
it will instantly show up in the equality operation `operator==` with no
further code changes... but it will *not* show up in the hashing operation!
You'll have to remember to go add it in there by hand.

This isn't a huge breaking problem, though, because even if the hash
function forgets to hash in field `d`, it still preserves the important
invariant that `a == b` implies `a.hash() == b.hash()`. Our code doesn't
*break* because of the oversight; it just gains a little bit of technical debt.


## Stringifying

Stringifying is a dubious "success story" of operator-ification. Java and
JavaScript and so on do stringification like this:

    class Widget {
        Gadget a, b;
    public:
        std::string toString() const {
            return a.toString() + " " + b.toString();
        }
    };

This doesn't work in C++ for the usual reason: primitive types like `int`
don't have member functions. So we could lift stringification into a named
function... which was finally kinda-sorta done in C++11.

    class Widget {
        int a, b;
    public:
        std::string toString() const {
            using std::to_string;
            return to_string(a) + " " + to_string(b);
        }
    };

    std::string to_string(const Widget& w) {
        return w.toString();
    }

Except that [`std::to_string`](https://en.cppreference.com/w/cpp/string/basic_string/to_string)
isn't really advertised as a customization point, and the STL doesn't bother to provide it
for any type other than the primitive numeric types. Heck, we don't even have `to_string(char)`
or `to_string(std::string)`, let alone `to_string(bool)`!

So what's the accepted way to implement `to_string`, since C++98?  Turn it into an
operator, of course!

    class Widget {
        int a, b;
    public:
        // Piece A
        friend std::ostream& operator<<(std::ostream& os, const Widget& w) const {
            return os << a << " " << b;
        }
    };

If we trust that everybody in our codebase plays along with iostreams and implements
`operator<<`, then we can implement generic stringification as

    // Piece B
    namespace my {
        template<class T, class = void> struct has_tostring : std::false_type {};
        template<class T> struct has_tostring<T, decltype(void(std::declval<const T&>().to_string())) : std::true_type {};

        template<class T>
        std::string to_string(const T& t) {
            if constexpr (has_tostring<T>::value) {
                return t.to_string();
            } else {
                std::ostringstream oss;
                oss << t;
                return std::move(oss).str();
            }
        }
    } // namespace my

The comments `// Piece A` and `// Piece B` refer to [my previous blog post on
customization points for functions](/blog/2018/03/19/customization-points-for-functions/).


## Conclusion?

I don't have a strong conclusion here, except to observe that C++ keeps swinging at the
"customized operation" ball and missing, over and over and over. Each customization point
we add — `swap`, `hash`, `to_string`?? — ends up with its own idiosyncratic design, and
the only ones that really stick comfortably are the ones where we move them from category 2
(named functions) into category 1 (nameless operators). C++'s idea of "fixing" the problems
with an operation is invariably to turn it into an operator (see: `<<`, `<=>`, `:=:`).

[P1063](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1063r0.pdf) extends this
thesis to "fix" some of the problems with the Coroutines TS. (To be fair, I like P1063 a lot.)

> We propose replacing the [Coroutines TS] `co_await` keyword with an operator-like token,
> which we tentatively suggest spelling `[<-]`...
