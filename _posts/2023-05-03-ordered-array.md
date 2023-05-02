---
layout: post
title:  "有序数组循环后如何复原为有序"
date:   2023-05-03 09:24:58 +0800
categories: jekyll update
tags:
  algorithm
---
有序数组A如{1,2,3,4,5,6}，循环后，变为B:{3,4,5,6,1,2}，假定我们并不知道partition point,也就是不知道循环后原来的数组首元素如1在循环后数组B中的位置，那么如何复原为原始有序数组A:{1,2,3,4,5,6}呢？

算法分两步

- 使用partition_point找到partition point,partition point前的元素都大于等于B的首元素。比如B数组为{3,4,5,6,1,2}前四个元素都大于等于B的首元素3，那么partition point指向第五个元素1。
- 对数组B循环，将partition point指向的元素作为循环后结果数组的首元素。结果数组和A完全相等。

实现如下：

      #include <ranges>
      #include <algorithm>
      #include <iostream>
      #include <functional>
      #include <ranges>
      int main(){
        int a[]= {3,4,5,6,1,2};
        auto pos=std::ranges::partition_point(a, std::bind_front(std::less_equal{},a[0]));
        std::ranges::rotate(std::ranges::begin(a),pos,std::ranges::end(a));
        std::ostream_iterator<int> oit(std::cout, ",");
        std::ranges::copy(a, oit);
      }


