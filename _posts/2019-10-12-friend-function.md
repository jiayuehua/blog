---
layout: post
title:  "友元函数的技巧"
date:   2019-10-12 07:23:58 +0800
categories: jekyll update
---
友元函数大家习惯将声明和定义分开写,如
```cpp
#include <iostream>
namespace Me
{
struct Int {
  int                  i = 0;
  friend std::ostream& operator<<(std::ostream& os, const Int& i);
};
std::ostream& operator<<(std::ostream& os, const Int& i)
{
  os << i << " ";
  return os;
}
}  // namespace Me
```
然而，将友元函数直接写在类的定义内是更好的写法
```cpp
namespace He
{
struct Int {
  int                  i = 0;
  friend std::ostream& operator<<(std::ostream& os, const Int& i)
  {
    os << i << " ";
    return os;
  }
};
}  // namespace He
```
这时友元函数仅能通过ADL查找到，如
```c++
struct Bar {
};
void foo()
{
  Me::Int i;
  std::cout << i << std::endl;
  He::Int j;
  std::cout << j << std::endl;
  Me::operator<<(std::cout, i);
  He::operator<<(std::cout, j); //error
  Bar bar;
  std::cout << bar << std::endl;
}
int main() { foo(); }
```
这时 He::operator<<(std::cout, j)将失败，因为在He namespace找不到 operator<<，而只能通过ADL如std::cout<<j。
好处是在运算符重载决议出错时，将不考虑将He::operator<<纳入报错的重载集。比如对于bar的输出std::cout << bar << std::endl,错误信息里会包含Me::operator<<，而不会包含 He::operator<<。试试新的友元函数的定义方式吧，表达力更强，也更易排错。
