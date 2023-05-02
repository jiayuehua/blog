---
layout: post
title:  "poly, Folly的typeerase库"
date:   2023-05-02 09:24:58 +0800
categories: jekyll update
tags:
  type-erasure 
---
Folly库的Poly实现了可复用的typeerase，比起从头实现typeerase，简易很多。文档可参考folly库的folly/docs/poly.md。

例子：

    #include <folly/Poly.h>
    #include <iostream>
    struct IDrawable {
      template <class Base>
      struct Interface : Base {
        void draw(std::ostream& out) const { folly::poly_call<0>(*this, out); }
      };
      template <class T>
      using Members = folly::PolyMembers<&T::draw>;
    };
    using drawable = folly::Poly<IDrawable>;
    using drawableRef = folly::Poly< IDrawable const&>;
    struct Square {
      void draw(std::ostream& out) const { out << "Square\n"; }
    };
    struct Circle {
      void draw(std::ostream& out) const { out << "Circle\n"; }
    };
    void f(drawable const& d) { d.draw(std::cout); }
    void g(drawableRef  d) { d->draw(std::cout); }

    int main()
    {
      f(Square{});
      f(Circle{});
      Square s{};
      Circle c{};
      g(s);
      g(c);
      g(Square{});
      g(Circle{});
    }

可以看到要将值语义改为引用语义，只需要将folly::Poly的模板参数改为引用。而调用drawableRef对象时需要改用arrow运算符，上文的f函数和g函数说明了drawable和drawableRef的不同用法。

