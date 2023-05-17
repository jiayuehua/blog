---
layout: post
title:  "timeout queue"
date:   2023-05-17 09:34:58 +0800
categories: jekyll update
tags:
  algorithm 
---

timeout queue是同步的超时队列，当超时时，回调已超时的注册的callback

    #include <iostream>
    #include <gtest/gtest.h>
    #include <thread>
    #include <chrono>
    #include <folly/TimeoutQueue.h>

    using std::literals::chrono_literals::operator""s;
    using std::literals::chrono_literals::operator""ms;
    TEST(timeoutqueue, Basic) {
      folly::TimeoutQueue queue;
      std::chrono::steady_clock clock;
      auto i= queue.add(clock.now().time_since_epoch().count(), 1, [](int64_t, int64_t) { std::cout << "1s later\n"; });//registe timeout callback
      std::this_thread::sleep_for(2s);
      queue.runOnce(clock.now().time_since_epoch().count());//output "1s later"
    }

    TEST(timeoutqueue, loop) {
      folly::TimeoutQueue queue;
      std::chrono::steady_clock clock;
      auto i= queue.addRepeating(clock.now().time_since_epoch().count(), 1, [](int64_t, int64_t) { std::cout << "1s later\n"; });//call back only call once when the last time timer is due.
      std::this_thread::sleep_for(4s);
      queue.runLoop(clock.now().time_since_epoch().count());//only output 1 message
    }

