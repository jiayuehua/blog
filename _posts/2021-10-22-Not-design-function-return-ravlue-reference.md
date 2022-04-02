---
layout: post
title:  "Don't design function that return rvalue reference"
date:   2021-10-22 09:23:58 +0800
categories: jekyll update
---

设计函数和lambda时，尽量不要返回右值引用。这是因为1 可能返回dangling reference，2可能导致self move。返回dangling reference的情况大家比较熟悉，这里重点讨论self move。
一个对象self move前后，并不能保证该对象的状态不变。详细论述参看eric niebler的post-conditions on self move。比如如下代码，在clang上将输出空行:
```cpp
std::string sb("BBBBBBBBBBBBBBBBBBBB");
sb=std::move(sb);
std::cout<<sb<<std::endl
```
我们想实现一个高效的拼接字符串的lambda，实现如下
```cpp
#include <iostream>
#include <string>
#include <utility>
int main()
{
auto concat=
[](std::string&& l,const std::string &r)->std::string&&
{
     l+=r;
     return std::move(l);
};
std::string sa("AAAAAAAAAAAAAAAAAAAA");
std::string sb("BBBBBBBBBBBBBBBBBBBB");
sa=concat(std::move(sa),sb);
std::cout<<sa<<std::endl;
}
```
这里concat拼接后再赋值回原来的sa，因为返回lambda参数的右值引用，还是存在self move的风险，上述代码也一样输出空行。
你或许会说只要小心使用concat不self move就没有类似问题。可是lambda常用于回调，这导致即便你的代码中没有self move，可是库调用你的返回右值引用的lambda可能会发生self move。例如我们想拼接一组string
```cpp
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <numeric>
int main()
{
auto concat=
[](std::string&& l,const std::string &r)->std::string&&
{
     l+=r;
     return std::move(l);
};
std::string sa("AAAAAAAAAAAAAAAAAAAA");
std::vector<std::string>v(10,sa);
auto r=std::accumulate(v.begin(),v.end(),std::string{},concat);
std::cout<<r<<std::endl;
}
```
上述代码也会输出空行。
如何解决该问题呢，将lambda改写为返回value。如concat改写为
```cpp
auto concat=
[](std::string&& l,const std::string &r)->std::string
{
     l+=r;
     return std::move(l);
};
```
便可避免self move的问题，程序将正确输出结果，不再是空行。



