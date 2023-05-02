---
layout: post
title:  "boost swap 源码解析"
date:   2013-07-16 09:24:58 +0800
categories: jekyll update
tags:
  adl 
---
  
boost swap 源码

    #include <algorithm> //for std::swap
    #include <cstddef> //for std::size_t
       
    namespace boost_swap_impl
    {
    template<class T>
    void swap_impl(T& left, T& right)
    {
    using namespace std;//use std::swap if argument dependent lookup fails
    swap(left,right);
    }
       
    template<class T, std::size_t N>
    void swap_impl(T (& left)[N], T (& right)[N])
    {
    for (std::size_t i = 0; i < N; ++i)
    {
    ::boost_swap_impl::swap_impl(left[i], right[i]);
    }
    }
    }
       
    namespace boost
    {
    template<class T1, class T2>
    void swap(T1& left, T2& right)
    {
    ::boost_swap_impl::swap_impl(left, right);
    }
    }

可以看到实现位于swap_impl的重载中，第二个函数交换数组，如果是多维的数组会递归调用，直到交换的不是数组。

那么不是数组时的实现位于第一个swap_impl中，这里使用using namespace std;语句使得std::swap参与重载决议，如果用户实现了一个平凡的swap函数，那么用户的swap函数会优先于std::swap，从而用户的swap被首先执行。

用户的swap能够首先执行依赖于两个C++规则

1 koenig查找

2 重载中普通函数的精确匹配优于模板函数

理解第二条可以执行下面例子程序，结果一目了然的说明了该规则。

   

    #include <iostream>
    void foo(int)
    {
        std::cout<<"int\n";
    }
    template <class T>
    void foo(T)
    {
        std::cout<<"T\n";
    }
    int main()
    {
        foo(11);// 调用第一个foo
        return 0;
    }
