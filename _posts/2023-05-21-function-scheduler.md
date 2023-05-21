---
layout: post
title:  "在当前线程周期回调函数"
date:   2023-05-21 08:00:02 +0800
categories: jekyll update
tags:
  async 
---

FunctionScheduler 可在当前线程周期回调函数

    #include <folly/experimental/FunctionScheduler.h>
    #include <iostream>
    #include <gtest/gtest.h>
    using namespace folly;
    using namespace std;
    TEST(repeatfunc,repeatfunc) { 
      FunctionScheduler fs;
      fs.addFunction([]() { cout << "Hello, world!" << endl; }, 1s);
      fs.start();
      this_thread::sleep_for(10s);
      fs.cancelAllFunctions();
    }

只需要addFunction时指明要回调的函数和时间间隔。调用start启动，当不再需要回调时调用cancelAllFunctions. 本例中，每隔1s打印一次"Hello, world!"，共打印10次。

可以看到,比使用folly eventbase的asynctimer简单很多。
