---
layout: post
title:  "vector bool"
date:   2019-12-08 17:23:58 +0800
categories: jekyll update
tags: container
---
vector\<bool\>  和vector\<int\>有啥区别？区别是vector\<bool>的reference_type 不是bool&,而是代理类型。这种代理类型可以转换为bool，这种代理类型还有一个特点，拷贝构造时，相当于拷贝了bit的"地址",而当赋值时，则会真正的拷贝底层的bit。比如如下swap代码:

```cpp
  std::vector<bool> vb{false,true};
  auto t = vb[0];
  vb[0] = vb[1];
  vb[1] = t;
```
试图交换vb的前两个元素，可是这是行不通的。第二行t和vb[0]的"地址"都是第0比特，第三行将第1比特的值赋值给第0比特，也就是t所指向的比特，第四行将t所指向的比特再赋值给第1比特，结果第0比特和第1比特的值都是true。
gcc的std::swap对vector\<bool\>::reference做了重载，gcc的swap(vb[0],vb[1])能正确交换前两个元素。
以下代码的三段示例分别用不同的方式swap vb的前两个元素，只有一个是正确的方式。理解了这三段代码的行为，就理解了vector\<bool\>::reference的奥妙



```cpp
#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <vector>
#include <algorithm>
#include <utility>

int main()
{
  bool a[] = { false,true};
  std::vector<bool> vb(a, a + sizeof(a));
  fmt::print("\n--0---\n");
  std::swap(vb[0], vb[1]);
  for (auto i = vb.begin(); i != vb.begin()+ 2; ++i)
  {
    bool b = *i;
    fmt::print("{},", b);
  }
  fmt::print("\n--1---\n");
  auto t = vb[0];
  vb[0] = vb[1];
  vb[1] = t;


  for (auto i = vb.begin(); i != vb.begin()+ 2; ++i)
  {
    bool b = *i;
    fmt::print("{},", b);
  }
  fmt::print("\n--2---\n");
  vb[0] = true;
  auto ta = vb[0];
  auto tb = vb[1];
  std::swap<decltype(vb[0]) >(ta, tb);

  for (auto i = vb.begin(); i != vb.begin()+ 2; ++i)
  {
    bool b = *i;
    fmt::print("{},", b);
  }
}
```
