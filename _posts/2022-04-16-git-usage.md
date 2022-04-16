---
layout: post
title:  "对一组数应用下标的permutation重排"
date:   2022-04-16 09:24:58 +0800
categories: jekyll update
tags:
  algorithm
---
raymond chen开发了很有用的apply_permutation算法，接口如下

    template<typename Iter1, typename Iter2>
    void
    apply_permutationii(
        Iter1 first,
        Iter1 last,
        Iter2 indices);

其中first last代表随机range的起始和结束迭代器，而indices表示下标的一个permutaion的起始迭代器，而permutation本身表示应用permutation后range中当前位置的元素在原位置中的下标。

用例子来说明

    int main()
    {
        std::vector v{1,2,3,4};
        std::vector ids{1,2,3,0};
        apply_permutation(v.begin(),v.end(),ids.begin());
        fmt::print("{}\n",v);
        // [2, 3, 4, 1]
    }

结果是一目了然的，比如ids[0]==1, apply_permutation后第0个元素来自permutation前的第1个元素，以此类推。

我们改用concepts和range来实现接口


    template<std::ranges::random_access_range Range, std::ranges::random_access_range Indices>
    void
    apply_permutation(
        Range&& range,
        Indices indices)
    {
        apply_permutation(range.begin(),range.end(),indices.begin());
    }

  我们所以对indices参数做拷贝是因为apply_permutation中会改写indices。附上
[完整例子代码](https://godbolt.org/z/cGbcYqnss) 以及 [ Raymond Chen 的文章](https://devblogs.microsoft.com/oldnewthing/20170104-00/?p=95115)链接。

