---
layout: post
title: 'A pro-default-constructibility experiment from 2007'
date: 2018-05-10 00:02:00 +0000
tags:
  concepts
  constructors
  rant
  science
---

Here's a short followup to my previous post
["Default-constructibility is overrated"](/blog/2018/05/10/regular-should-not-imply-default-constructible)
(2018-05-10).

Today I was reading a paper titled
["An Empirical Comparison of the Accuracy Rates of Novices using the Quorum, Perl, and Randomo Programming Languages"](http://ecs.victoria.ac.nz/foswiki/pub/Events/PLATEAU/Program/plateau2011-stefik.pdf)
(Andreas Stefik et al., _PLATEAU_, October 2011). A sufficiently cynical reader will be able to guess both the nature of the "Randomo" language
and the experimental results claimed by the authors:

> [W]e call Randomo a Placebo-language, where some of the syntax was chosen with a
> random number generator [...] We compared novices that were programming for the first time
> using each of these languages, testing how accurately they could write simple programs [...]
> Results showed that [...] Perl users were unable to write programs more accurately than those
> using a language designed by chance.

But this is a blog mostly about C++, not about some impenetrable line-noise language designed by
throwing darts at the ASCII table!

So the interesting thing for us is down in this paper's bibliography, where they reference
["Usability Implications of Requiring Parameters in Objects’ Constructors"](http://www.cs.cmu.edu/afs/cs/Web/People/marmalade/papers/Stylos2007CreateSetCall.pdf)
(Stylos and Clarke, _International Conference on Software Engineering_, May 2007).

> Quorum does not allow constructors with required parameters

Stylos and Clarke performed an experiment in 2005 where three sets of professional programmers
of different experience levels were given various tasks to perform in C++, C#, or Visual Basic
respectively.  (The mapping from expertise to programming language was hard-coded by the experimenters
in the most obviously stereotypical fashion possible.)  The experimental method is not _quite_ clear
from the paper, but as I understand it, within each group, five participants were asked to complete
their task using an unfamiliar third-party API where objects were initialized via "two-phase initialization":

    var foo = new FooClass();
    foo.Bar = barValue;
    foo.Use();

and the other five participants were asked to complete their task using an API where
objects were initialized via "one-phase initialization":

    var foo = new FooClass(barValue);
    foo.Use();

The two APIs were otherwise identical. Also, although the code snippets above come directly from the
published paper, my understanding is that the actual APIs in question used names more meaningful than
"foo", "barValue", and "use". For example:

> In Task 2, participants were asked to write code that performed the same function as the code
> in Task 1 ["Write the code they would expect would read in a file and send its contents in the
> body of an email message."], however this time using the Visual Studio IDE and real APIs.
> Participants were given a template project in which to write their code and the project was
> linked to one of two libraries, depending on the experimental condition. The libraries each
> provided APIs for File and Mail operations, the difference being that one provided only default
> constructors (taking no arguments) for each object and the other provided only required
> constructors (requiring all parameters to be provided on construction).
>
> This task was designed to compare between participants the ease of use of the create-set-call
> [two-phase construction] APIs to the required-constructor APIs.


## Experimental conclusions

The experimenters offer some thought-provoking conclusions:

> We consistently found that [programmers in the lower two groups] assumed that a default constructor
> exists for any class. This was often evident by participants writing code to call a default constructor
> and not noticing until the next line of code or two that the constructor call would not compile.
> [...] These programmers were much more likely to initially assume the compiler error resulted from
> incorrect syntax — a missing parenthesis or keyword — than a more semantic error. This often
> caused participants to doubt their own syntactic understanding of the language [...]

By way of explanation, notice that the experimenters describe their lowest-expertise tier of subjects
in these terms:

> _Opportunistic programmers_ [i.e., the lowest tier] work from the bottom up on their current task
> and do not want to worry about the low-level details. They want to get their code working and
> quickly as possible without having to understand any more of the underlying APIs than they have to.
> <b>They are the most common persona</b> [emphasis added] and prefer simple and easy-to-use languages [...]

So of course they'll love an API that allows `foo = new FooClass;` to compile! Such an API allows them
to more quickly [move on from the boring activity of _writing_ code](https://youtu.be/D7Sd8A6_fYU?t=40m55s)
(the famous "code complete" milestone) to the more interesting and time-consuming activity,
which of course is _debugging until it works_. (Testing and documenting are merely optional side quests.)

Some of the experimenters' conclusions betray antipatterns in their experimental setup:

> Though we found required constructors to be less usable when _creating_ code, we did not find the
> same to be true when participants _debugged_ code. Even when code used ambiguous constructor
> parameters such as `true, true`, programmers did not a have significantly harder time debugging
> this code compared with seemingly more self-descriptive code like `obj.sharing = true; obj.caching = true;`.
> This was because all of our participants used IDE features like code-completion to easily access
> constructor parameter information when it was not directly visible in the code.

In other words, the experimenters' self-designed APIs used spaghetti-messes of boolean parameters,
which is a known antipattern in every language I'm familiar with. Consider:

    class FooClass {
        bool Bar;
        bool Baz;
        bool Quux;
    public:
        FooClass(bool bar, bool baz, bool quux) :
            Bar(bar), Baz(baz), Quux(quux) {}
    };

    FooClass foo(true, false, true);

If the experimental subjects were being forced to use APIs like this, honestly I'm surprised
that the experimenters' conclusions were not _more strongly_ in favor of two-phase initialization!
They were essentially using two-phase initialization as a poor man's version of
Python's [keyword arguments](https://stackoverflow.com/questions/1419046/python-normal-arguments-vs-keyword-arguments):

    # keyword arguments: supported in Python but not in C++
    foo = FooClass(
        Bar=True,
        Quux=True
    )

    # two-phase initialization is syntactically similar,
    # just with a little more repetition of the word "foo"
    # and a slightly different placement for the parens
    foo = FooClass()
    foo.Bar = True
    foo.Quux = True

For the specific case of boolean parameters, C++ codebases such as LLVM have a well-established
idiom that eliminates the spaghetti:

    enum FooOptions : uint8_t {
        FO_BAR = 0x1,
        FO_BAZ = 0x2,
        FO_QUUX = 0x4,
    };

    class FooClass {
        bool Bar;
        bool Baz;
    public:
        FooClass(FooOptions o) :
            Bar(o & FO_BAR), Baz(o & FO_BAZ),
            Quux(o & FO_QUUX) {}
    };

    FooClass foo(FO_BAR | FO_QUUX);


## Why default-construction?

The most relevant conclusions show up in section 4.9, the results of sit-down interviews with
the experimental subjects after the experiment. Quoted in full here:

> In the post-task interviews, nearly all of the participants expressed a preference
> for the create-set-call [two-phase initialization] pattern. Following are some of the
> justifications they gave for their preference.
>
> - <b>Initialization flexibility:</b> By allowing objects to be created before all the property
> values are known, create-set-call allows objects to be created in one place and initialized
> somewhere else, possibly in another class or package. This was a common justification given
> by pragmatic [i.e., mid-expertise] programmers.
>
> - <b>Less restrictive:</b> In general, APIs should let their consumers decide how to do things,
> and not force one way over another.
>
> - <b>Consistency:</b> Most APIs have default constructors, and so people will expect them.
> This reason was given by two programmers who created APIs that were used by other members
> of their teams.
>
> - <b>More control:</b> Several systematic [i.e., high-expertise] programmers cited the fact
> that create-set-call let them attempt to set each property individually and deal with any
> errors that might come up using return-codes, while constructors only allowed for exceptions.

These are all quite valid and familiar arguments in favor of giving your C++ types a default constructor.
For the sake of argument, I'll construct my most weaselly counter-arguments for each one:

- <b>No mutable aliases:</b> By disallowing objects to be created before all the property values
  are known, we prevent the common antipattern of creating an object in one place and then
  passing the partially formed object as an "in-out parameter" to the place that really knows
  how to construct it. Better to let that place do the actual construction itself, and return
  by value. (Notice that this advice has changed since the experiment was performed in 2005!)

- <b>More restrictive:</b> In general, APIs should be
  [easy to use and hard to misuse](https://lwn.net/Articles/275780/) — see also
  [Ben Deane's session of that title at C++Now 2018](https://cppnow2018.sched.com/event/EC7i/).
  If one way is _more correct_, we should funnel our users into that way and disallow anything
  more error-prone.

- <b>Everyone else is doing it:</b> Well, this was the point of
  [my previous rant about `Regular`](/blog/2018/05/10/regular-should-not-imply-default-constructible).
  "Because everyone else is doing it" is a hard argument to overcome; the only way I know to
  overcome it is to [refute the major premise](/blog/2018/05/02/trivial-abi-101) by training
  everyone else to do the right thing instead!

- <b>Error handling:</b> It's true, error handling is a stumbling block, and another valid reason to
  fall back on two-phase initialization for _certain_ "business-logicky" types. In exception-free code,
  operations that can fail _must not_ be expressed as constructors, and usually also _must not_ be
  expressed as overloaded operators, because there's no way for something like `Path("/foo/bar")`
  or `path1 += path2` to signal error. You'd instead want to express them as a noexcept default constructor
  and some noexcept member functions `bool Path::try_assign(const char *)`, `bool Path::try_append(Path)`.
  (It is unclear to what extent the programming tasks in this experiment were concerned with
  exception-safety, and whether exception-unsafety was counted by the experimenters as a source
  of "error.")

So, this is a pretty neat paper. I wish they'd published more of their actual experimental setup
so that the results could be [reproduced](http://blogs.discovermagazine.com/neuroskeptic/2018/01/02/is-reproducibility-central-science/),
and I believe those results must have been greatly influenced by the C++03-era idioms they were using.
But their face-to-face interviews with real programmers turned up good talking points.

I'm still not a fan of default constructors.
