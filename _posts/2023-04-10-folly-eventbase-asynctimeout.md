---
layout: post
title:  "the detect idiom"
date:   2023-04-10 09:24:58 +0800
categories: jekyll update
tags:
  template
---
考虑这么一个需求，我们需要判断一个类是否有嵌套的iterator typedef，比如vector\<int> 有vector\<int>::iterator typedef，而std::optional\<int>没有嵌套的iterator typedef。我们如何检测呢？
使用detect idiom
首先定义一个alias 模板

    template <typename T>
    using HasIterator = typename T::iterator;

然后将该模板判别式和要检测的类传给folly::is_detect_v:

     #include <folly/Traits.h>
     folly::is_detect_v<HasIterator, std::vector<int>> // true
     folly::is_detect_v<HasIterator, std::optional<int>> // false
  
而在C\++20里可借助concept完成检测：

    #include <iostream>
    #include <optional>
    #include <vector>
    template <class T>
    concept has_iterator = requires { typename T::iterator; };

    int main()
    {
      if constexpr (has_iterator<std::vector<int>>) {
        std::cout << "vector has iterator typedef\n";
      }
      if constexpr (has_iterator<std::optional<int>>) {
        std::cout << "optional has iterator typedef\n";
      }
    }

