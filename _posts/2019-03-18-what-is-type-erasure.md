---
layout: post
title: "What is Type Erasure?"
date: 2019-03-18 00:01:00 +0000
tags:
  c++-learner-track
  library-design
  metaprogramming
  type-erasure
---

I love type erasure in C++. But I find every so often I'll be in a conversation with someone,
and they'll use the phrase, and they won't mean quite the same thing as I mean, and it'll
take a while before we realize that we're talking about completely different ideas.
So I think it'll be useful to write down what I mean by the phrase "type erasure" in C++.


## C++ type erasure is not Java type erasure

First of all, in some languages, such as Java, "type erasure"
[means something completely different](https://docs.oracle.com/javase/tutorial/java/generics/erasure.html).
In Java, it means the procedure applied by the compiler when you write a "generic" function
— which looks deceptively similar to a C++ template, *but is not a template!*

    public static <T> T find(T[] anArray, T elem) {
        for (T e : anArray) {
            if (e.equals(elem)) {
                return e;
            }
        }
        return null;
    }

The intent is that you could call `find(someShapes, myShape)` or `find(someAnimals, myCat)`. The compiler
makes those calls work.

In C++, it would make them work by _deducing_ what `T` is for each call, then stamping
out specialized versions of `find<Shape>` and `find<Animal>` (or actually, C++ would fail to deduce `T` in
that second case).

But in Java, the compiler actually emits code for a single version of `find` right away! That single version
is implemented in terms of Java's
[root object type](https://stackoverflow.com/questions/8669693/advantage-of-singly-rooted-class-hierarchy) `Object`.
In C++ terms, the compiler emits code for `find<Object>` — exactly as if you had originally written

    public static Object find(Object[] anArray, Object elem) {
        for (Object e : anArray) {
            if (e.equals(elem)) {
                return e;
            }
        }
        return null;
    }

Then, at each call-site, the compiler "remembers" the original signature of `count` and tries to match up the
parameters in a sensible way. [For example:](https://wandbox.org/permlink/ago5PxMQ9GC8UrOB)

    Shape s = find(someShapes, myCircle);

Assuming that `Circle` inherits from `Shape`, the most-constrained value of `T` that could work for this call is
`T=Shape`. Therefore the compiler emits "un-erasing" code at the call-site, exactly as if you'd originally written

    Shape s = (Shape) find<Object>( (Object[])someShapes, (Object)myCircle );

(`find<Object>` is not real Java code, by the way. Java has no syntax to refer to "a particular specialization"
of a generic method, because generic methods cannot be specialized. In Java, `find<Object>` is the *only* version
of `find`, and so having a special syntax to refer to it would be redundant.)

> UPDATE: Thanks to Reddit commenter [jonathansharman](https://www.reddit.com/r/cpp/comments/b2nef8/what_is_type_erasure/eiuxp81),
> I now know that Java does have a syntactic feature
> called [type witnesses](https://stackoverflow.com/questions/24932177/type-witness-in-java-generics) which
> [allows you to write](https://wandbox.org/permlink/wBpBGfyUcS0umIBV) calls like `Foo.<Object>find(someShapes, myCircle)`
> and `Foo.<Shape>find(someShapes, myCircle)`.
> The type in angle brackets overrides the compiler's normal deduction of `T` — which doesn't really affect anything except for
> the call's return type. You can't "call a different specialization" this way, because there's still only
> one version of `find` being codegenned.

Anyway, my original point was: In the Java world, the process of replacing all the `T`s in a generic method
(or generic class) with `Object`s is referred to as "type erasure." But this process does not happen in the
C++ world, and so it's definitely *not* what I mean when I talk about type erasure in C++!


## By "type erasure," I do not mean classical polymorphism

Typically when we get our wires crossed in conversation about type erasure, it's because the other person is using
"type erasure" to mean plain old classical polymorphism — which I might sometimes call "OOP," with the same colloquial
imprecision that leads me to call the standard library the "STL."

Using classical polymorphism, we can do manually in C++ exactly what Java's type erasure did behind the scenes.
Let's take a simpler example, though, for pedagogical reasons:

    template<class T>
    int run_twice(T callback) {
        return callback(1) + callback(1);
    }

    int y = run_twice([](int x) { return x+1; });
    assert(y == 4);

And here's where we might end up:

    struct AbstractCallback {
        int operator()(int x) const { return call(x); }
    private:
        virtual int call(int) const = 0;
    };
    struct Plus1Callback : AbstractCallback {
        int call(int x) const override { return x+1; }
    };

    int run_twice(const AbstractCallback& callback) {
        return callback(1) + callback(1);
    }

    Plus1Callback cb;
    int y = run_twice(cb);
    assert(y == 4);

Notice that we have to add Java's invisible level of pointer-indirection at the same time that we add our
classically polymorphic object hierarchy. We get the indirection practically for free in this case; we
just have to pass `callback` by reference instead of by value, which is a good idea regardless.
However, it does mean that someone besides us must take responsibility for the callback's lifetime.

We also have to consider everything that the algorithm might want to do with a `AbstractCallback` —
the _affordances_ that an object must have to be useable by our
`run_twice` algorithm — and expose them via instance methods on `AbstractCallback`.
In this carefully selected case, the only affordance needed is `int call(int)`. Our algorithm doesn't
even need to destroy objects of type `AbstractCallback`, so we can get away without a virtual destructor
(although in practice you'd probably add one anyway).

If we were implementing the `find` algorithm via this classical-type-hierarchy route, we'd
need to compare objects for equality, and therefore our base class would need to expose a pure virtual
`equals` method (just like our original Java example).

But all this is not what I mean when I say "type erasure."

To me, this is just classical polymorphism. (I might add "...run amok.")
There is no single recognizable design pattern happening here which I would dignify
by the name "type erasure." In particular, in C++ terms, there is no type being "erased" here!
We simply have a concrete type _hierarchy_, and we're using the static type system. Whatever I pass in
as the `callback` parameter, it must BE-AN `AbstractCallback` in the classical OOP sense. I can't pass
in anything whose concrete type doesn't inherit publicly and unambiguously from `AbstractCallback`.
There is a hard requirement on my `callback`'s static type. In my book that isn't a "type-erased" API!


## I mean ad-hoc dynamic polymorphism

When I talk about "type erasure" in C++, I'm talking about the pattern shown below:

    struct Callback {
        template<class T> Callback(T);
        int operator()(int) const;
    };

    int run_twice(const Callback& callback) {
        return callback(1) + callback(1);
    }

    int y = run_twice([](int x) { return x+1; });
    assert(y == 4);

I haven't shown the implementation of `Callback` yet, but the important thing is that it's
got

- a templated constructor and

- a completely non-virtual interface.

It could even be marked `final`, theoretically. We're never going to inherit from it.
Meanwhile, the definition of `run_twice` looks exactly like our original template definition,
except that it no longer has the `template<class T>` part.

C++ programmers should recognize this pattern as the pattern used by `std::function` in
the standard library. It's also used by C++17's `std::any`, and in a few other obscure places.

So how do you implement the guts of `Callback` at the library level? Well...


## Go calls them "interfaces"

What C++ calls "type erasure" is very similar to
[what the Go language calls "interfaces."](http://jordanorelli.com/post/32665860244/how-to-use-interfaces-in-go)
(Disclaimer: I am not a Go programmer.)
In Go, you'd have something like [this:](https://play.golang.org/p/TIp8reZctOb)

    type Plus1 struct {}

    func (d Plus1) call(x int) int {
        return x + 1
    }

    type Callback interface {
        call(int) int
    }

    func run_twice(callback Callback) int {
        return callback.call(1) + callback.call(1)
    }

    func main() {
        fmt.Println(run_twice(Plus1{}))
    }

Notice that there is no explicit relationship between the `Callback` interface and the concrete class `Plus1`.
The implementor of `Plus1` doesn't even need to know that the `Callback` interface exists! (Well, okay,
they [kind of do need to know](/blog/2018/09/24/concepts-as-door-opening-robots/)
the detailed requirements of the interface, but at least they don't have to be able to
spell its name.)

The Go compiler looks at the call-site where we pass a `Plus1` object to a function expecting a `Callback` interface.
It looks at the interface definition to see what member functions need to be available on a `Callback`. It statically
verifies that those methods are present on `Plus1`. And then it creates a vtable, just like the one inside the
derived class `Plus1Callback`. In fact, we could say that the compiler creates the `Plus1Callback` class on the fly,
without being explicitly asked! It bundles up a pointer to the original `Plus1` object with a pointer to that
custom "`Plus1Callback`" vtable, and passes that whole bundle to `run_twice`. It's as if in C++
[you wrote](https://wandbox.org/permlink/SjUE7MAdigxeG2Yl)

    struct Plus1 {
        int call(int x) const { return x+1; }
    };

    struct AbstractCallback {
        virtual int call(int) const = 0;
    };

    template<class T>
    struct WrappingCallback : AbstractCallback {
        const T *cb_;
        explicit WrappingCallback(const T &cb) : cb_(&cb) {}
        int call(int x) const override { return cb_->call(x); }
    };

    int run_twice(const AbstractCallback& callback) {
        return callback.call(1) + callback.call(1);
    }

    int main() {
        printf("%d\n", run_twice(WrappingCallback<Plus1>(Plus1{})));
    }

This is a zero-overhead, zero-heap-allocation way of wrapping a reference to the `Plus1` object so that it can be
used by `run_twice` exactly as if it were classically derived from `Callback`. However, notice that the `Plus1`
object's lifetime is not *owned* by the `WrappingCallback<Plus1>` object; what we have here is functionally equivalent
to a non-owning [`function_ref`](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0792r3.html).


## Taking ownership

If we want to express _taking ownership_ of the controlled object, we need to move the object into a region of storage
where we can control its lifetime via explicit `delete` or placement-destruction syntax. That is, we need to make a copy of
the original `Plus1` object either on the heap or in a small buffer optimization (SBO) buffer whose storage we control.
The by-far easiest way to do that is to heap-allocate our `WrappingCallback`, [like this:](https://wandbox.org/permlink/BRlwt7rM6ODqDHz2)

    struct Plus1 {
        int call(int x) const { return x+1; }
    };

    struct AbstractCallback {
        virtual int call(int) const = 0;
        virtual ~AbstractCallback() = default;
    };

    template<class T>
    struct WrappingCallback : AbstractCallback {
        T cb_;
        explicit WrappingCallback(T &&cb) : cb_(std::move(cb)) {}
        int call(int x) const override { return cb_(x); }
    };

    struct Callback {
        std::unique_ptr<AbstractCallback> ptr_;

        template<class T>
        Callback(T t) {
            ptr_ = std::make_unique<WrappingCallback<T>>(std::move(t));
        }
        int operator()(int x) const {
            return ptr_->call(x);
        }
    };

    int run_twice(const Callback& callback) {
        return callback(1) + callback(1);
    }

    int main() {
        int y = run_twice([](int x) { return x+1; });
        assert(y == 4);
    }

> The small buffer optimization (SBO) is out of scope for this blog post. I do cover it in
> my "STL From Scratch" training course, which I've given at the past two CppCons and
> may be giving again this coming September. I'm also available for corporate C++ training,
> especially if you're in the New York area!

Notice that the `Callback` type we built here is not copyable, because it contains a `unique_ptr`.
If you want to make a copy of a `Callback`, you must be able to make a copy of the `AbstractCallback`
it points to, which means that `AbstractCallback` must "know" how it behaves when copied, the same way
it currently "knows" how it behaves when called (via its virtual `call` method) and when destroyed
(via its virtual destructor).

When you implement type erasure (in C++ or even in Go), it always starts with making a list of
the things you want to be able to do with your type-erased object — call it, destroy it, copy it,
and so on. Don Norman, in the book [_The Design of Everyday Things_](https://amzn.to/39INVrT) (1988),
calls this a list of _affordances_. A `std::function` _affords_ copying and calling. A `std::any`
_affords_ copying, but not calling. A [`unique_function`](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0228r3.html)
_affords_ calling and moving, but not copying.

> Some of the non-virtual interface might not require talking to `T`. For example, a `function_ref`
> itself does _afford_ copying — I can make a copy of a `function_ref` — but its copy constructor
> just copies a pointer; it doesn't need to copy the underlying `T` and therefore the underlying
> `T` does not need to _afford_ copying. When you make your list, make sure you indicate whether
> each entry is an affordance that `Whatever` provides, or an affordance that it _requires_ of the
> underlying `T`. In general these will be the same thing, but `function_ref` is the
> [exception that proves the rule](https://en.wikipedia.org/wiki/Exception_that_proves_the_rule).

Each affordance in your list turns into a virtual member function of `AbstractWhatever`, which will be
overridden by `WrappingWhatever<T>` appropriately for `T`. Finally, the top-level `Whatever` will
store a `unique_ptr<AbstractWhatever> ptr_` (and/or an SBO buffer), and provide a clean non-virtual
interface implemented completely in terms of calling virtual member functions on `*ptr_`.

In this way you can write a type-erased wrapper for any `Whatever` whatever — callbacks, counters,
output streams, input generators.


## Type erasure usually deals with unary behaviors

Notice that all my examples involve _behaviors_. This should
make sense, intuitively, since _behavior_ is the thing that's hard to boil down into a fixed
representation. _Data_, by definition, is easy to boil down into a fixed representation. There's
no need for us to invent a type-erased `Boolean` type, because we could just use `bool`.

Hypothetically, I suppose I could imagine a use-case for

    struct Number {
        std::unique_ptr<AbstractNumber> ptr_;
        template<class T> Number(T t) : ptr_(std::make_unique<WrappingNumber<T>>(std::move(t)) {}
        Number(const Number& n) { ptr_ = n.ptr_->clone(); }
        Number(Number&& n) = default;
        explicit operator bool() const { return !ptr_->iszero(); }
        Number operator-() const { auto r = *this; r.ptr_->negate(); return r; }
        Number& operator++() { ptr_->inc(); return *this; }
        Number operator++(int) { auto r = *this; ptr_->inc(); return r; }
        Number& operator--() { ptr_->dec(); return *this; }
        Number operator--(int) { auto r = *this; ptr_->dec(); return r; }
    };

However, notice that _almost_ all its operations are phrased as mutative, imperative _behaviors_
("clone", "negate", "increment", "decrement"), and _all_ of its operations are unary. It is
easy to dispatch decisions about unary behaviors to a single `T`; it's very difficult to
[multiply dispatch](https://eli.thegreenplace.net/2016/a-polyglots-guide-to-multiple-dispatch/)
decisions about binary behaviors (such as `operator+=`) to _both_ `T` and `U`, when `WrappingNumber<T>`
doesn't know anything about `U` and `WrappingNumber<U>` doesn't know anything about `T`.


## Conclusion

When I say "type erasure" in C++, I mean more than just a base class with a classical polymorphic interface.
I mean a _non-virtual_ interface which encapsulates and hides the polymorphism from the end-user,
and allows them to use "[duck typing](https://en.wikipedia.org/wiki/Duck_test)"
without bothering to inherit from any library class. (This can allow several libraries
to interoperate seamlessly, even if they don't know about each other's code.)

Naïve type erasure, using heap allocation, is very easy to implement. You start by writing down a list
of _affordances_ that your `T` must have; you turn each affordance into a virtual method on `AbstractWhatever`;
and then you wrap up a pointer to an `AbstractWhatever` inside your type-erased `Whatever` class.

Type erasure does well with unary behaviors such as `++whatever`. It doesn't do well with multiple dispatches
such as `whatever + whatever`.

Type erasure can be useful at ABI boundaries where you can't use templates. (The piece that's templated on `T`
is the constructor, which happens on the caller's side; the object that passes across the ABI boundary
is of the concrete, type-erased type `Whatever`.)

Every "non-beginner" C++ programmer should know how to write a simple type-erased type!
