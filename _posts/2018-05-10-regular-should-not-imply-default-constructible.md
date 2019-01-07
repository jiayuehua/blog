---
layout: post
title: 'Default-constructibility is overrated'
date: 2018-05-10 00:02:00 +0000
tags:
  concepts
  constructors
  naming
  rant
  templates
  wg21
---

The Ranges Technical Specification includes very many concept definitions (based on the Concepts TS),
including for example [`Integral`](http://en.cppreference.com/w/cpp/experimental/ranges/concepts/Integral)
and [`Predicate`](http://en.cppreference.com/w/cpp/experimental/ranges/concepts/Predicate).
It also provides a concept named [`Regular`](http://en.cppreference.com/w/cpp/experimental/ranges/concepts/Regular)
which implements a variation on the "Regular" concept described by Alexander Stepanov in his paper
[''Fundamentals of Generic Programming'' (1998)](http://stepanovpapers.com/DeSt98.pdf).

Stepanov writes (pp. 2–3):

> The critical insight which produced generic programming is that highly reusable components must be
> programmed assuming a minimal collection of [...] concepts [...]
> Successful production of a generic component is not simply a matter of identifying the minimal requirements
> of an arbitrary type or algorithm – it requires identifying the common requirements of a broad collection
> of similar components. The final requirement is that we accomplish this without sacrificing performance
> relative to programming with concrete structures. A good generic library becomes a repository of highly
> efficient data structures and algorithms, based on a small number of broadly useful concepts [...]

Under the Stepanov definition, a type is "Regular" if and only if it provides all of the following operations
with appropriate (that is, non-surprising and self-consistent) semantics:

- Default constructor
- Copy constructor
- Destructor
- Assignment
- Equality (`a == b`)
- Inequality (`a != b`)
- Ordering (`a < b`)

Remember that this (pioneering!) definition was written in 1998. Twenty years later, a programmer looking
at this list through "modern C++" glasses should be puzzled by the inclusion of at least two or three of
these primitive operations, and the omission of a couple more. First of all, in C++11 and later, we expect
to see these operations added:

- Move constructor
- Move assignment
- Swap (`swap(a, b)`)

Note that if we have efficient and nonthrowing move construction, move assignment, and destruction,
then we could use the "sensible default" of `std::swap` instead of requiring our type to provide `swap`
itself; but then again, if we have efficient equality, then we could just as well use the "sensible default"
of `!(a == b)` instead of requiring our type to provide `a != b` itself. So *absolute minimalism* in the
choice of primitives is clearly not one of Stepanov's design goals here.

There are two weirder primitives that Stepanov *does* include in "Regular." First:

- Ordering (`a < b`)

This is *so* weird and out of place that Stepanov himself calls it out on page 4:

> The ordering case is interesting. C++ does not define total ordering operations on pointer types [...]
> But this subject is beyond the scope of the current paper.

It is *so* weird, in fact, that the Ranges TS concept `Regular` does not require ordering. This is
the single solitary place — as far as I know — where Ranges' `Regular` deliberately diverges from
Stepanov's "Regular." I think the authors of the TS made the right call here; being totally ordered
is not so intrinsically important or useful that it ought to be part of a `Regular` type.

In particular, I can imagine making a value-semantic class type representing a `playing_card`, where
we can make a very strong and intuitive case for `playing_card(Queen, Spades) != playing_card(Queen, Hearts)`
but we have no intuitive basis for claiming that there is an intrinsic "ordering" between 
`playing_card(Queen, Spades)` and `playing_card(Queen, Hearts)`.


## Stepanov's Regular includes default-constructibility

But I want to talk about the second weird primitive that Stepanov includes in "Regular," which is:

- Default constructor

Stepanov does not include any caveat in his 1998 paper about default-constructibility, even though
it suffers from much the same problem with built-in types:

    int i, j;
    int *p = &i, *q = &j;

    bool b = (p < q);  // This comparison yields undefined behavior: not a good primitive!
                       // Stepanov called this one out explicitly as problematic.

    j = i;  // This assignment yields undefined behavior: not a good primitive!?!?
            // Stepanov doesn't call this problematic

The undefined behavior above isn't the fault of the assignment operator!
It comes from reading the value of the default-initialized (a.k.a. uninitialized) `int` variable `i`.
If we want to eliminate undefined behavior from our code, we should make rules that discourage
default-initialization of `int` variables; and if we want to eliminate undefined behavior from
*generic* code, we should make rules that discourage the default-initialization of `T` variables.


## "Modern" user-defined types should not provide zero-argument construction

Let's look at that `playing_card` example again:

    struct playing_card {
        enum face_kind { Jack=11, Queen=12, King=13 };
        enum suit_kind { Clubs, Diamonds, Hearts, Spades } suit;
        int rank;
        explicit playing_card() = default;
        explicit playing_card(int v, suit_kind s) : suit(s), rank(v) {}
        friend bool operator==(playing_card a, playing_card b) {
            return a.suit == b.suit && a.rank == b.rank;
        }
        friend bool operator!=(playing_card a, playing_card b) {
            return !(a == b);
        }
    };

This seems like a very clean and "Regular" `playing_card` (minus the ordering requirement, which
we ditched). But this `playing_card` has subtle traps laid for the unwary!

    bool guess(const playing_card& card) {
        return card == playing_card(7, Diamonds);
    }

This `guess` function looks nice and safe, right? You give it a card, and it tells you whether
your card is the seven of diamonds. What are some ways we could "break" this function and cause
it to do unexpected things?

Obviously we could pass in something that is not actually a playing card...

    int garbage = 42;
    guess(reinterpret_cast<const playing_card&>(garbage));

But this is clearly violating the "rules" of C++. The language's type-system will prevent us
from doing this by accident. Okay, well, we could pass in something of the right type but
which has bad behavior at runtime...

    playing_card *p = nullptr;
    guess(*p);

This is even more obviously undefined behavior. The language's rules won't save us at compile time,
but we've used C++ long enough to know that this code is not quite kosher, either.

Okay, well, what about *this* code?...

    playing_card pc;
    guess(pc);

This code *also* has undefined behavior! And this time, it's not because the programmer is doing
something sneaky. It's the fault of the `playing_card` class, which is "doing as the ints do"
by providing a default constructor that leaves all its fields uninitialized.

We could eliminate this runtime undefined behavior by giving `playing_card` a "safer" default constructor:

    - explicit playing_card() = default;
    + explicit playing_card() : suit(Clubs), rank(2) {}

Now our default-constructed `pc` will get the value "2 of Clubs", which means that `guess(pc)`
will have well-defined behavior at runtime.

However, I claim that *we should do better than that*. We should *omit* the zero-argument constructor
from this class! The zero-argument constructor gives us something like "a playing card with the default value,"
but that's not a thing that makes sense in our domain. Playing cards always have specific suits and ranks.
There's no such thing as a "default" playing card — we can't pull a card out of thin air! So, just get
rid of that constructor. Then we don't have to worry about *avoiding unwanted behavior at runtime;*
our troublesome code will be rejected by the compiler automatically!

    error: no matching constructor for initialization of 'playing_card'
        playing_card pc;
                     ^


## Zero-argument constructors in the STL

In the standard library, most types do have zero-argument constructors. Even a lot of types you
would naturally expect *not* to be default-constructible:

- `std::regex` — Can't make a regex object without a regular expression, right?
- `std::unique_lock` — Has to be associated with a mutex at construction time, right?
- `std::chrono::seconds` — Has to be an actual number of seconds, right?

Nope! All implicitly constructible from zero arguments!

`std::chrono::seconds` default-constructs to garbage, "as the ints do."

`unique_lock` and `regex` are examples of a common pattern. Both are move-enabled types
where, for efficiency, the move constructor needs to steal the guts of the right-hand operand.
Which means the right-hand operand will be left "gutless". Which means we need
to be able to deal with this "gutless" state; which means we *might as well* create a default constructor
that puts the object into the "gutless" state. I mean we're not *losing* anything by doing so (except perhaps
the chance to catch some bugs).

    class S {
        std::mutex m;
        int value;
    public:
        void increment() { std::unique_lock<std::mutex> lk; ++value; }
        int get_value() const { std::unique_lock<std::mutex> lk; return value; }
    };

    // Did you spot the bug?

On the other side of the fence, some types with *no* zero-argument constructor include:

- `lock_guard`
- `unsynchronized_pool_resource`
- `regex_error`
- `filesystem_error`

I wish I could think of better examples.
`lock_guard` and `unsynchronized_pool_resource` are immobile;
`regex_error` and `filesystem_error` are exception types which means they can't really be used as examples
of "modern C++" style.

However, considering our original `playing_card` example, and our intuition about types like `regex`,
I'd like to stick with my claim that modern C++ value types *generally shouldn't* provide zero-argument constructors.

The STL doesn't ever *need* default-constructibility for anything. We don't even default-construct iterator
or pointer types, which are probably the cases where default-constructibility makes the most intuitive sense.
Nobody writes... well... heck, I can't even come up with a plausible *strawman* here.

    template<class InputIt, class T>
    InputIt find(InputIt first, InputIt last, const T& value)
    {
        InputIt result;  // zero-argument constructor called!
        for ( ; first != last; ++first) {
            if (*first == value) {
                result = first;
                break;
            }
        }
        return (first == last) ? last : result;
    }

No STL algorithm looks like that! It's just ridiculous. We don't default-construct things in generic code.
And we don't default-construct things in *performant* code; notice that the above strawman is basically
using a default-constructed iterator as a very poor man's `optional` (with a redundant test at the bottom
of the function). What we meant was

    template<class InputIt, class T>
    InputIt find(InputIt first, InputIt last, const T& value)
    {
        InputIt result = last;
        for ( ; first != last; ++first) {
            if (*first == value) {
                result = first;
                break;
            }
        }
        return result;
    }

or just, you know,

    template<class InputIt, class T>
    InputIt find(InputIt first, InputIt last, const T& value)
    {
        for ( ; first != last; ++first)
            if (*first == value) return first;
        return last;
    }

Not a default-construction in sight.


## So why did Stepanov include zero-argument-constructibility in "Regular"?

Many thanks to John Lakos for pointing me to
[his February 2015 interview with Alexander Stepanov and Daniel Rose](http://www.informit.com/articles/article.aspx?p=2314360),
where Stepanov answers this exact question:

> The role of a default constructor is that it constructs the object so it can be
> assigned to or destroyed. Nothing more. [EoP](https://amzn.to/2RCzbEU) calls these "partially formed" objects.
> Default construction does not guarantee that the initial value is meaningful, or even the same across
> invocations. Of course, if you write your own default constructor for your own class, you may choose
> to initialize it with a useful value, but this is not part of the requirement.
>
> In particular, I want a programmer to be able to write
>
>     T a;  // value of a is undefined
>     if (test) a = T(1);
>     else a = T(2);
>
> This is a very common case, and I don’t want a programmer to struggle with conditional
> expressions in initialization. Moreover, the idea of partially formed objects is an important one.
> One could, for example, have an array of partially formed objects generated by a default constructor
> if one writes
>
>     T a[5];  // values of a are undefined
>
> So far, I never encountered a case where it caused difficulty, so I do not see a need for a special
> concept where default constructor is not available.

Notice that this text is from the year 2015, but it's describing a distinctly "pre-modern" view of C++.
In this decade, I hope that working C++ programmers do not struggle to write

    T a(test ? 1 : 2);  // value of a is never remotely questionable

The array case is more thought-provoking. We have to use our imagination to come up with a reason
for wanting five `T` objects in a row like that. That is, we need to *exclude* possible solutions such as

    std::vector<T> a;
    for (int i=0; i < 5; ++i)
        a.push_back(some_actual_value());

or

    T a[5] = { 10, 20, 30, 40, 50 };

Well, maybe we're simultaneously kicking off five numbered processes
that each return a `T`, and we need places to store their results as they come in, potentially
out of order.

    T a[5];  // values of a are undefined
    for (int i=0; i < 5; ++i) {
        async_launch(
            some_process, i,
            [](auto result) { a[i] = result; }
        );
    }
    wait_for_all();
    return reduce(a, a+5);

We could of course write

    std::optional<T> a[5];  // all default-constructed to "nullopt"

but that will cost us an extra something-like-40 bytes.
And we could of course write

    alignas(T) char a[5][sizeof(T)];
    for (int i=0; i < 5; ++i) {
        async_launch(
            some_process, i,
            [](auto result) { ::new ((void*)&a[i]) T(result); }
        );
    }
    wait_for_all();
    return reduce((T*)a, (T*)(a+5));

except that that's ridiculous. (Note in passing that we cannot
use `aligned_union_t<1, T>` here; while C++17 guarantees that 
`sizeof(aligned_union_t<1, T>)` is greater than or equal to `sizeof(T)`,
it may not be *exactly* equal.)

My alternative solutions here are not very good. Does this mean that Stepanov
was right — that most types *should* be default-constructible, so as to permit
arrays like this?

I don't think so. I think the more likely conclusion is that *generic code
does not, and should not, create arrays like this.*  By avoiding such operations,
we enable our generic algorithms to work even with types (like `playing_card`
and `regex_error`) which are value-semantic but deliberately *not*
default-constructible.


## One more old-school rationale for default-constructibility

Before I read [Stepanov's interview](http://www.informit.com/articles/article.aspx?p=2314360)
quoted above,
my wild guess as to what he was thinking was: hey, it's 1993! Look at the 1990s design
of iostreams: idiomatic use of `std::cin` *requires* that you construct "uninitialized"
objects all over the place.

    std::string get_user_input() {
        std::string input;  // zero-argument construction!
        std::cin >> input;
        return input;
    }

    int main() {
        std::string name = get_user_input();
        std::cout << "Hello, " << name << "!" << std::endl;
    }

Since it's 1993, we don't get move semantics on that `return`. In fact,
our compiler probably doesn't even implement copy elision, which became widespread
only around 2003 according to my extremely cursory research (namely, [a blog post by
Stan Lippman from 2004-02-03](https://blogs.msdn.microsoft.com/slippman/2004/02/03/the-name-return-value-optimization/)).
So what we really write is:

    void get_user_input(std::string& input) {
        std::cin >> input;
    }

    int main() {
        std::string name;  // zero-argument construction!
        get_user_input(name);
        std::cout << "Hello, " << name << "!" << std::endl;
    }

In this primordial-soup environment, full of uninitialized variables and two-phase initializations,
it might really have seemed "natural" to make sure that every type had a cheap default constructor.

But these days, "modern" days, we don't write code like that anymore. So I think
modern programmers can deal with the idea of types that aren't default-constructible.


## Should `Regular` not imply default constructibility?

Casey Carter's [P0898R1 "Standard Library Concepts"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0898r1.pdf)
proposes the following hierarchy of concepts for C++2a:

    template<class T>
    concept Regular = Semiregular<T> && EqualityComparable<T>;

    template<class T>
    concept Semiregular = Copyable<T> && DefaultConstructible<T>;

    template<class T>
    concept Copyable = CopyConstructible<T> && Movable<T> && Assignable<T&, const T&>;

    template<class T>
    concept Movable = is_object_v<T> && MoveConstructible<T> && Assignable<T&, T> && Swappable<T>;

    // ...

    template <class T, class... Args>
    concept Constructible = Destructible<T> && is_constructible_v<T, Args...>;

This is fine so far (well, did you notice that `Constructible` implies `Destructible`?), but
I am mildly irked that the nicest name — `Regular` — is occupied by the 1990s-era concept
that I consider undesirable,
and that the *most intuitively desirable* concept for building other value-semantic concepts
— that is, `Copyable<T> && EqualityComparable<T>` — doesn't get a special name at all.

In practice this might not end up mattering much. It does occur to me that most STL algorithms
won't constrain on `Regular`; they'll constrain on very free-reined concepts like
`EqualityComparableWith<U::value_type>` and `Predicate<T, U>`. But as the algorithms get
"higher-level", the risk increases of a pesky `DefaultConstructible` sneaking in where it's unwanted.
For example, in [N3351 "A Concept Design for the STL"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3351.pdf) (2012),
it was proposed that `std::sort` ought only to sort `Sortable` objects, where `Sortable`
implied `Semiregular` implied `DefaultConstructible`. However, obviously it *is* possible to sort an array
without ever creating an uninitialized object of the array's element type!  It would be very sad
if C++ evolved to disallow this:

    struct boxed_integer {
        int value;
        constexpr boxed_integer(int v) : value(v) {}
        constexpr auto operator<=>(const boxed_integer&) = default;
    };

    int main() {
        std::vector<boxed_integer> vec = { 3, 1, 4, 2 };
        conceptified::sort(vec);
    }

Fortunately, the `range-v3` library [seems to get this example right](https://godbolt.org/g/2Ny4Aa).
`range-v3`'s `Sortable` concept does *not* require that the element type be `DefaultConstructible`,
or in fact even `Copyable`; `Movable` is enough.


## TLDR

To the extent that `Regular` is intended as "what we expect from a well-behaved value type", it should
*not* imply `DefaultConstructible`. Or, vice versa, if "what we expect from a well-behaved value type"
is essentially `Copyable && EqualityComparable`, it would be really nice to have a short clean name for
that concept. I weakly propose the short clean name "`Regular`"!
