---
layout: post
title: "The space of design choices for `std::function`"
date: 2019-03-27 00:01:00 +0000
tags:
  library-design
  parameter-only-types
  type-erasure
---

Let's say we were designing C++11's [`std::function`](https://en.cppreference.com/w/cpp/utility/functional/function)
from scratch. For convenience, I'll refer to our `function`-alike class template as `F`.

Assume that we aren't messing with the *core, core* basics: `F` is parameterized by a function signature, and it
[type-erases](/blog/2019/03/18/what-is-type-erasure) its controlled object. We definitely want stuff like this
to compile:

    int run_twice(const F<int(int)>& callback) {
        return callback(1) + callback(1);
    }
    auto f = [a=1](int x) { return x + a; };
    assert(run_twice(f) == 4);

Our design work doesn't end there, though! We have to make a whole lot of design choices.
Each choice has upsides and downsides. Let's list all the design "knobs" we can think of.

## Core semantics

### Owning or non-owning?

If I put a lambda into an `F`, does `F` *take ownership of* the value of that lambda, or does it just take a reference
or shallow copy?

    F<int()> f;
    {
        auto t = [s = std::string("hello world")]() { return s.size(); };
        f = t;
    }
    int x = f();  // defined or undefined behavior?

If `f` does take ownership of a copy of `t`, then we have something like `std::function`.
If `f` just takes a non-owning reference to the lambda, then we have something like
[`function_ref`](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0792r3.html).

`function_ref` can be extremely lightweight — the size of two pointers, no heap allocation ever. It's a good
vocabulary type for callbacks (if you can't afford to make a template taking `const X&` for some reason).

But `function` actually manages lifetime. There are plenty of use-cases where you *need* an owning `function` type
that knows how to clean up after itself.

In fact, the phrasing "knows how to clean up after itself" is the key intuition here. "Should `F` be owning or
non-owning?" is just an abstract way of asking, "Should `F` know how to destroy its controlled object `t`, or
not?" It's a question of affordances, which is to say, it's a question about what belongs in `F`'s (notional)
"vtable." (Recall _affordances_ from my previous post ["What is Type Erasure?"](/blog/2019/03/18/what-is-type-erasure),
which borrows the idea from Don Norman's book [_The Design of Everyday Things_](https://amzn.to/39INVrT).)


### Copyable or non-copyable?

Suppose `F` does take ownership of its contents; so, it knows how to destroy its wrapped object `t`.
Does it also know how to make a copy of the object? If so, we have something like `std::function`.
If not, then we have something like [`std::unique_function`](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0228r3.html).

The upside of `unique_function` is that you can use a `unique_function` to store a move-only lambda
(such as a lambda that captures a `std::promise` by value). The downside, of course, is that `unique_function`
itself is move-only, which can limit our ability to make containers full of `unique_function`s or use
them as members of classes that do need to be copyable for some reason.


### Shareable?

If you're already thinking about how to implement `F`'s type erasure, then you might have realized that there's
a middle ground between "copyable" `std::function` and "move-only" `std::unique_function`. At the machine level, the former
keeps a `unique_ptr<AbstractWidget> m` and its copy constructor calls `m->clone()` to get a new copy of the wrapped object.
The latter keeps a `unique_ptr<AbstractWidget> m` and its copy constructor is deleted. But what if we just changed that
`unique_ptr` into a `shared_ptr`?!

[If we did that](https://wandbox.org/permlink/AYrkx28jMQD2VbAx), then copy-constructing one `F` from another would
result in two `F` objects that _shared ownership_ of a single wrapped callback.

    auto f = [i=0]() mutable { return ++i; };
    F<int()> alpha = f;
    F<int()> beta = alpha;
    F<int()> gamma = f;
    assert(alpha() == 1);
    assert(beta() == 2);  // beta shares alpha's heap-managed state
    assert(gamma() == 1);  // gamma's different provenance means different state

I've never encountered any reason to invent such a `shared_function`, but I must admit, it's *possible.*
If we expect to deal with _stateless, idempotent_ functions (so the confusing sharing semantics won't trip
us up) that are _expensive to copy_ and yet _frequently copied_, well, maybe we'd consider trying to use
`shared_function` in that case.


### Do we allow immovable objects?

One step beyond "move-only" is "not even movable." If we plan to use our `F` primarily as a function parameter type,
and pass it invariably by const reference, then `F` might not need to be copyable _or even movable!_

    void foo(const F<int()>& callback);

    void bar(const F<int()>& callback) {
        foo(callback);
    }

    int main() {
        bar([](){ return 42; });
    }

In the above snippet, there is only ever one `F<int()>` object; it's created in `main` and destroyed in `main`. It never
needs a move constructor.

Alternatively, if our `F` stores its wrapped object on the heap, then _moving_ an `F` object can be
done simply by moving the `unique_ptr` controlling the wrapped `T`; the wrapped `T` object itself never needs to move.
So we can actually implement a move-only `unique_function` capable of storing immovable `T`s! That's kind of neat.


## SBO affects semantics

### Do we have SBO at all?

Small buffer optimization (SBO), also known as small object optimization (SOO),
is an optimization of the general type-erasure technique. In SBO, we don't always heap-allocate our `WrappingWidget`;
if it's small enough we'll just store it directly inline. This is very similar to the small string optimization (SSO),
and in fact I personally will not get mad at all if you use the term "SSO" as a colloquial synonym for "SBO"/"SOO".

SBO is observable by the user.

    auto f = [i=0]() { printf("%p\n", &i); };
    F<void()> alpha = f;
    alpha();
    F<void()> beta = std::move(alpha);
    beta();

If the managed copy of lambda `f` is stored out-of-line on the heap, then we'll expect `alpha()` and `beta()` to print
the same address. If it's stored inline within `alpha`, and then physically moved over into `beta`, then we'll see
`alpha()` and `beta()` print two different addresses.


### Expose the SBO buffer size and alignment?

Every standard library vendor provides a `std::function` that incorporates some form of SBO. But their buffer sizes
[differ](https://wandbox.org/permlink/eU2vTWrVIZoMjnSS): libc++'s buffer is 24 bytes, while libstdc++'s is only 16 bytes.
This can make it hard to write portable code, or at least your code might suffer strange performance degradations
if it falls into the gap between different vendors' SBO sizes.

If we're writing our own `F` from scratch, maybe we should expose the SBO size explicitly to the user!

    template<class Signature, size_t Capacity = 24>
    class F;

    using CustomCallback = F<int(), 32>;

Any time we expose a "size," we should also expose an "alignment," since size and alignment are so closely intertwined in C++.

    template<class Signature, size_t Capacity = 24, size_t Align = alignof(std::max_align_t)>
    class F;

    using SSECallback = F<int(), 32, 32>;  // suitable for lambdas that capture MMX vector types

The upside of this idea is that it gives a great deal of power to the user. The downside is that it
can induce paralysis in the user — [giving too many choices can be counterproductive](https://en.wikipedia.org/wiki/Overchoice),
especially if those choices _usually_ don't matter in the big picture. That's why `std::function`
doesn't expose this knob to the user.

On the other hand, the size and alignment knobs are an important part of the interface of
[`inplace_function`](https://github.com/WG21-SG14/SG14/blob/master/SG14/inplace_function.h).


### What do we do with objects that don't fit in SBO?

If our `F` has SBO, it will try to store every controlled object in SBO, but sometimes it'll find (at
compile time, of course) that some object is too big or too highly aligned for the SBO buffer. In that case
it'll have to allocate the controlled object somewhere else.

`std::function` does not support the C++11 allocator model. But we could imagine a type-erased `F`
that receives an allocator on construction and then uses that allocator every time it needs to
heap-allocate a controlled object.


### Do we have anything _but_ SBO?

Alternatively, we could imagine an `F` whose idea of "heap-allocating" is to `static_assert` failure!
If we give a compiler error whenever the wrapped object doesn't fit into SBO, then we have
[`inplace_function`](https://github.com/WG21-SG14/SG14/blob/master/SG14/inplace_function.h).


### Should we store non-nothrow-move-constructible types in SBO?

Suppose we want `F`'s move constructor to be `noexcept`. ([This is a very important property. We
really want to have this.](http://ibob.github.io/blog/2018/07/03/compiler-generated-move/))
Well, `F`'s move constructor may in general wind up calling the move constructor of any
arbitrary `T` that we placed into the SBO buffer. If that `T`'s move constructor throws, then `F`'s
move constructor will throw.

Conclusion: If we want `F` to be nothrow movable, then we must not use SBO for any `T` that is
not nothrow movable.

As with the size/alignment filter, we have several possible routes for dealing with a type `T`
that fails the nothrow-movable filter.

- Store non-nothrow-movable objects on the heap, just as if they were too big for SBO.
    (This is one of two options available to a standard-conforming `std::function`.)

- Store non-nothrow-movable objects in SBO *anyway*, and just mark our move constructor as unconditionally
    `noexcept(false)`. (This is the other option available to a standard-conforming `std::function`.
    [libc++ does this.](https://wandbox.org/permlink/gdmR1fOXAIyxy66Y))

- Optimistically store non-nothrow-movable objects in SBO, marking our move constructor as unconditionally
    `noexcept(true)`. If move-constructing a `T` actually does throw an exception at runtime, then
    we'll `std::terminate` the program. We'll document this library precondition: feel free to use
    non-nothrow-movable `T`s with `F<void()>`, but your `T` still *shall not* throw exceptions from
    its move constructor! If your `T` throws, the program's misbehavior is your own fault.

- `static_assert(std::is_nothrow_move_constructible_v<T>)` inside the constructor of `F`.


### Should we store non-trivially-relocatable types in SBO?

My previous post ["What library types are trivially relocatable in practice?" (2019-02-20)](/blog/2019/02/20/p1144-what-types-are-relocatable/)
indicated that libstdc++'s `std::function` was trivially relocatable, because it avoids placing any `T`
into its SBO buffer unless that `T` is trivially copyable (which implies trivially relocatable).
Non-trivially copyable `T`s are heap-allocated by libstdc++, just as if they were too big for SBO.

On the other hand, libc++'s and MSVC's `std::function` will happily store non-trivially-relocatable types
into their SBO buffers.

The upside of libstdc++'s choice is that its `std::function` is trivially relocatable.
The downside is that it does more heap-allocation than libc++'s `std::function`. For example,
libc++'s `std::function` can store `[p = std::make_shared<int>()]() { return p; }` in SBO without
any extra allocation; but libstdc++ will do an extra heap-allocation since that lambda type is
not trivially copyable.

Alternatively, as with the previous section, we could handle non-trivially-relocatable types like this:

- `static_assert(is_trivially_relocatable_v<T> && sizeof(T) <= Capacity && alignof(T) <= Align)` inside the constructor of `F`.

This would produce a fundamentally different kind of `F`. It would be incapable of storing large,
overaligned, or non-trivially relocatable types. But, on the upside, `F` itself would be guaranteed
to be small, neatly aligned, and trivially relocatable! We might give it a name like
`trivially_relocatable_inplace_function`.

We might also find a niche use-case for `trivially_copyable_inplace_function`, which would be able to store
only trivially copyable types, but in exchange _would itself be_ trivially copyable.


## Wide versus narrow contract

### Does `F` need an "empty" state?

A default-constructed `std::function` is in an "empty" state, which compares falsey in `if` statements like

    std::function<void()> f;  // constructed empty
    if (f) {
        puts("this line is not executed");
    }

A moved-from `std::function` may or may not be in the "empty" state, and in fact
[libc++ and libstdc++ differ on this point](https://wandbox.org/permlink/ahQw39yoEK0km9sA).

But if we're writing `F` from scratch, do we even care about having a dedicated "empty" state?
We could eliminate `F`'s `operator bool() const` and just trust the user not to query an uninitialized `F` object.


### Should it be in-contract to call an "empty" `F`?

An empty `std::function` not only compares falsey, but throws
[`std::bad_function_call`](https://en.cppreference.com/w/cpp/utility/functional/bad_function_call) when you call it.
I would strenuously argue that if you're calling an empty `std::function`, you're Doing It Wrong. That's a bug
in your program; you should fix it.

Given that a correct program should never really try to call an empty `F`, should we bother to specify
what happens in that case? Maybe we should make "calling an empty `F`" a contract violation, or `assert(false)`
if it happens, or simply say that such a call is unsupported — that is, make it undefined behavior.

The upside of giving `operator()` a narrow contract is that it eliminates the entire reason for `bad_function_call`
to exist, and eliminates `F`'s unfortunate dependency on `-fexceptions`.  The downside is that it's a bit of
undefined behavior that wasn't there before.


### If we do have an empty state, should a moved-from `F` be guaranteed empty?

As mentioned above, [libc++ and libstdc++ differ on this point](https://wandbox.org/permlink/ahQw39yoEK0km9sA).
The upside of guaranteeing this is that it nails down some behavior that could cause portability concerns.

    auto sptr = std::make_shared<Widget>();
    F<void()> f = [sptr]() { };
    auto g = std::move(f); // LINE A
    assert(!f);  // LINE B

On `LINE A`, can we be confident that `sptr.use_count()` remains 2, or might `f` retain a copy of the `shared_ptr`
even after being moved-out-of? (That is, might `F` implement "move `F`" in terms of "copy `T`"?)

On `LINE B`, can we be confident that the assertion will pass? (That is, might `F` implement "move `F`" in terms of
"move `T`," rather than in terms of "relocate `T`"?)


### Should `F` be constructible from `nullptr`?

`std::function<void()>(nullptr)` is a valid, "empty," `std::function` object.
I dunno, that seems pretty crazy to me.


### Should `F` be constructible from `(void(*)())nullptr`?

`std::function<void()>((void(*)())nullptr)` is _also_ a valid, "empty," `std::function` object.
That is, every time you store a raw function pointer into a `std::function`, it inserts code to check whether
that function pointer is null or not. If the pointer is null, you get an "empty" `std::function`; if
it's non-null, you get a "non-empty" `std::function` (of course).

The upside of this facility is that it makes `std::function<void()>` a little more of a drop-in replacement
for `void (*)()` — if you put `nullptr` in, you get something falsey out. The downside is that it causes
extra codegen for a situation (storing null function pointers) that in most codebases will never actually arise.
You're paying for something you don't intend to use.


## "Type un-erasure"

`std::function` provides "type un-erasure" functionality through
[`f.target<T>()`](https://en.cppreference.com/w/cpp/utility/functional/function/target) and
[`f.target_type()`](https://en.cppreference.com/w/cpp/utility/functional/function/target_type).
(In [my book](https://amzn.to/2JHe9Cp) I call `f.target<T>()` a "go fish" function, because you
must supply the `T` you think is stored. If your guess is wrong, it simply returns `nullptr`.)

If you provide this functionality, then for every type `T` you store, you must generate code to
return its `typeid`, which means a lot of code and data bloat — and a hard dependency on `<typeinfo>`
and RTTI, which is a dealbreaker for a lot of codebases.

Even if you do provide a `target` method (to retrieve a pointer to the stored object so that the object
can be manipulated directly), you might follow `std::function`'s lead in making it a template, or you might
just implement a non-templated `target` method that returns `void *`. Or maybe `const void *`.

Or maybe you think it's silly that `f.target<Fruit>() == nullptr` when `f`'s target is of type `Apple`,
and you want `target` to respect class hierarchies as much as possible. [You can implement that.](https://wandbox.org/permlink/BSCtFWopnla36ikz)


## Implicit conversions and SFINAE

- Can I implicitly convert `F<int()>` to `F<void()>`? `std::function` silently lets you do this, but anecdotally,
    [I found a fair number of typo-bugs in HyperRogue](/blog/2019/01/06/hyper-function/#incidentally-today-i-learned-tha)
    when I wrote my own `F` that incidentally caught this implicit conversion.

- Can I implicitly convert `F<long()>` to `F<int()>`?

- If SBO size is exposed, can I implicitly convert `F<int(), 24>` to `F<int(), 32>`? What about vice versa (yikes!)?

And going all the way back to our earlier discussion of `static_assert`s, [suppose I write](https://wandbox.org/permlink/rd29ZcLGqTJyHnyN)

    void f(F<int()> f) {
        puts("YES");
    }

    void f(...) {
        puts("NO");
    }

    int main() {
        f([](){ return; });
    }

Should this program be required to print "NO", or is the convertibility of `[](){ return; }` to `F<int()>` something crazy enough
that we don't need to support SFINAE'ing on it, and it's okay if this program simply refuses to compile thanks to ambiguity and/or
a hard `static_assert`?

If SBO size is exposed, should I be able to SFINAE on the convertibility of `[some, captures](){}` to `F<void(), 24>`?
(This open question is the subject of [SG14 `inplace_function` issue #149](https://github.com/WG21-SG14/SG14/issues/149).)

The upside is that proper SFINAE-ability enables all sorts of clever metaprogramming tricks.
The downside is that proper SFINAE-ability... enables all sorts of clever metaprogramming tricks.


## Conclusion

These are just the "knobs" I thought of on the first pass. I bet there are more!

When you see all these little design decisions together — and especially when you realize that `std::function` got most of
them "wrong" — maybe it's easier to understand why it's taken 20 years to get `std::unique_function` and `std::function_ref`
on track for C++2a; and why the bikeshedding in committee will probably continue for a while longer before either of those
types is merged; and (most importantly, I say!) why I think it's important for every serious C++ programmer to be able to
write their own type-erasing wrapper. There are just too many knobs for any single library solution to
*perfectly* solve your codebase's needs. If you care about micro-optimization — and what C++ programmer doesn't? ;) —
then you will probably end up writing two or three type-erased function types in your career.

-----

**UPDATE, 2019-03-31:** I forgot about some knobs I meant to include! And Reddit suggested one more, too.

## What about (varargs) ellipses?

Does it make sense to support `F<int(const char *, ...)>`? That would neatly communicate "anything callable with
the same signature as `printf`."  However, C++ perfect forwarding and old-school C varargs do not mix;
you're not going to be able to implement the guts of `F<int(const char *, ...)>` so that it can forward to
`printf`. The impossibility of forwarding old-school varargs is the reason `va_list` and `vprintf` exist!

So `F<int(const char *, ...)>` is a non-starter.


## What about const-correctness?

Type-erased function types must deal with `const` in much the same way as they deal with copyability. When you call an
arbitrary callable type (such as a lambda), sometimes it'll be "const callable" and sometimes it won't be.
(Just like sometimes a lambda is copyable and sometimes it's not.)

    template<class T>
    void use(const T& some_lambda) {
        some_lambda();
    }

    auto cc = [i=0]() { return i+1; };
    auto ncc = [j=0]() mutable { return ++j; };

    use(cc);  // OK
    use(ncc);  // ERROR

So, when we wrap up a lambda into our type-erased `F`, we must decide whether `F::operator()` is going to be const-qualified
or not. (Just as we must decide whether `F` is going to have a copy constructor or not.)

There are at least three alternatives here. First, we could make `F` const-callable, which naturally requires
that every wrapped `T` be const-callable. This would mean that we couldn't wrap `ncc`
into an `F` object (because it's not const-callable).

Second, we could make `F::operator()` *not* const-qualified. This would let us wrap up `ncc`
into an `F` object; and it would also let us wrap `cc` (for the same reason that `unique_function` can easily hold
a lambda whose move-constructor is tantamount to its copy-constructor). However, non-const-callable types interact
pretty annoyingly with the rest of C++:

    const auto& cc = [i=0]() { return i+1; };
    cc();  // OK
    const F<int()>& fcc = cc;  // OK
    fcc();  // ERROR

Here, there is no way to call `operator()` on a `const F<int()>` object, because its `operator()` is not const-qualified.

A third alternative is to do as `std::function` does: make `F` const-callable, but have it internally *cast away the `const`*
from the wrapped `T` object, so that it ends up calling `T`'s non-const-qualified `operator()` (if `T` has one). This produces
the most ergonomic experience for the user —

    auto cc = [i=0]() { return i+1; };
    const F<void()> f1 = cc;  // OK
    f1();  // OK

    auto ncc = [j=0]() mutable { return ++j; };
    F<void()> f2 = ncc;  // OK
    f2();  // OK

— but at the cost of const-correctness
[and therefore at the cost of thread-safety](https://www.justsoftwaresolutions.co.uk/cplusplus/const-and-thread-safety.html).
In the code below, we end up invoking `f2`'s `operator() const` from two threads concurrently, which should be fine; but
in reality it's not fine, because `f2`'s `operator() const` invokes `operator()` (non-const) on the wrapped copy of `ncc`,
and so our two modifications to the captured `j` will race with each other.

    auto ncc = [j=0]() mutable { return ++j; };
    F<void()> f2 = ncc;  // OK
    auto thread_body = [](const auto& f) { return f(); };
    return std::async(thread_body, f2).get() + std::async(thread_body, f2).get();

Yet a fourth alternative would be for our `F` to provide *both* signatures — both `operator()` and `operator() const` — and
require `T` to be callable as both const and non-const. But this is almost indistinguishable in practice from our first
alternative. Such an `F` cannot wrap any `T` which is not const-callable.


### `F<int()>` could be different from `F<int() const>`

Perhaps the cleanest way out of the const-correctness mess is to put the choice back on the user.
C++ lets us name both `int()` and `int() const`; the latter is an
[abominable function type](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0172r0.html).
So it is tempting for us to say that `F<int()>` should be non-const-callable (and not require const-callability of its `T`),
whereas `F<int() const>` should be const-callable (and require const-callability of its `T`). Most users will probably
get by just fine with `F<int()>`, and then the few who really *need* const-callability can just slap some `const`s inside
their angle brackets and continue on their way.

This is the solution preferred by [`folly::Function`](https://github.com/facebook/folly/blob/master/folly/docs/Function.md),
by [`fu2::function`](https://naios.github.io/function2/#how-to-use), and (currently) by the proposed
[`std::unique_function`](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0228r3.html).


## What about ref-qualifiers?

Once we open the Pandora's box of abominable function types, we must ask: What is the meaning of `F<int() volatile>`?
What is the meaning of `F<int()&>`? What is the meaning of `F<int()&&>`?

`F<int()&&>` is particularly interesting because it's been historically attractive to armchair library designers.


### Should `F<int()&&>` be rvalue-callable?

If `F<int() const>` is const-callable with the signature `operator()(...) const`, then surely it's only logical that
`F<int()&&>` (if it is well-formed at all) should be rvalue-callable with the signature `operator()(...) &&`.
That is, the user would write something like

    void foo(F<int()&&> callback) {
        std::move(callback)();
    }

Because we've been trained not to do anything with a moved-from object, we intuit that `callback` will
be called only once along any given codepath. We might use the signature `F<int()&&>` to communicate a little
extra information about a callback that was going to be used to satisfy a future, or was going to be sent off
to a thread pool.

However, is this another case of foisting unnecessary complication on the user? I'm not aware of any rvalue-callable
objects already existing in real-world code, so the difference between `F<int()>` and `F<int()&&>` seems purely
cosmetic, so far.

> Notice that `std::function<int()>` is rvalue-callable only because it is const-callable;
> and `std::function<int()&&>` is ill-formed.


### Should `F<int()&&>` be "one-shot"?

If we did implement an rvalue-callable `F`, then we might wonder whether calling `F` should put it into the
"moved-from" state. If we say yes, then this choice interacts with our previous choices about the behavior of
that moved-from state. If the moved-from state is visibly "empty," then we support code like this:

    void use(int x, F<int()&&> callback) {
        if (x == 0) {
            std::move(callback)();
        }
        if (callback) {
            puts("I guess x wasn't 0");
        }
    }


## What about `noexcept` qualifiers?

Now that `noexcept` is part of the typesystem in C++17, we have a new postfix qualifier that can appear in a function type.
In C++17 and later, `F<int() noexcept>` is a *different type* from `F<int()>`, although it is not abominable.

`std::function<int() noexcept>` is ill-formed. Should `F<int() noexcept>` be well-formed?

If so, clearly `F<int() noexcept>::operator()` should itself be `noexcept`. And then we have the same situation with
`noexcept` that we had with `const` — or for that matter with non-nothrow-movable types. Either we rigidly enforce that
`F<int() noexcept>` can wrap only nothrow-callable types, or else we break "noexcept-correctness" and risk a call
to `std::terminate`.

    auto lam = []() { puts("hello world"); };
    static_assert(!noexcept(lam()));

    F<void() noexcept> wrapped = lam;  // OK?
    static_assert(noexcept(wrapped()));


## What about multiple overloaded signatures?

Reddit commenter NotAYakk points out that [David Krauss's `cxx_function`](https://github.com/potswa/cxx_function/)
supports overloaded signatures. So we can imagine an `F` that permits something like this:

    F<void(int), void(std::string)> visitor = ...;
    std::variant<int, std::string> visitee = ...;
    std::visit(visitor, visitee);

Here `std::visit` will dynamically call either `visitor(42)` or `visitor("hello")`, depending on the active member of
`visitee`. So `F<void(int), void(std::string)>` needs to have two different overloads of `operator()`, and each
wrapped type `T` must _afford_ both behaviors:

    void operator()(int x) { return ptr_->callme_with_int(x); }
    void operator()(std::string s) { return ptr_->callme_with_string(s); }

So for example

    auto alpha = [](int x) { std::cout << x; };
    visitor = alpha;  // ERROR

would be an error because `alpha` does not _afford_ calling-with-a-string-argument; but

    auto beta = [](const auto& x) { std::cout << x; };
    visitor = beta;  // OK!

would be okay, because `beta` _affords_ both calling-with-an-int and calling-with-a-string.

Since type erasure requires an explicitly enumerated list of affordances, there is no way to
make a type-erased function type that can take *any parameter at all*. That is, there's no way
to make an "`F<void(auto)>`," even if that syntax were grammatically available, which it's not.
