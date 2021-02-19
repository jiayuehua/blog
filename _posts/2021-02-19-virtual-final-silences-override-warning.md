---
layout: post
title: "A hole in Clang's `-Wsuggest-override`"
date: 2021-02-19 00:01:00 +0000
tags:
  classical-polymorphism
  compiler-diagnostics
  llvm
excerpt: |
  Consider this code:

      struct B {
          virtual int f() = 0;
          virtual int g() = 0;
      };

      struct C : B {
          int f() override;

          virtual int g() final;  // overrides
          virtual int h() final;  // doesn't
      };
---

Consider this code ([Godbolt](https://godbolt.org/z/96de4f)):

    struct B {
        virtual int f() = 0;
        virtual int g() = 0;
    };

    struct C : B {
        int f() override;

        virtual int g() final;  // overrides
        virtual int h() final;  // doesn't
    };

In any real codebase, you should never ever use `virtual` and `final` together like this.

> Use only `virtual` (preferably with `=0`) in root classes.
>
> Use only `override` or `final` (without `virtual`) in non-root classes.

So this code is extremely unrealistic.

However, I was surprised that Clang's `-Wsuggest-override` diagnostic is not
smart enough to complain here. The syntax of this code fails to indicate its meaning —
obviously, because `C::g()` and `C::h()` have the same syntax but different meanings.
Therefore, the compiler really ought to discourage the programmer from writing
this "ambiguous" syntax.

By the way, kudos to Clang for implementing `-Wsuggest-override` at all.
I recommend that everybody turn it on immediately!

> `-Wall` includes
> `-Winconsistent-missing-override`, which warns within classes that are
> already using `override` in some places but not others.
> `-Wsuggest-override` is more advanced; it diagnoses
> missing `override` keywords even in classes that _don't_ already use it.

----

I think I want a diagnostic spelled something like `-Wsuggest-omit-virtual`.

Stylistically, the `virtual` keyword should be used only on roots; the _absence_ of `virtual`
unambiguously signals the intentional absence of root-ness, in the same way that
the _absence_ of `override`/`final` (in modern code) unambiguously signals
the intentional absence of overriding-ness.
The proper shape of `class C` is thus:

    struct C : B {
        int f() override;

        int g() final;  // overrides
        virtual int h() final;  // doesn't
    };

`g`'s syntax now differs from `h`'s, which is appropriate, because `g`'s meaning
differs from `h`'s.
The use of `virtual` and `final` together in `h`'s signature indicates that
`h` is both a root _and_ a leaf — highly unrealistic, but technically possible!

See also:

* ["Two musings on the design of compiler warnings"](/blog/2020/09/02/wparentheses/) (2020-09-02)
