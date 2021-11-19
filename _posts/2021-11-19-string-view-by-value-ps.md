---
layout: post
title: 'A footnote on "Three reasons to pass `std::string_view` by value"'
date: 2021-11-19 00:01:00 +0000
tags:
  library-design
  parameter-only-types
excerpt: |
  Several readers have responded to my recent post
  ["Three reasons to pass `std::string_view` by value"](/blog/2021/11/09/pass-string-view-by-value/) (2021-11-09),
  pointing out that (while everything I said is true of the Itanium x86-64 ABI used on Linux and Mac) unfortunately
  [Microsoft's x86-64 ABI](https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention#parameter-passing)
  does _not_ pass `std::string_view` in registers, but instead passes it
  by "hidden pointer," so that at the machine level, you get basically the
  same codegen as if you had been passing by reference all along. Except that
  actually you're passing a _copy_ by reference, so you get one extra
  16-byte copy in addition to all the other stack traffic MSVC is doing.

  This is a big problem baked into Microsoft's x86-64 ABI, and
  nerfs a lot of the performance benefits mentioned in my post.
  However, my advice remains the same:
  Even if your primary platform today is Windows, you should habitually
  pass `std::string_view` by value!
---

Several readers have responded to my recent post
["Three reasons to pass `std::string_view` by value"](/blog/2021/11/09/pass-string-view-by-value/) (2021-11-09),
pointing out that (while everything I said is true of the Itanium x86-64 ABI used on Linux and Mac) unfortunately
[Microsoft's x86-64 ABI](https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention#parameter-passing)
does _not_ pass `std::string_view` in registers, but instead passes it
by "hidden pointer," so that at the machine level, you get basically the
same codegen as if you had been passing by reference all along. Except that
actually you're passing a _copy_ by reference, so you get one extra
16-byte copy in addition to all the other stack traffic MSVC is doing.

This is a big problem baked into Microsoft's x86-64 ABI, and
nerfs a lot of the performance benefits mentioned in my post.
However, my advice remains the same:
Even if your primary platform today is Windows, you should habitually
pass `std::string_view` by value!

Let's see how, exactly, MSVC demolishes the performance gains listed in
my previous post.


## 1. Eliminate a pointer indirection in the callee?

Microsoft's ABI means that we _don't_ eliminate indirection in the callee
when we pass class types (of size `>8`) by value. [Godbolt:](https://godbolt.org/z/5cPcjdTcj)

    int byvalue(std::string_view sv) { return sv.size(); }

    int byref(const std::string_view& sv) { return sv.size(); }

    ---

    byvalue:
        mov eax, DWORD PTR [rcx+8]
        ret 0

    byref:
        mov eax, DWORD PTR [rcx+8]
        ret 0

Because Microsoft's x86-64 ABI passes by hidden pointer, we end up with the same
x86-64 code in both cases.

However, just to illustrate that this is a performance
issue with the Microsoft x86-64 ABI, and not with "Visual Studio" in general, in
the Godbolt above I also included MSVC's output for the 64-bit ARM processor.
On ARM, Microsoft follows a smarter calling convention modeled after the Itanium ABI,
and _does_ pass `string_view` in registers. So in the unlikely case that you
dual-compile for Windows on ARM, you should definitely pass `string_view` by value!


## 2. Eliminate a spill in the caller?

Microsoft's ABI means that we _don't_ eliminate the spill. [Godbolt:](https://godbolt.org/z/cWhPaYhWz)

    int byvalue(std::string_view sv);
    int byref(const std::string_view& sv);

    void callbyvalue(std::string_view sv) { byvalue("hello"); }

    void callbyref(std::string_view sv) { byref("hello"); }

    ---

    .Lhello:
        .asciz "hello"

    callbyvalue:
        sub rsp, 56
        mov QWORD PTR $T2[rsp+8], 5
        lea rax, OFFSET FLAT:.Lhello
        mov QWORD PTR $T2[rsp], rax
        lea rcx, QWORD PTR $T1[rsp]
        movaps xmm0, XMMWORD PTR $T2[rsp]
        movdqa XMMWORD PTR $T1[rsp], xmm0
        call byvalue
        add rsp, 56
        ret 0

    callbyref:
        sub rsp, 56
        lea rax, OFFSET FLAT:.Lhello
        mov QWORD PTR $T1[rsp+8], 5
        lea rcx, QWORD PTR $T1[rsp]
        mov QWORD PTR $T1[rsp], rax
        call byref
        add rsp, 56
        ret 0

You can see the extra 16-byte copy happening in `callbyvalue`: that's the `movaps/movdqa` instruction
pair. But you can also see that there's no reason for MSVC to be doing that — its optimizer
should have coalesced those temporaries! In general, this extra copy _can_ cause MSVC's codegen
for pass-by-value to be worse than the codegen for pass-by-reference; but in this _specific_
case, I'd just call this a missed optimization and file a bug against the compiler. There's no
reason either of these functions needs a 56-byte stack frame. (And notice: passing by reference
doesn't reduce the size of MSVC's stack frame! This codegen is just abymally bad and I don't
know why.) This is with `-O2`, by the way.

Again, my Godbolt includes MSVC's codegen for 64-bit ARM, showing that you _do_ get this
performance benefit when you pass by value on ARM. Even on ARM, MSVC's codegen quality is
worse than I'd expect; but the pass-by-value version does clearly win out.


## 3. Eliminate aliasing?

Having seen a sample of MSVC's optimization smarts in the previous section, you should be
unsurprised to learn that MSVC doesn't seem to feed its optimizer with aliasing information.
[Godbolt:](https://godbolt.org/z/56MvzT1qa)

    void byvalue(std::string_view sv, size_t *p) {
        *p = 0;
        for (size_t i=0; i < sv.size(); ++i) *p += 1;
    }

    void byref(const std::string_view& sv, size_t *p) {
        *p = 0;
        for (size_t i=0; i < sv.size(); ++i) *p += 1;
    }

    ---

    byvalue:
        mov r8, QWORD PTR [rcx+8]
        xor eax, eax
        mov QWORD PTR [rdx], rax
        test r8, r8
        je .Lbottom
        npad 2
     .Ltop:
        inc rax
        mov QWORD PTR [rdx], rax
        sub r8, 1
        jne .Ltop
     .Lbottom:
        ret 0

    byref:
        xor eax, eax
        mov QWORD PTR [rdx], rax
        cmp QWORD PTR [rcx+8], rax
        jbe .Lbottom
        mov r8d, eax
        npad 2
     .Ltop:
        inc r8
        inc rax
        mov QWORD PTR [rdx], r8
        cmp rax, QWORD PTR [rcx+8]
        jb .Ltop
     .Lbottom:
        ret 0

Because this is a _Visual Studio optimizer_ issue and not a _Windows x64 ABI_
issue, this one applies equally to MSVC-on-x64 and MSVC-on-ARM. The only
way to claw back this performance, as far as I know, is to switch to Clang
or GCC.

----

So, given that `string_view`'s pass-by-value codegen and its pass-by-reference codegen
are pretty much equally awful on MSVC, why do I still specifically recommend pass-by-value?

- Even on Windows, where pass-by-value doesn't gain you boatloads of performance,
    it still doesn't _lose_ any performance to speak of.
    (Yes, there's that 16-byte copy we observed in section 2; but you're
    hitting the stack either way, so it's the difference between one
    L1 cache hit or two. I don't think you'll notice, what with all the other
    stuff going on in this post.)

- Passing `string_view` by value is unambiguously the right thing to do, if your
    code is ever going to run on x86-64 Linux or Mac. You don't want to hard-code
    a bunch of pass-by-reference and then have to undo it later.

- Even if you're a 100% Visual Studio shop, passing `string_view` by value
    is unambiguously the right thing to do, if your code is ever going to run
    on ARM or any other non-x86 architecture. Microsoft's wonky ABI applies only
    to Windows on x86-64, not to Windows on other architectures.

- If you're writing (possibly open-source) library code that might one day
    be copy-pasted onto a non-Windows-x86-64 platform, then again you
    should pass `string_view` by value, so you don't have to undo it later.

- If you ever show your code to any other human, they're likely
    to ask you, "Hey, why are you passing by reference here? Wouldn't it
    be more idiomatic to pass by value?" In general, if there are two
    ways of doing something, and their performance is equivalent, but one
    way is idiomatic and the other way is idiosyncratic, you should prefer
    the idiomatic way.

To repeat the bottom line from my previous post:

- Small, trivially copyable, "parameter-only" types like C++17 `string_view`,
    C++20 `span`, and C++2b `function_ref` are explicitly designed to occupy
    the same category as `int` and `char*`. Pass them by value!
