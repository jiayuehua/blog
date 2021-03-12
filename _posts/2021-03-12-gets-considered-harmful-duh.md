---
layout: post
title: "C's removal of `gets` could have been done better"
date: 2021-03-12 00:01:00 +0000
tags:
  compiler-diagnostics
  war-stories
---

The other day, someone filed [a bug against uthash](https://github.com/troydhanson/uthash/issues/228)
(a C library I maintain) claiming that the provided example code segfaults in C11.
(Keep in mind that this example code hasn't been significantly updated since the 1990s.)
Fortunately, the segfault ended up not having anything to do with uthash proper.
Instead, it boiled down to [this](https://godbolt.org/z/Pd8ojo):

    #include <stdbool.h>
    #include <stdio.h>
    #include <string.h>

    int main() {
        char buf[10];
        printf("Reading...\n");
        fflush(stdout);
        bool r = (strcmp(gets(buf), "hello") == 0);
        printf("hello was %sread.\n", r ? "" : "not ");
        fflush(stdout);
    }

This is perfectly valid, if deprecated, C99 code.
(Replace `bool` with `int` and it would be perfectly valid C89 code.)
But of course it is not _safe_ code.

See, `gets` is the opposite of `puts`.
`puts(p)` reads characters from `p` and prints them to stdout
until it encounters a null byte `'\0'`; then prints a newline and stops.
`gets(p)` scans characters from stdin and writes them into `p`
until it encounters a newline; then writes a null byte `'\0'` and stops.
The number of bytes printed by `puts` is completely controlled by whoever
controls your process's address space; this is fine.
The number of bytes written to memory by `gets` is completely controlled
whoever's in charge of stdin — i.e., the user of the program. By using
`gets`, you're basically giving the user permission to overwrite the
entire address space of your process, starting from `p`.

So: `gets` is _fundamentally_ unsafe. It's not just that it can be
misused (the way, say, `strcpy` or C++'s `std::copy` can be misused);
it's that it _cannot be used at all_ without giving the user control
over your process.

Therefore, the function `gets` was removed from the C standard library in 2011.
The C11 version of `<stdio.h>` no longer contains a declaration for `gets`.

Unfortunately, C lacks C++'s "declare-before-use" rule! In C, it is
perfectly valid to call `gets` with no declaration in scope. The compiler
will simply assume that the undeclared function returns `int` (and takes
whatever parameter type you passed to it). Of course a good compiler will
also print a warning diagnostic — but it _won't_ give a hard error!

Also, in C, implicit conversions between `int` and `char*` (or any integer
type and any pointer type) are allowed. Again, a good compiler will print
a warning diagnostic; but it _won't_ reject the conversion, the way it
would in C++.

Finally, recall that C (like C++) is a separate-compilation language.
We'll compile `main.c` into `main.o`, and then the linker will link
`main.o` against `libc.a` to produce a finished executable.
Notice I did not say `libc99.a` or `libc11.a`! In practice, there is
only one libc for a given platform: it includes symbols for
the superset of all supported C dialects. So `libc.a`
will provide a linker symbol for the `gets` function, even though
`gets` was removed from C11's `<stdio.h>`.

So, in C11, we have the perfect storm: Calling `gets` is still valid C11,
because it is legal to call a function with no prototype in scope.
The compiler will simply assume it returns `int` (not `char*`). Furthermore,
the compiler will happily generate code to convert that 32-bit
`int` return into a 64-bit pointer value. And then the linker will
happily link the program, because `gets` still exists in libc.
The result is that where a C99 compiler produces this codegen
for `strcmp(gets(buf), hello)`:

    call gets
    movq %rax, %rdi    # preserve all 64 bits
    leaq hello(%rip), %rsi
    call strcmp

a C11 compiler instead produces:

    call gets
    movslq %eax, %rdi  # sign-extend from 32 bits
    leaq hello(%rip), %rsi
    call strcmp

This of course produces an immediate segfault inside `strcmp`. (Well, unless
the high 33 bits of your original buffer's address happened to be all-zero
or all-one; but that never happens in practice.)
By removing `gets` from `<stdio.h>`, C11 turned a "code execution
vulnerability" into a "denial of service." Instead of giving the
_possibility_ of a buffer overflow, now it gives the _certainty_
of a segfault!


## Don't ignore compiler warnings

This perfect storm is possible only if you ignore compiler warnings.
(Notice that [my Godbolt example](https://godbolt.org/z/Pd8ojo) used `-w`
to hide all the warnings.) You should never, ever ignore a
compiler warning! In fact, you should use `-Wall -Wextra` (on GCC or Clang)
or `-W4` (on MSVC) and fix everything it complains about.
(But see ["Don't put `-Weverything` in your build flags"](/blog/2018/12/06/dont-use-weverything/)
(2018-12-06).)

Also, obviously, don't use `gets`! (The particular interestingness
that motivated this blog post would never have happened if not for
the uthash example code's use of `gets`.)

But also, hey WG14, be aware that your chosen method of removing things
from C11 didn't work out as intended?

Perhaps C could have used something like C++'s `=delete`.

    char *gets(char*) = delete;

would pretty clearly convey that the library author knows what
you're trying to do, and what you're trying to do is _wrong_. (Although,
sadly, C++ `=delete` doesn't provide any way for the author to indicate
_why_ it's wrong or what you should try instead.)

When `gets` disappears from `<stdio.h>`, the burden falls on the
_compiler vendor_ to try to figure out what you meant. For example,
GCC says

    warning: implicit declaration of function 'gets';
        did you mean 'fgets'?

Having a way to mark a library signature as "gone for a reason"
would let the compiler determine unambiguously — and inexpensively! —
that you were trying to use a function that is now gone, and fail
at compile time, rather than generate a program that doesn't work.
