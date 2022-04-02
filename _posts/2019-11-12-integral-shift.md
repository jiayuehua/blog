---
layout: post
title:  "integral shift"
date:   2019-11-12 07:23:58 +0800
categories: jekyll update
---
C++中的整型的移位操作可能导致undefined behavior。从美国女程序员Satabdi Das学到
```cpp
#include <cstdint>
#include <iostream>
int main()
{
  std::u32int_t i=0;
  std::u32int_t k=31;

  auto j = i<<k;
  k=32;
  j = i<<k;//UB
  k=33;
  j = i<<k;//UB
}
```
如果k大于等于i的二进制bit位数32，或者k小于0，将导致undefined behavior。谢谢你 Satabdi Das!
