---
layout: post
title: "ADL can interfere even with uglified names"
date: 2019-09-26 00:01:00 +0000
tags:
  argument-dependent-lookup
  pitfalls
  standard-library-trivia
---

Back in September 2017, on [libc++ review D37538](https://reviews.llvm.org/D37538),
Eric Fiselier showed me the following piece of code.

    struct Incomplete;
    template<class T> struct Holder { T value; };

    void __private_foo(...) {}

    int main() {
        Holder<Incomplete> *p = nullptr;
        ::__private_foo(p); // OK.
        __private_foo(p); // Error: Incomplete is incomplete.
    }

Library writers know that you should never make an unqualified call to a function that
your user might hijack via ADL. For example, if your algorithm invokes `rotate(x, y, z)`
unqualified, you're inviting the user to provide their own customized `rotate` via ADL.

However, the above snippet demonstrates an even worse situation! Here, the name `__private_foo`
is standing in for some STL helper whose name is reserved to the implementation namespace.
It begins with two underscores, so we _know_ that the user cannot
legally provide their own customized `__private_foo`. So can we make an unqualified
call to `__private_foo`?

No, we cannot!

An unqualified call to `__private_foo` definitely
will not find anything via ADL; but the compiler doesn't know that. The compiler must still
go through the motions of building the lists of associated namespaces and associated entities
for the ADL call.
(For more, see ["What is ADL?"](/blog/2019/04/26/what-is-adl/) (2019-04-26).) The argument type
is `Holder<Incomplete>*`, which means that ADL must consider any `__private_foo` functions
which are friends of `Holder<Incomplete>`. The compiler must instantiate `Holder<Incomplete>`
in order to find out whether it has any friends named `__private_foo`.

Instantiating `Holder<Incomplete>` gives a hard compiler error.

To repeat the punch line: When you make a qualified call to `::__private_foo(p)`, it works
and calls the function you expected. When you make an unqualified call to `__private_foo(p)`,
for this particular type, _it gives a hard compiler error_ — despite the fact that the user
never attempted to provide an ADL version of `__private_foo`! Merely invoking ADL at all
can cause hard errors, in situations like this.

The conclusion for many standard library functions is that you must namespace-qualify calls
to helper functions, even if those helpers' names are uglified. Uglifying a name prevents
users from actually ADL-overloading it; but it doesn't prevent hard errors in cases like this.

----

This is the source of at least one family of bugs in libc++, as of this writing.
[Godbolt](https://godbolt.org/z/0wJxpE):

    #include <algorithm>

    struct Incomplete;
    template<class T> struct Holder { T t; };

    int main() {
        using Elt = Holder<Incomplete>*;

        Elt a[100];
        Elt *p = a;
        return std::distance(p, p);
    }

Libraries other than libc++ are happy with this code: `sizeof(Elt)` is definitely known,
so `std::distance(p, p)` should be well-defined. But libc++ pipes `std::distance` through
a helper function `__distance`, whose name is properly uglified but improperly unqualified,
and so we get a hard error:

    <source>:4:37: error: field has incomplete type 'Incomplete'
    template<class T> struct Holder { T t; };
                                        ^
    c++/v1/iterator:632:12: note: in instantiation of template class
    'Holder<Incomplete>' requested here
        return __distance(__first, __last, typename iterator_traits<_InputIter>::iterator_category());
               ^
    <source>:11:10: note: in instantiation of function template
    specialization 'std::__1::distance<Holder<Incomplete> **>'
    requested here
        std::distance(p, p);
             ^

That `__distance` should have said `_VSTD::__distance` — and I'm sure that within a few days
of this post, it will!

----

There's something else noteworthy here. Consider that the definition
of `std::distance(p, p)`, even on libstdc++ or MSVC, must ultimately involve a subtraction of the form

    template<class _It>
    auto __distance(_It __first, _It __last,
                    random_access_iterator_tag) {
        return __last - __first;
    }

Isn't this, also, an unqualified-call scenario? That is, shouldn't we be looking up candidates
for `operator-` in the associated namespaces of type `_It`? Why doesn't this unqualified use
of `operator-` run afoul of the same ADL trap?

Blame [[over.match.oper]/1](http://eel.is/c++draft/over.match.oper#1):

> If no operand of an operator in an expression has a type that is a class or an enumeration,
> the operator is assumed to be a built-in operator and interpreted according to [expr.compound].

That's right — when `_It` is `Holder<Incomplete>**`, the subtraction `__last - __first` is
assumed to be a built-in operator! Built-in operators are not functions. (See also
["Pointer comparisons with `std::less`: a horror story"](/blog/2019/01/20/std-less-nightmare/) (2019-01-20).)
Therefore there is no function call and no name lookup; therefore there is no ADL; therefore
there is no trap!

    SomeType t;
    SomeType *p;
    operator-(t, p);  // ADL
    t - p;  // ADL
    operator-(p, p);  // ADL
    p - p;  // no ADL; the built-in operator is assumed

Again ([Godbolt](https://godbolt.org/z/4XYx7f)):

    namespace N {
        struct A {
            A(A *) {}
        };
        void operator<(A,A);
    }

    int main() {
        N::A *pa = nullptr;
        operator<(pa, pa);  // OK: ADL finds N::operator<
        pa < pa;  // OK: built-in operator, no ADL happens
    }

This is surprising. But it seems that without [[over.match.oper]/1](http://eel.is/c++draft/over.match.oper#1),
`std::distance` would never have worked at all, at least not on any case involving an incomplete associated type.
