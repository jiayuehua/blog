---
layout: post
title: "`const` is a contract"
date: 2019-01-03 00:01:00 +0000
tags:
  c++-learner-track
  c++-style
  move-semantics
  slogans
excerpt: |
  Here's a slogan that needs more currency:

  > `const` is a contract.
---

Here's a slogan that needs more currency:

> `const` is a contract.

Let's start off with the one place this is _not_ true: variable declarations.

    int main() {
        int x = 42;
        const int y = 43;
        // ...
    }

Here `y` is an object of const type. This means that it's constant, immutable;
nobody can change its value, and its value cannot change. This is what
newcomers to the language usually expect `const` to mean. This is the easy part.

"But what if I `const_cast` away the const and modify `y` anyway?" Then you have
undefined behavior.

    const_cast<int&>(y) += 1;  // Undefined behavior!

An _object_ defined `const` is really and truly constant, immutable, unchanging.


## In function prototypes

So what about in a function prototype?

    void f(const int& p) { ... }

Here `p` is a reference to a const-qualified `int`. Is that `int`'s value
constant, immutable, unchanging? **No!**

    static int g = 0;
    void f(const int& p) {
        printf("%d ", p);
        g = 1;
        printf("%d\n", p);
    }
    int main() {
        f(g);
    }

[The above program is required to print `0 1`.](https://wandbox.org/permlink/zOD8uX7lCeX14zEh)
The compiler is not allowed to assume that the value referenced by `p` is immutable!

So what good is `const`?

> `const` is a contract.

This keyword `const` is a contract — a promise — between the function `f` and its caller.
By putting `const` in its signature, `f` is saying, "Hello caller, I promise never to modify the referent
of `p`." (At least, not directly via `p`.)

The caller can look at that and know that `f` is promising not to modify the referent of `p`;
therefore, it is safe to pass something to it that the caller doesn't have permission to
modify.


## The compiler enforces contract law

Alice's aunt (who lives in Maine) owns a very rare and delicate Ming vase.
Alice promised her aunt that she wouldn't break the vase, or modify it in any way;
and so Alice's aunt has grudgingly permitted Alice to touch the vase.

    void alice(const Ming& vase) {
        // ...
    }

    int main() {
        Ming vase;
        alice(vase);
    }

Notice that Alice's aunt in Maine actually _can_ modify the vase; it's not literally immutable.
But Alice _herself_ has made a promise — a contract — that says she, Alice, will not attempt
to modify the vase.

Now Bob says, "Let me touch the vase too!"

    void bob(Ming& vase);

Bob isn't making any promises like the one Alice gave her aunt. Alice strongly suspects that
Bob *is* going to try to modify the vase; after all, he didn't promise that he wouldn't!
What should Alice do? Is it okay for Alice to let Bob touch her aunt's vase?

**No,** of course it's not okay!
[Alice doesn't let Bob touch the vase.](https://wandbox.org/permlink/DJQRLRfQRtIFYNKt)

    void bob(Ming& vase);

    void alice(const Ming& vase) {
        bob(vase);  // ERROR
    }

Now Charlie asks to touch the vase; and he adds, significantly, "I promise not to modify the thing I touch."
Only because Charlie promises not to modify the vase, Alice will let Charlie touch it:

    void charlie(const Ming&);
        // "I promise not to modify the thing I touch."

    void alice(const Ming& vase) {
        charlie(vase);  // "OK, Charlie, since you promised."
    }

The compiler does not permit Alice to ignore the promise she's made to her aunt.


## Naughty on purpose with `const_cast`

C++ does have a thing called `const_cast` that Alice can use to be naughty on purpose:

    void bob(Ming& vase) {
        vase.smash();  // OK; and honestly this is exactly what I expected Bob to do
    }

    void alice(const Ming& vase) {
        vase.smash();  // ERROR; the compiler prevents this
        bob(vase);     // ERROR; the compiler prevents this
        const_cast<Ming&>(vase).smash();  // OK, but naughty
        bob(const_cast<Ming&>(vase));     // OK, but naughty
    }

    int main() {
        Ming vase;
        alice(vase);
    }

In this example, Alice is not _ignoring_ the promise she's made to her aunt; she is actively
and explicitly _breaking_ that promise. Normally, you don't want to do this in your programs.
But C++ allows it...

...as long as the vase object is not _literally_ immutable!

    int main() {
        const Ming vase;
        const_cast<Ming&>(vase).smash();  // Undefined behavior!
    }


## Pointers behave just like references in this respect

When we take a function parameter of type `const int *`, we're saying "Please give me the
address of an `int`, and I promise not to modify that `int`."

And the `const` can apply at each different level, independently:

When we take a function parameter of type `const int **`, we're saying "Please give me a pointer to
an `int*`, and I promise not to modify the `int` it points to (but I might modify the `int*` itself)."

When we take a function parameter of type `int *const *`, we're saying "Please give me a pointer to
a pointer to an `int`, and I promise not to modify that `int*` (but I might modify the `int` it points to)."

When we take a function parameter of type `const int *const *`, we're saying "Please give me a pointer to
an `int*`, and I promise not to modify _either_ the `int*` or the `int` it points to."

(Very minor handwaving here: "pointer to `int*`" and "pointer to `const int*`" are actually
different types, and behave according to covariance and contravariance, which is fun times.
Intrepid readers can find more details in [the standard](http://eel.is/c++draft/conv.qual#3),
until I find a better explanation to link to.)


## `const` on a member function affects the `this` pointer

Here are two non-member ("free") functions.
One promises not to modify the `Ming` it receives via parameter `p`; the other makes no such promise.

    void handle_with_care(const Ming *p);
    void juggle(Ming *p);

Here are two member functions. One promises not to modify the `Ming` it receives
via _the hidden `this` parameter_; the other makes no such promise.

    struct Ming {
        void polish() const;
        void smash();
    };

Guideline:

> After you write a member function, look at your implementation. Does it mutate the `this` object?
> If it does not, then you should mark the member function `const`.

Recall that `const` is a contract. Suppose Alice has been permitted to touch her aunt's vase, on
condition that she not modify it in any way. Should the compiler allow Alice to polish the vase?
Should the compiler allow Alice to smash the vase?

> Before you write a member function, think about what it does. If someone has a `const Foo&`
> that they've promised not to modify, should they be allowed to call your member function?
> If so, then you _must_ mark the member function `const`!


## `const` on a by-value parameter is not a meaningful contract

We've discussed `const Foo&` and `const Foo *` parameters. What about `const Foo` by itself?

    void replicate(Ming vase);
    void counterfeit(const Ming vase);

Here we have two functions that accept a Ming vase _by copy_. If we read the function signatures
as contracts, we see that `replicate` says, "Give me a copy of your vase; and I make no promises
about what I'll do with the copy." Whereas `counterfeit` says, "Give me a copy of your vase; _and
I promise_ not to modify my copy."

That's not much of a promise! Why should your aunt care whether you modify a _copy_ of her vase?
She knows her original won't be modified. It's none of her business what you do with _your copy_.

Therefore, [C++ treats these two function signatures as exactly equivalent.](https://wandbox.org/permlink/HTeV68LV1BCPN4di)
There is no difference, from the compiler's point of view, between taking `Ming` by value
and taking `const Ming` by value. (There is a difference _inside_ the body of the function, but
Alice's aunt doesn't care what you do in there.)

Therefore, most C++ programmers (including me!) recommend that you never take a parameter
"by const value" like this:

    void counterfeit(const Ming vase);  // BAD STYLE!

There's a benefit to adopting this guideline. Especially for students coming to C++
from Java, a *very very common* failure mode is to accidentally leave off the ampersand on
an otherwise idiomatic pass-by-const-reference:

    bool operator==(const Ming& a, const Ming b) {
        // ...
    }

Because passing by-const-value is _definitely bad_, we know there's _something_ wrong with `const Ming b`.
Having been alerted to a problem, it doesn't take us long to identify the missing ampersand.
(There's a tool called `clang-tidy` that can help with this... at least, I think it can.)


### `const` on a by-value parameter can be a pessimization

I looked for this kind of typo in my current employer's codebase — code written by professional
programmers with years of C++ experience. I found an absolutely astounding number of them!
The most common and egregious variation in our codebase looks like this:

    class Widget {
        std::string name_;
    public:
        void set_name(const std::string name) { name_ = name; }
    };

This function will pass all the unit tests you throw at it,
but it makes an additional heap-allocation compared to either

        void set_name(const std::string& name) { name_ = name; }

or

        void set_name(std::string name) { name_ = std::move(name); }

We'll see more about pessimizations later in this blog post.


## `const` on a by-reference return type

Sometimes we want our caller to promise something to _us_. Let's flip around our Ming vase example.
Now instead of Alice directly permitting Bob to touch the vase, Alice will expose a public API method
that allows _any caller_ to gain access to the vase. Here's our initial code, without any `const` anywhere.

    struct Alice {
        Ming *vase_;
        explicit Alice(Ming *vase) : vase_(vase) {}
        Ming& getVase() { return *vase_; }
    };

    struct Bob {
        void ask_for_access(Alice& a) {
            Ming& vase = a.getVase();
            vase.smash();  // You probably saw this coming.
        }
    };

    int main() {
        Ming vase;
        Alice alice(&vase);
        Bob bob;
        bob.ask_for_access(alice);
    }

Alice's aunt wouldn't approve of this version! Let's make Alice promise not to modify the vase.
When Alice asks her aunt for a pointer to the vase, she must additionally promise not to modify
that vase:

    struct Alice {
        const Ming *vase_;
        explicit Alice(const Ming *vase) : vase_(vase) {}
        Ming& getVase() { return *vase_; }  // ERROR!
    };

Now the compiler gives us an error in `getVase()`. See, Bob — or anybody — can call `getVase`
to get a reference to the vase, and then they can touch it; and then they can smash it; and
that would break the promise that Alice made to her aunt. The compiler will not let Alice ignore
her promise!

So when Alice returns the reference from `getVase`, Alice must _extract
a promise from the caller._ The _caller_ must promise not to modify the vase!
We indicate that by putting `const` in the appropriate place on the return type.

    const Ming& getVase() { return *vase_; }  // OK!
        // "I'll let you touch this vase, but you must promise not to modify it."

Now the compiler is happy with Alice... but _Bob_ is unhappy!

    struct Bob {
        void ask_for_access(Alice& a) {
            Ming& vase = a.getVase();  // ERROR! Binding `Ming&` to `const Ming&` discards qualifiers.
            vase.smash();
        }
    };

The only way for Bob to touch Alice's aunt's vase is for Bob to promise that he won't smash it.

    struct Bob {
        void ask_for_access(Alice& a) {
            const Ming& vase = a.getVase();  // OK, binding `const Ming&` to `const Ming&`
            vase.smash();  // NOT ALLOWED
        }
    };

Of course, if Bob makes his own copy of the vase, he can do whatever he wants with the copy.

    struct Bob {
        void ask_for_access(Alice& a) {
            Ming vase = a.getVase();  // OK, making a copy via the copy constructor
            vase.smash();  // OK, smashing the copy
        }
    };

Before we leave this example, we should make it fully const-correct. That means applying our
earlier guideline:

> Before you write a member function, think about what it does. If someone has a `const Foo&`
> that they've promised not to modify, should they be allowed to call your member function?
> If so, then you _must_ mark the member function `const`!

Here's the fully const-correct code, with the one remaining `ERROR` exactly where we want it.
(Bob has promised Alice not to modify the vase, so the compiler won't let him smash it.)

    struct Alice {
        const Ming *vase_;
            // Alice promises not to smash this vase.
        explicit Alice(const Ming *vase) : vase_(vase) {}
            // Alice promises not to smash whatever vase she's given.
        const Ming& getVase() const { return *vase_; }
            // Alice extracts a similar promise from her callers.
            // Also, getVase() does not modify this Alice object!
    };

    struct Bob {
        void ask_for_access(const Alice& a) const {
            // This function doesn't modify the Alice object, and doesn't modify this Bob, either.
            const Ming& vase = a.getVase();
                // OK, binding `const Ming&` to `const Ming&`
            vase.smash();
                // ERROR: cannot smash a const Ming vase
        }
    };

    int main() {
        Ming vase;
        Alice alice(&vase);
        Bob bob;
        bob.ask_for_access(alice);
    }


## `const` on a return type is not a meaningful contract (and is sometimes a pessimization)

We've discussed `const Foo&` and `const Foo *` return types. What about `const Foo` by itself
as a return type?

    const Ming getImmutableVase();

Const-by-value return types are wacky. [Here's a Wandbox
showing their behavior](https://wandbox.org/permlink/Uw33cjVvGmsqRZsO), but before you look at it,
think about what a const-by-value return type should mean. It means the function is giving you
a _new copy_ of its return object; but that copy is marked `const`.

> `const` is a contract.

The function is extracting from you a promise that you will not modify the return object —
even though you're getting your own copy of it! The function is saying: "I'll give you a copy
of what I have; but _you have to promise_ not to modify your copy." This doesn't make much sense!
In the example above, when Bob made a copy of the vase, we didn't care if Bob smashed his copy.
And we shouldn't care in this case either.

Suppose Alice has

    const Ming getVase() const { return *vase_; }

And suppose Bob does

    Ming vase = alice->getVase();  // make a copy
    vase.smash();  // smash the copy

This is totally fine! And it remains fine even if you ask the compiler to fill in type `Ming`
for you:

    auto vase = alice->getVase();  // `auto` is deduced as `Ming`
    vase.smash();  // smash the copy

(And yes, [copy elision](https://en.cppreference.com/w/cpp/language/copy_elision)
happens in both cases.) But here's the weird part:

    alice->getVase().smash();  // ERROR: cannot smash a const Ming vase

The compiler remembers the const-qualification and enforces your promise not to modify the return value,
_even though that promise is effectively meaningless!_

Just to add a dash of confusion: the situation for primitive types such as `int` is different.
Primitive return types behave like parameter types: const-qualifying them is rightly
seen as pointless and so the compiler just ignores the `const` in those cases.

    const int getInt();
    const Ming getMing();

    static_assert(is_same_v< decltype(getInt()), int >);  // the const is quietly dropped!
    static_assert(is_same_v< decltype(getMing()), const Ming >);

Okay, now you can go look at [the Wandbox example](https://wandbox.org/permlink/Uw33cjVvGmsqRZsO)
and have your mind blown a little bit by the inconsistency of it all.

This inconsistency is amusing, but it never really gets in our way, because the inconsistency happens
only when we const-qualify a by-value return — and that's meaningless, so we should never do that!
Guidelines:

> `const` on a by-value parameter is never right. (It often indicates a missing ampersand.)
>
> `const` on a by-value return type is never right. (It often indicates a missing ampersand.)


### The missing ampersand typo

Many times in student code — and even in production code written by professional programmers —
I've seen APIs like this:

    class Person {
        std::string firstname_;
        std::string lastname_;
        int age_;
    public:
        // ...
        int age() const { return age_; }
        const std::string firstname() const { return firstname_; }
        const std::string lastname() const { return lastname_; }
    };

Did you spot the red flag? Those last two methods were intended to be either

        const std::string& firstname() const { return firstname_; }
            // return by-reference, as long as the caller promises not to modify the name

or

        std::string firstname() const { return firstname_; }
            // return by-copy

We don't know for sure which version was intended, but we know that what's there _now_
is a typo.


### The misplaced `const` typo

Furthermore — more in student code than in professional code — I often see this mistake:

        const int age() { return age_; }
        const std::string firstname() { return firstname_; }
        const std::string lastname() { return lastname_; }

This usually means that the student is confused about where the `const` goes on a method.
In the above position, `const` is saying that `age()` returns a non-modifiable `int`; but
`age()` doesn't promise anything about whether it modifies `*this` itself. Compare to:

        int age() const { return age_; }
        std::string firstname() const { return firstname_; }
        std::string lastname() const { return lastname_; }

In this trailing position, `const` means that `age()` promises not to modify `*this` — which
means now the compiler will allow us to call `age()` on a const-qualified `Person` object.
This is what we want! If I have a handle to a `Person` whom I've promised not to modify,
I should still be able to call `.age()` on it, right? Right.


### Returning move-semantic types by const value is a pessimization

Consider this code:

    std::string foo();
    const std::string bar();

    int main() {
        std::string x;
        x = foo();   // OK: move-assignment
        x = bar();  // OK, but: copy-assignment!
    }

Do you see what's happening here? The assignment `x = foo()` ends up calling `string::operator=(string&&)`
because the right-hand side is a prvalue of type `string`. But `x = bar()` cannot call
`string::operator=(string&&)` because binding the parameter of type `string&&` to
a prvalue of type `const string` would discard qualifiers. So instead it calls
`string::operator=(const string&)`, which does a whole new heap-allocation in order to make a _copy_
of the returned string.

So in the code from the previous section —

        const std::string firstname() const { return firstname_; }

— not only did the typo make it difficult to understand our intent, but, even if we _had_ meant to return
by value, we were accidentally pessimizing a lot of places in our code!

Be aware that Scott Meyers' [_Effective C++, Third Edition_](https://amzn.to/2C3Yd64)
(Item 3, page 18) once ["implied"](http://www.aristeia.com/BookErrata/ec++3e-errata.html)
that returning by const value could be a good thing.

    const Rational operator*(const Rational& lhs, const Rational& rhs);

This advice was removed via an erratum in 2009, and in 2014 Meyers' [_Effective Modern C++_](https://amzn.to/2RxfzCa)
(Item 25, page 172) gave a similar C++11 example with no return-by-const-value in sight:

    Matrix operator+(Matrix&& lhs, const Matrix& rhs);


## Guidelines for reliably const-correct code

1. `const` is a contract. When `const` appears in a function signature, it means that _someone_
    is making a promise to _someone else_ not to modify _something_.

2. Taking parameters by const reference is frequently useful, and makes a meaningful promise to your caller.
    Taking parameters by const _value_ is never meaningful to the caller.
    The function signatures in your header files should never contain "const-by-value" parameters.

3. Returning by const reference is sometimes useful, and extracts a meaningful promise from your caller.
    Returning by const _value_ is never meaningful for scalar types, and is actually harmful for
    move-semantic class types.
    Your functions should never return "by const value."

4. Violations of these guidelines frequently indicate typos, pessimizations, or misunderstandings
    of the role of `const`.


## Grep your codebase today!

    # Look for pass-by-const-value typos
    git grep '[(,] *const [A-Za-z0-9_:]* [A-Za-z0-9_:]* *[,)=]'

    # Look for return-by-const-value typos
    git grep 'const [A-Za-z0-9_:]* [A-Za-z0-9_:]*([^")]*) const'
    git grep 'const [A-Za-z0-9_:]* [A-Za-z0-9_:]*([^")]*\( \|$\)'

The latter regex will inevitably have some false positives, because it cannot distinguish between

    const bool get_value(int);  // a (BAD) function declaration
    const bool my_value(true);  // a (FINE) variable definition

Of course this ambiguity is eliminated if you use idiomatic variable definitions —

    const bool my_value = true;

— but that's a subject for [a different post](/blog/2019/02/18/knightmare-of-initialization/). :)
