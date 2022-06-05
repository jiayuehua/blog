---
layout: post
title:  "template revisit"
date:   2022-06-03 09:24:08 +0800
categories: jekyll update
tags:
  initialisation
---
这里我们重温一下template的重要特性，这里的内容只覆盖到C\++17。

首先template的参数有啥特点？在我看来，最大的特点就是参数必须是编译期能确定的，一代代的标准只不过把更多的编译器能确定的东西作为模板参数合法化。




    template<const char *str>
    class RegexParser{};

    extern const char regexstr[] = "[a-zA-Z]*"; //external linkage
    static const char regexstr11[] = "C++11 [a-zA-Z]*"; //internal linkage

    void foo(){
        RegexParser<regexstr> regex; //okay for all C++ versions
        RegexParser<regexstr11> regex11; //okay since C++11 with internal linkage

        static const char regexstr17[] = "C++17 [a-zA-Z]*"; //static declaration
        RegexParser<regexstr17> rp; //ok since C++17
     }

首先，不能直接将string literal作为模板实参来实例化模板。比如RegexParser<"hello,world"> 将报错，这是因为指针作为模板参数时必须有linkage。
但是我们可以将指向char array的指针作为模板实参。在C\++98里要求该char array必须具有external linkage，也就是说必须是全局变量，而且必须用extern修饰，例如regexstr。在C\++11里只要该char array是全局变量就可，所以regexstr11在C\++11里是合法的。而C\++17进一步放宽，函数内static 变量也可以，如regexstr17。
[代码链接](https://godbolt.org/z/M83c4f5fn)
