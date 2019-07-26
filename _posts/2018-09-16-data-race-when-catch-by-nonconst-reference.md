---
layout: post
title: 'Data race when catching by non-const reference'
date: 2018-09-16 00:01:00 +0000
tags:
  c++-style
  copy-elision
  exception-handling
---

People say "always catch exceptions by `const` reference," which I think is good advice.
I've worked on at least one codebase that catches by non-`const` reference, presumably
just because it's shorter to type. This has never caused any problem for them in practice,
as far as I know. But in theory, it's a bad idea, because in C++ it *is* possible to
construct a program where two different catch-blocks, executing *concurrently*, actually
reference the exact same exception object. In that situation, any mutation to the
exception object will cause a data race. The easiest way to prevent the race is to
prevent mutation: to catch by const reference!

[Here's my toy test program.](https://wandbox.org/permlink/AuojSkdo9cZVFUGT)
We start with our exception object, which has a non-`const`-qualified method
that mutates the object and checks to see if anyone else is mutating it at the
same time (because that would mean a data race in a real program):

    struct Atomic {
        std::atomic<int> value_;

        void detect_racey_accesses() {
            for (int i=0; i < 10000; ++i) {
                int last_value = value_.load();
                ++value_;
                int next_value = value_.load();
                if (next_value != last_value + 1) {
                    printf("Detected a race: %d != %d+1\n", next_value, last_value);
                }
            }
        }
    };

(Now, technically, concurrent calls to `detect_racey_accesses` do *not* cause data race
in the C++ sense, because I'm using an `atomic<int>`; the data race here is at the logical
level but not the physical level. In real code, where `value_` would be a plain old `int`,
we *would* have a physical data race here.)

And here's the program that throws an `Atomic`, catches the same object by reference
from two different threads, and then detects racey mutation of that object:

    int main()
    {
        std::promise<std::exception_ptr> prom;

        std::thread T1(
            [fut = prom.get_future()]() mutable {
                std::exception_ptr ex = fut.get();
                try {
                    std::rethrow_exception(ex);
                } catch (Atomic& a) {
                    a.detect_racey_accesses();  // A
                }
            }
        );
        std::thread T2(
            [prom = std::move(prom)]() mutable {
                try {
                    try {
                        throw Atomic();
                    } catch (...) {
                        prom.set_value(std::current_exception());
                        throw;
                    }
                } catch (Atomic& b) {
                    b.detect_racey_accesses();  // B
                }
            }
        );

        T1.join();
        T2.join();
    }

Lines `A` and `B` run concurrently, and the object referenced by `Atomic& a` on
line `A` is the exact same object as the object referenced by `Atomic& b` on line `B`.

So, catch your exceptions by `const` reference — and follow the "`const` means thread-safe"
rule in your own code — and you will avoid this extremely contrived but extremely
surprising situation.

----

This also explains why, in contrived code such as

    catch (Foo& f) {
        return f;
    }

we do not, and cannot, get either copy elision or
[implicit move](https://groups.google.com/a/isocpp.org/forum/#!topic/std-proposals/eeLS8vI05nM)
on `return f` — it *must* make a copy, because the original exception object might still be in use
by another thread.
