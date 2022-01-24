---
layout: post
title: "`const` all the things?"
date: 2022-01-23 00:01:00 +0000
tags:
  antipatterns
  c++-style
  holy-wars
  pitfalls
excerpt: |
  Last week someone posted a /r/cpp thread titled
  ["Declaring all variables local to a function as const"](https://old.reddit.com/r/cpp/comments/s823vk/declaring_all_variables_local_to_a_function_as/):

  > Ok, so I'm an old-school C++ programmer using the language now since the early '90s.
  > I'm a fan of const-correctness for function and member declarations, parameters, and
  > the like. Where, I believe, it actually matters.
  >
  > Now we have a team member who has jumped on the "everything is const" bandwagon.
  > Every code review includes dozens of lines of local function variables now declared
  > `const` that litter the review.
  >
  > Intellectually, I understand the (what I consider mostly insignificant) arguments
  > in favor of this practice, but in nearly 30 years I have never had a bug introduced
  > into my code because a local function variable was mutable when I didn't expect it.
  > It does nothing for me to aid in code analysis or tracking. It has at most a tiny
  > impact on performance.
  >
  > Maybe I'm just an old dog finally unable to learn a new trick. So, on this fine
  > Wednesday, who's up for a religious war? What are y'all doing?

  TLDR: I'm not putting `const` on all the things, either.
---

Last week someone posted a /r/cpp thread titled
["Declaring all variables local to a function as const"](https://old.reddit.com/r/cpp/comments/s823vk/declaring_all_variables_local_to_a_function_as/):

> Ok, so I'm an old-school C++ programmer using the language now since the early '90s.
> I'm a fan of const-correctness for function and member declarations, parameters, and
> the like. Where, I believe, it actually matters.
>
> Now we have a team member who has jumped on the "everything is const" bandwagon.
> Every code review includes dozens of lines of local function variables now declared
> `const` that litter the review.
>
> Intellectually, I understand the (what I consider mostly insignificant) arguments
> in favor of this practice, but in nearly 30 years I have never had a bug introduced
> into my code because a local function variable was mutable when I didn't expect it.
> It does nothing for me to aid in code analysis or tracking. It has at most a tiny
> impact on performance.
>
> Maybe I'm just an old dog finally unable to learn a new trick. So, on this fine
> Wednesday, who's up for a religious war? What are y'all doing?

TLDR: I'm not putting `const` on all the things, either.


## In function signatures: the good

First, let's clarify that it's _important_ to put `const` in the proper place on
pointers and references — er, that is, on pointees and referees.

    auto plus(std::string s, std::string t) {
        return s + t;
    }

The above code is bad because it makes unnecessary copies. `plus` doesn't need its own
copies of `s` and `t`; it can get by with just _references_ to its caller's strings.

    auto plus(std::string& s, std::string& t) {
        return s + t;
    }

The above code is _worse_, because it takes references only to non-const strings. This
signature is saying "Please give me references to two of your strings, and by the way,
_I might modify them._" This (A) is not what we mean, and (B) unnecessarily prevents
the caller from calling `plus("hello", "world")`. What we mean to write is:

    auto plus(const std::string& s, const std::string& t) {
        return s + t;
    }

Pass-by-const-reference is an optimization of pass-by-value: looks the same to the caller,
generates faster code.

Incidentally, the same logic applies even to the hidden `this` parameter.
The below code is bad...

    struct Greeting {
        std::string s;
        auto plus(const std::string& t) {
            return s + t;
        }
    };

...because it says "Please give me a `this` pointer to a `Greeting`, and by the way,
_I might modify it._" This means that if you have a variable like `const Greeting& g`,
you can't call `g.plus("world")`. For more on the "contract law" that forbids things
like that, see ["`const` is a contract"](/blog/2019/01/03/const-is-a-contract/) (2019-01-03).


## In function signatures: the ugly _is_ the bad

In most modern industry codebases, you'll see parameters passed in only three ways
(at least to ordinary functions):

- Pass cheap things by value: `int`.

- Pass expensive things by const reference: `const Widget&`.

- Pass out-parameters by pointer: `Widget*`.

This means that if you see anything else, it's quite probably wrong.
[You can even grep for the wrongness.](/blog/2019/01/03/const-is-a-contract/#grep-your-codebase-today)

> When I say "only three ways," I'm ignoring a few non-ordinary kinds of functions:
> move-constructors which take by rvalue reference, overloaded assignment operators
> which take their left-hand side by non-const lvalue reference, and certain function
> templates which take forwarding references. For more on the bullet points above, see
> ["An obvious guideline on function-call style"](/blog/2020/07/23/grand-unified-theory-of-caller-style/) (2020-07-23).

Consider this function:

    auto plus(const std::string s, const std::string& t) {
        return s + t;
    }

This function is wrong. How do I know? _It passes by const value._
The programmer meant to type `const std::string& s`, but their finger slipped
and they forgot the ampersand. Their code is running slower than it should.
Fortunately, we never "pass by const value" intentionally; so literally everywhere
you see this, you can be sure it's a mistake.

    bool isReady(const std::shared_ptr<Connection> conn);

The function above is also wrong. The programmer probably intended
`std::shared_ptr<const Connection>`; maybe this code used to say
`const Connection *` and someone updated it a little too haphazardly.
Or maybe the programmer meant just `std::shared_ptr<Connection>`;
or _maybe_ they meant `const std::shared_ptr<Connection>&` for efficiency
(but in that case why aren't they simply passing `const Connection&`
from the caller?). In this case it takes some effort to decide what's right;
but we know instantly that what's there _now_ is wrong. Why?
It passes by const value.

    std::string plus(const std::string& s, std::string& t);

The code above is also wrong, because it passes `t` by non-const reference.
If `t` were really an out-parameter, it would be passed by pointer:
`std::string *t`. The most likely explanation is that the programmer
meant to pass by const reference and just forgot the `const`.


## Data members: Never `const`

Lesley Lai has a blog post on this: ["The implication of const or reference
member variables in C++"](https://lesleylai.info/en/const-and-reference-member-variables/) (September 2020).
I generally agree with his comments, and I'll phrase my conclusion stronger:
Const data members are _never_ a good idea.

    struct Employee {
        std::string name_;  // an employee's name may change...
        const std::string id_;  // ...but never their unique ID
    };

The code above is wrong. You might _think_ an employee's ID never changes,
but that's a property of value semantics — what does it mean to be "the same" employee —
and not at all a property of the bits and bytes in `struct Employee`. Consider:

    Employee e1 = {"Alice", "12345"};
    Employee e2 = {"Bob", "24601"};
    e1 = e2; // oops
    std::swap(e1, e2); // oops

What good is a value-semantic `Employee` class if you can't assign or swap
values of that type?

"Ah," you say, "but _my_ `Employee` type isn't value-semantic! It's a polymorphic
class type, with inheritance and stuff!" So you have this:

    class EmployeePerson final : public Person {
    public:
        explicit EmployeePerson(~~~);
        std::string name() const override { return name_; }
        void setName(std::string name) override { name_ = std::move(name); }
        std::string id() const override { return id_; }
        ~EmployeePerson() override = default;
    private:
        std::string name_;  // an employee's name may change...
        const std::string id_;  // ...but never their unique ID.
    };

Maybe you `=delete` some special members, too, to indicate that the class is
non-copyable and non-movable by design. So now (as Lesley says) there's no
specific disadvantage to const-qualifying `id_`. You might even think it was
self-documenting: "this data member is `const`, so it can't change." But
you know a better way to tell that a private data member can't change?
_There's no public API for changing it!_

That's right: the point of making a class with private members is to
preserve invariants among those members. "This never changes" is just one
possible invariant. Some people hear "never changes" and think it sounds
a bit like "`const`," so they slap `const` on that data member; but you
shouldn't lose sight of the fact that the way we preserve invariants in
C++ isn't with `const`, it's with `private`.

    class EmployeePerson final : public Person {
    public:
        explicit EmployeePerson(~~~);

        // An employee's name may change...
        std::string name() const override { return name_; }
        void setName(std::string name) override
            { name_ = std::move(name); }

        // ...but never their unique ID.
        std::string id() const override { return id_; }

    private:
        std::string name_;
        std::string id_;
    };

As with the function-signature rules, once you have a blanket rule, it's
easier to catch mistakes. A const or reference data member, in our
codebase, is _always_ a mistake. Some of these mistakes are actively harmful
(as with our value-semantic `Employee`); some are merely harmless
(as with our polymorphic `EmployeePerson`). Fix them all: then nothing
harmful will slip through the cracks.


### Footnote: Const data members and movability

We've seen that a const data member disables assignment and swap (because
you can't overwrite the value of a const data member on the left-hand side).
It also pessimizes move-construction (because you can't pilfer the guts of
a const data member on the right-hand side).

    struct Employee {
        std::string name_;
        const std::string id_;
    };

    Employee e1 = {"alice", "12345"};
    Employee e2 = std::move(e1);

Very many C++ programmers will look at this and say, "`id_` is const
and can't be moved, so this line will invoke `Employee`'s copy constructor."
That is _incorrect!_ The `Employee` type still gets both a defaulted copy constructor
and a defaulted move constructor, and this line _will_ invoke
`Employee`'s move constructor. That defaulted move constructor
will be equivalent to:

    Employee(Employee&& rhs) :
        name_(std::move(rhs.name_)),
        id_(std::move(rhs.id_)) {}

So when you move-construct an `Employee`, you do move-construct its `name_`.
But you don't move-construct its `id_`, because `std::move(rhs.id_)` will be
of type `const string&&`, which you can't pass to `string`'s
move constructor (because it expects `string&&` with the const discarded),
so _for `id_` only_ you'll call `string`'s copy constructor.

My point is that having a const data member _does_ pessimize move-constructions —
enough to make const data members a bad idea — but _not_ as much as people often think.


## Return types: Never `const`

Just as it's always a mistake to pass "by const value," it's also a mistake
to return "by const value." [Grep your codebase today!](/blog/2019/01/03/const-is-a-contract/#grep-your-codebase-today)

    struct Widget {
        std::string name_;
        const std::string name() const { return name_; }
    };

The member function above is wrong. How do I know?
_It returns by const value._ The programmer probably meant
to return `const std::string&`, but forgot the ampersand.
If that's what happened, then in code like the below,
the programmer is expecting zero copies; but we actually
get one unnecessary copy-construction.

    Widget w = {"carol"};
    std::cout << w.name() << "\n";

But maybe the return-by-value was intended. (Return-by-value
is easier to reason about. You needn't worry that the name
might be mutated — or destroyed — before you've gotten around
to reading it.) Return "by const value" is still bad!
Consider this usage:

    void fill_in(const Widget& w, std::string *result) {
        *result = w.name();
    }

Here we expect one copy: the name should be copy-constructed
(into the return slot) and then move-assigned (into `*result`).
But actually we get two copies! The name is copy-constructed
into the return slot, and then _copy-assigned_ into `*result`,
because we can't pilfer from a right-hand side whose type is
const-qualified.

Returning "by const value" is always wrong. Full stop.

> Back before move semantics existed, Scott Meyers'
> _Effective C++ Third Edition_ (Item 3) actually suggested
> that "const by-value" returns were a good thing; but
> [Scott retracted that suggestion](https://www.aristeia.com/BookErrata/ec++3e-errata.html)
> in 2009 when it became clear that C++11 would have move semantics.
> If anyone tries to justify return-by-const-value in this
> decade by saying "Scott Meyers says to do it," tell them
> they're wrong!


## Local variables: Rarely const

I wouldn't go so far as to say I _never_ mark local variables
`const`; but as a general rule, I don't.
As in the polymorphic-data-member case above, I don't need a
keyword just to tell me that a variable isn't modified during
its lifetime. I can see that fact at a glance, because:

- The variable is local to the current scope, which is small enough
  to fit comfortably on one screen. (Keep scopes small. Avoid
  global variables.)

- The variable is well-named, not confusable at a glance with any
  of the other variables used in this scope.
  ([Name length is proportional to scope length.](https://moderatemisbehaviour.github.io/clean-code-smells-and-heuristics/names/n5-use-long-names-for-long-scopes.html))

- It's obvious which of its uses (if any) are potentially mutating.
  (Pass out-parameters by pointer. `f(x, &y)` modifies `y`
  but does not modify `x`.)

- Above all, _the code makes sense._ I can guess that a variable
  initialized with `std::format("hello {}!", first)` probably never
  changes; a variable initialized with `0` must change (because
  otherwise there'd be no reason to declare that variable).
  The code doesn't violate my expectations often enough for me
  to _desire_ constant reassurance. (Pun intended, I guess.)

For an actual compile-time constant, I'd certainly consider marking
it `constexpr`:

    constexpr size_t chunkStart = sizeof(BlockHeader) + sizeof(ChunkHeader);

And (speaking of block headers and chunk headers) I'd say it could
be quite useful to const-qualify the occasional local variable in
code that egregiously violates the clean-code guidelines above. If you
can't see the whole scope at a glance, or you have lots of variables
with confusingly similar names, or you can't tell whether the variable's
initial value makes sense or not... well, `const` might give you a
welcome clue. But it might be even better to refactor that confusing code!
Shrink your scopes; name your variables well; declare variables only
at the place where you're ready to initialize them. Soon you'll
find each redundant keyword more annoying than helpful.

Meanwhile, marking every local variable as `const` can have all the
same downsides as returning by const value:

    std::string identity(const std::string hi)
    {
        return hi;
    }

Here [implicit move](https://stackoverflow.com/questions/9779079/why-does-c11-have-implicit-moves-for-value-parameters-but-not-for-rvalue-para)
means that the return statement treats `hi` as an rvalue;
but because it was const-qualified, that rvalue is of type `const std::string&&`,
and can't be pilfered from. A copy is made.

And, as with the pass-by-const-value situation, it's sometimes unclear
whether a const-qualified variable was intentional or not. In the below code,
should that be `shared_ptr<const Widget>`? Should that be `Vec::const_iterator`?
(Fortunately, the `auto` keyword has made this kind of situation much less common.)

    const std::shared_ptr<Widget> sp = ~~~;
    const Vec::iterator it = v.begin();

Then there's this monstrosity:

    void setFullName(const NameParts& np, Employee *e)
    {
        const std::string firstName = np.firstName();
        const std::string lastName = np.lastName();
        const std::string fullName = firstName + lastName;
        e->setName(fullName);
    }

The above function suffers from the "too many intermediate variables" antipattern.
Really, we could fix all its performance woes by writing simply

    void setFullName(const NameParts& np, Employee *e)
    {
        e->setName(np.firstName() + np.lastName());
    }

But suppose there were many more lines of code interspersed between
the variable definitions and the final call to `e->setName(fullName)`.
That call makes an unnecessary copy. The programmer might try to fix it
by writing `e->setName(std::move(fullName))`... but guess what?
`fullName` was const-qualified, so the `move` does nothing!

To get the benefits of move semantics, you _must_ un-const-qualify the
variable you intend to move from. And one benefit of move semantics is
that it often happens relatively invisibly, without the programmer's being
consciously aware of it! So, to get the _full_ benefits of move semantics,
const-qualify... _nothing!_ Except of course for the things you must
qualify for const-correctness: namely, the targets of pointers and references
that might refer to objects of your caller's that you aren't allowed to
modify.


### Footnote: Const locals and NRVO

Since const-qualification inhibits "implicit move," you might wonder whether
it also inhibits copy elision, a.k.a.
[NRVO](/blog/2019/08/02/the-tough-guide-to-cpp-acronyms/#rvo-nrvo-urvo).
[It does not.](https://stackoverflow.com/questions/56194688/can-a-c-compiler-perform-rvo-for-a-named-const-variable-used-for-the-return-va)

    std::string f() {
        const std::string result = "hello";
        return result;  // NRVO is possible
    }

With respect to NRVO, this `const` is harmless.
But still, this `const` cannot _increase_ your performance! All it can do
is lurk, eager to _decrease_ performance if you get careless.

The below code ([Godbolt](https://godbolt.org/z/x6Ex5Tez5)) is identical
to the above code, except I've added a single early return. NRVO is still technically possible,
but neither Clang nor GCC (as of this writing) perform it. So they fall back
on implicit move; but implicit move can't pilfer, because `result` is const-qualified.
`result` is copy-constructed into the return slot.

    std::string g(bool early) {
        if (early)
            return "";
        const std::string result = "hello";
        return result; // oops
    }


## Conclusion: Don't `const` all the things

- Definitely don't const-qualify function return types. Return-by-const-value
  has only downsides, no upsides.

- Definitely don't const-qualify the data members of value-semantic class types.

- Don't const-qualify the data members of polymorphic (or otherwise immobile) class types.
  Having one simple rule for all data members saves brain cells. Use `private`
  to enforce class invariants.

- Don't const-qualify function parameters — except by accident, when
  you mean to write `const X& x` but forget the ampersand. Pass-by-const-value
  indicates typos. Fix your typos.

- Rarely, if ever, should you const-qualify local variables.
  Prefer to refactor so that each variable's purpose is clear and each scope
  fits on a single screen.

But of course:

- _Do_ write const-correct code. For example, const-qualify member functions
  when you intend them to be callable even on an object you can't modify.

- _Do_ pass expensive types by const reference.

`const` is one of the most important keywords in C++. It has a place.
But that place is not "everywhere."
