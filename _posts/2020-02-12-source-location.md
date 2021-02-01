---
layout: post
title: 'How to replace `__FILE__` with `source_location` in a logging macro'
date: 2020-02-12 00:01:00 +0000
tags:
  c++-style
  pearls
  preprocessor
---

Today on the std-proposals mailing list, I posted what I understand to be the
current best-practice idiom for using C++2a's `std::source_location`. I'm repeating
it here, so that I have something to link people to when they ask.

There is basically only one reason real code wants the "source location" (file and
line number): it's going to print it out in a programmer-readable log message.

> You might say, "What about capturing it into an exception that I'm about to throw?"
> In that case, what you want is not a single source location but a _stack trace_.
> C++2a won't be able to help you there. You must keep using your existing idiom, which
> is likely based on [`backtrace(3)`](http://man7.org/linux/man-pages/man3/backtrace.3.html).

First, here's the "old" (C++03 through forever) idiom that we're trying to replace.
([Godbolt.](https://godbolt.org/z/4GzxtV))

    struct OldDebugStream {
        template<class T>
        OldDebugStream& operator<<(T msg) {
            std::cout << msg;
            return *this;
        }
    };
    OldDebugStream ods_;
    #define ods ods_ << __FILE__ << ":" << __LINE__ << ":"

    int main()
    {
        ods << "Hello world! " << 42 << "\n";
    }

Easy peasy lemon squeezy. Everyone does this today. You might be familiar with this idiom
from [Google Glog](http://rpg.ifi.uzh.ch/docs/glog.html) or
[Boost.Log](https://www.boost.org/doc/libs/1_72_0/libs/log/doc/html/log/tutorial.html).

    LOG(ERROR) << "Hello world" << 42;  // Glog
    BOOST_LOG_TRIVIAL(error) << "Hello world" << 42;  // Boost.Log

Mind you, their macros also do useful things besides capture `__FILE__` and `__LINE__`;
for example, they can prevent codegen entirely for log messages below a certain compile-time-configured level.


## The new idiom

First, you have to know that the way to get the current source location is to write
`std::source_location::current()`. That's a lot of typing — even more than `__FILE__` and `__LINE__`!
You could (but this is a bad idea) merely "update" your macro to

    #define ods ods_ << std::source_location::current().file_name() << ":" \
                     << std::source_location::current().line() << ":"

But you're trying to get rid of the macro altogether. (Why? I don't know. Personally, I like macros.)
So the next thing to know is that `source_location::current()` is magic in C++2a
such that if you put it in a defaulted function parameter, it'll be evaluated in the caller's
context rather than at the point where the expression syntactically appears in the code.
This is novel as far as I know. I don't know how to get any other function to behave this
way. Notably, if you wrap `source_location::current()` in a helper function, it loses this
behavior — even if the helper function is `consteval`! ([Godbolt.](https://godbolt.org/z/ix3FnU))

So we need a function that can take a defaulted function parameter.
But the only functions we ever called in our old idiom were named `operator<<`, and
`operator<<` can't take defaulted parameters! How can we insert a new function call, without
making it look like a function call at the call-site?

Answer: We can insert a new _implicit conversion_. An implicit conversion from
`NewDebugStream` to `NewDebugStream::Annotated` will call the constructor of
`NewDebugStream::Annotated`, and that constructor can take defaulted parameters.
Let's see it in action ([Godbolt](https://godbolt.org/z/NbLvBh)):

    using SourceLoc = std::source_location;

    struct NewDebugStream {
        struct Annotated {
            /*IMPLICIT*/ Annotated(NewDebugStream& s,
                      SourceLoc loc = SourceLoc::current())
            {
                *this << loc.file_name() << ":" << loc.line() << ":";
            }
        };
        template<class T>
        friend Annotated operator<<(Annotated a, T msg) {
            std::cout << msg;
            return a;
        }
    };
    NewDebugStream nds;

    int main()
    {
        nds << "Hello world! " << 42 << "\n";
    }

Q.E.D., we [_can_](https://www.reddit.com/r/tipofmytongue/comments/2limc0/)
use `std::source_location` to replace macros in our logging idioms,
without changing our call-sites.
