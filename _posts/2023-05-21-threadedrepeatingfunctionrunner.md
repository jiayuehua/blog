---
layout: post
title:  "在其它线程周期回调函数"
date:   2023-05-21 08:00:03 +0800
categories: jekyll update
tags:
  async 
---

不同于FunctionScheduler ,ThreadedRepeatingFunctionRunner是在另一线程周期回调函数，例子

    #include <folly/experimental/ThreadedRepeatingFunctionRunner.h>
    #include <iostream>
    #include <gtest/gtest.h>
    #include <chrono>
    #include <thread>
    #include <functional>
    using namespace folly;
    using namespace std;
    using std::literals::chrono_literals::operator""s;
    using std::literals::chrono_literals::operator""ms;
    //running in another thread 
      struct MyClass final {
        MyClass() : count_(0) {}
     
        ~MyClass() {
          threads_.stop();  // Stop threads BEFORE destroying any state they use.
        }
     
        void init() {
          threads_.add("hello", [this]()noexcept {
            return this->incrementCount();
          });
        }
     
        std::chrono::milliseconds incrementCount() noexcept {
          cout<<count_<<endl;
          ++count_;
          return 1000ms;
        }
     
      private:
        std::atomic<int> count_;
        // CAUTION: Declare last since the threads access other members of `this`.
        ThreadedRepeatingFunctionRunner threads_;
      };
    TEST(threadedrepeat,runner) { 
      MyClass repeat;
      repeat.init();
      std::this_thread::sleep_for(5s);
    }

ThreadedRepeatingFunctionRunner类型的threads_的add函数第一个参数是名字，第二个参数是要周期回调的函数，周期回调的函数返回的参数是duration,单位ms,用于指回调的周期，比如lambda调用的incrementCount返回1000ms，退出前需要调用threads_的stop函数，停止周期回调和退出work线程。
