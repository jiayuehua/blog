---
layout: post
title: 'Field-testing P2266 "Simpler Implicit Move"'
date: 2021-08-07 00:01:00 +0000
tags:
  implicit-move
  llvm
  proposal
excerpt: |
  A few months ago, Matheus Izvekov implemented my WG21 proposal
  [P2266 "Simpler Implicit Move"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2266r1.html)
  in Clang. As of Clang 13, it's enabled by default under `-std=c++2b`.

  We've received some feedback from the field already (leading up to the Clang 13 release).
  Here are the punch lines as far as I can recall. These examples will all appear in
  P2266R2, whenever I get around to submitting it.

  Many thanks to Matheus Izvekov for the implementation, and to Stephan Bergmann for reporting
  these issues!
---

A few months ago, Matheus Izvekov implemented my WG21 proposal
[P2266 "Simpler Implicit Move"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2266r1.html)
in Clang. As of Clang 13, it's enabled by default under `-std=c++2b`.

We've received some feedback from the field already (leading up to the Clang 13 release).
Here are the punch lines as far as I can recall. These examples will all appear in
P2266R2, whenever I get around to submitting it.

Many thanks to Matheus Izvekov for the implementation, and to Stephan Bergmann for reporting
these issues!


## Microsoft's `std::getline`

<b>Symptom:</b> Code fails to compile.

<b>Fix:</b> `return static_cast<istream&>(x);`

