---
layout: post
title: '_Contra_ implicit declarations of struct types'
date: 2018-05-16 00:01:00 +0000
tags:
  language-design
  pitfalls
  rant
---

Today one of my coworkers ran into this snippet of code:

    namespace N {
        struct sockaddr *Foo();
    }

    #include <sys/socket.h>

    int main() {
        struct sockaddr *x = N::Foo();
    }

[Clang complains:](https://wandbox.org/permlink/CzJwe8RvryFzqnbw)

    prog.cpp: In function 'int main()':
    prog.cpp:8:33: error: cannot convert 'N::sockaddr*' to 'sockaddr*' in initialization
         struct sockaddr *x = N::foo();
                                     ^
    prog.cpp:2:12: note: class type 'N::sockaddr' is incomplete
         struct sockaddr *foo();
                ^~~~~~~~

Moving the `#include` above `namespace N` makes the compiler Do The Right Thing, of course.
The question is, why do we allow the compiler to do the wrong thing in the first place?

Wouldn't it be nice if Clang produced a warning any time you implicitly declared a
brand-new struct type inside the declaration of some larger entity, such as a function
or variable declaration? It's a cinch you didn't *mean* to do that; yet compilers
currently accept it without question.
