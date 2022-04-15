---
layout: post
title:  "stl排列组合算法"
date:   2020-12-27 09:23:58 +0800
categories: jekyll update
tags:
  algorithm
---
标准库的排列组合算法现在只支持next_permutation和prev_permutation。排列组合有着极为重要的应用，howard hinnart开发了15个泛型算法，可以表示日常排列组合的大多数应用。
我们挑选其中三个做说明。
```cpp
template <class BidirIter, class Function>
Function
for_each_permutation(BidirIter first,
                     BidirIter mid,
                     BidirIter last,
                     Function f);
```
计算从distance(first,last)个元素中挑选distance(first,mid)个元素的所有排列，对每一个排列回调f函数。
f的例子
```cpp
auto f = []<typename BidirIter >(BidirIter b, BidirIter e){
    std::for_each(b,e,[](auto e){std::cout<<e<<',';});
    std::cout<<'\n';
}
```
如果回调以上f，将打印挑选出来的每个排列的各个元素。


```cpp
template <class UInt>
UInt
count_each_combination(UInt d1, UInt d2);
```
计算从d1+d2的元素中挑选出d1个元素的所有组合的数量，如
count_each_combination(2,1) ==3。
```
template <class BidirIter>
std::uintmax_t
count_each_circular_permutation(BidirIter first,
                                BidirIter mid,
                                BidirIter last);
```

计算从distance(first,last)个元素中挑选distance(first,mid)个元素的循环排列的数量。
如
```cpp
std::vector<int> v{1,2,3};
auto n = count_each_circular_permutation(v.begin(),v.end(),v.end());//n==2
```

这里只有两种。所有排列都可以由{1,2,3}, {1,3,2}这两种循环后表示出来，而这两种不能相互循环表示。

代码和benchmark可以从howard的[Combinations and Permutations](https://howardhinnant.github.io/combinations/combinations.html)文章中获得。



