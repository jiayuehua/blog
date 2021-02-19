---
layout: post
title: "Field-testing Herb Sutter's Modest Proposal to Fix ADL"
date: 2018-08-13 00:01:00 +0000
tags:
  argument-dependent-lookup
  compiler-diagnostics
  llvm
  pitfalls
  proposal
---

Yesterday I made a Clang patch implementing Herb Sutter's proposal
[P0934R0 "A Modest Proposal: Fixing ADL"](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0934r0.pdf) (February 2018).
You can find the patch [here](https://github.com/Quuxplusone/clang/commit/9ff89141fedd36af27820f5cc3a0edd8a1bef2db),
or try it live [on Godbolt](https://godbolt.org/g/qvjmdS) as part of my experimental P1144 branch.

The Godbolt link above shows the troubling behavior of present-day C++:

    namespace B {
        template<class> struct B {};

        template<class T>
        void g(const B<T>&) {
            puts("hello from B");
        }

        template<class T>
        void f(B<T> b) {
            g(b);
        }
    }

This snippet works just the way you'd expect, for *most* values of `T`. But for some values of `T`, it is
"hijackable":

    namespace A {
        struct A {};
        template<class T> void g(T&) {
            puts("ha ha");
        }
    }

    int main() {
        B::B<A::A> b;
        f(b);
    }

Here the inner ADL call to `g(b)` considers both `A::g` and `B::g`, and finds that `A::g` is the best match.
So `f(b)` actually prints "ha ha" and not "hello from B" as we probably expected it to.

P0934 proposes that we "fix" ADL by eliminating this trap: what if we just *didn't permit* ADL to consider
the namespaces associated with template parameters (such as the `A` in `B::B<A::A>`) anymore?
This is a "modest proposal" in the sense of
[Jonathan Swift's original "modest proposal"](https://en.wikipedia.org/wiki/A_Modest_Proposal): it may be
a non-starter for pragmatic reasons, but at least it serves to point out the trouble with the current state
of affairs. Could the proposed new state of affairs really be any *worse* than what we've got now?

Well, now there's an implementation, so it's possible to check some existing codebases and see how they'd
fare in Herb's brave new world.

My implementation works very much like my previous diagnostic `-Wreturn-std-move` (which has now spawned
a proposal of its own: [P1155 "More implicit moves"](http://quuxplusone.github.io/draft/d1155-more-implicit-moves.html#wording)).
We run overload resolution *twice* — once with Herb's proposed rule, and once with the C++17 rule —
and report a warning if and only if the two resolutions produce different results.

LLVM's own codebase hits my diagnostic exactly [three times](https://github.com/Quuxplusone/clang/commit/59e6af843f8c2e66fd8f5508266ac56abe006f23), each clearly unintentional but also harmless:

```
llvm/tools/clang/lib/Driver/Job.cpp:326:11: error: function 'llvm::makeArrayRef' was found via argument-dependent lookup from a
      template type argument [-Werror,-Wadl]
    Env = makeArrayRef(ArgvVectorStorage);
          ^~~~~~~~~~~~

llvm/tools/clang/unittests/Basic/VirtualFileSystemTest.cpp:978:12: error: function 'clang::vfs::getVFSFromYAML' was found via
      argument-dependent lookup from a template type argument [-Werror,-Wadl]
    return getVFSFromYAML(std::move(Buffer), CountingDiagHandler, "", this,
           ^~~~~~~~~~~~~~

In file included from llvm/tools/clang/unittests/Rename/RenameClassTest.cpp:10:
llvm/tools/clang/unittests/Rename/ClangRenameTest.h:82:5: error: function 'clang::tooling::formatAndApplyAllReplacements' was
      found via argument-dependent lookup from a template type argument [-Werror,-Wadl]
    formatAndApplyAllReplacements(FileToReplacements, Context.Rewrite, "llvm");
    ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

[HyperRogue](https://github.com/zenorogue/hyperrogue) never hits the diagnostic, which actually surprised
me a little, because it has so many nested namespaces and is so cavalier about full namespace-qualification.
But it makes sense when I consider that HyperRogue doesn't declare many class templates of its own (which would
correspond to `B::B` in our example).

I built some pieces (about 200,000 lines) of my employer's codebase and never hit the diagnostic, which does not
surprise me, because we're very good about namespace hygiene.

I'd be very interested to know how it fares on a *really* large codebase, such as
[Chromium](https://www.chromium.org/developers/how-tos/get-the-code). Anyone care to find out and tell me?

----

UPDATE: Tomasz Kamiński points out that this diagnostic is triggered on (and P0934 proposes to break)
`std::reference_wrapper<T>`'s implicit conversion to `T&`. [Example:](https://godbolt.org/g/eDYHR2)

    namespace A {
        struct A {};
        void foo(const A&);
    }

    int main() {
        A::A a;
        std::reference_wrapper<A::A> p(a);
        foo(p);
    }

The devil's-advocate argument is that this feature of `reference_wrapper` must be used relatively infrequently
in real code, or I would have noticed it in one of the codebases above! :)

----

For even more followup on this post, see
["Fixing ADL, Round 2."](/blog/2018/08/14/fixing-adl-field-test-2/)
