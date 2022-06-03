---
layout: post
title:  "direct aggeregate initialisation 2"
date:   2022-06-03 09:24:58 +0800
categories: jekyll update
tags:
  initialisation
---
## direct aggeregate initialisation 介绍
C\++20引入了direct aggeregate initialisation，那么它和传统的list aggeregate initialisation有啥区别。

我们用例子来说明:

比如array的aggeregate initialisation

    int a[]{1,2,3};
    int a[](1,2,3);

    struct A{
      int i;
    bool b;
    };
    A aa{42,true};//list aggeregate init
    A ab(42,true);//direct aggeregate init

可以看到，可以使用小括号做direct aggeregate init，而传统的大括号init称为list aggeregate init.

## direct aggeregate init  和list aggeregate init的区别。

direct aggeregate init 可以做窄转化

    A ac{42.1, true};//fail
    A ad(42.1, true);

direct aggeregate init 使得容器的emplace_back可以使用aggeregate初始化。

    vector<A> v;
    v.emplace_back(42,true);//valid in C++20 //unvalid in C++17

direct aggeregate init可以很方便的用在宏中

    assert( A {1,true}.b )//fail
    assert( A (1,true).b )

然而direct aggeregate 不能做递归展开

    struct AA{
      A f;
      A s;
    };
    AA {1,true,2,false};
    AA (1,true,2,false);//fail

direct aggeregate init不能延长右值引用成员的生存期

    struct A{
      int && i;
      double d;
    };
    A a{1,2};//ok
    A b(1,2);//dangling reference



小括号内是空时，不会做direct aggeregate init

    struct Bar{
        explicit Bar()=default;
    };

    struct S{
      int a;
      Bar b;
    };
    int main(){
        //fail
        //S c= S{1,{}};
        //S d= S(1,{});
        //S e= S{1};
        //S f = S(1);
        //S g=S{};
        S h = S();
      
    }

对象c d e f g初始化失败因为aggeregate init时要对成员b调用Bar的copy initialisation将调用Bar的构造函数而Bar的构造函数却是explicit的，不能做copy initialisation.  
然而h却成功了，因为这时小括号内是空的，将使用zero initialisation而永不会使用aggeregate init.


