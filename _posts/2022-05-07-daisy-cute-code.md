---
layout: post
title:  "what you can learn from being too cute"
date:   2022-05-07 09:24:58 +0800
categories: jekyll update
tags:
  idiom
---

daisy在其pure virtual cpp的演讲中讲解了一段非常有趣的代码
	
    auto impl(std::string_view v = "") {
      static std::string stored;
      static int _ = (stored = v, []{ throw 0; }(), 0);
      __builtin_unreachable();  // since _ initializer throws
      return []{ return stored; };  // spooky access
    }
    void set(auto value) {
      try { impl(value); } catch (int) {}
    }
    auto get = decltype(impl()){};
    int main() {
      set("hello ");
      std::cout << get();
      set("world!\n");
      std::cout << get();
    }

 这段代码的意思是，如果你想要设置一个值，你可以使用set函数，如果你想要获取一个值，你可以使用get函数。
 更神奇的是使用一个函数且没有if语句实现的。
我们主要看看impl函数， static int _ = (stored = v, []{ throw 0; }(), 0);这一行最关键，对于static成员，如果没有构造成功，重新执行该语句时，会重新初始化，而我们这里将参数v存储在了stored里，然后抛异常，也就是说这条语句永不会成功初始化_, 然后我们在[]{ return stored; };将新赋值的stored存在要返回的lambda的内部语句，再看看set函数，要真正执行impl ，然后捕获抛出的异常，而get是通过不执行impl，只拿impl返回的类型，然后默认构造一个lambda，每次执行get(),将执行[]{return stored;},这里的stored正是set真正执行初始化statis init _时充入的，注意这里的stored是static变量。也就是说set时只执行impl前半部分赋值stored，而get时通过只决议返回类型，借助lambda惰性求值只执行impl后半部分。

天才的daisy啊！