Since C++11, the STL has had two overloads of [`std::getline`](https://en.cppreference.com/w/cpp/string/basic_string/getline).
With some cruft simplified, they are:

    std::istream& getline(std::istream&, std::string&, char);
    std::istream& getline(std::istream&&, std::string&, char);

I suppose the point of the second overload is to allow the programmer to write e.g.

    std::string line;
    std::getline(std::ifstream("MobyDick.txt"), line, '.');
    assert(line == "Call me Ishmael");

The Microsoft STL [until recently](https://github.com/microsoft/STL/pull/2025)
implemented these overloads as follows:

    std::istream& getline(std::istream&& in, std::string& line, char delim) {
        // ...
        return in;  // X
    }

    std::istream& getline(std::istream& in, std::string& line, char delim) {
        return std::getline(std::move(in), line, delim);
    }

That is, they made the rvalue version the primary implementation, and made the
lvalue version simply delegate to the rvalue version! (This code reminds me of
[the old joke](https://jcdverha.home.xs4all.nl/scijokes/6_2.html) about
the mathematician and the room that is not on fire.)

Prior to P2266 (that is, in standard C++20), line `X` works fine, because although
`in` names an implicitly movable entity
([[class.copy.elision]/3](https://timsong-cpp.github.io/cppwp/n4861/class.copy.elision#3.sentence-1)),
there is no copy-initialization context here.
`in` (being a named variable)
is an lvalue, and happily binds to the lvalue `std::istream&` being returned.

After P2266, line `X` fails to compile, because `in` (being the move-eligible
_id-expression_ operand of a `return` statement) is an xvalue, and refuses to bind to an lvalue `std::istream&`.
The simplest fix IMHO would have been to swap the lvalue and rvalue versions:

    std::istream& getline(std::istream& in, std::string& line, char delim) {
        // ...
        return in;  // now OK
    }

    std::istream& getline(std::istream&& in, std::string& line, char delim) {
        return std::getline(in, line, delim);
    }

[The alternative fix actually adopted by Microsoft](https://github.com/microsoft/STL/pull/2025)
is to insert a cast, so that the returned expression is no longer an _id-expression_
at all:

    std::istream& getline(std::istream&& in, std::string& line, char delim) {
        // ...
        return static_cast<std::istream&>(in);
    }

Thanks to the STL team (and specifically
[the other STL](/blog/2019/08/02/the-tough-guide-to-cpp-acronyms/#the-other-meaning-of-stl-in-c-co))
for already landing the fix.


## LibreOffice `OString`

<b>Symptom:</b> Code fails to compile.

<b>Fix:</b> `return OString(x);`

Stephan Bergmann reported [this breakage](https://git.libreoffice.org/core/+/433ab39b2175bdadb4916373cd2dc8e1aabc08a5%5E%21)
in LibreOffice's [`OString` class](https://api.libreoffice.org/docs/cpp/ref/a00125_source.html).
Their `OString` has a very weird (and IMHO very ill-advised)
constructor overload set... but it also exposed a surprising aspect of P2266 I hadn't
thought about before! Essentially, LibreOffice `OString` tries to avoid taking the `strlen` of
string literals:

    char nc[10];
    strcpy(nc, "foobar");
    const char cc[] = "foobar";
    OString nco = nc;        // construct from char[10]
    OString cco = cc;        // construct from const char[7]
    OString slo = "foobar";  // construct from const char[7]
    
LibreOffice wants `cco` and `slo` to use a different constructor from `nco`. The constructor
taking `const char[7]` can assume that the length of the new string being allocated is `6`;
the one taking `char[10]` must do a `strlen` to find out the actual length. (We are willing to
pretend that literals like `"foo\0bar"` don't exist; but we cannot pretend that `strcpy` doesn't
exist.) So the overload set looks basically like this ([Godbolt](https://godbolt.org/z/bdeP458Pb)):

    template<class T>
    concept IsCharPtr = std::same_as<T, char*> || std::same_as<T, const char*>;

    template<class T> constexpr bool IsNonConstCharArray = false;
    template<int N> constexpr bool IsNonConstCharArray<char[N]> = true;
    template<> constexpr bool IsNonConstCharArray<char[]> = true;

    template<class T> constexpr bool IsConstCharArray = false;
    template<int N> constexpr bool IsConstCharArray<const char[N]> = true;
    template<> constexpr bool IsConstCharArray<const char[]> = true;

    class OString {
        std::string s_;
    public:
        template<class T> requires IsCharPtr<T>
        OString(const T& ptr) : s_(ptr, strlen(ptr)) {}

        template<class T> requires IsNonConstCharArray<T>
        OString(T& arr) : s_(arr, strlen(arr)) {}

        template<class T> requires IsConstCharArray<T>
        OString(T& arr) : s_(arr, sizeof(arr) - 1) {}
    };

Here's the problem ([Godbolt](https://godbolt.org/z/Wz8v4zsrs)):

    OString problem()
    {
        char nc[10];
        strcpy(nc, "foobar");
        return nc;  // OK in C++20, ERROR in C++2b
    }

Here `nc` is an implicitly movable entity, and the operand of the
return statement is a simple _id-expression_, so implicit move kicks in.
In C++20, we do two overload resolutions: first treating `nc` as an xvalue
(which finds no viable candidates) and second treating `nc` as an lvalue
(which successfully finds the `IsNonConstCharArray` candidate).

In C++2b with P2266, we see that `nc` is a move-eligible _id-expression_
and treat it simply as an xvalue. P2266's single overload resolution finds
no way of converting an xvalue of type `char (&&)[10]` into an `OString`,
so the code fails to compile.

Before now, I'd never really thought about rvalue arrays. We don't see
array xvalues in everyday life, at least not in C++20. But combining this
C++2b proposal with `OString`'s very weird overload set makes them suddenly
appear and demand our attention.

[The fix adopted by LibreOffice](https://git.libreoffice.org/core/+/433ab39b2175bdadb4916373cd2dc8e1aabc08a5%5E%21)
was simply to insert explicit conversions to `OString` everywhere that the
compiler complained about:

    OString no_more_problem()
    {
        char nc[10];
        strcpy(nc, "foobar");
        return OString(nc);  // OK in C++20 and C++2b
    }

This code has the same behavior in all versions of C++. It's arguably
clearer for the reader, as well.


## LibreOffice `o3tl::temporary()`

<b>Symptom:</b> Code fails to compile.

<b>Fix:</b> `return static_cast<T&>(x);`

The function [`o3tl::temporary`](https://docs.libreoffice.org/o3tl/html/temporary_8hxx_source.html)
is documented to "cast an rvalue to an lvalue" — it's basically the opposite of
`std::move`. It creates a "temporary lvalue" for the purposes of passing to a function
taking out-parameters by lvalue reference and/or pointer, where the caller doesn't
actually care about the results. For example, [`std::modf`](https://en.cppreference.com/w/cpp/numeric/math/modf):

    double fractional_part(double x) {
        return std::modf(x, &o3tl::temporary(0.0));
    }

In C++11 through C++20, the implementation is simplicity itself:

    template<class T> T& temporary(T&& x) { return x; }

Remember, `x` is a variable with a name, so it's an lvalue. C++20's implicit move doesn't kick in
here, because we're returning a reference type. So an lvalue `T&` happily binds to `x` — we can "launder"
our rvalue into an lvalue without any special effort on our part.
In EWG discussion of P2266R1, the effortlessness of "laundering" rvalue into lvalues was generally
regarded as a bad thing. P2266 makes it a bit more effortful. The above code won't compile
after P2266, because `x`, being a move-eligible _id-expression_, will be treated as an xvalue, and
that lvalue `T&` won't bind to it.

[The fix adopted by LibreOffice](https://git.libreoffice.org/core/+/21da7d80aa1ee0f9661dcde37bc4629d5eb9d50e%5E%21)
is simply to insert the explicit cast:

    template<class T> T& temporary(T&& x) { return static_cast<T&>(x); }

Again, this makes the code clearer, and is backward-compatible across all versions of C++.

----

These are all the breakages that have been reported due to the new implicit-move rules
in Clang 13's `-std=c++2b` mode. If you know of other breakages in real-world codebases,
please tell me about them! (For example, send me an email.)

Note that it's easy to _contrive_ examples of code that works in C++20 but fails in `-std=c++2b`
mode (or vice versa). You can even find such examples ready-made inside C++ compiler test suites.
I'm interested only in real live examples from production code.
