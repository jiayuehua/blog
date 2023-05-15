---
layout: post
title:  "token bucket"
date:   2023-05-16 09:24:58 +0800
categories: jekyll update
tags:
  algorithm 
---

令牌桶算法是网络流量整形（Traffic Shaping）和速率限制（Rate Limiting）中最常使用的一种算法。它可以用来限制发送到网络上的数据的数目，并允许突发数据的发送。

folly中tokenbucket的使用方法:

    #include <folly/init/Init.h>
    #include <folly/TokenBucket.h>
    #include <iostream>
    #include <gtest/gtest.h>
    #include <thread>
    #include <chrono>
    using std::literals::chrono_literals::operator""s;
    using std::literals::chrono_literals::operator""ms;

    TEST(tokenbucket,consume) {
       // 100 tokens per second, and a maximum burst size of 500 tokens
      folly::TokenBucket bucket(100, 500);
      //now there should be 500 tokens in the bucket
      // tokenbucket's available tokens size is less_equal then burst size: 500 in its lifetime.

      for (int i = 0; i < 100; ++i) {
        EXPECT_TRUE(bucket.consume(1));
      }
      EXPECT_TRUE(bucket.consume(50));
      {
        auto n= bucket.available();//350
      //now there should be 350 tokens in the bucket
        std::cout << "available: "<< n << std::endl;
      }
      std::this_thread::sleep_for(1s);//450
      //now there should be 450 tokens in the bucket
      EXPECT_FALSE(bucket.consume(1000));//consume size bigger than available
      {
        //now there should still be 450 tokens in the bucket
        auto n= bucket.available();
        std::cout << "available: "<< n << std::endl;//450
      }
      auto n = bucket.consumeOrDrain(1000); //consume 1000 or all available tokens, so n==450
      std::cout << "consumedordrain: "<< n << std::endl;//450
      //available 0
      std::this_thread::sleep_for(1s);
      auto available = bucket.available();//100
      std::cout << "available: "<< available << std::endl;
      auto time      = bucket.consumeWithBorrowNonBlocking(400);//after 3s we can get 400 tokens
      if (time)
      {
        std::cout << "time: "<< *time << std::endl;
      }
      else
      {
        std::cout << "time: "<< "null" << std::endl;
      }
      {
          auto time      = bucket.consumeWithBorrowNonBlocking(800);//800>burstsize(500),So can't be success
          if (time)
          {
            std::cout << "time: "<< *time << std::endl;
          }
          else
          {
            std::cout << "time: "<< "null" << std::endl;
          }
      }
    }
