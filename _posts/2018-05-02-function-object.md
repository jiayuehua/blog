---
layout: post
title:  "C++函数对象 的模板参数 技巧"
date:   2018-05-02 07:23:59 +0800
categories: jekyll update
tags: closure
---
假定有两组数，需要将对应的元素相加，用stl咋做

```cpp
void fooTransform()
{
  std::vector<int> va{
    1,2,3,4,5
  };
  std::vector<int> vb{
    3,6,8,10,15
  };
  std::vector<int> r(vb.size());
  
  range::transform(va, vb, r.begin(), std::plus<int>());
  for (int i: r)
  {
    std::cout << i << std::endl;
  }
}
```
然而看到了吗，上文中的函数对象必须用类型int实例化为std::plus<int>，有没不用模板实例化的方法？ 有的，答案是使用普通类的模板函数，比如plus可重新实现为
```cpp
struct Plus {
  template<class T>
  T operator()(const T& l, const T&r)const
  {
    return l + r;
  }
};
```
现在上述代码可修改为
```cpp
void fooTransform()
{
  std::vector<int> va{1, 2, 3, 4, 5};
  std::vector<int> vb{3, 6, 8, 10, 15};
  std::vector<int> r(vb.size());
  range::transform(va, vb, r.begin(), Plus());
  for (int i : r) {
    std::cout << i << std::endl;
  }
}
```
看，现在不再需要给Plus类用 int 实例化了，只需要Plus()便可，方便许多。然而使用新的方法并不是没有代价的，比如假定我们要将一组数加7，用std::plus的实现如下
```cpp
void TransformPlus7()
{
  std::vector<int> va{
    1,2,3,4,5
  };
  std::vector<int> vb{
    3,6,8,10,15
  };

  range::transform(va, vb.begin(), std::bind1st(std::plus<int>(), 7));
  for (int i: vb)
  {
    std::cout << i << std::endl;
  }
}
```
我们只需要使用std::bind1st将 7 绑定为std::plus<int>()的第一个参数就可。 然而上述代码 std::plus<int>() 如果替换为新的的Plus()，将不能编译通过，原因是Plus不能直接支持std::bind1st 绑定。
Plus还有办法绑定吗？有的。

可以使用std::bind绑定， tansform那行可替换为
using namespace std::placeholders;
range::transform(va, vb.begin(), std::bind(Plus(), 7, _1));
这时是可以编译通过的。

然而对于需要绑定的情况，更好的做法是使用lambda, 比如 std::bind1st(std::plus<int>(), 7) 可替换为[](int i) { return i + 7; }，代码更易读。

你或许会问为啥标准库的std::plus不支持不带类型的实例化呢，标准委员会意识到了该问题，在C++14中，std::plus<void>特化为和上文Plus一样的实现，而std::plus<>的默认模板参数是void，这就意味这可以使用std::plus<>来达到和Plus一模一样的效果。其他函数对象类似，如std::minus<>等。

建议优先使用不带具体类型的函数对象，如果需要绑定，请使用lambda代替，同时，使用C++14和C++17吧，写优雅的代码更容易，也更易读。
 