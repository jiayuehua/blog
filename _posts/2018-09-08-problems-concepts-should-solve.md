---
layout: post
title: 'The big problems for C++11 SFINAE'
date: 2018-09-08 00:01:00 +0000
tags:
  concepts
  metaprogramming
  rant
---

So I've finally started working on slides for my CppCon 2018 talk "Concepts As She Is Spoke."
(To see the original work to whose title this is an allusion,
[click through](https://publicdomainreview.org/collections/english-as-she-is-spoke-1884/).)
I intended to make it as bland and tutorial in nature as I can, but my natural tendency to find
the hard problems keeps interfering. Let me take the 25 slides I just wrote and turn them into
a blog post, so that I can qualmlessly delete these slides and start over.

Consider the following C++11-era function template.

    template<class T>
    std::string stringify(const T& t) {
        std::ostringstream oss; oss << t;
        return std::move(oss).str();
    }

What *kinds* of arguments can we pass to this function?

"Anything with an `operator<<`," I hear you say. That's a reasonable first approximation.

If the caller tries to do `stringify((const char *)nullptr)`, we'll have trouble at runtime.
That's because the type `(const char *)` satisfies the *syntactic* requirements of this function
(it has an `operator<<`), but some of its values violate the *semantic* requirements of this function
(they cannot be printed).  But we can write off this annoyance to
"[historical deficiencies](https://www.infoq.com/presentations/Null-References-The-Billion-Dollar-Mistake-Tony-Hoare)
of the C++ type system" and plow forward with our first approximation.

In C++11, if we wanted to sanity-check that the user had actually instantiated this
template with an `operator<<`-able type (as opposed to getting some cryptic compiler error
downstream), we'd write up a little type trait:

    template<class T, class = void>
    struct has_output_operator : std::false_type {};

    template<class T> struct has_output_operator<T, decltype(void(
        std::declval<std::ostream&>() << std::declval<T>()
    ))> : std::true_type {};

    template<class T>
    inline constexpr bool has_output_operator_v = has_output_operator<T>::value;

    template<class T>
    std::string stringify(const T& t) {
        static_assert(has_output_operator_v<T>); // HERE
        std::ostringstream oss; oss << t;
        return std::move(oss).str();
    }

The line marked `HERE` solves our problem. The actual trait definition takes
seven lines of boilerplate, which is less cool, but it's the best we could do
in C++11. (And yeah I'm using *several* C++17isms in this code, such as `inline`
and `static_assert`-without-message, but that's not relevant.)

----

In C++11, to *constrain* `stringify(x)` to compile only when `x`
has an `operator<<`, we'd have to write even a little more boilerplate:

    template<class T, class = std::enable_if_t<has_output_operator_v<T>>>
    std::string stringify(const T& t) {
        std::ostringstream oss; oss << t;
        return std::move(oss).str();
    }

As mentioned in the previous post, we can apply a cute terse syntax at the cost
of two more lines of boilerplate:

    template<class T, class = std::enable_if_t<has_output_operator_v<T>>>
    using HasOutputOperator = T;

    template<class T>
    std::string stringify(const HasOutputOperator<T>& t) {
        std::ostringstream oss; oss << t;
        return std::move(oss).str();
    }

But pause. Anyone see a problem with this constrained template? (Either the terse
one or the non-terse one; they have the same problem.)

Right! We're constraining on properties of `T`, but we're actually *using* properties
of `const T&`, and the two sets of properties generally don't have to match!
A.k.a., value categories are why we can't have nice things. (See [my previous post
on the subject](/blog/2018/06/12/attribute-noexcept-verify).)  What we probably
meant to write was:

    template<class T, class = std::enable_if_t<has_output_operator_v<const T&>>>
    std::string stringify(const T& t)

or

    template<class T>
    std::string stringify(HasOutputOperator<const T&> t)

Sidebar: The terse syntax *looks* more philosophically correct in this instance, but notice that
if we were taking `T` by value instead of by any kind of reference, we'd have to go back to
putting the constraint at the end of the `template` clause, rather than inside the parameter
declaration. Example:

    template<class T, class = HasOutputOperator<T&>>
    std::string stringify(T t)

Whether this nitpick about value categories actually matters to you probably depends
on whether you're planning to do your constraints on big OOPy concepts like
`SequenceContainer` and `MessageHolder`, or tiny typesystemy concepts like
`CopyConstructible` and `Callable`. If you're sticking to the big OOPy concepts,
then the only way you'll really run into this issue is by people doing malicious
things like

    struct Evil {};
    std::ostream& operator<< (std::ostream&, Evil&&);

    static_assert(has_output_operator_v<Evil>);
    static_assert(not has_output_operator_v<const Evil&>);

When working with generic code, you have to have a certain sense of laissez-faire;
you can't possibly design a generic algorithm that is *foolproof* against this kind
of mischief. But certainly we want to avoid falling into traps *unnecessarily* or
*accidentally*.


## Step back: Why do we want a trait?

In this example, *why is it important to us* that certain specializations — let's say,
`stringify(std::make_optional(1))` — should not participate in overload resolution?

I can think of two reasons.

1. *Metaprogramming higher up the call stack.* One of our users wants to constrain and
do one thing if `stringify(x)` compiles, and a different thing if `stringify(x)` does
not compile. Therefore we need to make sure that `stringify(x)` will SFINAE.

2. *Implementation detail of crafting an overload set.* We ourselves are planning to
provide several overloads of `stringify(x)`, and we want a different overload to be
selected when `operator<<` is unavailable. Therefore we need to make sure that the other
overload is a *better match* than this one, and one way to accomplish that is to make this
one SFINAE away completely.

Notice that these two reasons are essentially one reason — *someone* wants to dispatch
and do A or B depending on whether C is possible. The difference is just in who the
*someone* is: in (1) it's our caller, in (2) it's ourselves.

So how do we craft an overload set in C++11, a la scenario (2)?
Let's say our "different overload" tries to call `x.stringify()` if it exists, and
otherwise returns the string "unstringable". Naïvely we'd try this:

    template<class T, class = std::enable_if_t<has_output_operator_v<const T&>>>
    std::string stringify(const T& t) {
        std::ostringstream oss; oss << t;
        return std::move(oss).str();
    }

    template<class T, class = std::enable_if_t<has_stringify_method_v<const T&>>>
    std::string stringify(const T& t) {
        return t.stringify();
    }

    template<class T>
    std::string stringify(const T& t) {
        return "unstringable";
    }

which of course leads to ambiguities. So we need to force some kind of *ordering* onto
the alternatives. We'd like to prioritize the `x.stringify()` approach over the `oss << x`
approach, for performance. C++17 to the rescue!

    template<class T>
    std::string stringify(const T& t) {
        if constexpr (has_stringify_method_v<const T&>) {
            return t.stringify();
        } else if constexpr (has_output_operator_v<const T&>) {
            std::ostringstream oss; oss << t;
            return std::move(oss).str();
        } else {
            return "unstringable";
        }
    }

Surprisingly, we didn't run into any arcane pitfalls on this one!

----

But maybe we'd like to combine both (1) and (2), and say that a type with neither
an `operator<<` nor a `.stringify()` method is not stringifiable by any means.

    template<class T, class = std::enable_if_t<
        has_stringify_method_v<const T&> ||
        has_output_operator_v<const T&>
    >>
    std::string stringify(const T& t) {
        if constexpr (has_stringify_method_v<const T&>) {
            return t.stringify();
        } else if constexpr (has_output_operator_v<const T&>) {
            std::ostringstream oss; oss << t;
            return std::move(oss).str();
        } else {
            static_assert(false_v<T>, "unreachable");
        }
    }

Now this is starting to get weird. That `enable_if` condition is quite bulky; should
we provide a better way for our upstream consumers to ask whether `stringify(x)` compiles?

    template<class T, class = void>
    struct is_stringifiable : std::false_type {};

    template<class T> struct is_stringifiable<T, decltype(void(
        stringify(std::declval<T>())
    ))> : std::true_type {};

    template<class T>
    inline constexpr bool is_stringifiable_v =
        is_stringifiable<T>::value;

This is kind of like the STL's `std::is_swappable_v`: it's named after a specific algorithm
and simply tells you whether that algorithm is supported for this type or not.

Or should we save some boilerplate and just write it like this?

    template<class T>
    inline constexpr bool is_stringifiable_v =
        has_stringify_method_v<const T&> || has_output_operator_v<const T&>;

Then we could move it to the top of our header and rewrite `stringify`'s declaration
to look a lot neater...

    template<class T, class = std::enable_if_t<is_stringifiable_v<T>>>
    std::string stringify(const T& t)

But this is crazy, right? "`T` can be stringified if and only if `T` is stringifiable"?
The compiler is happy with it, but are *we* happy with it? (I'm not.)

----

Sidebar: We should take that same uncomfortable feeling and channel it any time
we are shown an example like

    template<class T> requires Sortable<T>
    void sort(T t);

or

    template<class T> requires Swappable<T>
    void swap(T& a, T& b);

These are *not* appropriate uses of concepts.

----

Okay, so, to recap, here are the [Big Problems](https://en.wikipedia.org/wiki/Hilbert%27s_problems),
as I see them, with C++11 type traits and SFINAE.
These are just the places where we ran into *trouble* with our simple `stringify` example,
places where we could really use some help and guidance from the language to make
our programming job easier.

1. We want to assert-on, or constrain-on, properties of our parameters, but sometimes
those properties are semantic — or even run-time dynamic — rather than syntactic.
Recall `stringify((const char*)nullptr)`.

2. Sometimes the properties are purely syntactic, but we are nonetheless tripped up by value categories.
Recall `stringify(Evil{})`.

3. Sometimes we dodge the value-category bullet by defining a trait whose usefulness and
generality are unnecessarily curtailed. Recall `const HasOutputOperator<T>&` versus `HasOutputOperator<const T&>`,
and go look again at our last definition of `is_stringifiable_v`, which sneaks in a `const&` in
the middle of the trait chain for no good reason except that it made for easier reasoning.

4. We are often unsure if our traits belong "above" or "below" their associated functions,
and/or how many public traits we actually need. Recall `is_stringifiable_v<T>` and `is_swappable_v<T>`.

5. The syntax for defining a new type trait in C++11 is bulky.

Now for the good news:

*C++2a Concepts fixes (or ignores) every single one of these problems!*
