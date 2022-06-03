---
layout: post
title:  "direct aggeregate initialisation 2"
date:   2022-06-03 09:24:58 +0800
categories: jekyll update
tags:
  initialisation
---
## direct aggeregate initialisation ����
C\++20������direct aggeregate initialisation����ô���ʹ�ͳ��list aggeregate initialisation��ɶ����

������������˵��:

����array��aggeregate initialisation

    int a[]{1,2,3};
    int a[](1,2,3);

    struct A{
      int i;
    bool b;
    };
    A aa{42,true};//list aggeregate init
    A ab(42,true);//direct aggeregate init

���Կ���������ʹ��С������direct aggeregate init������ͳ�Ĵ�����init��Ϊlist aggeregate init.

## direct aggeregate init  ��list aggeregate init������

direct aggeregate init ������խת��

    A ac{42.1, true};//fail
    A ad(42.1, true);

direct aggeregate init ʹ��������emplace_back����ʹ��aggeregate��ʼ����

    vector<A> v;
    v.emplace_back(42,true);//valid in C++20 //unvalid in C++17

direct aggeregate init���Ժܷ�������ں���

    assert( A {1,true}.b )//fail
    assert( A (1,true).b )

Ȼ��direct aggeregate �������ݹ�չ��

    struct AA{
      A f;
      A s;
    };
    AA {1,true,2,false};
    AA (1,true,2,false);//fail

direct aggeregate init�����ӳ���ֵ���ó�Ա��������

    struct A{
      int && i;
      double d;
    };
    A a{1,2};//ok
    A b(1,2);//dangling reference



С�������ǿ�ʱ��������direct aggeregate init

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

����c d e f g��ʼ��ʧ����Ϊaggeregate initʱҪ�Գ�Աb����Bar��copy initialisation������Bar�Ĺ��캯����Bar�Ĺ��캯��ȴ��explicit�ģ�������copy initialisation.  
Ȼ��hȴ�ɹ��ˣ���Ϊ��ʱС�������ǿյģ���ʹ��zero initialisation��������ʹ��aggeregate init.


