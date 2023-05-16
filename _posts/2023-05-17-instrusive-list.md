---
layout: post
title:  "intrusive list"
date:   2023-05-16 09:24:58 +0800
categories: jekyll update
tags:
  container 
---
介入式容器的 元素的生命周期由元素自身管理而不是由容器管理。
folly 提供了IntrusiveList和CountedLinklist，是boost instrusive_list的一些alias,能覆盖大部分使用场景。



    #include <folly/init/Init.h>
    #include <folly/ObserverContainer.h>
    #include <folly/IntrusiveList.h>
    #include <iostream>
    #include <gtest/gtest.h>

    class Node
    {
     public:
      int data;
      folly::IntrusiveListHook hook_;
    };
    TEST(intrusive, automode)
    {
      using List=folly::IntrusiveList<Node,&Node::hook_>;//元素的析构函数使元素本身从list中自动unlink
      Node a {3};
      Node b {5};
      List l ;
      l.push_back(a);
      l.push_back(b);
      for (auto&& i : l)
      {
        std::cout<<i.data<<std::endl;
      }
      std::cout<<l.size()<<std::endl;//O(n) time
    }

    class SafeNode
    {
     public:
      int data;
      folly::SafeIntrusiveListHook hook_;
    };
    TEST(intrusive, countLinkList)
    {
      using List=folly::CountedIntrusiveList<SafeNode,&SafeNode::hook_>;
      SafeNode a {3};
      SafeNode b {5};
      List l ;
      l.push_back(a);
      l.push_back(b);
      for (auto&& i : l)
      {
        std::cout<<i.data<<std::endl;
      }
      {
        SafeNode c {5};
        l.push_back(c);
        // ...
        l.pop_back();// should be called to prevent dangling reference,NOT auto unlink
      }
      std::cout<<l.size()<<std::endl;//constant time
      l.erase(l.iterator_to(b));//erase node b
      for (auto&& i : l)
      {
        std::cout<<i.data<<std::endl;
      }
      std::cout<<l.size()<<std::endl;//constant time
      l.pop_front();
    }

