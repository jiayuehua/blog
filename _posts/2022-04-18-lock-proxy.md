---
layout: post
title:  "C++惯用法 lock_proxy"
date:   2022-04-18 09:24:58 +0800
categories: jekyll update
tags:
  idiom
---
arthur 在其名篇[arrow_proxy](https://quuxplusone.github.io/blog/2019/02/06/arrow-proxy/)中详细描述了如何使用arrow_proxy解决zip_view的reference生存期的问题。bjarne和andrei描述过lock_proxy，用于实现lock_ptr:

    #include <mutex>
    #include <iostream>
    template<class Pointer,class Mutex>
    struct lock_proxy {
        Pointer r;
        std::scoped_lock<Mutex> l;
        lock_proxy(Pointer rp,Mutex* m):r(rp),l(*m){}
        Pointer operator->() {
            return r;
        }
    };
    struct A{
        typedef std::mutex Mutex;
        Mutex m;
        void foo()
        {
            std::cout<<"world\n";
        }
    };
    template<class T>
    struct LockPtr{
        typedef typename T::Mutex M;
        T* r;
        M* m;
        lock_proxy<T*, std::mutex> operator->(){
            return lock_proxy<T*, std::mutex> (r,m);
        }
    };
    int main()
    {
        A a;
        LockPtr<A> lptr{&a,&a.m};
        lptr->foo();
    }

上述代码例子中，LockPtr 使用A类型对象a的指针和a的互斥锁成员m的指针初始化，lptr->foo()时会借用lockproxy类型的临时对象的构造函数加锁，然后调用foo()函数，调用结束后整个表达式lptr->foo()完成evaluation，lockproxy临时对象生存期结束，释放锁。lockproxy和通用的arrow_proxy略有不同，arrow_proxy中主要用来借助成员变量reference实现对zip_view等的reference的存在进行支持，而lockproxy主要借助临时对象的RAII在构造函数使用其成员scoped_lock加锁，在析构函数使用成员scoped_lock解锁。非常灵巧的惯用法。
