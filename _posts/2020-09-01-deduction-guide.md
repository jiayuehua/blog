---
layout: post
title:  "C++17 deduction guide"
date:   2020-09-01 09:23:58 +0800
categories: jekyll update
---
C++17引入了CTAD, class template argument deduction。比如使用标准库的std\::vector时，可以直接使用std\::vector{1,2,3}，而不必要指定模板参数std\::vector\<int>{1,2,3}。
当默认的CTAD推导的类型不合期望时，可以使用deduction guide。如果构造函数使用了deduction guide。那么将分两阶段构造对象

- 使用deduction guide推导实例化模板类。
- 使用刚实例化的模板类和用户提供的参数列表调用合适的构造函数。

deduction guide只参与第一步。

另外deduction guide将使用函数模板推导来实例化模板类。
比如下文的例子原来构造函数仅仅接受右值参数。使用deduction guide后，变成了forwarding reference。如果用户提供的是左值，模板类那么将实例化为S<int&>，而如果用户提供的是右值，模板类将实例化为S\<int>。请认真领会。

```cpp
#include <type_traits>

template<class T>
struct S
{
  S(T&& t){}
};
//deduction guide
template<class T>
S(T&&t)->S<T>;

int main()
{
  auto a = 4;
  S s(9);
  S b(a);
  static_assert(std::is_same_v<decltype(b),S<int&> >);
  static_assert(std::is_same_v<decltype(s),S<int> >);
}

```
