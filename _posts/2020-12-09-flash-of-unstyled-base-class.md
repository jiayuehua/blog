---
layout: post
title: '"Flash of unstyled base class"'
date: 2020-12-09 00:02:00 +0000
tags:
  classical-polymorphism
  concurrency
  pitfalls
  war-stories
---

This week I ran into an interesting bug in the unit tests for some
multithreaded code. The setup for the test involved a
"process status daemon" that (in production) polled some global state
once per second, and also provided some auxiliary functions that we
wanted to test. The "once per second" code came from an old and
widely-used internal helper library. In this retelling, I've taken
the liberty of updating it for C++20:

    class OncePerSecond {
        std::thread t_;
        std::binary_semaphore sem_{0};
        bool stopped_ = false;

        virtual void do_action() { }

    public:
        explicit OncePerSecond() {
            t_ = std::thread([this]() {
                do {
                    do_action();
                } while (!sem_.try_acquire_for(std::chrono::seconds(1)));
            });
        }
        void stop() {
            if (!std::exchange(stopped_, true)) {
                sem_.release();
                t_.join();
            }
        }
        virtual ~OncePerSecond() { stop(); }
    };

To use this `OncePerSecond` gadget, you derive from it and override
the `do_action` method to do whatever-it-is that you want to do.
For example:

    int *production_state;

    class StatusDaemon : public OncePerSecond {
        std::map<int, int> config_;
        void do_action() override {
            printf("The global state is %d\n", *production_state);
        }
    public:
        int config_size() const { return config_.size(); }
    };

Creating an instance of `StatusDaemon` kicks off a background thread
that prints "The global state is..." every second.
(In production, of course, you'd initialize `production_state` to
point to something before you created your `StatusDaemon` object.)
Destroying the `StatusDaemon` object cleans up its thread.


## The segfaulting unit test

Now, one of the [GTest](https://github.com/google/googletest#googletest)
unit tests for this component was segfaulting. So I took a look at it:

    class NeuteredStatusDaemon : public StatusDaemon {
        void do_action() override { }
    };

    TEST(StatusDaemon, config_starts_empty) {
        NeuteredStatusDaemon sd;
        EXPECT_EQ(sd.config_size(), 0);
    }

Maybe one in every 100 or 1000 times, this test segfaults!
Do you see the problem?


## The diagnosis

The author of this unit test clearly knew that `StatusDaemon::do_action()` was
going to be problematic. That function depends on `production_state` to be
initialized to its production value, [which in our case we have not got](/blog/2018/06/17/piling-swivel/).
So the author added `NeuteredStatusDaemon`, which is just like `StatusDaemon` except
that its `do_action` is a no-op.

But gdb shows that the segfault is happening inside `StatusDaemon::do_action()`!
See, even though `StatusDaemon`'s `do_action` was overridden, there are still
a couple of points where that overridden virtual method is still callable:

* During construction: while the object under construction is still constructing
    its `StatusDaemon` base, but before it's begun constructing its
    `NeuteredStatusDaemon` parts.

* During destruction: after it's already destroyed its `NeuteredStatusDaemon` parts,
    but before it's begun destroying its `OncePerSecond` parts.

Observe that `stop` is called only from the `OncePerSecond` destructor.
_This is a fatal flaw!_ It means that the worker thread (the one who runs
`do_action`) is not told to stop until after most parts of the `NeuteredStatusDaemon`
have already been destroyed. So, the worker thread may in fact access the vptr
of `sd` during the brief window of time in which `sd` (dynamically speaking) is
no longer a `NeuteredStatusDaemon`, and is not yet a `OncePerSecond`, but is — very
briefly and fatally — a production `StatusDaemon`.

Sidebar: `OncePerSecond` implements `do_action` as a no-op,
so if the worker thread happens to call `do_action` during the relatively larger window
during which `sd` is (dynamically speaking) a `OncePerSecond` object, that's not likely
to cause a problem in practice. But it is still a data race on the vptr, and therefore
still undefined behavior!

> If a method must be overridden in every derived class, make it pure virtual.
> Don't violate this guideline just to "fix a segfault"; understand the segfault first!

The bug is easier to reproduce if we make `OncePerSecond::do_action` pure virtual.


## The solution

The solution to our crashing unit test is to make sure that `NeuteredStatusDaemon`
calls `stop` from its destructor, before starting to destroy its members and bases.

    class NeuteredStatusDaemon : public StatusDaemon {
        void do_action() override { }
        ~NeuteredStatusDaemon() override { stop(); }
    };

In fact, we must audit _every_ class derived from `OncePerSecond` to make sure that
the most-derived class's destructor calls `stop`. For example, even though our
production `StatusDaemon` generally works in practice, it still has undefined behavior
as far as C++ is concerned, because of the data race between `~StatusDaemon()`'s
write to the vptr and the worker thread's read from the vptr. We should explicitly
define `~StatusDaemon()` to call `stop`, as well.

I have not figured out any clever pattern to enforce "you must always override X in the
most-derived class." Honestly, I suspect that `OncePerSecond` is simply a bad fit
for classical inheritance.


## Explain the title please

The UB here is essentially a C++ version of the web-design phenomenon known as
["Flash of Unstyled Text"](https://en.wikipedia.org/wiki/Flash_of_unstyled_content) (FOUT).
In a FOUT, the browser might start rendering the text of a page (one might say the
"base functionality" of the page) before having loaded all the necessary JavaScript and
fonts and so on (one might say the "more derived" parts). This might result in a brief
window of time during which the base functionality is visible — producing potentially
surprising and unintended results for anyone who happens to glance at their screen right then.

Web pages usually care about such a "flash" happening during the web
equivalent of _construction_. The serious problem we observed with `NeuteredStatusDaemon`
surfaced during _destruction_.

UPDATE, 2020-12-10: Reddit commenter "goranlepuz" points out that our constructor also has UB:
the first call to `do_action` races with the construction of the `StatusDaemon`
parts of the object. This is partly because I oversimplified — our real code uses a
separate `start` method to kick off the thread — but to be honest, (1) I didn't realize
that my rewrite was introducing that extra bug, and (2) it's not obvious who should be
responsible for calling `start`! Probably the best way to fix our issue in this case
is to pull out a `start` method and then ensure that our unit test _does not call it._


## Putting it all together

The complete code for this example (including a C++14-friendly
rewrite of `OncePerSecond`) is [here](/blog/code/2020-12-09-flash-of-unstyled-base-class.cpp).

Compile with `-std=c++14` (or `-std=c++20 -DUSE_CXX20` if your compiler
provides the `<semaphore>` header already), and `-lgtest -lgtest_main` plus
whatever `-I -L` options you need for your local install of GTest.

Running with `./a.out --gtest_repeat=-1` should eventually reproduce the
segfault — at least it does on my machine!
