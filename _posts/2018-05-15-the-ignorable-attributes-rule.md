---
layout: post
title: 'The Ignorable Attributes Rule'
date: 2018-05-15 00:01:00 +0000
tags:
  attributes
  language-design
  wg21-folkloristics
---

Here's another of those WG21 design principles that is often brought up vaguely
during discussions, and then inevitably someone doesn't understand what it is, or knows
what it is but disagrees that it should apply in their case, or whatever.

Unlike [the Lakos Rule](/blog/2018/04/25/the-lakos-rule), this one doesn't have a really
catchy name. It's commonly phrased something like this:

> compiling a valid program with all instances of a particular attribute ignored
> must result in a correct interpretation of the original program

That phrasing comes from Richard Smith's
[P0840R0 "Language support for empty objects"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0840r0.html)
(2017), which proposes an attribute `[[no_unique_address]]` as a core-language replacement for the
Empty Base Optimization. (Thanks to [Corentin Jabot's blog post](https://medium.com/@corentin.jabot/318eadc2c003)
for pointing me to this specific quotation.)

Why is this a "rule"?  Past defenses of the rule could sound a bit dogmatic. For example, here's
[Herb Sutter in 2012](https://herbsutter.com/2012/04/02/reader-qa-keywords-and-attributes/):

> `[[attributes]]` are specifically designed to be ignorable and shouldn’t be used for things having
> language semantic meaning.

And here's [Bjarne Stroustrup's "C++11 FAQ"](http://www.stroustrup.com/C++11FAQ.html#attributes)
(which was last updated August 2016 as of this writing, but this text dates from
[circa July 2009](https://web.archive.org/web/20090723072641/http://www.research.att.com:80/~bs/C++0xFAQ.html#attributes)):

> There is a reasonable fear that attributes will be used to create language dialects.
> The recommendation is to use attributes to only control things that do not affect the
> meaning of a program but might help detect errors (e.g. `[[noreturn]]`) or
> help optimizers (e.g. `[[carries_dependency]]`).

But in P0840R0 (October 2017), Richard Smith explains the logic behind the rule as follows:

> [T]he key constraint here is that of program portability: suppose a program uses a vendor-specific attribute,
> or a standard attribute from a later version of C++, or even a standard attribute that their implementation
> just doesn't implement yet. The result of compiling their program on that implementation should still be a
> program that behaves correctly, according to the specification of the attribute.

The oldest, yet fullest, explanation comes from the primary source material: Jens Maurer and Michael Wong's
[N2761 "Towards support for attributes in C++ (Revision 6)"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2761.pdf)
(September 2008). I'll quote it here, with some minor spell-checking throughout.


## Guidance on when to use/reuse a keyword and when to use an attribute

> So what should be an attribute and what should be part of the language?
>
> It was agreed that [an attribute] would be something that helps but can be ignorable with few serious side-effects.
>
> If you are proposing a new feature, the decision of when to use the attribute feature and when to overload
> or invent a new keyword should follow a clear guideline. At the Oxford presentation of this paper, we were
> asked to offer guidance in order to prevent wholesale dumping of extension keywords into the attribute
> extension. The converse possibility is that no one will use the attribute feature and all will elect to
> create or reuse keywords in the belief that this elevates their feature in importance.
>
> Certainly, we would advise anyone who proposes an attribute to consider comments on the following areas,
> which will help guide them [in the direction of using an attribute]:
>
> - The feature is used in declarations or definitions only.
> - The feature is of use to a limited audience only (e.g., alignment).
> - The feature does not modify the type system (e.g., `thread_local`) and hence does not require new mangling.
> - The feature is a "minor annotation" to a declaration that does not alter its
>   semantics significantly. (Test: Take away the annotation. Does the remaining
>   declaration still make sense?)
> - [The feature is] a vendor-specific extension.
> - [The feature is] a language binding on C++ that has no other way of tying to a type or scope (e.g. OpenMP).
> - How does this change overload resolution?
> - What is the effect in typedefs; will it require cloning?
>
> Some guidance for when *not* to use an attribute, and use/reuse a keyword instead:
>
> - The feature is used in expressions as opposed to declarations.
> - The feature is of use to a broad audience.
> - The feature is a central part of the declaration that significantly affects its requirements/semantics (e.g., `constexpr`).
> - The feature modifies the type system and/or overload resolution in a significant
>   way (e.g., rvalue references). (However, something like near and far pointers
>   should probably still be handled by attributes, although those do affect the type system.)
> - The feature is used everywhere on every instance of class, or statements
>   [Arthur says: I think this means "it'll be used frequently, so we want a keyword spelling for it."]
>
> [...]
>
> After the meeting in Toronto, we added specific guidance on the choice of when to use an attribute to avoid misuse.
> There was general agreement that attributes should not affect the type system, and not change the meaning of a
> program regardless of whether the attribute is there or not. Attributes provide a way to give hints to the compiler,
> or can be used to drive out additional compiler messages that are attached to the type, or statement.
> They provide a more scoped way of relating to C++ statements than what pragmas can do. As such, they can
> detect [ODR violation](http://en.cppreference.com/w/cpp/language/definition) more easily.
>
> We created a list of good and bad attributes that can be used as guidelines.
> Good choices in attributes include:
>
> - `align(unsigned int)`
> - `pure` (a promise that a function always returns the same value)
> - `probably(unsigned int)` (hint for `if`, `switch`, ...) — `if [[ probably(true) ]] (i == 42) { ... }`
>   [Arthur says: A weakened form of this is coming in [P0479](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0479r5.html).]
> - `noreturn` (the function never returns)
> - `deprecated` (functions)
> - `noalias` (promises no other path to the object)
> - `unused` (parameter name)
> - `final` on virtual function declaration and on a class
> - `not_hiding` (name of function does not hide something in a base class)
> - `register` (if we had a time machine)
> - `owner` (a pointer is owned and it is the owner’s duty to delete it)
>
> Bad choices in attributes include:
>
> - C99 `restrict` (affects the type system)
> - `huge` (really long long type, e.g. 256 bits)
> - C++ `const`
>
> [...]
>
> What makes [Mike Spertus's `[[owner]]` proposal] a good candidate for attributes is that code that runs
> with these attributes also runs identically if the attributes are ignored, albeit with less type checking.


## The `final` controversy

Sharp-eyed readers will have noticed that N2761 lists "`final`" among its examples of "good" attributes.
Yet in C++11, `final` became a (contextual) keyword, not an attribute! What happened?

Well, it was originally put into the working draft as an attribute. But [the U.S. delegation objected
in strong terms](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2011/n3296.html#US44):

> Even if attributes continue to be standardized over continued objections from both of the
> two vendors who are cited as the principal prior art,
> [Arthur says: GNU `__attribute__((x))` and Microsoft `__declspec(x)`]
> we can live with them with the exception of the virtual override controls.
> This result is just awful, as already shown in the example in 7.6.5 (excerpted):
>
>     class D [[base_check]] : public B {
>         void some_func [[override]] ();
>         virtual void h [[hiding]] (char*);
>     };
>
> Here we have six keywords (not counting `void` and `char`): three normal keywords
> and three `[[decorated]]` keywords. There has already been public ridicule of C++0x
> about this ugliness. This is just a poor language design, even in the face of backward
> compatibility concerns (e.g., that some existing code may already use those words as
> identifiers) because those concerns have already been resolved in other ways in existing
> practice (see below).
>
> More importantly, this is exactly the abuse of attributes as disguised keywords that
> was objected to and was explicitly promised not to happen in order to get this proposal
> passed. The use of attributes for the virtual control keywords is the most egregious abuse
> of the attribute syntax, and at least that use of attributes must be fixed by replacing
> them with non-attribute syntax. These virtual override controls are language features,
> not annotations.

The U.S. delegation went on to propose how they thought it should be done, with contextual
keywords; and in fact the fix happened, and that's how we got contextual keywords in C++ today.

I'm glad `override` is spelled `override` and not `[[override]]`; but still,
you know, we can blame America for [this](https://wandbox.org/permlink/hbtvlfpkhmFvDNoW).

    using override = int();

    struct T {
        virtual override final;
    };

    struct U final : T {
        override final final override;
    };

Speaking of, who else is
[looking forward to `module`](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0795r0.pdf)?

> Using `module` as a module name would need to be explicitly disallowed:
>
>     module module; //bad
