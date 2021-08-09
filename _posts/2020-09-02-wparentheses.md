---
layout: post
title: 'Two musings on the design of compiler warnings'
date: 2020-09-02 00:01:00 +0000
tags:
  c++-style
  compiler-diagnostics
  language-design
  rant
---

Today I finally write down some musings on two peripheral aspects of
compiler diagnostic design that I think are very important for certain kinds of warnings.
My first musing relates to _fixits_, and my second to _suppression mechanisms_.


## What is a fixit? ##

Most C++ compilers these days don't just tell you _that_ a problem occurred
(like [`ed`'s famous `?` prompt](https://www.gnu.org/fun/jokes/ed-msg.txt)),
and _what_ the problem is ("expected identifier," for example).
They'll try to tell you _how to fix_ the problem.

Compare GCC 4.1's error messages with GCC 4.6's. ([Godbolt.](https://godbolt.org/z/KM8P9E))
For this input:

    struct S {}
    struct T {};

GCC 4.1 complains merely that you have put `multiple types in one declaration`.
GCC 4.6 more helpfully complains that it `expected ';' after struct definition`,
and points to the exact position where it believes a semicolon should be inserted.

GCC 7 goes even further, and suggests a _fixit_ — a machine-readable "spellcheck suggestion"
that some IDEs are able to apply with a single click.

    test.cpp:1:12: error: expected ';' after struct definition
     struct S {}
                ^
                ;

In medical terms, the compiler has gone from simply detecting the _symptom_
("this declaration seems to contain two types, which isn't valid C++")
to producing a _diagnosis_ of the disease ("I bet you forgot a semicolon somewhere");
and a compiler that supports fixits is actually _prescribing a treatment plan_.
Each of these steps is a quantum leap beyond the previous one in terms of both user-friendliness
and implementation complexity.


## What is a suppression mechanism? ##

Suppression mechanisms relate to compiler _warnings_, not compiler errors.
Consider the following snippet ([Godbolt](https://godbolt.org/z/fTnao9)):

    if (argc = 1)
        puts("hello");

Every mainstream compiler warns on this suspicious-looking code (GCC and Clang with `-Wall`; MSVC with `-W4`).
Here's GCC's diagnostic:

    warning: suggest parentheses around assignment used as truth value [-Wparentheses]
        4 |     if (argc = 1)
          |         ~~~~~^~~

Notice that GCC doesn't even bother to explain really what its diagnosis is;
it emits a passive-aggressive warning message that does nothing more than suggest how to
shut itself up!

The diagnosis, of course, is that frequently people mean to write `if (argc == 1)`
but their finger slips and they write `if (argc = 1)` instead, which unfortunately
is also valid C++ code. Every mainstream compiler has evolved the same way to deal with this:
if you really mean `=`, then you write an extra pair of parentheses around the expression, like so:

    if ((argc = 1))

The extra pair of parentheses is what I'm calling a "suppression mechanism."
It's just an arbitrary way of silencing a warning diagnostic — telling the compiler
"no, I really _meant_ to write this legal but unusual construct."

Another well-known suppression mechanism is writing `(void)` in front of unused results
to silence `-Wunused-value` and `-Wunused-result` warnings ([Godbolt](https://godbolt.org/z/WdrhrE)):

    [[nodiscard]] int foo();
    void test(int x) {
        (void)x;
        (void)foo();
    }

And a trivial suppression mechanism is implied by the name of the `-Wparentheses` warning:

    warning: '&&' within '||' [-Wlogical-op-parentheses]
        return (a && b || c);
                ~~^~~~ ~~

Here the compiler just wants you to add a pair of parentheses to clarify the precedence for the reader.


## Musing: A single fixit must preserve (actual) behavior or (likely) intent, but can't do both ##

It's easy for the programmer to "fix" diagnostics like `-Wunused-result` and `-Wlogical-op-parentheses`,
because they're just asking the programmer to clarify intent that is already pretty clear:
_Yes_, I mean to discard this result. _Yes_, I mean to evaluate `(a && b) || c`.

The more interesting diagnostics are the ones where the compiler believes it has identified a mismatch
between the code's actual _behavior_ and the programmer's _intent_. Our `if (argc = 1)` example
above was like that. A more realistic example might be something like

    return (a < b < c);

Here it is almost certain that the programmer intended to write

    return (a < b && b < c);

However, the literal behavior of the code as written is equivalent to

    return ((a < b) < c);

When the compiler suggests a fixit for this warning, it has to choose: Should we suggest the fix
that changes the behavior of the code to what you probably meant to type — the "spellcheck" option?
Or should we show how to silence the warning while preserving the existing code's behavior — the
"Add To Dictionary" option, if you will? (Or, continuing our medical analogy, there's the "treatment"
option and the "[DNR](https://en.wikipedia.org/wiki/Do_not_resuscitate)" option.)

Showing just the "treatment" option puts the compiler in the awkward position of "recommending" that you change
the behavior of code that is, after all, perfectly legal C++ already; blindly applying the fixit to
working code might break that code. But showing just the "suppression" option might encourage a programmer to
blindly apply the fixit to _broken_ code, thus leaving the bug in place but making it harder to detect
in the future.

For certain kinds of warnings, Clang shows _both_ fixits. Two examples of this in Clang 10
are ([Godbolt](https://godbolt.org/z/aWf9s6)):

    warning: using the result of an assignment as a condition without parentheses [-Wparentheses]
        if (x = foo())
            ~~^~~~~~~
    note: place parentheses around the assignment to silence this warning
        if (x = foo())
              ^
            (        )
    note: use '==' to turn this assignment into an equality comparison
         if (x = foo())
              ^
              ==

and

    warning: & has lower precedence than !=; != will be evaluated first [-Wparentheses]
        return (foo() & mask != 0);
                      ^~~~~~~~~~~
    note: place parentheses around the '!=' expression to silence this warning
        return (foo() & mask != 0);
                      ^
                        (        )
    note: place parentheses around the & expression to evaluate it first
        return (foo() & mask != 0);
                      ^
                (           )

Notice that in both cases, Clang decides to print the "suppression" option first
and the "treatment" option second.
But sometimes Clang prints the "treatment" option first and the "suppression" option second:

    warning: size argument in 'strncmp' call is a comparison [-Wmemsize-comparison]
        return strncmp(a, b, len < 0);
                             ~~~~^~~
    note: did you mean to compare the result of 'strncmp' instead?
        return strncmp(a, b, len < 0);
               ^                    ~
                                )
    note: explicitly cast the argument to size_t to silence this warning
        return strncmp(a, b, len < 0);
                             ^
                             (size_t)( )

and

    warning: logical not is only applied to the left hand side of this comparison [-Wlogical-not-parentheses]
        return x == y && !x == z;
                         ^  ~~
    note: add parentheses after the '!' to evaluate the comparison first
        return x == y && !x == z;
                         ^
                          (     )
    note: add parentheses around left hand side expression to silence this warning
        return x == y && !x == z;
                         ^
                         ( )

Of course, there are many more cases where Clang emits _only_ the "suppression" option
as a fixit, leaving the programmer to figure out the "treatment" on their own. (GCC 10.2
emits a fixit for only the last of these four examples; and it's the "suppression" option.)

Even in the absence of any machine-readable fixits, the phrasing of the warning message
itself can induce the human reader to think in terms of suppression or in
terms of treatment. A warning message can be phrased as "Please confirm that you meant X";
or "You did X; did you mean Y?"; or even as "Your attempt to do Y failed."


## Musing: Suppression mechanisms are about edit distance, and about signaling ##

Compiler warnings of the kind we're discussing here are basically of the form
"You wrote X, but I think you meant Y." This happens only when X and Y are
in some sense _close together_. Sometimes the "closeness" is semantic, not syntactic
(as when the programmer means to invoke copy elision but writes `return std::move(x)` instead).
But for our purposes today, let's just think about _syntactic_ closeness. You meant
to write Y, but a minor typographical slip caused you to write X. In the examples above,
the slips are things like "omit one `=`," or "omit a pair of parens" or
"put `< 0` inside the parens instead of outside."

Take X="equality-compare `!a` to `b`" and Y="negate the sense of `a == b`."
There are some pieces of code that clearly intend to express X, such as `(!a)==(b)`.
There are some that clearly intend to express Y, such as `a != b`.
But you wrote `(!a==b)`, which falls into a gray area: it's not clear which of
X and Y you really intended to express.

The job of the compiler-diagnostic-developer is to create some separation between
the space of inputs that the compiler considers "clearly X"
and the space of inputs that the compiler considers "clearly Y."
Essentially, we create an error-correcting code by deliberately increasing the
[edit distance](https://en.wikipedia.org/wiki/Hamming_distance)
between pairs of inequivalent C++ programs — deliberately increasing the number of keystrokes
the programmer would have to screw up in order to transform a working (and warning-free)
C++ program into an inequivalent broken (yet warning-free) C++ program.

Furthermore, in cases where Y is more commonly intended than X, it should be relatively _easier_
to write Y than X. In our example that's true even at the core-language level: `a != b` is
already easier to write than `!a == b`. But when we increase the distance between X-space and Y-space,
we shrink X-space by more than we shrink Y-space. If you really intend to express `!a == b`, we'll force you
to retreat all the way to `(!a) == b`. This is kind of analogous to the
[signaling principle](https://en.wikipedia.org/wiki/Signalling_(economics))
in economics or evolutionary biology: Basically, if you want the compiler to accept your intent,
your code must adopt some cumbersome and frankly maladaptive ornamentation in order to _prove_
its worthiness to the compiler.

According to the prevailing theory of
[sexual selection in the animal kingdom](https://en.wikipedia.org/wiki/Handicap_principle),
when a peahen is deciding whether to accept
a particular peacock, she uses his [ornamental](https://en.wikipedia.org/wiki/Biological_ornament)
tail as a proxy for his success. A peacock with a big
cumbersome tail _must_ be healthy and successful, and thus a good mate for the peahen. A peacock
with an unremarkable tail won't merit mating with.

When the C++ compiler is deciding whether to accept a particular program without complaint,
it uses certain ornamental syntactic flourishes as a proxy for the code's intent.
A questionable snippet that adopts those flourishes — say, `if ((argc = 1))` with an extra set
of parentheses, or `(void)x;`, or `strncmp(a, b, size_t(len < 0))` — _must_ be intentional,
and thus a good match for the compiler. A questionable snippet that lacks any flourishes — say,
`if (argc = 1)` or `x;` or `strncmp(a, b, len < 0)` — won't merit suppressing the warning.

----

Here's this musing's payoff: When deciding whether to warn on a sketchy construct, it's
tempting to encode absolute rules such as "Don't warn on `(argc = 1)` if it's surrounded by a pair of
parentheses." But we should instead think in terms of _relative_ rules — rules that compare the
"degree of ornamentation" of what the programmer actually wrote versus the "degree of ornamentation"
of the code they would have written without their (hypothetical) typo. [Observe:](https://godbolt.org/z/Gq9Kd6)

    if ((argc == 1) || (argc = 2)) {
        puts("Wrong number of arguments");
    }

As of September 2020, neither Clang nor GCC catches this single-character typo. They see that
the subexpression `(argc = 2)` is parenthesized, which suffices to suppress their warning.

I think what they _should_ do is compare the program as written to the "typo-corrected" program

    if ((argc == 1) || (argc == 2)) {
        puts("Wrong number of arguments");
    }

That's obviously a plausible and unremarkable program, so the likelihood is high that the programmer
did indeed typo `=` for `==`.
If the programmer really wants to silence the warning in this case, I think the programmer should
be forced to write

    if ((argc == 1) || ((argc = 2))) {
        puts("Wrong number of arguments");
    }

Now the hypothetically "typo-corrected" program would be

    if ((argc == 1) || ((argc == 2))) {
        puts("Wrong number of arguments");
    }

which is remarkable and implausible — it's clearly got an ornamental excess of parentheses!

To put this yet another way: The compiler should separate the space of acceptable programs into
"those that use `=` with a remarkably high number of parentheses" and "those that use `==` with
an unremarkable number of parentheses." Then, for purposes of warning diagnostics,
the compiler should essentially treat `=` and `==` as synonymous; it can rely on the number
of parentheses to indicate the programmer's intent.

    x == 2;  // oops, likely meant x = 2
    bool help1 = argc = 1;  // oops, likely meant argc == 1 because that would be reasonable
    bool help2 = (argc = 1);  // oops, likely meant (argc == 1)
    bool help3 = ((argc = 1));  // OK: clearly didn't mean ((argc == 1))
    int i1 = (argc = 1) ? 1 : 2;  // oops, likely meant (argc == 1)
    int i2 = ((argc = 1)) ? 1 : 2;  // OK: clearly didn't mean ((argc == 1))

As of September 2020, [Clang 10.1 and GCC 10.2](https://godbolt.org/z/qv8TPx) don't do very well on these test cases.

----

This post was inspired by the mailing-list thread
["[cfe-dev] parentheses flag warning"](http://lists.llvm.org/pipermail/cfe-dev/2020-May/065473.html)
(May 2020).
