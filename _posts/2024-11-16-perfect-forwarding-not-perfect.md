---
layout: post 
title:  "完美转发不完美"
date:   2024-11-16 08:00:03 +0800
categories: jekyll update
tags:
  forwarding-reference  
---

C++的完美转发并不完美，这是因为对于如下例子代码

    struct Boulder{
     Boulder(){}
     Boulder(const Boulder&)=default;
     Boulder(Boulder&&)=default;
     Boulder& operator=(const Boulder&)=default;
     Boulder& operator=(Boulder&&)=default;
     ~Boulder(){}
    };
    void f(Boulder)
    {}
    template<class T>
    void g(T&&t)
    {
       f(std::forward<T>(t));
    }

    int main()
    {
      f(Boulder{});
      g(Boulder{});
    }

对于f函数调用，只调用Boulder的构造函数和析构。然而对于由g完美转发后在调用f时候，却不得不调用移动构造，因为f的函数参数是值类型，其参数现在为xvalue而不再是prvalue.

然而有一个技巧可以修复该问题，对于C++安腾ABI 而言，如果将f的参数改为右值引用，godbolt显示修改前后f将产生一样的汇编代码。
[godbolt](https://godbolt.org/z/89E4szjbT)

因此如果想省去g中完美转发的开销，可以借助这一点，将g改写为

    template<class T>
    void g(T&&t)
    { 
       typedef void(*MyF)(Boulder&&);
       auto nf=reinterpret_cast<MyF>(&f);
       (*nf)(std::forward<T>(t));
    }

这样*nf的参数是右值引用，省去了移动构造的开销。

上述g中的nf调用虽然理论上是UB,但我们毕竟是对真实的物理机操作，实际中是可以正确执行的。
 
