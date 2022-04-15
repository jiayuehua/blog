---
layout: post
title:  "value wrapper and SFINAE failure"
date:   2021-10-08 09:23:58 +0800
categories: jekyll update
tags: 
   SFINAE
---

C\++17中的value wrapper 如optional 和variant给我们带来很大便利。假定我们要实现一个自己的value wrapper，代码如下:
```cpp

#include <iostream>
#include <fmt/format.h>
template<class T>
struct wrapper{
    T t;
    template<class F>
    auto pass_to(F f)const->decltype(f(t))
    {
      fmt::print("pass_to const\n");
      return  f(t);
    }
    template<class F>
    auto pass_to(F f)->decltype(f(t))
    {
      fmt::print("pass_to  not const\n");
      return f(t);
    }
};
struct Foo{
    void do_something(){};
};
int main()
{
    wrapper<Foo> f;//[1]
    f.pass_to([](auto &&x){x.do_something(); }); //[2]
}

```
这里wrapper是我们的value wrapper。[2]处的f.pass_to将回调lambda，lambda的参数为wrapper\<Foo\>的唯一成员变量t。然而上述代码有个bug，你能看出来吗？

bug是pass_to的sfinae将产生hard error。如gcc 的error output:
```
<source>: In instantiation of 'main()::<lambda(auto:19&&)> [with auto:19 = const Foo&]':
<source>:7:39:   required by substitution of 'template<class F> decltype (f(((const wrapper<Foo>*)this)->wrapper<Foo>::t)) wrapper<Foo>::pass_to(F) const [with F = main()::<lambda(auto:19&&)>]'
<source>:25:14:   required from here
<source>:25:42: error: passing 'const Foo' as 'this' argument discards qualifiers [-fpermissive]
   25 |     f.pass_to([](auto &&x){x.do_something(); });
      |                            ~~~~~~~~~~~~~~^~
<source>:20:10: note:   in call to 'void Foo::do_something()'
   20 |     void do_something(){};
      |          ^~~~~~~~~~~~

```

两个pass_to都有->decltype(f(t))的sfinae，因此当我们传递lambda时，需要先进行sfinae决议，然而在sfinae的时候需要知道f(t)的返回类型，对于没有尾置返回的lambda，编译器必须解析lambda的全部代码决定返回类型而不能只解析lambda的签名。当如下const pass_to 成员函数做sfinae时，t是常成员变量，x是t左值引用，也是常量，而do_something()函数并不是常成员函数，因此Sfinae 时x.do_something()产出hard error。

```cpp
...
 template<class F>
    auto pass_to(F f)const->decltype(f(t))
    {
      fmt::print("pass_to const\n");
      return  f(t);
    }
...
```
有啥办法解决对pass_to函数sfinae时产生hard eror的问题吗？在c\++20以前是没有办法的，C\++23引入deduce this的新语言特性可完美解决该问题。

例子代码选自cppcon上sy brand的How to write a value wrapper演讲。
