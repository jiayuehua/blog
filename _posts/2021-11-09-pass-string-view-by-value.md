---
layout: post
title: "Three reasons to pass `std::string_view` by value"
date: 2021-11-09 00:01:00 +0000
tags:
  c++-learner-track
  c++-style
  library-design
  parameter-only-types
---

It is idiomatic to pass `std::string_view` by value. Let's see why.

First, a little background recap. In C++, everything defaults to pass-by-value;
when you say `Widget w` you actually get a whole new `Widget` object.
But copying big things can be expensive. So we
introduce "pass-by-const-reference" as an _optimization_ of "pass-by-value,"
and we tell people to pass big and/or expensive things like `std::string` by
const reference instead of by value.

But for small cheap things — `int`, `char*`, `std::pair<int, int>`, `std::span<Widget>` —
we continue to prefer the sensible default behavior of pass-by-value.

Pass-by-value has at least three performance benefits
over pass-by-(const)-reference. I'll illustrate all three of them
via `string_view`.

> All of these code snippets are presented in isolation, showing either
> _only_ the callee or _only_ the caller. If the compiler is allowed to see
> both caller and callee together, and is given `-O2`, and decides to inline
> the callee into the caller, then it can usually undo all of the harm
> caused by an unidiomatic pass-by-reference. So, frequently, you can
> pass `string_view` by reference and get away with it. But you should
> pass `string_view` by value, so that the compiler doesn't have to do
> those heroics on your behalf. And so that your code-reviewer doesn't
> have to burn brain cells pondering your unidiomatic decision to pass by
> reference. Briefly: Pass small cheap types by value! It has only upsides!
>
> Okay, now let's see those three performance benefits I promised.


## 1. Eliminate a pointer indirection in the callee

Pass-by-const-reference means that you pass the address of the thing.
Pass-by-value means that you pass the thing itself, in registers when possible.
(If the thing you're passing is "non-trivial for purposes of ABI" — such as if it
has a non-trivial destructor — then "the thing itself" ends up being passed on
the stack, so there will be memory indirection involved either way.
But trivial types like `int` and `string_view` and `span` don't have to worry
about that; these types are passed in registers.)

Pass-by-value eliminates a pointer indirection in the callee, which means
it eliminates a load from memory. [Godbolt:](https://godbolt.org/z/zqr6TjEoG)

    int byvalue(std::string_view sv) { return sv.size(); }

    int byref(const std::string_view& sv) { return sv.size(); }

    ---

    byvalue:
        movq %rsi, %rax
        retq

    byref:
        movl 8(%rdi), %eax
        retq

In the `byvalue` case, the `string_view` is passed in the register pair `(%rdi, %rsi)`,
so returning its "size" member is just a register-to-register move. In contrast,
`byref` receives a _reference_ to a `string_view`, passed in register `%rdi`, and has
to do a memory load in order to extract the "size" member.


## 2. Eliminate a spill in the caller

When you pass by reference, the caller needs to put the thing's address into a register.
So the thing must _have_ an address. Even if everything else in the caller could have been
done with the thing in registers, the very act of passing the thing
forces the caller to spill it onto the stack.

Pass-by-value eliminates the need to spill the argument, which sometimes
means it eliminates the need for a stack frame in the caller at all. [Godbolt:](https://godbolt.org/z/dvYnW6abK)

    int byvalue(std::string_view sv);
    int byref(const std::string_view& sv);

    void callbyvalue(std::string_view sv) { byvalue("hello"); }

    void callbyref(std::string_view sv) { byref("hello"); }

    ---

    .Lhello:
        .asciz "hello"

    callbyvalue:
        movl $.Lhello, %edi
        movl $5, %esi
        jmp byvalue    # tail call

    callbyref:
        subq $24, %rsp
        movq $.Lhello, 8(%rsp)
        movq $5, 16(%rsp)
        leaq 8(%rsp), %rdi
        callq byref
        addq $24, %rsp
        retq

In `callbyvalue`, we just set up the `string_view` argument's data pointer and size member in
`%rdi` and `%rsi` respectively, and then jump to `byvalue`. In `callbyref`, on the other hand,
we need to pass the _address_ of a `string_view` argument; so we make space on the stack. And
then when `byref` returns, we need to clean up that space we made.

Previously on this blog:
["It's not always obvious when tail-call optimization is allowed"](/blog/2021/01/09/tail-call-optimization/) (2021-01-09).


## 3. Eliminate aliasing

When we pass by reference, we're passing the callee a reference to an object they know nothing
about. The callee doesn't know who else might be holding a pointer to that object. The callee
doesn't know whether any of its own pointers might point at that object (or at any of its pieces).
So the compiler must optimize the callee very conservatively, respecting those unknowns.

Pass-by-value gives the callee a brand-new copy of the object — a copy that definitely does not
alias with any other object in the program. So the callee has greater opportunities for
optimization. [Godbolt:](https://godbolt.org/z/4MbMTxvhf)

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
        movq %rsi, (%rdx)
        retq

    byref:
        movq $0, (%rsi)
        cmpq $0, 8(%rdi)
        je   .Lbottom
        movl $1, %eax
     .Ltop:
        movq %rax, (%rsi)
        leaq 1(%rax), %rcx
        cmpq 8(%rdi), %rax
        movq %rcx, %rax
        jb   .Ltop
     .Lbottom:
        retq

In `byvalue`, Clang is smart enough to see that a loop incrementing `*p` by 1,
`sv.size()` times, starting from zero, is tantamount to a simple assignment
`*p = sv.size()`. But in `byref`, Clang can't make that same leap. Why not?
Well, because `byref` needs to behave "correctly" even when called like this:

    std::string_view sv = "hello";
    size_t *size_p = &sv.__size_;  // address of sv's "size" member
    byref(sv, size_p);

In that situation, every increment of `*size_p` changes the result of `sv.size()`,
causing the loop to run forever (or rather, until the value
of `sv.__size_` wraps around to zero and halts the loop). So the loop in `byref`,
unlike the loop in `byvalue`, is _not_ equivalent to a simple assignment! The compiler
must generate machine code corresponding to its more complicated behavior.

`byvalue` doesn't have to worry about that evil caller, because there is no
(well-defined) way for the caller to pass in a _copy_ of a `string_view` alongside
a pointer that points into the interior of that copy.

In this example, we're talking specifically about the possibility
of aliasing targeting the `string_view` object's own data members,
not the characters viewed by it. Those characters, of course, _can_
be aliased by pointers elsewhere in the program; but we're not thinking
about that in this specific example. Don't let that confuse you!

----

To sum up:

- C++ passes everything by value by default.

- The pass-by-value default is optimal for small and cheap-to-copy types like
    `int`, `char*`, and `pair<int, int>`.

- Pass-by-value has at least three performance benefits, detailed above.
    But if the performance cost of making a copy outweighs all of these benefits —
    for large and/or expensive-to-copy types like `string` and `vector` —
    then prefer to pass by const reference. Pass-by-const-reference is
    an optimization of pass-by-value.

- Small, trivially copyable, "parameter-only" types like C++17 `string_view`,
    C++20 `span`, and C++2b `function_ref` are explicitly designed to occupy
    the same category as `int` and `char*`. Pass them by value!

----

See also:

* ["A footnote on 'Three reasons to pass `std::string_view` by value'"](/blog/2021/11/19/string-view-by-value-ps/) (2021-11-19)
