---
layout: post
title: "Lifetime extension applies to whole objects"
date: 2020-11-16 00:01:00 +0000
tags:
  implementation-divergence
  lifetime-extension
  parameter-only-types
---

Nicolai Josuttis just taught me something I didn't know about lifetime extension!
My mental model of lifetime extension in C++ had always been, basically,
that the compiler was secretly turning a reference variable into a by-value variable.
This mental model works well for the simple cases:

    const std::string& r = "hello world";

behaves basically the same as

    const std::string s = "hello world";

in all respects, except that `decltype(r)` remains a reference type, and
`r` isn't a candidate for copy elision. If you look on the stack frame,
you'll find that the compiler allocates `sizeof(std::string)` bytes for
the lifetime-extended temporary associated with `r`, just as it does for
an actual by-value variable like `s`.

In particular, this means that the following code is perfectly valid C++ with
no dangling references:

    int max1(int x, int y) { return x < y ? y : x; }
    int main() {
        const int& r = max1(1, 2);  // basically 'int r ='
        return r;  // returns 2
    }

even though this very similar code has UB ([Godbolt](https://godbolt.org/z/j4hj1E)):

    const int& max2(const int& x, const int& y) { return x < y ? y : x; }
    int main() {
        const int& r = max2(1, 2);
        return r;  // UB: dangling reference to temporary '2'
    }

But!

My mental model turns out to be incorrect in (at least) one interesting way.
If the reference `r` is being bound directly from a _subobject_ of a temporary,
then the whole temporary is extended — not just the subobject! I had already
intuitively grasped this idea as it applies to base-class subobjects:

    // Slicing occurs
    const Base b = Derived();

    // No slicing occurs
    const Base& r = Derived();

In the by-value case, we construct a `Derived` object, then use
`Base(Base&&)` to copy _only the `Base` parts of it_ into the stack variable `b`. The
`Derived`-ness of the object is "sliced away" and forgotten.

In the by-reference case, we get a `const Base&` reference that refers to a `Derived`
object. The entire temporary object, of type `Derived`, is lifetime-extended.

Now for the subtle bit. Watch what happens if we bind the reference `r` directly from a _member_ subobject
of a temporary. This snippet prints "lifetime-extension," then "destroyed the temporary S,"
in that order!

    struct S {
        int m;
        int& getM() { return m; }
        ~S() { puts("destroyed the temporary S"); }
    };

    int main() {
        const int& r = S().m;
        puts("lifetime-extension");
    }

Binding a reference to a member subobject can extend the lifetime of the entire object,
in the same way that binding a reference to a base-class subobject can extend the lifetime
of the entire object. But notice that the binding must be _directly_ to the member `m`;
binding to the result of `S().getM()` will not trigger lifetime extension, not even if
`getM()` is inlined.

> The paper standard [is clear](https://eel.is/c++draft/class.temporary#6.3) that even array-indexing expressions,
> such as `S().a[0]`, should count as subobject references. But be warned that major implementations
> diverge from the standard as soon as you get away from the simplest stuff. In particular,
> neither Intel ICC (EDG) nor MSVC implement the paper standard's rule about member subobjects
> ([Godbolt](https://godbolt.org/z/ozxYG6)), although they both seem to handle base subobjects correctly.

Here's a clever test case ([Godbolt](https://godbolt.org/z/eraGYT)):

    struct Example {
        char data[6] = "hello";
        std::string_view sv = data;
        ~Example() { strcpy(data, "bye"); }
    };

    int main() {
        auto&& sv = Example().sv;
        std::cout << sv << '\n';
    }

This program has well-defined behavior and (on Clang and GCC) prints "`hello`."
The local variable `sv` is a reference to the `sv` data member of a lifetime-extended `Example`
temporary. However, if you change `auto&&` to a simple `auto`, then suddenly the program's behavior
is undefined, because now the local variable `sv` is a dangling `string_view` referring into
the guts of an already-destroyed `Example` temporary.
When compiled with `clang++ -O0`, the program demonstrates UB by printing "`byeo`."

----

This blog post is _not in any way_ intended to encourage the use of C++ lifetime extension in production code!
Lifetime extension is subtle, arcane, subject to implementation divergence, and much too easy for the programmer
to break under refactoring. This post is merely intended to demonstrate that formally speaking, lifetime extension
does something subtler and more complicated than simply "turning reference variables into by-value variables."

See also:

* ["Field-testing 'Down with lifetime extension!'"](/blog/2020/03/04/field-report-on-lifetime-extension/) (2020-03-04)
