---
layout: post
title:  "moveonly 和nonmovenoncopy"
date:   2023-05-17 09:44:00 +0800
categories: jekyll update
tags:
  reference-semantic 
---

    #include <gtest/gtest.h>
    #include <folly/Utility.h>
    #include <type_traits>

    //如何快捷的创建一个moveonly类? 继承自folly::MoveOnly

    struct Apple : private folly::MoveOnly {
    };
    static_assert(std::is_move_assignable_v<Apple>);
    static_assert(std::is_move_constructible_v<Apple>);
    static_assert(!std::is_copy_assignable_v<Apple>);
    static_assert(!std::is_copy_constructible_v<Apple>);

    //如何快捷的创建一个noncopyablenonmovable类？继承自folly::NonCopyableNonMovalbe

    struct Banana : private folly::NonCopyableNonMovable {
    };
    static_assert(!std::is_move_assignable_v<Banana>);
    static_assert(!std::is_move_constructible_v<Banana>);
    static_assert(!std::is_copy_assignable_v<Banana>);
    static_assert(!std::is_copy_constructible_v<Banana>);


