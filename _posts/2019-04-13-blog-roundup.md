---
layout: post
title: "Fork considered harmful; paint splatters considered Perl"
date: 2019-04-13 00:01:00 +0000
tags:
  blog-roundup
  paradigm-shift
---

The other day I ran across the paper
["A `fork()` in the Road"](https://www.microsoft.com/en-us/research/uploads/prod/2019/04/fork-hotos19.pdf)
(2019) by Andrew Baumann et al. I highly recommend reading it!

> We catalog the ways in which `fork` is a terrible abstraction for the modern programmer to use,
> describe how it compromises OS implementations, and propose alternatives.
>
> As the designers and implementers of operating systems, we should acknowledge that `fork`'s
> continued existence as a first-class OS primitive holds back systems research, and deprecate it.
> As educators, we should teach `fork` as a historical artifact, and not the first process creation
> mechanism students encounter.

One sentence I really want to call out is this one:

> `fork` has lost its classic simplicity; it today impacts all the other operating system abstractions
> with which it was once orthogonal. Moreover, a fundamental challenge with `fork` is that, since it
> conflates the process and the address space in which it runs, `fork` is hostile to user-mode
> implementation of OS functionality, breaking everything from buffered IO to kernel-bypass networking.

I had never really thought about "buffered I/O" as an example of
"user-mode implementation of OS functionality." However, in a sense, it really is — and in that sense,
`fork` can really screw it up!
[This StackOverflow question](https://stackoverflow.com/questions/2530663/printf-anomaly-after-fork)
presents the C program

    #include <stdio.h>
    #include <unistd.h>

    int main()
    {
        int pid;
        printf( "Hello, my pid is %d\n", getpid() );

        pid = fork();
        if( pid == 0 ) {
            printf( "I was forked! :D %d\n", getpid() );
        } else {
            waitpid( pid, NULL, 0 );
            printf( "%d was forked!\n", pid );
        }
        return 0;
    }

and asks why `./a.out | cat` produces more lines of output than `./a.out`. The reason is that
when `fork()` copies the address space of the process, it also copies the *contents of the I/O buffers*
that live within the stdio machinery. Any output that was buffered ends up getting output by
both the parent and the child.

So that's interesting.

----

The other interesting paper I read this morning was Colin McMillen's SIGBOVIK submission
["93% of Paint Splatters are Valid Perl Programs"](http://colinm.org/sigbovik/2019.pdf)
(April 2019). I wish the paper included more reproducible details on how the paint splatters
were OCR'ed, and especially wish it showed the (possibly several) OCR'ings of the splatters
that were *not* ultimately considered valid Perl programs.

An example of an invalid Perl program (from the paper):

    fifi;%:'i1i:

And an example of a valid Perl program (from the paper):

    gggijgziifiiffif

The paper does explain that the above program (A) "by pure coincidence happens to accurately represent
the authors' verbal reaction upon learning that unquoted strings were a feature intentionally
included in the Perl language" and (B) does not in fact "lint clean" with `perl -w`:

    Unquoted string "gggijgziifiiffif" may clash
    with future reserved word at test.pl line 1.

What is this, C++?

The entire proceedings of SIGBOVIK 2019 are a godawful 225 pages and can be found [here](http://sigbovik.org/2019/proceedings.pdf).
