---
layout: post
title: "When can the C++ compiler devirtualize a call?"
date: 2021-02-15 00:01:00 +0000
tags:
  classical-polymorphism
  implementation-divergence
  sufficiently-smart-compiler
---

Someone recently asked me about devirtualization optimizations: when do they happen?
when can we rely on devirtualization? do different compilers do devirtualization
differently? As usual, this led me down an experimental rabbit-hole. The answer
seems to be: Modern compilers devirtualize calls to `final` methods pretty reliably.
But there are many interesting corner cases — including some I haven't thought of,
I'm sure! — and different compilers _do_ catch different subsets of those corner cases.

First, let's observe that devirtualization can (probably?) be done _more effectively_ via
[LTO](/blog/2019/08/02/the-tough-guide-to-cpp-acronyms/#lto),
using whole-program analysis. I don't know anything about the state of the art in
link-time devirtualization, and it's hard to experiment with on Compiler Explorer,
so I'm not going to talk about LTO at all. We're looking purely at what the compiler
itself can do.

There are basically two situations where the compiler knows enough to
devirtualize. They don't have much in common:

## When we know the instance's dynamic type

The archetypical case here is

    void test() {
        Apple o;
        o.f();
    }

It doesn't matter if `Apple::f` is virtual; all virtual dispatch ever does is
invoke the method on the actual dynamic type of the object, and here we _know_
the actual dynamic type is exactly `Apple`. Static and dynamic dispatch should
give us the same result in this case.

A sufficiently smart compiler will use dataflow analysis to optimize non-trivial
cases such as

    Derived d;
    Base *p = &d;
    p->f();

It turns out that even this simple dodge is enough to fool MSVC and ICC.
The next test case is

    Derived da, db;
    Base *p = cond ? &da : &db;
    p->f();

This is too much for Clang, but GCC actually manages to survive it... until
you move the conversions to `Base*` inside the conditional! Here is where
even GCC's analysis fails ([Godbolt](https://godbolt.org/z/GE7vsE)):

    Derived da, db;
    Base *p = cond ? (Base*)&da : (Base*)&db;
    p->f();


## When we know a "proof of leafness" for its static type

Okay, let's suppose that we're receiving a pointer from somewhere else in the
system. We know its static type (e.g. `Derived*`), but we don't know the actual
dynamic type of the object instance to which it points. Still, the compiler can
devirtualize a call to `Derived::f` if it can somehow prove that no type in the
entire program can ever override `Derived::f`.


### Proof-by-`final`

The simplest "proof of leafness" is if you've marked `Derived` as `final`.

    struct Base {
        virtual int f();
    };
    struct Derived final : public Base {
        int f() override { return 2; }
    };
    int test(Derived *p) {
        return p->f();
    }

A pointer of type `Derived*` must point to an object instance that is
"at least `Derived`" — i.e., `Derived` or one of its children.
Since `Derived` is `final`, it isn't allowed to have children; therefore
the dynamic type of the instance must be exactly `Derived`, and the compiler
can devirtualize this call.

Or, you can mark the specific method `Derived::f` as `final`.

The same analysis should apply no matter whether `Derived::f` is declared
in `Derived` itself, or inherited from `Base`. So for example the compiler
should be equally able to devirtualize

    struct Base {
        virtual int f() { return 1; }
    };
    struct Derived final : public Base {};
    int test(Derived *p) {
        return p->f();
    }

GCC, Clang, and MSVC pass this test ([Godbolt](https://godbolt.org/z/MnqoM7), case `one`); ICC 21.1.9 is fooled.

An utterly bizarre proof-of-leafness is to observe that when class `C`'s destructor is
final, `C` must be childless — because if `C` had a child, the child would
have to have a destructor (since you can't make a class without a destructor),
which would then override `C`'s destructor, which isn't allowed.
Clang actually both warns on final destructors, and optimizes on them.
Every other vendor considers this situation [very silly](https://en.wikipedia.org/wiki/The_Colonel_(Monty_Python))
and doesn't dignify it with a codepath as far as I can tell.


### Proof-by-internal-linkage

A class whose name has internal linkage cannot be named outside the current translation unit.
Therefore, it cannot be _derived from_ outside the current translation unit, either!
As long as it has no children in the current TU — or at least no children that override its
methods — calls to its virtual functions are devirtualizable.

    namespace {
        class BaseImpl : public Base {};
    }
    int test(Base *p) {
        return static_cast<BaseImpl*>(p)->f();
    }

If `p` really does point to an object instance that is "at least `BaseImpl`,"
then the compiler can prove that the instance must be exactly `BaseImpl`.
(And if `p` doesn't point to an instance that is "at least `BaseImpl`,"
the program has undefined behavior anyway.)

This strikes me as a case that might actually come up pretty commonly in real codebases.
It's common to have a base class exposed publicly in the header file, and then one
or more derived implementations scoped tightly to a single .cpp file. If you
go the extra mile and put those derived implementations into anonymous namespaces,
you might be helping out the compiler's devirtualization logic. Of course, by definition,
any such benefit will be limited to that single .cpp file!

Another way a type's name can get internal linkage is when it's a class template instantiation
where one of the template parameters involves a name with internal linkage. If the name `T`
has internal linkage, then `E<T>` also has internal linkage, even if `E` itself has external
linkage — because you can't name `E<T>` without naming `T`. (Notice that here `T` must be
a "true name"; we're not talking about type aliases.)

It's also possible to make a type whose name has external linkage, but where the
compiler can prove that the type must be incomplete in every other TU. For example,

    namespace {
        class Internal {};
    }
    class External { Internal m; };

Any other TU is allowed to forward-declare `class External;` as an incomplete type,
but those TUs can't ever complete the type because they can't name the types of its
data members. You can't derive from an incomplete type. So all types derived from `External`
(if any) must be present in this TU; if there are none here, well, that's a proof-of-leafness!
GCC alone detects this situation.


## Table of results

[Godbolt for the known-dynamic-type case](https://godbolt.org/z/GE7vsE);
[Godbolt for the proof-of-leafness case](https://godbolt.org/z/MnqoM7).
In the latter, I've made separate tests for `Derived::f`-defined-directly-in-`Derived`
and `Derived::g`-inherited-from-`Base`. GCC frequently gets `f` right
but fails to devirtualize `g`.
I've filed [GCC bug #99093](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=99093) about that.

|-----------|-------------------------|-----|-------|------|-----|
| Test case | Point                   | GCC | Clang | MSVC | ICC |
|-----------|-------------------------|-----|-------|------|-----|
| `one`     | trivial                 | ✓   | ✓     | ✓    | ✓   |
| `two`     | cast to `Base*`         | ✓   | ✓     |      |     |
| `three`   | conditional, then cast  | ✓   |       |      |     |
| `four`    | cast, then conditional  |     |       |      |     |
|-----------|-------------------------|-----|-------|------|-----|
| `one`     | final class             |  ✓   | ✓     | ✓    | `f`   |
| `two`     | final method            |  ✓   | ✓     | ✓    | ✓   |
| `three`   | silly final destructor  |      | ✓     |      |     |
| `four`    | silly old-school trick  |      |       |      |     |
| `five`    | I.L. class              | `f`  |       |      |     |
| `six`     | I.L. template parameter | `f`  |       |      |     |
| `seven`   | I.L. base               | `f`  |       |      |     |
| `eight`   | I.L. member             | `f`  |       |      |     |
| `nine`    | I.L. with child         | `f`  |       |      |     |
| `ten`     | local class             | `f`  |       |      |     |
|-----------|-------------------------|-----|-------|------|-----|

Steve Dewhurst taught me `four`'s "silly old-school trick": Virtual bases
are always constructed in the context of the most-derived class. So,
if class `C` has a virtual base all of whose constructors are private,
then no child of `C` will ever be able to construct itself, and therefore
no child of `C` can exist. (Of course that virtual base will have to list
`C` among its friends, in order for `C` itself to be constructible.)
I think this trick is foolproof, and thus constitutes a (very silly)
proof-of-leafness for `C`; but of course no compiler cares
to trace through this logical tangle, even if it _is_ foolproof.

----

Can you think of some way to construct a "proof-of-leafness" that I've missed?
Tell me about it!
