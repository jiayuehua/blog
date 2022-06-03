---
layout: post
title:  "direct aggregate initialisation"
date:   2022-06-03 09:24:08 +0800
categories: jekyll update
tags:
  initialisation
---

## direct aggeregate initialisation 介绍

C\++20引入了direct aggeregate initialisation，那么它有啥有点，和传统的list aggeregate initialisation有啥区别。

我们用例子来说明:

比如array的aggeregate initialisation

    int a[]{1,2,3};
    int a[](1,2,3);

    struct A{
      int i;
    bool b;
    };
    A aa{42,true};//list aggeregate init
    A ab(42,true);//direct aggeregate init

可以看到，可以使用小括号做direct aggeregate init，而传统的大括号init称为list aggeregate init.

