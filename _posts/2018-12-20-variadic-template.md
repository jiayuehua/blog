---
layout: post
title:  "使用variadic template实现函数重载"
date:   2018-12-20 07:23:52 +0000
categories: jekyll update
---
```cpp
#include <sparsehash/dense_hash_map>
#include <iostream>
#include <string>
#include <numeric>
#include <limits>
#include <type_traits>
template <typename... Ts>
struct overloaded : Ts...
{
  overloaded(Ts... arg):Ts(arg)... {};
  using Ts::operator()...;
};
template <typename... Ts>
auto make_overloaded(Ts... arg)->overloaded<Ts...>
{
  return overloaded<Ts...>(arg...);
}
int main()
{
  using namespace std;
  make_overloaded(
    [](int t){std::cout<<t <<std::endl;},
    [](double d){std::cout<<d<<std::endl;}
  )(100);
 
}
```
maked_overloaded可以将多个函数对象整合为一个函数对象，可以构造visitor, 用于C++17中variant的std::visit函数。