---
layout: post
title: '"Understanding Real-World Concurrency Bugs in Go"'
date: 2019-06-06 00:01:00 +0000
tags:
  concurrency
  rant
  science
---

Via the O'Reilly Programming Newsletter: Tengfei Tu, Xiaoyu Liu, Linhai Song, and Yiying Zhang's
["Understanding Real-World Concurrency Bugs in Go"](https://songlh.github.io/paper/go-study.pdf) (2019).

> We conduct the first empirical study on Go concurrency bugs using six open-source, production-grade
> Go applications [Docker, Kubernetes, etcd, gRPC, CockroachDB, and BoltDB ...]
> In total, we have studied 171 concurrency bugs in these applications. [...]
> Surprisingly, our study shows that it is as easy to make concurrency bugs with message passing
> as with shared memory [...] [A]round 58% of blocking bugs are caused by message passing.
> In addition to the violation of Go’s channel usage rules (e.g., waiting on a channel that no one
> sends data to or close), many concurrency bugs are caused by the mixed usage of message passing
> and other new semantics and new libraries in Go[.]

The paper is replete with simplified examples of real code bugs; I highly recommend it to anyone
interested in Go, or concurrency in general. Here's the first buggy code sample. Can you spot the bug?

    func finishReq(timeout time.Duration) r ob {
      ch := make(chan ob)
      go func() {
        result := fn()
        ch <- result
      }()
      select {
        case result = <- ch:
          return result
        case <- time.After(timeout):
          return nil
      }
    }

Since `ch` is an [unbuffered channel](https://www.ardanlabs.com/blog/2014/02/the-nature-of-channels-in-go.html),
the `ch <- result` operation will block until some other goroutine evaluates `<- ch`.
If `fn()` takes so long (or the scheduler is so nasty) that we hit the "timeout" case,
then `finishReq` will return, and nobody will ever evaluate `<- ch`...
which means that `ch <- result` will block forever.
Go has garbage collection only for _objects_; it lacks any "garbage collection"
for goroutines blocked on channels whose last listener has died.

The solution in this case (as chosen by the maintainers of the codebase, not necessarily by
the authors of the paper) is to make `ch` a buffered channel:

      ch := make(chan ob, 1)

This ensures that the first (and only) call to `ch <- result` will not block, permitting the goroutine
to finish and be cleaned up. The channel and its one unread item will then be cleaned up by
garbage collection.

Related: ["Go channels are bad and you should feel bad"](https://www.jtolio.com/2016/03/go-channels-are-bad-and-you-should-feel-bad/)
(JT Olio, March 2016).

----

One (but perhaps not the only) original sin of Go channels is that they aren't really "channels";
they're "concurrent queues." A "channel" has two ends — the producer end and the consumer end.
A "concurrent queue" (or a Go channel) has no distinct ends; it's just an object that is shared by
one or two or N different threads, and any of those threads might decide to push or pop from the
queue at any time. If you've got proper directional channels, then whenever you have _only_ producers
left alive — or _only_ consumers left alive — it's safe for the runtime to close the channel and
unblock any threads that were blocked on it.

C++'s `std::promise` and `std::future` do not work like channels because they are "one-shot"; but they
do _almost_ work directionally. If I destroy the last producer (`std::promise`), then all waiting
consumers (`std::future`) immediately wake up and unblock with an exception.
However, if I destroy the last consumer (`std::future`) while there are still producers active,
there's no way for the producer to detect that. (To be fair, producing via `std::promise::set_value()`
never blocks.)  And the original sin there is that C++ promises and futures are not generated as pairs.
If I have only a producer, then I can generate a new consumer by calling `std::promise::get_future`.
So having one producer and zero consumers is a _natural and expected_ state for C++ programs to be in,
at least briefly.

Whatever replaces `promise` and `future` in C++ should fix this original sin.
