---
layout: post
title: "Concept definition-checking and its woes"
date: 2019-07-22 00:02:00 +0000
tags:
  concepts
---

C++2a concepts (formerly known as ["Concepts Lite"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3701.pdf)
and/or the [Concepts TS](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4674.pdf))
famously do not support "definition checking."
The idea of definition checking is that the programmer might write

    template<class T,
        class ValueType = typename iterator_traits<T>::value_type>
    concept InputIterator =
        Copyable<T> &&
        requires(T t) {
            { ++t } -> Same<T&>;
            { *t } -> ConvertibleTo<ValueType>;
            { t == t } -> ConvertibleTo<bool>;
        };

    template<class T>
    concept RandomAccessIterator =
        InputIterator<T> &&
        requires(T t, int n) {
            { t + n } -> ConvertibleTo<T>;
        };

and then an algorithm such as

    template<class It, class T> requires InputIterator<It>
    It find_n(It first, int n, const T& value)
    {
        It last = first + n;
        for (It it = first; it != last; ++it) {
            if (*it == value) {
                return it;
            }
        }
        return last;
    }

Now, in C++2a–concepts-land, where templates are still effectively duck-typed,
we can invoke this template as follows, and it's all totally fine with the compiler.
([Godbolt.](https://concepts.godbolt.org/z/Icwf1P))

    int main() {
        int a[10] = {3,1,4,1,5,9,2,6,5,3};
        int *p = find_n(a, 10, 6);
        assert(p == a+7);
    }

By contrast, in a world with "definition checking," the compiler would have complained
as soon as we defined function template `find_n`, even before we instantiated it at all!

    error: expression involving constrained parameters,
    '<InputIterator> + <int>', is not part of the concept
    requirements of 'InputIterator'
        It last = first + n;
                  ^~~~~~~~~

The compiler is telling us that we made a mistake here: we advertised that `find_n`
can accept any `InputIterator`, but in fact it uses operations such as `+` that are
not supported by some `InputIterator`s (such as `istream_iterator<int>`).
Maybe we meant that `find_n` should accept only `RandomAccessIterator`s;
or maybe we should refactor `find_n` so that it _can_ work with arbitrary `InputIterator`s.
Either way, definition-checking has alerted us to the bug.

This is why definition-checking sounds like a cool and useful thing.
Now let's see why we can't have it in C++.


## An oft-quoted objection is no problem at all

I frequently see people opining that what makes definition-checking hard is that it
doesn't allow for temporary ad-hoc code. The canonical example is "printf debugging."
Consider:

    template<class It, class T> requires InputIterator<It>
    It find_n(It it, int n, const T& value)
    {
        for (int i = 0; i != n; void(++i), ++it) {
    #ifdef DEBUG_ME
            save_to_log(it);
    #endif
            if (*it == value) {
                return it;
            }
        }
        return last;
    }

People say, "A hypothetical definition-checking compiler should be happy with this code...
but, if you pass `-DDEBUG_ME`, it'll complain."

    error: expression involving constrained parameters,
    'save_to_log(<InputIterator>)', is not part of the
    concept requirements of 'InputIterator'
            save_to_log(it);
            ^~~~~~~~~~~~~~~

However, this is a bad example, because it is trivially easy to tell the compiler that
`save_to_log(it)` is well-formed in this context. C++17 gives us `if constexpr` to express
local constraints.

    template<class T>
    concept Loggable = requires(T t) {
        { save_to_log(t) };
    };

    template<class It, class T> requires InputIterator<It>
    It find_n(It it, int n, const T& value)
    {
        for (int i = 0; i != n; void(++i), ++it) {
    #ifdef DEBUG_ME
            if constexpr (Loggable<T>) {
                save_to_log(it);
            } else {
                save_to_log(__PRETTY_FUNCTION__, " with non-loggable iterator type");
            }
    #endif
            if (*it == value) {
                return it;
            }
        }
        return last;
    }

A hypothetical definition-checking compiler could certainly track the constraints involving
`T` that apply in any particular lexical scope, including those implicitly applied by
`if constexpr` and `static_assert` expressions that just happened to be of the form
[_constraint-expression_](http://eel.is/c++draft/temp.constr.decl#nt:constraint-expression).

So the oft-quoted "printf debugging" hurdle is not a hurdle at all.


## A less-quoted objection remains valid

Consider again the requirement on our concept `InputIterator`:

            { *t } -> ConvertibleTo<ValueType>;

And look at our template `find_n`:

    template<class It, class T> requires InputIterator<It>
    It find_n(It it, int n, const T& value)
    {
        for (int i = 0; i != n; void(++i), ++it) {
            if (*it == value) {
                return it;
            }
        }
        return last;
    }

Shouldn't our hypothetical definition-checking compiler still complain about this line?

    error: expression involving constrained parameters,
    '<ConvertibleTo<int>> == <int>', is not part of the
    concept requirements of 'ConvertibleTo'
            if (*it == value) {
                ^~~~~~~~~~~~

Our concept definition for `InputIterator` says that `*it` must be convertible to
`iterator_traits<It>::value_type`, but we never said anywhere that `iterator_traits<It>::value_type`
must be equality-comparable — and certainly never said that it must be equality-comparable
to the _unconverted_ type of `*it`, whatever that is. This is a problem for pathological types,
but also in practice for proxy iterators such as `vector<bool>::iterator`.
(See ["C++ idiom of the day: `arrow_proxy`"](/blog/2019/02/06/arrow-proxy/) (February 2019).)


## Compilers don't possess common sense

How about this line?

    error: expression involving constrained parameters,
    '<void> , <InputIterator>', is not part of the concept
    requirements of 'InputIterator'
        for (int i = 0; i != n; void(++i), ++it) {
                                ^~~~~~~~~~~~~~~

We humans know that `void() , x` is a well-formed expression for any `x`; but should the compiler
be expected to know that? How much "common sense" can the programmer expect out of their
hypothetical definition-checking compiler?

Finally, consider

    template<class T>
    concept Addable = requires(T t) {
        { t + 1 };
    };

    template<class T> requires Addable<T>
    void test(T x) {
        x + 2;  // OK??
    }

Should the compiler be expected to know that if `{ x + 1 }` is well-formed,
then `x + 2` is automatically also well-formed?
(And if so, watch out: [`x + 0` can still be ill-formed!](https://godbolt.org/z/De3YDO))

These problems, and problems like them, are the reason definition checking hasn't yet — and probably never will — come to C++.
