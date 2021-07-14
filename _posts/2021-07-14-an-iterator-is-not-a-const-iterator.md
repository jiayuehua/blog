---
layout: post
title: 'An `iterator` IS-NOT-A `const_iterator`'
date: 2021-07-14 00:01:00 +0000
tags:
  antipatterns
  implementation-divergence
  library-design
  stl-classic
excerpt: |
  Consider the following code snippet ([Godbolt](https://godbolt.org/z/GEzWPrcKM)).
  Would you believe that it compiles on MSVC (with Microsoft's STL) but not on
  libc++ or libstdc++?

      template<class C>
      struct Wrap {
          Wrap(C&);
          operator C::iterator() const;
      };

      template<class C>
      void f(C&, typename C::const_iterator);

      int main() {
          std::list<int> v;
          f(v, Wrap(v));
      }
---

Consider the following code snippet ([Godbolt](https://godbolt.org/z/GEzWPrcKM)).
Would you believe that it compiles on MSVC (with Microsoft's STL) but not on
libc++ or libstdc++?

    template<class C>
    struct Wrap {
        Wrap(C&);
        operator C::iterator() const;
    };

    template<class C>
    void f(C&, typename C::const_iterator);

    int main() {
        std::list<int> v;
        f(v, Wrap(v));
    }

I'm using several "slide code tricks" from C++17 and C++20 to shorten the above code.
Notice the [CTAD](/blog/2018/12/09/wctad/) in `Wrap(v)`, and the omitted `typename` in `operator C::iterator`.
Those core-language details are unrelated to the library quirk at issue. (And btw
I wouldn't _intentionally_ do either of them in production code, although they might
sneak in via typos.)

This code creates a `Wrap<std::list<int>>` object, and tries to pass it to `f`.
`f`'s second function parameter does not contribute to deduction, so `C` is
unambiguously deduced as `std::list<int>`. So, we're trying to implicitly convert
`Wrap<std::list<int>>` to `std::list<int>::const_iterator`.

In the STL, it is guaranteed that `C::iterator` is convertible to `C::const_iterator`,
so that functions like C++11's [`list::erase`](https://en.cppreference.com/w/cpp/container/list/erase) (which
takes a `const_iterator`) can be passed a regular `iterator` and still work.
However, the exact mechanism of that implicit conversion aren't documented, as far as I know.
libc++ and libstdc++ happen to implement it via a converting constructor. Microsoft STL,
surprisingly, [implements it](https://github.com/microsoft/STL/blob/b52c379/stl/inc/list#L88) as a _base class conversion_:

    template <class _Mylist>
    class _List_iterator :
        public _List_const_iterator<_Mylist>
    {

This affects which conversions are legal!
In C++, an implicit conversion sequence is allowed to involve _at most one_ user-defined
conversion (such as a constructor or conversion operator). It may be preceded and/or followed
by built-in conversions (such as integral promotions, base-class conversions, etc.) but
not stacked with other user-defined conversions.

    struct C1 {};
    struct B1 { operator C1() const; };
    struct A1 { operator B1() const; };
    C1 c1 = A1();  // Error, stacks two user-defined conversions

    struct C2 {};
    struct B2 : C2 {};
    struct A2 { operator B2() const; };
    C2 c2 = A2();  // OK, only one conversion is user-defined

Meanwhile, the problem with Microsoft's approach
(btw, my understanding is that they inherited this technique from [Dinkumware's](https://www.dinkumware.com/) STL,
so it's not really _Microsoft's_ fault per se) is that it violates the
[Liskov Substitution Principle](https://en.wikipedia.org/wiki/Liskov_substitution_principle).
In general-purpose code, public inheritance should be used
_only_ for <b>IS-A</b> relationships. Here Microsoft's STL is using what's
pejoratively called "implementation inheritance": they coincidentally need similar functionality
in both iterator types, so they make one derive from the other so they can reuse that
code. But this causes problems, because an `iterator` <b>IS-NOT-A</b> `const_iterator`.
How do I know? Well, I find a property that is true of all `const_iterator`s, that is
not true (or should not be true) of `iterator`s.

One such property is "assignability from another `const_iterator`."
[Godbolt](https://godbolt.org/z/KP9KxxzEc):

    const std::vector<int> v = {1,2,3};
    std::vector<int>::iterator b;
    std::vector<int>::const_iterator& rb = b;  // 1
    rb = v.cbegin();  // 2
    *b = 47;  // 3

Only Microsoft/Dinkumware's STL permits line `1`; on libc++ and libstdc++,
the type `const_iterator` is unrelated to `iterator` and so the reference binding
is not allowed. Line `2` exploits the property "assignability from another
`const_iterator`" which is true of all `const_iterator`s and thus (on Microsoft)
true of `iterator`s as well. This allows line `3` to modify the elements of our
const-qualified `vector`.

----

If you're writing your own container or view, with its own `iterator` and `const_iterator`
types, please don't give them an <b>IS-A</b> inheritance relationship! Just write two
separate types; or, write a class template "templated on constness" and instantiate it twice.

See also:

* ["Pitfalls and decision points in implementing `const_iterator`"](/blog/2018/12/01/const-iterator-antipatterns/) (2018-12-01)
