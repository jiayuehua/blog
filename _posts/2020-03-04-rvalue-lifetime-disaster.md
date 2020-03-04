---
layout: post
title: 'Thoughts on "The C++ Rvalue Lifetime Disaster"'
date: 2020-03-04 00:01:00 +0000
tags:
  copy-elision
  move-semantics
  paradigm-shift
---

I just watched Arno Schoedl's talk from the CoreHard conference (Minsk, 2019) titled
["The C++ Rvalue Lifetime Disaster."](https://www.youtube.com/watch?v=s9vBk5CxFyY)
It's a good talk, worth watching. I have some thoughts on it.

Arno's thesis is a thought-provoking flip-flop of my thesis from
["Value category is not lifetime"](/blog/2019/03/11/value-category-is-not-lifetime/) (2019-03-11).
We both observe that value category _strongly correlates_ with lifetime:

- Rvalues are frequently short-lived; it's frequently unsafe to take long-lived references to rvalues.

- Lvalues are frequently long-lived; it's frequently safe to take long-lived references to lvalues.

My thesis in the above blog post is: Since this rule uses the word "frequently" instead of "always,"
it's simply not safe for anyone ever to assume that value category _equals_ lifetime. Some libraries
(such as C++11 `regex` and C++20 Ranges) are built on such an invalid assumption; these libraries are bad.

Arno's thesis, on the other hand, is: Let's close the loophole! Can we tweak the rules of C++ so that
rvalues are _always_ short-lived and lvalues are _always_ long-lived? (He seems to believe that the answer
is "yes." I'm going to show why I think it's "no.")

----

The main reason that value category imperfectly correlates with lifetime
is because of the hack that C++98 put in for `const&` parameters,
and that C++11 had to preserve for backwards compatibility.

    void print(const std::string& x);
    std::string make_rvalue();
    void test(std::string lvalue) {
        print(lvalue);
        print(make_rvalue());
    }

In C++98, we needed a way to write a function like `print` that was able to take _either_ an lvalue or an rvalue.
The most salient thing about rvalues is that they're not meaningfully modifiable — rvalue expressions are things
like `x+5` or `&x` or `42`, that can't appear on the left-hand side of an assignment.
But if you're not planning to modify the argument, then conceptually you shouldn't care whether
it was originally an lvalue or an rvalue! So rvalue arguments were permitted to bind to `const&` parameters.

In C++11, we had to keep this old code working. So, just as in C++98, C++11's rvalues happily bind to
const lvalue references, even though (as in C++98) rvalues will not bind to non-const lvalue references.

C++11's rvalue references were invented to enable _move semantics._ The guarantee, when
you see `&&`, is that nobody else cares about the referred-to object. Either it's a temporary like `x+5`,
or it's an xvalue like `std::move(x)` where the ultimate owner has said "I don't care about this object
anymore." This guarantee must be rock-solid and absolute, because if it's not, then your code might end up
moving-out-of an object that someone else still cares about. For example:

    void pilfer(std::string&& x);
    void test(std::string lvalue) {
        pilfer(lvalue);  // MUST NOT COMPILE
        print(lvalue);   // because we still care about lvalue
    }

This doesn't necessarily mean that _all_ rvalue references refer to short-lived objects — remember,
_value category is not lifetime_ — but it's where that correlation comes from. Rvalue references
by definition refer to things nobody else cares about, which tends to include materialized temporaries.

Unfortunately, in C++11, it is tempting to flip that promise around. Rather than seeing double `&&` as a promise
of "pilferability," it is tempting to see single `&` as a promise of "longevity."

    const std::string *global;
    void keep(const std::string& x) {
        global = &x;  // Lvalues are long-lived, right?
    }
    void test() {
        keep(std::string("abc"));  // OOPS
    }

[At @33:14](https://www.youtube.com/watch?v=s9vBk5CxFyY&t=33m14s),
Arno gives a very helpful diagram of the implicit conversions that C++ permits for reference types.
I've taken the liberty of redrawing it for this blog post, and adding the word "[maybe]" where appropriate:

![](/blog/images/2020-03-04-current.png)

He captions this diagram: "Current C++ reference binding _strengthens_ lifetime promise."
However, you should mentally insert the word "perceived" in there. C++ does not actually
provide any "lifetime promise." But, if you conflate value category with lifetime, you may run into
situations where C++'s reference binding rules promote an rvalue reference (which makes no promise of
longevity) into a const lvalue reference (which you _perceive_ to promise longevity, even though it
doesn't).

Arno then ([@35:02](https://www.youtube.com/watch?v=s9vBk5CxFyY&t=35m02s)) says, "In order to fix it,
you have to turn some arrows around." Like this:

![](/blog/images/2020-03-04-flipped.png)

In this hypothetical world, the idiom for functions taking both lvalues and rvalues would not be
`print(const std::string& x)` but instead `print(const std::string&& x)` — "I don't need
mutability from my argument, _and_ I don't need longevity."


## The problem (philosopher's version)

> Longevity is relative.

In C++, there's no such thing as "long-lived, safe-to-reference" objects versus "short-lived, unsafe-to-reference"
objects. Every object is safe to reference _in a single instant_. In fact, every object is perfectly safe to
reference _until the end of its lifetime_. And lifetimes in C++ can be all kinds of lengths!

    template<class T>
    void test(const int& x, T do_something) {
        do_something();
        std::cout << x << "\n";
    }

    int main() {
        int *x = new int(42);
        test(*x, [](){});
        test(*x, [p=x]() { delete p; });
    }

One of these calls to `test` is safe; the other is unsafe. There will never be any way to distinguish them
at the typesystem level, no matter how much fiddling you do with reference qualifiers and value categories.

> Longevity is always relative.


## The problem (rabbit-hole version)

I also see a technical problem with Arno's world: We'd need some new behavior for variables of rvalue reference type.
In C++ today, named variables are _always_ lvalues. We can write:

    void modify(std::string& x);  // require mutability
    void pilfer(std::string&& x);  // require pilferability
    void print(const std::string& x);  // require nothing

    void test(std::string&& x) {  // require pilferability
        modify(x);  // OK
        print(x);   // OK
        pilfer(x);  // DOES NOT COMPILE
        pilfer(std::move(x));  // OK
    }

Named variables must be lvalues, because C++ cares most about preserving the "pilferability
guarantee." If someone _might_ touch `x` later, then it _must_ be treated as an lvalue by default.

In Arno's hypothetical language, it seems that the analogous code would be:

    void modify(std::string& x);  // require mutability and longevity
    void pilfer(std::string&& x);  // require mutability
    void print(const std::string&& x);  // require nothing

    void test(std::string&& x) {  // require mutability, but not longevity
        modify(x);  // Hmm... not OK?!
        print(x);   // OK
        pilfer(x);  // Hmm... OK?!
        pilfer(std::move(x));  // OK
    }

Two lines here are problematic. First, philosophically, pilfering is implemented in terms of mutation.
In order to pilfer from `x`, I need to modify `x`. So I still need to be able to pass `std::string&& x`
to `modify(std::string&)`, right?  Or else I need to change all my mutators' signatures from
`modify(std::string&)` to `modify(std::string&&)` ("require mutability but not longevity")... but then
I wouldn't be able to call them on plain old lvalues anymore! So we have a problem on the line marked
"Hmm... not OK?!"

Vice versa, the ability to mutate does not imply the ability to pilfer. We really don't want the line
`pilfer(x)` to compile, since `x` is a named variable and we might continue observing it later.
So there's also a problem with the line marked "Hmm... OK?!".


## Down with reference lifetime extension!

Arno makes another modest proposal ([@18:47](https://www.youtube.com/watch?v=s9vBk5CxFyY&t=18m47s)):

> If you ask me, deprecate temporary lifetime extension. Don't use it anymore.
> It's bad. It has been bad since we introduced rvalue references.

I would tweak that last sentence — it's always been bad, but it only became really glaringly out-of-place
once C++17 introduced delayed temporary materialization. (See
["Guaranteed Copy Elision Does Not Elide Copies"](https://blog.tartanllama.xyz/guaranteed-copy-elision/)
(Sy Brand, December 2018).) However, I can't say that it has _no_ reasonable uses. See these posts
of mine:

* ["On lifetime extension and `decltype(auto)`"](/blog/2018/04/05/lifetime-extension-grudgingly-accepted/) (2018-04-05)

* ["Field-testing 'Down with lifetime extension!'"](/blog/2020/03/04/field-report-on-lifetime-extension/) (2020-03-04)
