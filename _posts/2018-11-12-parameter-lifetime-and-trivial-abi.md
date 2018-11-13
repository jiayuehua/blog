---
layout: post
title: 'Callee-destroy versus caller-destroy parameter lifetimes'
date: 2018-11-12 00:01:00 +0000
tags:
  attributes
  copy-elision
  pitfalls
excerpt: |
  I was just discussing [Clang's `[[trivial_abi]]` attribute](/blog/2018/05/02/trivial-abi-101)
  with Mathias Stearn. Specifically, the fact that `[[trivial_abi]]` changes the Itanium ABI
  for trivial-ABI types so that parameters of trivial-ABI type are owned and destroyed by the
  *callee*, rather than by the *caller*.

  Now consider two pieces of C++ code. One of them works in practice today; the other
  does not. Neither one is guaranteed to work. They both have *implementation-defined* behavior
  (that is, it is implementation-defined whether their behavior is undefined or not).
---

I was just discussing [Clang's `[[trivial_abi]]` attribute](/blog/2018/05/02/trivial-abi-101)
with Mathias Stearn. Specifically, the fact that `[[trivial_abi]]` changes the Itanium ABI
for trivial-ABI types so that parameters of trivial-ABI type are owned and destroyed by the
*callee*, rather than by the *caller*.

Now consider two pieces of C++ code. One of them works in practice today; the other
does not. Neither one is guaranteed to work. They both have *implementation-defined* behavior
(that is, it is implementation-defined whether their behavior is undefined or not).

Here's the first snippet. Defined or undefined?

    const std::string& foo(std::unique_ptr<std::string> ptr) {
        return *ptr;
        // the controlled object will still be valid in our caller
    }

    int main() {
        std::cout << foo(std::make_unique<std::string>("defined or undefined?")) << "\n";
    }

Here's the second snippet. Defined or undefined?

    std::unique_lock<std::mutex> with(std::mutex& m) {
        return std::unique_lock<std::mutex>(m);
    }

    bool sink(std::unique_lock<std::mutex> lk) {
        return true;
        // the lock is automatically released at the end of this scope
    }

    std::mutex m;
    int main() {
        return sink(with(m)) && sink(with(m));
    }

----

[The first snippet compiles and runs with no issues.](https://wandbox.org/permlink/B9x6Vrg4LAXw2aqt)
It is implementation-defined as *correct* on the Itanium ABI!

[The second snippet compiles clean, but results in a double-lock of `m` and deadlock.](https://wandbox.org/permlink/wXiN49NOCmgm53lI)
It is implementation-defined as *undefined behavior* on the Itanium ABI!

Yes, really _implementation-defined_. [[expr.call] sentence 9.7:](http://eel.is/c++draft/expr.call#7.sentence-9)

> It is implementation-defined whether the lifetime of a parameter ends when the function
> in which it is defined returns or at the end of the enclosing full-expression.

On Visual Studio, using Microsoft's calling convention,
[the second snippet compiles and runs clean](https://rextester.com/CHQG51538) — the `unique_lock`
object is destroyed in the callee and there is no deadlock.

And, as you might now expect, on Visual Studio
[the *first* snippet produces undefined behavior](https://rextester.com/BWMIC40715)
because the `unique_ptr` is destroyed in the callee and the string freed *before* the caller
gets a chance to print it out!

----

As described in my blog post "[`[[trivial_abi]]` 101]((/blog/2018/05/02/trivial-abi-101))" (2018-05-02),
one side effect of the `[[clang::trivial_abi]]` attribute is that it turns caller-destroy types into
callee-destroy types — which can turn working code into non-working code, or vice versa, in cases
such as the above two snippets.

The solution for you in practice, of course, is never to write tricky code like the above!
Here are the guidelines:

- Don't *ever* return a reference that depends on the lifetime of a local variable (as done in our
first snippet). GCC and Clang will even give a warning if you return a reference to a parameter;
although they won't warn about our `unique_ptr` example due to its complexity.

- Don't rely on deterministic destruction of parameter objects. If you need deterministic
destruction, move the parameter into a non-parameter variable. This rule would have saved our
second snippet. However...

- Don't *ever* perform two concurrency-related operations in the same statement! Our original sin
in the second snippet was that we wrote `with(m)` twice on the same line. We should have written
something more like

        bool result = sink(with(m));
        if (result) {
            result = sink(with(m));
        }
        return result;

I admit I can't come up with a super great rewrite of the second snippet, because it's so contrived.
But contrived code can still teach us a lesson. How sure are you that your codebase doesn't contain
*any* instances of either of these pitfalls?
