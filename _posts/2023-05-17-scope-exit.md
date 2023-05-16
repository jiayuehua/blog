---
layout: post
title:  "SCOPE_EXIT，your friend of resource management"
date:   2023-05-17 09:34:58 +0800
categories: jekyll update
tags:
  RAII 
---

raii是C++资源管理中的主要手段。folly中提供了SCOPE_EXIT SCOPE_FAIL SCOPE_SUCCESS三个宏，可以方便的实现raii。

    #include <iostream>
    #include <gtest/gtest.h>
    #include <folly/ScopeGuard.h>

    TEST(raii, scope_exit)
    {
      int i = 0;
      {
        SCOPE_EXIT { i++; };//指定退出作用域时执行的语句
        EXPECT_EQ(i, 0);
      }
      EXPECT_EQ(i, 1);
    }
    void foo(int &i)
    {
      SCOPE_SUCCESS { ++i ;};//指定函数成功返回时，退出作用域时执行的语句
      return;
    }
    void fail(int &i)
    {
      SCOPE_FAIL { ++i ;};//指定函数抛异常时，退出作用域时执行的语句
      throw 1;
    }
    TEST(raii, scope_success)
    {
      int i = 0;
      {
        foo(i);
      }
      EXPECT_EQ(i, 1);
    }
    TEST(raii, scope_fail)
    {
      int i = 0;
      try
      {
        fail(i);
      }
      catch (...)
      {
         EXPECT_EQ(i, 1);
      }
    }
    TEST(raii, two_scope_exit)
    {
      int i = 1;
      {
      SCOPE_EXIT { i++; }; //SCOPE_EXIT可多次重复使用，类似析构函数，第二个SCOPE_EXIT中的语句先执行
      SCOPE_EXIT { i*=5; }; 
      }
      EXPECT_EQ(i, 6);
    }
