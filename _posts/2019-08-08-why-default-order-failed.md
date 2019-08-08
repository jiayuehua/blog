---
layout: post
title: "Mangling dependent parameter types, or, what happened to `std::default_order`"
date: 2019-08-08 00:01:00 +0000
tags:
  metaprogramming
  operator-spaceship
  pitfalls
  standard-library-trivia
  templates
  wg21-folkloristics
excerpt: |
  Consider this icky template code:

      template<class T> int f(int) { return 1; }
      template int f<int>(int);

      template<class T> int f(T) { return 2; }
      template int f(int);

      int main() {
          return f(0) + f<int>(0);
      }

  According to GCC, Clang, ICC, and the paper standard, this program is valid C++,
  and it is supposed to return `3`.
---

Consider [this ridiculously icky template code](https://godbolt.org/z/1Iom5n):

    template<class T> int f(int) { return 1; }  // A
    template int f<int>(int);  // B

    template<class T> int f(T) { return 2; }  // C
    template int f(int);  // D

    int main() {
        return f(0) + f<int>(0);
    }

According to GCC, Clang, ICC, and the paper standard, this program is valid C++,
and it is supposed to return `3`. (According to MSVC, this program is awful
and shouldn't even link.)

> First of all, I should mention that I gave a two-part talk at CppCon 2016
> titled "[Template Normal Programming](https://www.youtube.com/watch?v=vwrXHznaYLA)."
> Much of the stuff we're about to dive into is covered in that talk.
> I highly recommend watching it... if I do say so myself.

Here we have an overload set consisting of two function templates,
both named `f`. There's `template<class T> int f(int)` (on line A)
and there's `template<class T> int f(T)` (on line C).

Each template has one [_explicit instantiation definition_](https://www.youtube.com/watch?v=VIz6xBvwYd8&t=4m51s) for `T=int`.
The first instantiation definition (on line B) says, "I'm an instantiation of `f`
with the first template parameter set to `int`." There's only one `f` visible
at that point, so this is unambiguously an instantiation of template A.

The second instantiation definition (on line D) says "I'm an instantiation
of `f`," but it doesn't say *which* of our two `f` templates it's an instantiation
of, and also it doesn't provide any template arguments. This is a little bit
sneaky! The compiler has to do template argument deduction to figure out what
the template arguments to `f` should be. First it looks at template A; but template
A's parameter `T` is not deducible, so the compiler discards that possibility.
Then it considers template C. Template C's parameter `T` is deducible from
the first function parameter: we're saying that the first parameter has type
`int`, the primary template says that it has type `T`, therefore `T=int`
and we've got ourselves a candidate! Since this is the *only* candidate
(remember, template A was thrown out), the declaration is unambiguous:
line D is an instantiation of template C.

So now we have explicit instantiations of two functions, both named `f`,
both with a single template argument `T=int`, both taking a single
function parameter of type `int`. Crazy!

In `main`, we call `f(0)`. The compiler does overload resolution and
template argument deduction. This can't be a call to template A, because
template A's parameter `T` is not deducible. So it must be a call to
template C — that is, to the specialization instantiated on line D.

Then `main` calls `f<int>(0)`. Again the compiler does overload resolution.
This might be a call to template A or to template C. But template A is
[more specialized](http://eel.is/c++draft/over.match#best-2.5) than template C,
and so it is a better match. So this is resolved into a call to template A
— that is, to the specialization instantiated on line B.

So `main` returns 2 + 1, which is 3. Q.E.D.!

----

So why can't MSVC compile this code? Well, it's because of what I said
three paragraphs ago: we have here two functions, both named `f`, both
with a single template argument `T=int`, both taking a single function
parameter of type `int`. So MSVC quite reasonably produces the same
name-mangled symbol for both of these functions. Specifically, that
mangling is

    ??$f@H@@YAHH@Z

(In MSVC mangling, `H` means `int`; so `f@H` means `f<int>`.
`@@YAH` means "function returning `int`. And the last `H` is the
function parameter of type `int`.
I don't know who invented the MSVC mangling scheme, but I'm
going to guess [Jon Arbuckle](https://www.gocomics.com/garfield/1999/05/09).)

So we end up with two definitions for that symbol — one returning 1
and the other returning 2 — in the same assembly file! We get an
error from the assembler, or perhaps from the linker.

---

MSVC's logic seemed very reasonable. So why is it that GCC and Clang *can*
compile this sneaky code? Well, they both use the Itanium mangling scheme,
and Itanium says that we should mangle dependent function parameter types
as they appear in the source code. In Itanium's scheme,
the specialization on line B mangles as

    _Z1fIiEii

Here `fIiE` means `f<int>`, the next `i` means "returning `int`," and
the last `i` is the function parameter of type `int`.
But the specialization on line D mangles differently!

    _Z1fIiEiT_

Here the function parameter type is not mangled as `i`; it's mangled as
`T_`, which means "the first template parameter type." In this particular
specialization, the first template parameter type happens to be `int`;
but we don't mangle it as `i`, we mangle it as `T_`. This rule is crafted
*precisely* to permit valid-but-sneaky C++ code such as this example.

----

One unfortunate side effect of mangling dependent types is that we suddenly
have to invent a mangling for all possible ways a type can depend on a template
parameter. For example, Itanium will look at

    template<class T> void f(int (*)[sizeof(T() * 1 + 2 ^ 3)]) {}
    template void f<int>(int (*)[4]);

and produce the mangling

    _Z1fIiEvPAszeoplmlcvT__ELi1ELi2ELi3E_i

That is, the function parameter has type `P`ointer to `A`rray of size
`s`i`z`eof `e`xclusive-`o`r of (`pl`us of (`m`u`l`tiply of (`c`onstruct-from-`v`oid
of `T`) and the integer `1`) and the integer `2`) and the integer `3`).

Or again, Itanium distinguishes between these two templates whereas
MSVC does not:

    template<int> struct A {};

    template<int I> A<I+0> f() { return {}; }
    template A<2> f<2>();  // _Z1fILi2EE1AIXplT_Li0EEEv

    template<int I> A<I> f() { return {}; }
    template A<2> f();  // _Z1fILi2EE1AIXT_EEv

----

Another unfortunate side effect is that when a function parameter's type
is itself the result of a template instantiation, we can't just encode
the final type; we have to encode *how we got there.*  For example
([Godbolt](https://godbolt.org/z/a1fLV_)):

    template<class> struct A { using type = int; };
    template<class> using B = int;

    template<class T> T f(typename A<T>::type) { return 1; }
    template int f(int);

    template<class T> T f(B<T>) { return 2; }
    template int f(int);

Itanium mangles the first `f` as `_Z1fIiET_N1AIS0_E4typeE` and the second
`f` as `_Z1fIiET_i`. Alias templates are "seen through"
by the template machinery: `B<T>` is really just an alias for `int`
in all respects, and therefore it is not considered to be template-dependent
and can be mangled as simply `i`.
On the other hand, `typename A<T>::type` depends on `T` and must be
mangled as `NAIS0_E4typeE`.  (MSVC mangles these two functions identically.)

----

This feature of the Itanium ABI is what sank Alisdair Meredith's
[P0181 "Ordered By Default"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0181r1.html)
(February 2016), a.k.a. `std::default_order`.
He proposed essentially that the default template argument for `std::set`'s comparator
should be changed from `std::less<T>` to "`std::less<T>` if it exists; otherwise `std::default_strong_comparator<T>`,
which can be customized for types that want to be storeable-in-sets without providing `operator<`."
In modern terms:

    template<class T>
    using void_if_lessable = decltype(
        (declval<T>() < declval<T>()), void()
    );

    template<class T, class = void>
    struct default_strong_comparator {
        struct type {
            bool operator()(const T& a, const T& b) const {
                return std::strong_order(a, b) < 0;
            }
        };
    };

    template<class T>
    struct default_strong_comparator<T, void_if_lessable<T>> {
        using type = std::less<T>;
    };

    template<class T,
             class C = typename default_set_comparator<T>::type,
             class A = std::allocator<T>>
    class set {
        // ...
    };

At first glance, this seems like a ABI-non-breaking, backward-compatible change.
`set<int>` continues to mean `set<int, std::less<int>, std::allocator<int>>`
just as it did before (because `default_set_comparator<int>::type` is a type alias
for `std::less<int>`). We check the mangling of

    void f(set<int>);

and see that indeed it mangles to the same thing as before ([Godbolt](https://godbolt.org/z/57nLKo)).

...But then the mangling of template-dependent function parameter types comes
and bites us!

    template<class T> int f(set<int, typename default_strong_comparator<T>::type>) { return 1; }  // A
    template int f<int>(set<int>);  // B

    template<class T> int f(set<int, std::less<T>>) { return 2; }  // C
    template int f(set<int>);  // D

Here are two distinct templates `f`. Template C is more specialized than template A.
Instantiations B and D have different Itanium manglings:

    _Z1fIiEi3setIT_N25default_strong_comparatorIS1_vE4typeESaIS1_EE  // B
    _Z1fIiEi3setIT_St4lessIS1_ESaIS1_EE  // D

Now consider this function template `g` ([Godbolt](https://godbolt.org/z/IhCFWl)):

    template<class T> int g(set<T>) { return 3; }  // E
    template int g(set<int>);  // F

If `set<T>` means `set<T, std::less<T>>`, then the specialization on line F
should be mangled as `_Z1gIiEi3setIT_St4lessIS1_ESaIS1_EE`. But if `set<T>`
means `set<T, default_strong_comparator<T>::type>`, then the specialization
on line F should be mangled as `_Z1gIiEi3setIT_N25default_strong_comparatorIS1_vE4typeESaIS1_EE`.

Thus, changing the default template argument of `std::set` can actually cause the
name-mangling of a function to change! And `g` is a very plausible function template
to write, too; this change wouldn't go unnoticed in real codebases.

----

This subtle mangling issue sank P0181 `default_order`.

National Body Comment "FI 18" on the C++17 CD ([N4664](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4664.pdf), March 2017)
read in its entirety:

> Comments:
> It was thought that using default_order as the
> default comparison for maps and sets was not
> abi-breaking but this is apparently not the case.
>
> Proposed change:
> Revert the change to the default comparison of maps and sets.
>
> Observations of the secretariat:
> Accepted

This comment was turned verbatim into [LWG issue 2863](https://cplusplus.github.io/LWG/issue2863)
(opened February 2017, closed March 2017). I don't think a code snippet
specifically demonstrating the problem was ever published in any public venue.

So I wrote this blog post.
