---
layout: post
title:  "Don't design class that contain Rvalue reference member"
date:   2021-10-15 09:23:58 +0800
categories: jekyll update
tags: 
   rvalue
---
设计类的时候，千万不可以让类有右值成员的应用，很容易导致dangling reference。因为类的成员右值引用只能将所引用的临时对象的生存期延长到类实例构造函数结束，而不是整个类实例的生存期。

```cpp
#include <iostream>
#include <utility>
struct S{
    S(){
        std::cout<<"Cons\n";
    }
    ~S(){
        std::cout<<"DeCons\n";
    }
};
template <class T>
struct  Wrapper{
    T&&t;
    Wrapper():t(T{}){
        std::cout<<"Wrapper con\n";

    }
    void print()const
    {
        std::cout<<"print\n";
    }
};
int main()
{
    Wrapper<S> w;
    w.print();
}
```
输出
```
Cons
Wrapper con
DeCons
print
```
上述代码中，Wrapper\<S>有一个S类型的右值应用成员t，可以看到Wrapper\<S>的对象构造完成时，右值引用的临时对象S{}的生存期结束，构造完成后成员t成为危险的悬空引用。
凡是能引用临时对象的都不应作为类的成员，比如const &,const&&,还有initializer_list。initializer_list可以看作一种特殊的右值引用，所引用的临时对象是数组类型。
