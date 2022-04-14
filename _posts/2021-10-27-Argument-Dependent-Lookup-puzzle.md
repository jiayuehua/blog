---
layout: post
title:  "Argument Dependent Lookup puzzle"
date:   2021-10-27 09:23:58 +0800
categories: jekyll update
tags:
  adl
---

adl是指函数通过其参数所在的名字空间来寻找，对于模板函数，其行为和普通函数略有不同。
比如如下代码
    int h;

    template<class i>
    void g();

    namespace N {
      struct A {};
      template<int i> int f(A);
      template<int i> int g(A);
      template<class T> int h(T);
    }

    int main(){
    // OK in C++20: lookup of `f` finds nothing, `f` treated as a template name
    // fail in C++17: lookup of `f` finds nothing, `f` is NOT treated as a template name
    auto a = f< 0>(N::A{});
    // OK : lookup of `g` finds a function, `g` treated as a template name

    auto b = g<0>(N::A{});
    // error: `h` is a variable, not a template function
    //auto c = h<N::A>(N::A{});
    // OK, `N::h` is qualified-id
    auto d = N::h<N::A>(N::A{});
N\::f是一个函数模板，在C\++17中，main函数中f<0>(N\::A{})将失败，这是因为在C++17中，一个非限定标识f要被解析为函数模板，必须在当前空间或者全局空间能找到一个函数模板f；否则对于f< 0 >(N\::A{})，这里的<解析为小于号，而>解析为大于号，整条语句解析为表达式而不是函数调用，因为找不到f的定义，所以报错。
相对的g因为在全局名字空间中有g的模板，所以g被解析为模板，所以g<0>(N::A{})可以正确的使用ADL找到名字空间N中的函数模板N\::g。

在C\++20中，只要当前或者全局名字空间没有定义同名的非函数及非函数模板的标识，那么便可将类似f<0>()等中的f解析为模板。因此在C\++20中,f< 0>(N\::A{} )，
f将解析为函数模板，通过ADL调用N\::f。
对于h，因为全局定义h为int变量，所以h<N\::A>(N\::A{})中h解析为变量而不是函数模板，在c++20中也会报错。

[adl godbolt](https://gcc.godbolt.org/z/6ajebMMxs)


