---
layout: post
title: "A better 404 polyglot"
date: 2020-12-23 00:01:00 +0000
tags:
  esolang
excerpt: |
  Between 2009-ish and 2018, StackOverflow's 404 page
  [displayed](https://meta.stackoverflow.com/questions/252184/whats-the-joke-in-the-stack-overflow-404-page-code)
  the following polyglot program:

      # define v putchar
      #   define print(x) ⏎
      main(){v(4+v(v(52)-4));return 0;}/*
      #>+++++++4+[>++++++<-]>⏎
      ++++.----.++++.*/
      print(202*2);exit();
      #define/*>.@*/exit()

  This program was originally due to Mark Rushakoff, who later
  [proposed a shorter version](https://meta.stackexchange.com/questions/27112/amusing-404-page-not-found-images-for-trilogy-sites#27846):
---

Between 2009-ish and 2018, StackOverflow's 404 page
[displayed](https://meta.stackoverflow.com/questions/252184/whats-the-joke-in-the-stack-overflow-404-page-code)
the following polyglot program:

    # define v putchar
    #   define print(x) ⏎
    main(){v(4+v(v(52)-4));return 0;}/*
    #>+++++++4+[>++++++<-]>⏎
    ++++.----.++++.*/
    print(202*2);exit();
    #define/*>.@*/exit()

This program was originally due to Mark Rushakoff, who later
[proposed a shorter version](https://meta.stackexchange.com/questions/27112/amusing-404-page-not-found-images-for-trilogy-sites#27846):

    # define v putchar
    #  define print(x) main(){v(4+v(v(52)-4));return 0;}/*
    #++++++++4[>++++++<-]>++++.----.++++<*/
    print(202*2)
    #undef /*>.@*/v

The claim was that this polyglot worked in "Befunge-93, Brainf*ck, Python, Ruby, Perl, and C."
(And it's also valid PHP.)
But it never really worked in Befunge-93, because all it does is `4*.@`. Apparently
Mark had used some interpreter with a sort of implicit string-mode, where `e` was a command
meaning "push the ASCII value of `e`." In actual Befunge-93 `e` is a no-op, and in Funge-98
`e` means "push hex `e`, i.e., 14," so that part of the polyglot worked _only_ in that one
non-standard interpreter.

This morning I [nerdsniped](https://xkcd.com/356/) myself
into trying to write a more aesthetically satisfying polyglot 404.
My finished product is as follows
([tio.run](https://tio.run/##PY1BCsMgFET3OYV8N@pH7SK7Wuk9QgopScBFNSRaKKVnt78uOrOamQezl/urVj4va4gL20o@xFOGmNljClHId2usQk/ufuGy7TTzweOgHSL60bnRm6uy0J96kOdPw4QATTJEoPH4F0xxZgRK2fES6bZ9WuUhpe3QoIm5uZjymmg2ytb6BQ)):

    #define puts(v)int main(){puts(/*+>+>+
    puts=print#[>+[-<+++>]<<]>.@*/"404");}
    puts(("----.++++.>++++++++++"and 404))
    #undef puts/*>"oops-"-+++^<notfound.*/

This polyglot works in Befunge-93, Brainfuck, C89, Python, and Ruby.
(No more Perl or PHP.)

----

In Brainfuck, C89, Python, and Ruby, my polyglot prints exactly `"404\n"` to
standard output. In Befunge-93, it prints `"404 "` instead — which is
unfortunate, but basically I decided that it was more interesting
to reuse the `.` character ("print ASCII" in Brainfuck,
"print integer" in Befunge) than to create two completely
different paths for Brainfuck and Befunge.

----

The Python `print` function prints a newline after whatever you give it.
The Ruby `print` function does _not_ print a newline, but
a different Ruby function, `puts`, does. (And C provides _only_ `puts`.)

I was surprised to learn that in both Python 3 and Ruby,
the following code is legal:

    puts=print
    puts(404)

In Python 3, I know what it's doing: `print` is a built-in function,
and so we're creating a variable `puts` in the current (global) scope
with initial value `print`. (Functions are more or less first-class
objects in Python.) So then when we call `puts(404)`, we're really
calling `print` with `404`.

In Ruby, I think what's happening is that `print` is a method _call_.
We're not giving it any arguments, so it prints nothing. It returns
`nil`, which we use as the initial value for a new variable named `puts`. Then
[Ruby does something weird](https://stackoverflow.com/questions/40430807/how-can-ruby-tell-the-difference-between-variables-and-method-names-if-they-have):
since you can never(?) call variables like functions, the expression
`puts(404)` is treated as a call to the global function named `puts`,
_not_ a use of the local variable named `puts`.
So when we call `puts(404)`, we're really _still_
calling `puts` with `404`!

But I don't know Ruby, so take that explanation with an enormous
grain of salt, please.

----

In both Python and Ruby, non-empty strings are truthy, and `and`
is a low-precedence operator that evaluates to its right-hand operand
if the left-hand one is truthy (which in this case it is).

----

I lazily lifted the Brainfuck representation of 52, `>+>+>+[>+[-<+++>]<<]>`,
from Esolang's page on [Brainfuck constants](https://esolangs.org/wiki/Brainfuck_constants).

I wonder how challenging it would be to write a Brainfuck "superoptimizer"
for producing specific tape configurations (e.g. `52 48`) or specific
ASCII outputs (e.g. `404`). It seems that there are reasonably well-known
algorithms for producing _single_ numbers in isolation, but I predict that
the shortest program to produce `52 48` on the tape, for example, ought
to be shorter than the sum of the lengths of programs to produce `52`
and `48` independently.

Esolang has a page on [Brainfuck code generation](https://esolangs.org/wiki/Brainfuck_code_generation),
but it's not obvious to me whether any of the links on that page are
relevant to my paragraph above.
