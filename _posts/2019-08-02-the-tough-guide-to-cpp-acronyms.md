---
layout: post
title: "A C++ acronym glossary"
date: 2019-08-02 00:01:00 +0000
tags:
  c++-learner-track
  slack
  wg21-folkloristics
---

Someone on Slack recently noted C++'s penchant for cryptic acronyms. So I thought I'd write down
a glossary of the ones I'm aware of. I'm sure I forgot some. If you think of one I've missed, please
let me know! (And no, I'm not adding [DesDeMovA](https://www.youtube.com/watch?v=fs4lIN3_IlA).)

Cppreference also has [a page of acronyms](https://en.cppreference.com/w/cpp/language/acronyms).

## AAA

"Almost Always Auto." A phrase, and coding style, introduced by Herb Sutter
in [Guru of the Week #94](https://herbsutter.com/2013/06/13/gotw-94-special-edition-aaa-style-almost-always-auto/)
(June 2013). It means writing

    auto dx = x1 - x2;
    auto p = std::make_unique<int>(42);
    auto i = 0;

instead of

    double dx = x1 - x2;
    std::unique_ptr<int> p = std::make_unique<int>(42);
    int i = 0;

respectively. (See also ["The Knightmare of Initialization in C++"](/blog/2019/02/18/knightmare-of-initialization/) (2019-02-18).)

## ADL

"Argument-dependent lookup." See ["What is ADL?"](/blog/2019/04/26/what-is-adl/) (2019-04-26).

## ADT

"Abstract data type"; that is, any class type with which the user interacts only via a
high-level ("abstract") interface. In C++, thanks to [linguistic interference](http://www.glottopedia.org/index.php/Interference)
from other meanings of the word "abstract," you might see "ADT" used specifically to refer to
STL-style class templates, such as `std::priority_queue`; that is, any class template which
is parameterized ("abstracted") over an open set of parameter types.

All ADTs are also _user-defined types_ (UDTs). Due to confusion over whether library types such as
`std::string` were or were not "user-defined," the paper standard has mostly stopped using the term
"UDT" in favor of "[program-defined type](http://eel.is/c++draft/definitions#defns.prog.def.type)."
`std::string` is not a "program-defined type."

## BMI, CMI

"Binary Module Interface." Just as .cpp files are compiled into .o files, and some compilers provide
ways to "pre-compile" .h files into [PCHes](#pch), compilers that support C++2a Modules will have to provide
some way to compile C++ modules into some format that is precompiled, perhaps binary, perhaps compressed,
to make `import` statements quick to compile.

The term "BMI" is not used by the paper standard. There is no standard BMI format. Each vendor will have
their own format, just like they do today for precompiled headers and object files (although some of those
formats may be governed by other standards, such as [ELF](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format)).
[P0822 "C++ Modules are a Tooling Opportunity"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0822r0.pdf) (Gaby Dos Reis, October 2017)
gives a very high-level sketch of the BMI format that Microsoft calls "IFC," and which is modeled
on something else called ["Internal Program Representation" (IPR)](https://github.com/GabrielDosReis/ipr).

Because BMIs are not necessarily "binary" (in the sense of being highly compressed), [GCC calls them](https://gcc.gnu.org/wiki/cxx-modules)
"Compiled Module Interfaces" (CMI).

_BMI files are not a distribution format._ When you distribute a module, you'll be distributing its source
code (as one or more files [maybe with the extension .mpp](https://www.youtube.com/watch?v=E8EbDcLQAoc&t=11m00s)).
You won't distribute the BMI file (extension .ifc) that MSVC produces, any more than you'd distribute the
object file (extension .obj) that MSVC produces — in fact the urge to distribute .ifc files should be
even _rarer_ than the urge to distribute .obj files. Probably the best analogy really is to .pch files —
one of the things C++2a Modules are aiming to replace.

## CPO

"Customization point object." This is a notion introduced by Eric Niebler's Ranges library,
which means it'll be new in C++2a.

> A customization point object is a function object with a literal class type
> that interacts with program-defined types while enforcing semantic requirements on that interaction.
> —N4810 [[customization.point.object]/1](http://eel.is/c++draft/customization.point.object#1)

That is, a CPO is an _object_ (not a function); it's callable; it's constexpr-constructible
(that's what "literal" means in this context); it's customizable (that's what it means to
"interact with program-defined types"); and it's concept-constrained.

(WG21 has a fetish for describing concept constraints as "semantic requirements,"
even though C++2a Concepts are a purely syntactic feature because
[nobody knows how to specify semantic requirements](https://quuxplusone.github.io/blog/2018/09/08/problems-concepts-should-solve/#okay-so-to-recap-here-are-the-bi).
The compiler can ensure that some `T` provides syntax for both `==` and `!=`, but it won't check their semantics.)

In practice, this looks more or less like

    namespace detail {
        template<class A, class B>
        void swap_helper(A& a, B& b) {
            using std::swap;
            swap(a, b);
        }
    }

    inline constexpr auto swap =
        []<A, B>(A& a, B& b)
            requires Swappable<A> && Swappable<B>
        {
            return detail::swap_helper(a, b);
        };

(The C++2a Committee Draft has a lot of wording inherited from Eric's Ranges-v3 to deal with
something colloquially known as the "poison pill"; but I observed, and Eric confirmed,
that the poison pill hasn't been necessary ever since C++17 introduced a SFINAE-friendly `std::swap`.)

The benefit of a CPO over a named function is that it separates [the two pieces of the customization
point](https://quuxplusone.github.io/blog/2018/03/19/customization-points-for-functions/):

- A, the piece the user is required to specialize; and

- B, the piece the user is required to _invoke_ (and therefore must _not_ specialize).

In the above example, "A" would be your ADL overload of `swap`, and "B" would be the `swap` CPO itself.

Also, when you call a CPO, even if you don't qualify
its name, you don't get [ADL](#adl) — which means you get more predictable behavior.

So, to recap, a CPO is a "callable, constexpr-constructible, customizable, concept-constrained object."
Maybe it should have been called a "C<sup>6</sup>O" instead!

If you remove the adjectives "customizable, concept-constrained" from the above, then you have a
function object that turns off ADL — but is not necessarily a customization point.
The C++2a Ranges algorithms, such as `std::ranges::find`,
[are like this](http://eel.is/c++draft/algorithms.requirements#2). Any callable, constexpr-constructible
object is colloquially known as a "niebloid," in honor of Eric Niebler.
A CPO is simply a niebloid that wraps a user-definable customization point.

## CRTP

The "Curiously Recurring Template Pattern." See [Wikipedia](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern).
Occasionally misquoted as "Curiously Recur<b>sive</b>."

## CTAD

"Class template argument deduction." See [cppreference](https://en.cppreference.com/w/cpp/language/class_template_argument_deduction).
Occasionally misquoted as "<b>Constructor</b> template argument deduction,"
even in [the WG21 papers](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0702r1.html)
that originally added the feature.
(It's much easier to remember the acronym "CTAD"
than to remember what the feature does! It deduces the template arguments to the
_class_ template. The arguments to any particular constructor template are deduced,
as always, via [template argument deduction](https://en.cppreference.com/w/cpp/language/template_argument_deduction).)

## CWG, EWG, EWGI, LEWG, LEWGI, LWG

These are the main working groups of the [ISO C++ Committee](https://isocpp.org/std/the-committee)
(a.k.a. WG21). At least in theory, their responsibilities are as follows:

- The Evolution Working Group Incubator (EWGI, pronounced "oogie"; a.k.a. SG17) evaluates proposals for core-language features.

- The Evolution Working Group (EWG) designs core-language features.

- The Core Working Group (CWG) reviews core-language wording.

- The Library Evolution Working Group Incubator (LEWGI, pronounced "lewgie"; a.k.a. SG18) evaluates proposals for standard library facilities.

- The Library Evolution Working Group (LEWG) designs standard library facilities.

- The Library Working Group (LWG) reviews standard library wording.

EWGI and LEWGI are very new in the grand scheme of things; they met for the first time
at the San Diego meeting (November 2018).

By the way, "ISO WG21" stands for Working Group 21 of the
[International Organization for Standardization](https://en.wikipedia.org/wiki/International_Organization_for_Standardization);
and "SG17" means "Study Group 17." For a list of study groups, see [isocpp.org](https://isocpp.org/std/the-committee).

When you see "CWG" or "LWG" followed by a number, as in "[CWG1430](http://cwg-issue-browser.herokuapp.com/cwg1430)"
or "[LWG3237](https://cplusplus.github.io/LWG/issue3237)," it's referring to an _issue_ on CWG's
or LWG's plate — an open question raised by the wording of the Standard. LWG's FAQ gives
[an exhaustive list](https://cplusplus.github.io/LWG/lwg-active.html#Status) of states
an issue can be in, including resolved states such as "[DR](#dr)" and "NAD" (Not A Defect).
See "[A faster WG21 CWG issue browser](/blog/2019/05/22/cwg-issue-browser/)" (2019-05-22).

## DR

"Defect Report." This means a defect or open question raised by the wording of the Standard, which
has been discussed and prospectively resolved by [CWG and/or LWG](#cwg-ewg-ewgi-lewg-lewgi-lwg), resulting in an
amendment or erratum to the Standard of a technical nature (as opposed to a merely editorial fixup).
Formally, I believe the term "DR" refers to the _question_ or _problem_, whereas the ultimately
adopted _solution_ is formally a "technical corrigendum" (TC). In common parlance, you'll hear
"ah, that issue was resolved by DR 409" — "DR 409" being a colloquial shorthand for "the resolution
of the DR arising from [LWG issue 409](https://cplusplus.github.io/LWG/issue409)," and it is only from
context that we can tell it was LWG 409, not [CWG 409](http://cwg-issue-browser.herokuapp.com/cwg409).
Defect Reports themselves are not numbered, _per se_.

DRs are often applied retroactively. For example,
[N3922 "New Rules for auto deduction from braced-init-list"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3922.html) (James Dennett, February 2014),
being associated with an (unnumbered) DR,
was not merely adopted into the C++17 working draft, but also retroactively applied
to the already-published C++11 and C++14 standards.
This manifests as a [difference in `-std=c++11` behavior](https://godbolt.org/z/AI7WpW) between Clang 3.5.1
(shipped January 2015) and Clang 3.8 (shipped March 2016) — not as a difference between `-std=c++11` and
`-std=c++17` on any compiler! So in this sense, to "resolve _foo_ as a DR" connotes "to apply the same fix
uniformly across all language modes."

## EBO, EBCO

"[Empty Base (Class) Optimization](https://en.cppreference.com/w/cpp/language/ebo)."
This is the thing that, given the code

    struct A {};
    struct B { int i; };
    struct C : public A { int i; };

makes `B` and `C` have the same size
(and in fact [the same layout](https://www.youtube.com/watch?v=yTb6xz_FSkY)).

## EDG

[Edison Design Group](https://www.edg.com/company), a small company that makes compiler front-ends.
Its C++ front-end was first released in 1992. Essentially all of its employees are extremely core
members of the C++ standardization committee (WG21).

## EH, TDEH

"Exception Handling." See also: [SEH](#seh).

When talking about standard C++ exception handling, you may see references to "`setjmp`/`longjmp`
exception handling" versus "table-driven exception handling" (TDEH). The former is the old-school
implementation used in the code generators for compilers like Cfront ([source](https://pdfs.semanticscholar.org/6e3a/dc7855c03bac7458e41adb007557102bd52a.pdf)):
opening a new scope essentially calls `setjmp` to build a dynamic stack of "things that need to be
unwound when we throw," and then `throw` essentially calls `longjmp` as many times as it needs in
order to unwind the stack back to the appropriate handler.

TDEH has pretty much taken over the world in the past two decades. Opening a new scope in TDEH is
a free operation; for this reason it's also been colloquially called "zero-cost exceptions."
`throw` essentially consults a static data table of "things that need to be unwound when we throw
from this particular stack frame," and then unwinds one stack frame, and repeats, as many times as
it needs in order to unwind the stack back to the appropriate handler. TDEH pays a relatively larger
up-front cost in _data size_; `setjmp`/`longjmp` exception handling pays a relatively larger runtime cost
and also a larger cost in _code size_.

## FAM

"[Flexible array member](https://en.wikipedia.org/wiki/Flexible_array_member)."
This is the C99 feature that lets you write

    struct S {
        int x, y, z;
        char extra_space[];
    };
    struct S *ps = malloc(sizeof(S) + 10);
    strcpy(ps->extra_space, "some data");

The "flexible" member must have no array bound, and must appear as the last member of the struct.

Flexible array members are not part of C++, and likely never will be, officially.
Accessing off the end of an object will always technically be undefined behavior.
Nevertheless, C++2a's [destroying `delete`](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0722r1.html)
facility was designed specifically to support FAM-like techniques.

## GCC

Originally the "GNU C Compiler" (where "[GNU](https://en.wikipedia.org/wiki/GNU)"
famously stands for "GNU's Not Unix"). Since 1999 ([source](https://gcc.gnu.org/wiki/History)), the
acronym has stood for "GNU Compiler Collection." One of the big three C++ compiler vendors, besides
Clang and [MSVC](#msvc).

## HALO

"Heap Allocation eLision Optimization." This is the optimization on C++2a coroutines
referred to in [Gor Nishanov's talk](https://www.youtube.com/watch?v=8C8NnE1Dg4A&t=6m00s)
on the "disappearing coroutine" (CppCon 2016).
See "[Announcing `Quuxplusone/coro`](https://quuxplusone.github.io/blog/2019/07/03/announcing-coro-examples/)" (2019-07-03),
specifically [this example](https://coro.godbolt.org/z/5vjlk8); see also
[P0981 "HALO: the joint response"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0981r0.html)
(Richard Smith & Gor Nishanov, March 2018).

Normally, each time you enter at the top of a C++2a coroutine and create its return object
(regardless of whether you're multi-threading)
you'll have to heap-allocate enough space to store the coroutine's stack frame. However, in
some specific cases where the compiler can statically determine the lifetime of the coroutine
frame — determine that it will never "escape" from a very localized region of the code — then
the compiler can find a place higher up on the stack to allocate space for it. In that case,
the heap-allocation becomes unnecessary and can be "elided." This can happen quite often in the
generator/`co_yield` use-case, if your generator type is carefully crafted.
My understanding is that HALO will basically never happen in the multi-threaded/`co_await` use-case.

Even when the heap-allocation cannot be elided, C++2a `std::coroutine_traits` provides
rudimentary hooks for the programmer to customize the heap allocation mechanism. See
"[C++ Coroutines: Understanding the promise type](https://lewissbaker.github.io/2018/09/05/understanding-the-promise-type#customising-coroutine-frame-memory-allocation)"
(Lewis Baker, September 2018).

## ICE

"Internal compiler error." A compiler (for C++ or any other language) should always be able to compile
your code or else tell you what's wrong with it. If you give the compiler some input and it gets
so confused that _the compiler itself_ assert-fails, then you've discovered an internal compiler error.
Many compilers' assertion-failure messages actually contain the words "internal compiler error"
(for example, [GCC](https://github.com/gcc-mirror/gcc/search?q=DK_ICE)). I don't know the original
inventor of the term.

If the compiler segfaults or dies from an unhandled exception, you could reasonably call that an
"internal compiler error" too. Some compilers will install signal handlers or exception handlers
to turn such bugs into assertion failures that actually print "internal compiler error"; some won't.

Sadly for clarity of communication, "ICE" is also the initialism for "integral constant expression."

> An _integral constant expression_ is an expression of integral or unscoped enumeration type,
> implicitly converted to a prvalue, where the converted expression is a core constant expression.
> [Note: Such expressions may be used as bit-field lengths, as enumerator initializers if the
> underlying type is not fixed, and as alignments. —end note]
> —[N4810 [expr.const]/5](http://eel.is/c++draft/expr.const#7)

## IFNDR

"Ill-formed, no diagnostic required." To a first approximation, this means the exact
same thing as "undefined behavior" (UB). Specifically,

> If a program contains a violation of a rule for which no diagnostic is required,
> this document places no requirement on implementations with respect to that program.
> —N4810 [[intro.compliance]/2.3](http://eel.is/c++draft/intro.compliance#2.3)

The standard sometimes uses the phrase exactly
(e.g. [[dcl.attr.noreturn]/1](http://eel.is/c++draft/dcl.attr.noreturn#1)), and
sometimes uses variations (e.g. [[using.headers]/3](http://eel.is/c++draft/using.headers#3)):

> A translation unit shall include a header only outside of any declaration or definition,
> and shall include the header lexically before the first reference in that translation unit
> to any of the entities declared in that header. No diagnostic is required.

What this means, implicitly, is that *if the preceding "shall" statement
is violated by the user's program,* no diagnostic is required. So you'll have an ill-formed program
(which by definition is "not C++"). Because no diagnostic is required, your C++ compiler is not
required to tell you that its input was "not C++"; but, because the input wasn't C++, your C++ compiler
is not required to produce any particular output. It might produce
[nasal demons](http://www.catb.org/jargon/html/N/nasal-demons.html). It might even [ICE](#ice), although
that wouldn't be nice.

Frequently, the distinction between "IFNDR" and "[UB](#ub)" is that "IFNDR" connotes a static property
of the code, whereas "UB" connotes a runtime property. A division routine
can have conditional UB at runtime (if you pass it a divisor of zero), but code cannot be conditionally
IFNDR at runtime. Vice versa, if some mistake (such as an ODR violation) could conceivably result in
a linker error, then the mistake will typically be described in terms of "IFNDR" rather than "UB."
Recall that if a program is _ill-formed_, then the compiler doesn't have to generate code for it;
but if a program is _well-formed_, then the compiler must generate some sort of code for it,
even in the presence of undefined behavior.

## IILE

"Immediately invoked lambda expression." This is a relatively obscure idiom in C++, but we plucked
it from JavaScript, where it is called "[immediately invoked function expression](https://en.wikipedia.org/wiki/Immediately_invoked_function_expression)"
(IIFE). In JavaScript, IIFEs are typically used to avoid polluting the current scope with helper
variables — which is important because in JavaScript you're often working in the global scope.
In C++, the idiom is typically used to avoid mutation in the current scope. That is, rather than
write an initialization as

    void test(int *first, int *last, bool(*pred)()) {
        std::vector<int> v(first, last);
        std::sort(v.begin(), v.end());
        std::stable_partition(v.begin(), v.end(), pred);

        // Now do several things that don't involve modifying `v`
    }

you might move the initial sorting-and-partitioning into a helper function — which you make
a lambda so that it can use `first`, `last`, and `pred` without cumbersome argument-passing,
and so that you don't have to scroll around while reading the code. The end result:

    void test() {
        const std::vector<int> v = [&]() {
            std::vector<int> v(first, last);
            std::sort(v.begin(), v.end());
            std::stable_partition(v.begin(), v.end(), is_prime);
            return v;
        }();

        // Now do several things that don't involve modifying `v`
    }

For a dangerous example of using IILEs with C++2a Coroutines, see
["C++2a Coroutines and dangling references"](https://quuxplusone.github.io/blog/2019/07/10/ways-to-get-dangling-references-with-coroutines/#exciting-new-way-to-dangle-a-reference) (2019-07-10).

The C++2a draft standard's [notion of "immediate invocation"](http://eel.is/c++draft/expr.const#def:immediate_invocation)
has absolutely nothing to do with IILEs; it has to do with the evaluation of C++2a `consteval` functions.

## LTO

"Link-Time Optimization." Any kind of optimization that requires looking at the whole program —
thus also sometimes known as "whole-program optimization" (WPO) or "whole-program analysis" (WPA).
This is a special case of
"[interprocedural optimization](https://en.wikipedia.org/wiki/Interprocedural_optimization)" (IPO).

LLVM's docs have [a great example](https://llvm.org/docs/LinkTimeOptimization.html#example-of-link-time-optimization)
showing how LTO can iteratively remove dead (but non-static) functions, and then update global
invariants to cause even more code to go dead.

When I worked at Green Hills, their linker was known for its super aggressive link-time optimizations
such as [function outlining](https://jakewharton.com/r8-optimization-method-outlining/)
(i.e., the opposite of function inlining) and unused virtual function deletion (UVFD).

## MSVC

Microsoft Visual C++ — in C++ contexts, essentially a synonym for Microsoft Visual Studio (VS or MSVS).

## NSDMI

"Non-static data member initializer." This is the C++11 feature that allows you to write
"initializers" on the member variables of a class.

    struct S {
        static int s = 1;  // an initialized static data member
        int x;             // a non-static data member, sans initializer
        int y = 2;         // NSDMI!
    };

This term has gained some currency lately because of Corentin Jabot's proposal for
["`auto` NSDMIs"](https://cor3ntin.github.io/posts/auto_nsdmi/) — non-static data members
whose type is deduced (at class-definition time) from the type of their initializer.
`auto` NSDMIs are not (yet?) part of any draft standard.

## NTTP

"Non-type template parameter." This is a weird one, because you'd think by symmetry it ought to
be spelled "TNTP" — template type parameter, template template parameter, template non-type parameter,
right? But no: C++ has "template type parameters" and "non-type template parameters."

A template type parameter is like `template<class C>`.

A template template parameter is like `template<template<class> class TC>`.

A non-type template parameter is like `template<int V>` or `template<auto V>`.

## NVI

"Non-virtual interface." This rare acronym refers to the increasingly common (and, in my view, good)
practice of [separating the two pieces of the customization
point](https://quuxplusone.github.io/blog/2018/03/19/customization-points-for-functions/) even for plain
old classical polymorphism. The piece specialized by the derived class stays as a virtual
function (but becomes private); the piece invoked by the caller stays public (but becomes non-virtual).

    class AbstractWidget {
        virtual void do_frobnicate() = 0;
    public:
        void frobnicate() { this->do_frobnicate(); }
    };

    class DerivedWidget : public AbstractWidget {
        // note: implicitly "private"
        void do_frobnicate() override { ... }
    }

The Non-Virtual Interface pattern is used in `<iostream>`
([public non-virtual `sputn` and protected virtual `xsputn`](https://en.cppreference.com/w/cpp/io/basic_streambuf/sputn)),
and also in [PMR](#pmr)
([public non-virtual `allocate` and private virtual `do_allocate`](https://en.cppreference.com/w/cpp/memory/memory_resource)).

## ODR

The "One-Definition Rule." See [cppreference](https://en.cppreference.com/w/cpp/language/definition).
The ODR is quite long and involved — it takes up four printed pages in the paper standard! But
the important points are:

> Every program shall contain exactly one definition of every non-inline function or variable that
> is *odr-used* in that program outside of a discarded statement; no diagnostic required.
> —N4810 [[basic.def.odr]/10](http://eel.is/c++draft/basic.def.odr#10)

An *odr-use*, to a first approximation, is any use that requires the used entity to be defined somewhere.
(This excludes things like asking for the `sizeof` or `decltype` of a variable.)
By "discarded statement," they mean the untaken branch of an `if constexpr`.

> There can be more than one definition of a class type, enumeration type, inline function
> with external linkage, inline variable with external linkage, class template, non-static
> function template, concept, static data member of a class template, member function of a
> class template, or template specialization for which some template parameters are not specified
> in a program provided that [...] the definitions satisfy the following requirements. [...]
>
> - each definition of D shall consist of the same sequence of tokens
>
> —N4810 [[basic.def.odr]/12](http://eel.is/c++draft/basic.def.odr#12)

The first quote above disallows programs like

    // alpha.cc
    int i = 0;

    // beta.cc
    int i = 0;

The second quote disallows programs like

    // alpha.cc
    inline int foo() { return 0; }

    // beta.cc
    inline int foo() { return 1; }

(but if you changed that `1` to a `0`, then the program would be well-formed).

Both of these programs exhibit "ODR violations."
A program which violates the ODR is [IFNDR](#ifndr). In the first example above, you'll likely get
a linker error; in the second example, you'll likely get a compiled program with unexpected runtime
behavior.

## PCH

"[Pre-compiled header](https://gcc.gnu.org/onlinedocs/gcc/Precompiled-Headers.html)."
Normally, when you `#include "foo.h"`, you have to recursively open all
the files that it includes and parse all that C++ code (Include guards defend against doing this more
than once _per translation unit_, but if you have a hundred translation units, you'll still be opening all
those files a hundred times.) Therefore, most compilers support some way to pre-compile "foo.h"
so that when you say `#include "foo.h"`, the compiler actually just opens "foo.pch" — a preprocessed, parsed,
and compressed representation of _all_ the code recursively included by "foo.h".

Most compilers restrict the usage of PCH files — e.g. requiring that each translation unit can only
include one PCH, and it must be the very first non-comment line in the file. And notably, from
[GCC's docs](https://gcc.gnu.org/onlinedocs/gcc/Precompiled-Headers.html):

> The precompiled header file must have been produced by the same compiler binary
> as the current compilation is using.

That is, _PCH files are not a distribution format._ See also: [BMI](#bmi-cmi).

## PGO

"[Profile-Guided Optimization](https://en.wikipedia.org/wiki/Profile-guided_optimization),"
occasionally called "profile-driven optimization" (PDO).
You compile your program with profiling instrumentation;
then you run it through its paces to collect a profile; and then you feed that profile back
into a second invocation of the compiler. From the profile, the compiler can tell what loops
are hot, what functions are frequently called together, and so on; which can lead to better
codegen the second time around.

This is a topic that I wish I knew more about.

## PIMPL

"Pointer to IMPLementation." Variously capitalized "PIMPL," "PImpl," or "pImpl," this is a technique
for moving expensive implementation details out of your most commonly traveled .h files and
into separately compiled .cpp files. See [cppreference](https://en.cppreference.com/w/cpp/language/pimpl).

## PMR

"Polymorphic Memory Resources." C++17 added the PMR library, mostly in the header `<memory_resource>`,
and mostly in the nested namespace `std::pmr`. The most important components are
`std::pmr::memory_resource`, which is a traditional abstract base class; and
`std::pmr::polymorphic_allocator<T>`, which is an allocator (similar to `std::allocator<T>`) which
holds within itself a pointer to a `memory_resource` that it uses to fulfill requests for memory.

For more on PMR, see "[`<memory_resource>` for libc++](/blog/2018/06/05/libcpp-memory-resource/)" (2018-06-05)
and my talk "[An Allocator is a Handle to a Heap](https://www.youtube.com/watch?v=0MdSJsCTRkY)"
(C++Now 2018, CppCon 2018).

## POCCA, POCMA, POCS

"`propagate_on_container_copy_assignment`," "`propagate_on_container_move_assignment`,"
and "`propagate_on_container_swap`," respectively. When you have an STL container (such as `std::vector`)
with a custom allocator type, you can write

    A a1("foo");
    A a2("bar");
    assert(a1 != a2);  // for the sake of argument
    std::vector<int, A> v1(a1);
    std::vector<int, A> v2(a2);
    v1 = v2;             // A
    v1 = std::move(v2);  // B
    std::swap(v1, v2);   // C

Before line A, we clearly have `v1.get_allocator() == a1`. After line A, does `v1.get_allocator()` equal
`a1`, or `a2`? What about after line B? What about after line C?

The standard library's [`std::allocator_traits<A>`](https://en.cppreference.com/w/cpp/memory/allocator_traits)
exposes member typedefs named `propagate_on_container_copy_assignment`, `propagate_on_container_move_assignment`,
and `propagate_on_container_swap` that control these behaviors; they're inherited from the allocator type
`A` if possible, or else defaulted to `false_type`. If they're all `false_type`, then you have a traditional
allocator that fully enables "pilfering" the allocation from one vector into another (in cases B and C above).
If they're all `true_type`, then you have a "sticky" allocator that inhibits pilfering in cases where the
source and destination containers have different allocators. C++17's [PMR](#pmr) allocators are "sticky."
If some of POCCA/POCMA/POCS are `true_type` and some are `false_type` for the same allocator type,
then you probably have a bug.

(If your allocator type is stateless and/or sets `is_always_equal`, then the settings of
POCCA/POCMA/POCS don't really matter and might just as well be inconsistent.
For [historical reasons](https://stackoverflow.com/questions/42051917/why-does-stdallocator-require-propagate-on-container-move-assignment-to-be-tru),
`std::allocator` falls into that category.)

For more on this topic, see my talk "[An Allocator is a Handle to a Heap](https://www.youtube.com/watch?v=0MdSJsCTRkY)"
(C++Now 2018, CppCon 2018) and my training course
[_The STL From Scratch_](/blog/2019/06/21/stl-from-scratch-at-cppcon-2019/).

## POD

"[Plain Old Data](https://en.cppreference.com/w/cpp/named_req/PODType)."
This term has been deprecated in C++2a, along with the type trait `std::is_pod<T>`.

## RAII

"Resource Acquisition Is Initialization." This is a brush capable of very broad strokes, but it
boils down to the idea that you should have destructors that free your resources, copy constructors
that duplicate your resources, and copy-assignment operators that do both. It's as broad and vague
a slogan as "move semantics" or "value semantics," though; different people might express its
fundamental precepts in slightly different ways.

Shameless plug: I'll be giving a talk on "RAII and the Rule of Zero" at CppCon this September!
[Come see it!](https://cppcon2019.sched.com)

> It should really have been called Resource Freeing Is Destruction, but
> [that acronym was taken](https://en.wikipedia.org/wiki/Radio-frequency_identification).

## RTTI

"Runtime Type Information." This is the metadata that's generated for each user-defined class type
for use by C++ runtime features such as `dynamic_cast`, `typeid`, and exception-handling.
Many compilers provide a command-line switch such as `-fno-rtti` to limit or eliminate this
information (which of course limits or eliminates the programmer's ability to use `dynamic_cast`
and `typeid`).

For background on `dynamic_cast`'s use of RTTI, see my talk
"[`dynamic_cast` from scratch](https://www.youtube.com/watch?v=QzJL-8WbpuU)" (CppCon 2017).

## RVO, NRVO, URVO

"Return Value Optimization," a.k.a. "[copy elision](https://en.cppreference.com/w/cpp/language/copy_elision)"
(or, in C++11 and later, "move elision" is also a thing).

There are two places where copy elision (a.k.a. RVO) typically kicks in. The place where C++17
_mandates_ that it happen (thanks to
"[deferred materialization of temporaries](https://blog.tartanllama.xyz/guaranteed-copy-elision/),"
a.k.a. "guaranteed copy elision") is when the object being returned is a prvalue:

    return x+1;
    return Widget();

Since the temporary object here has no name, this is colloquially known as "unnamed return value optimization"
(URVO).

The other place where copy elision might happen, but is _optional_, is when returning a local variable
by name:

    Widget x; [...]
    return x;

Since this return value has a name, this is colloquially known as "named return value optimization" (NRVO).

Notice that "URVO" can be seen as a special case of guaranteed copy elision, which would also kick in
if you wrote something like

    void foo(Widget);
    void test() {
        foo(Widget());
    }

Our `Widget` is constructed directly into the parameter slot. This is arguably not "RVO"
since it doesn't involve "return values" — it is not even "copy elision" according to the paper standard —
but it is definitely still "deferred materialization of temporaries."

I am not aware of any way to trigger "NRVO" other than via a `return` statement. It is
technically allowed in `throw` statements as well, but
[no compiler implements that feature](/blog/2018/04/09/elision-in-throw-statements/).
It is also technically allowed in C++2a `co_return` statements, but neither Clang nor MSVC
implement that feature. (MSVC doesn't even do implicit move!)

For more on this topic, see my talk
"[RVO Is Harder Than It Looks](https://www.youtube.com/watch?v=hA1WNtNyNbo)" (CppCon 2018).

## SBO, SOO, SSO

"Small Buffer Optimization," referring to a small buffer held within the memory footprint of a type
that in the general case would have to dynamically allocate memory to hold something or other.
The small buffer is used to avoid that memory allocation in "small" cases. If our buffer is being
used to store an object, then we might say that our type has a "Small Object Optimization" (SOO).
If our buffer is being used to store a string, then we have a "Small String Optimization" (SSO).
For slightly more on SBO/SOO/SSO, see
"[The space of design choices for `std::function`](/blog/2019/03/27/design-space-for-std-function/#sbo-affects-semantics)" (2019-03-27).

## SCARY iterators

This silly initialism was introduced in
[N2911 "Minimizing Dependencies within Generic Classes for Faster and Smaller Programs"](http://www.open-std.org/jtc1/sc22/WG21/docs/papers/2009/n2911.pdf)
(Tsafrir, Wisniewski, Bacon, Stroustrup; June 2009). It refers to the template-metaprogramming technique
of keeping "policy parameters" such as allocators at the outermost possible level and not letting them
pollute the lower levels of the system. It's the difference between

    template<class T, class A>
    class vector {
        struct iterator { ... };
    };

    // Many distinct iterator classes
    static_assert(not std::is_same_v<
        vector<int, A1>::iterator,
        vector<int, A2>::iterator
    >);

and

    template<class T>
    class vector_iterator { ... };

    template<class T, class A>
    class vector {
        using iterator = vector_iterator<T>;
    };

    // Fewer iterator classes: hotter code
    static_assert(std::is_same_v<
        vector<int, A1>::iterator,
        vector<int, A2>::iterator
    >);

To quote the paper:

> The acronym <b>SCARY</b> describes assignments and initializations that are <b>S</b>eemingly erroneous
> (appearing <b>C</b>onstrained by conflicting generic parameters), but <b>A</b>ctually work with
> the <b>R</b>ight implementation (unconstrained b<b>Y</b> the conflict due to minimized dependencies).

See also: "[SCARY metafunctions](/blog/2018/07/09/scary-metafunctions/)" (2018-07-09).

For another example of an initialism that doesn't come (entirely) from initials, see [HALO](#halo).

## SEH

Microsoft Windows has a feature called
[Structured Exception Handling](https://en.wikipedia.org/wiki/Microsoft-specific_exception_handling_mechanisms#SEH) (SEH);
I don't know much about it except that it somehow unifies C++ exception handling and things that
would be considered "signals" in POSIX-land, such as floating point exceptions. If you pass the
`/EHc` flag to MSVC, or simply omit `/EHsc`, then you can
[catch division-by-zero as a C++ exception](https://rextester.com/KVWO69475)! This works only because
of SEH. There's no equivalent on non-Windows operating systems as far as I know.

## SFINAE

"Substitution Failure Is Not An Error." This is the slogan that helps you remember why the compiler
doesn't complain about

    template<class T>
    void f(T, typename T::type) { puts("hello"); }

    template<class T>
    void f(T, long) { puts("world"); }

    void test() {
        f(1, 2);  // "world"
    }

Here, we've already done template argument _deduction_ on our first candidate `f` and figured out
that if we call this candidate, then we'll set `T=int`. The "failure" happens during template
argument _substitution_ — when we try to extract a member type `T::type` from `T=int`, and find
that there is no such member. If we'd been compiling ordinary code and seen

    using T = int;
    void f(T, typename T::type) { puts("error"); }

then that would have been a hard error. But in this case, we merely have a _substitution failure_,
which is Not An Error. We simply discard this candidate `f` and start looking at the next
candidate, which turns out to work fine. This is exactly the same thing we would have
done if we'd failed to _deduce_ `T` originally — we simply discard the template from further
consideration and move on.

For more on SFINAE, see several of my conference talks:

- [Template Normal Programming, Part 1](https://www.youtube.com/watch?v=vwrXHznaYLA) (CppCon 2016)
- [Template Normal Programming, Part 2](https://www.youtube.com/watch?v=VIz6xBvwYd8) (CppCon 2016)
- [A Soupçon of SFINAE](https://www.youtube.com/watch?v=ybaE9qlhHvw) (CppCon 2017)

## SIOF

The "[Static Initialization Order Fiasco](https://isocpp.org/wiki/faq/ctors#static-init-order)."

C++ guarantees ([[expr.const]/2](http://eel.is/c++draft/expr.const#2)) that certain kinds of global
initializations — like `int i = 42;` — will get baked into the data section. The C++2a draft even
adds a new keyword, `constinit`, so that you can write

    constinit int i = i_sure_hope_this_function_is_constexpr(6, 9);

which means that the compiler _must_ put it in the data section or else give you a compiler error.

But for dynamically initialized global variables —

    std::string s = "hello";
    std::string t = s + " world";

If `s` and `t` are defined in the same [TU](#tu), then C++ guarantees their initializers will run
in the order you'd expect. But if they're defined in two different TUs, the linker might decide to
order the initializer for `t` _before_ the initializer for `s`. So `t`'s initialization uses `s`
as a string before `s` has actually been constructed, leading to [UB](#ub) at runtime.
([Wandbox](https://wandbox.org/permlink/qycov99Bj0TUPxsP).)

## STL

The "Standard Template Library." This name originally referred specifically to the library developed
by Alexander Stepanov and others at Silicon Graphics (the "SGI STL";
[PDF documentation here](http://stepanovpapers.com/STL/DOC.PDF)) and proposed for standardization
in 1993. Ironically, the name "Standard Template Library" _far_ preceded any ISO standard for C++!

The SGI STL became the basis for a lot of the C++98 Standard Library. Since "SL"
doesn't make for a great acronym, and since most of the standard library is templated in some way,
it is common for C++ programmers to refer to the entire C++ standard library as "the STL."
Others might use the term more narrowly to refer to the parts of the library that deal with
containers, iterators, algorithms, allocators, adaptors, and function objects (such as `std::less`),
excluding for example `std::shared_ptr` and `std::valarray`.
Still others might include only those parts of the modern Standard Library inherited directly from SGI,
excluding for example `std::unordered_set` and `std::move_iterator`.

Notably, [iostreams were invented by Bjarne Stroustrup circa 1984](https://stackoverflow.com/a/2753094/1424877);
therefore they are part of the "STL" only in that phrase's most inclusive (and most common) sense.

In [_Effective STL_](https://amzn.to/2ZGM0Tu) (2008), Scott Meyers defines his use of the term "STL"
as "the parts of C++'s Standard Library that work with iterators." (He explicitly excludes the
container adaptors `stack`, `queue`, and `priority_queue`, even though those _were_ part of the SGI STL,
on the grounds that they are not iterable.)
As for [_Mastering the C++17 STL_](https://amzn.to/34rWnt9) (O'Dwyer, 2017), I freely admit that
I used "STL" purely as a short form of the phrase "Standard Library" optimized for book covers and
search engines.

The other meaning of "STL" in C++ contexts is as the initials and preferred nickname of
Stephan T. Lavavej ([pronounced "Steh-fin Lah-wah-wade"](https://twitter.com/StephanTLavavej)).
As of 2019, STL happens to work on [MSVC](#msvc)'s implementation of _the_ STL.

## TBAA

"Type-based alias analysis." This is what lets the compiler conclude that in the following
snippet ([Godbolt](https://godbolt.org/z/pVVlZM)), `*pi` and `*pf` cannot alias each other.

    int foo(int *pi, float *pf) {
        *pi = 42;
        *pf = 3.14;
        return *pi;
    }

Since it would be [UB](#ub) to write `3.14` into a memory location and then read an `int`
back from that same memory location, and UB never happens in a correct program, the compiler
infers that `pi` must point somewhere different from `pf`. The generated code "remembers" the
value that was stored into `*pi` and returns 42 directly from the function, instead of generating
a load from memory.

On the other hand, if you changed `pi` from `int*` to `float*`, you would see the load happening,
because loading a `float` from a memory location that contains a `float` is _not_ UB.
So in that case the compiler couldn't assume that `pi` and `pf` don't alias. Other types that
might alias `float` include [plain `char`](https://godbolt.org/z/flEMi9) and
[`struct ContainsFloat`](https://godbolt.org/z/rJA1gW).

Type-based alias analysis is also known as "strict aliasing," because the analysis
[can be disabled](https://godbolt.org/z/SK3DUP) with the compiler option `-fno-strict-aliasing`.

## TMP

"Template metaprogramming." Personally, I find that the acronym "TMP" has a vague whiff of C++98
which is not shared by the simple spelled-out word "metaprogramming."

If you have lots of structs with `::type` and `::value` members, you're probably doing TMP.

## TU

"Translation Unit." This is what language-lawyers say instead of ".cpp file." When you invoke the compiler
to translate your C++ code into machine code, you give it just one _translation unit_ (TU) at a time.
[Formally](http://eel.is/c++draft/lex.separate), a "TU" is the sequence of tokens that you get
after preprocessing an input file so that all its `#include` directives have been expanded and all its
`#if` directives have been resolved.

If you `#include` a file, then the text of that file becomes part of your translation unit.
In C++2a Modules, if you `import` a module then that module's "module interface unit" (MIU)
does _not_ become part of your translation unit. Rather, a module interface unit is itself a
_kind_ of translation unit: it is translated in a separate step.

## UB

"Undefined behavior." C++ shares this notion with C, and it means the same thing to both languages:

> behavior for which this document imposes no requirements
> —N4810 [ [defns.undefined]](http://eel.is/c++draft/intro.defs#defns.undefined)

> This document imposes no requirements on the behavior of programs that contain undefined behavior.
> —N4810 [[intro.abstract]/4](http://eel.is/c++draft/intro.abstract#4)

Notably, in contrast to "[IFNDR](#ifndr)," the standard acknowledges that a program containing UB is still a
well-formed program (even if its runtime _behavior_ is undefined).

You might also see the occasional "IDB" ("[implementation-defined behavior](http://eel.is/c++draft/intro.defs#defns.impl.defined)");
and if someone wrote "USB" I might grok from context that it meant
"[unspecified behavior](http://eel.is/c++draft/intro.defs#defns.unspecified)."

## UDL

"[User-Defined Literal](https://en.cppreference.com/w/cpp/language/user_literal)," as in,
the C++11 feature where you declare an `operator ""_myudl` and then are able to use `12345_myudl`
and/or `"hello world"_myudl` as "literals" in your code. The Standard Library defines several
overload sets' worth of UDLs inside [`namespace std::literals`](https://en.cppreference.com/w/cpp/symbol_index/literals).

The paper standard [says](http://eel.is/c++draft/usrlit.suffix) that all programmer-defined UDLs
must have names starting with `_` (underscore). Of course, they _cannot_ start with two underscores
or an underscore followed by a capital letter, because those are [also verboten](http://eel.is/c++draft/lex.name#3.1).

## UFCS

"Universal Function Call Syntax," a name for some kind of feature that would let you write `x.f(y)`
and `f(x, y)` interchangeably. Nobody really knows how to get this into C++. For an excellent
rundown of all the different proposals and their differences and difficulties,
see Barry Revzin's blog post "[What is UFCS anyway?](https://brevzin.github.io/c++/2019/04/13/ufcs-history)"
(April 2019).

## VLA

"Variable-length array." This is the C99 feature that lets you write

    int main(int argc, char **argv) {
        int arg_values[argc - 1];
    }

VLAs are not part of standard C++ ([and never will be](https://stackoverflow.com/questions/1887097/why-arent-variable-length-arrays-part-of-the-c-standard)).
Furthermore, C11 made VLAs a "conditional feature" which even C compilers needn't support. C11-and-later
compilers which don't support VLAs are supposed to define `__STDC_NO_VLA__` to `1`.
