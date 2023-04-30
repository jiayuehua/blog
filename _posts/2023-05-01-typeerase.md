---
layout: post
title:  "type erasure"
date:   2023-05-01 09:24:58 +0800
categories: jekyll update
tags:
  design-pattern 
---
type erasure相比于经典的基于继承实现的运行时多态，有一大优点，接口类和实现类不再需要是基类和派生类的关系。
Type erasure有两个特点

 - 构造函数是模板
 - 完全非虚函数接口
 
我们简要介绍下如何实现type erasure。

## 值语义的type erasure

以经典的Shape和派生类Circle Square为例说明


    #include <memory>
    #include <iostream>

    using namespace std;

    class Base
    {
     public:
      virtual void draw()const = 0;
      virtual ~Base() {}

    };

    template<class T>
    class Derive : public Base
    {
      T t_;

     public:
      Derive(T ta) : t_(std::move(ta)) {}
      void draw() const override { t_.draw(); }
    };
    class Shape
    {
      std::unique_ptr<Base> pbase;
     public:
      template<class T>
      Shape(T t) : pbase(std::make_unique<Derive<T>>(std::move(t)))
      {
      }
      void draw() const { pbase->draw();
      }
    };
    class Circle
    {
     public:
      void draw() const { cout << "draw Circle"; }
    };
    class Square
    {
      void draw() const { cout << "draw Square"; }
    };

    int main()
    {
      Circle c;
      Shape  s(std::move(c));
      s.draw();
      return 0;
    }

可以看到使用type erasure实现的运行时多态，Shape和Circle不再是基类和派生类的关系。派生类只需实现非虚成员函数draw就可。而Shape的构造函数是模板。Shape有一成员pbase，pbase由Derive\<T>派生自Base类型的对象初始化，而Derive\<T>有一成员t_用于存储Shape的构造函数传入的参数t。Shape的draw函数的实现调用pbase的draw函数，而pbase的draw虚函数将调用派生类Derive\<T>的draw函数，而Derive\<T>的draw函数将调用成员t_的draw函数，而t_移动构造自形式参数t，也就是相当于调用t的draw函数。例如上述main中的Shape类型s的构造函数传入了Circle类型的c,那么s.draw()相当于调用Circle类型c的draw。

## reference语义的type erasure
上文的Derive保存一个T类型的对象t_,也就是说需要保存一个副本，相当于值语义，如果需要实现引用语义，那么Derive将保存T类型的指针而不是对象，ShapeRef的构造函数参数是z左值引用T&而不在值T:

    #include <memory>
    #include <iostream>

    using namespace std;

    class Base
    {
     public:
      virtual void draw()const = 0;
      virtual ~Base() {}

    };

    template<class T>
    class Derive : public Base
    {
      T* t;

     public:
      Derive(T* ta) : t(ta) {}
      void draw() const override { t->draw(); }
    };

    class ShapeRef
    {
      std::unique_ptr<Base> pbase;

     public:
      template<class T>
      ShapeRef(T& t) : pbase(std::make_unique<Derive<T>>(&t))
      {
      }
      void draw() const { pbase->draw();
      }
    };

    class Circle
    {
     public:
      void draw() const { cout << "draw Circle"; }
    };
    class Square
    {
      void draw() const { cout << "draw Square"; }
    };

    int main()
    {
      Circle c;
      ShapeRef  s(c);
      s.draw();
      return 0;
    }

很多Typerase实现引用语义的type erasure时使用void*指针保存指向t的指针，而且使用函数指针作为接口，函数指针指向由lambda实现的具体操作，lambda内部用reinterpret_cast做指针的强制转换。优点是不再需要在堆上分配存储，也就是不再需要例子中的std::unique_ptr\<Base> pbase,缺点是较繁琐。

