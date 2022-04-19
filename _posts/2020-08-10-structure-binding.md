---
layout: post
title:  "structure binding"
date:   2020-08-10 09:23:58 +0800
categories: jekyll update
tags: tuple
---
structure binding是C++17中的新的语言特性，用于查看一个结构体或者一个数据结构中的各个元素。比如如下代码可以很方便的查看插入map的操作成功了吗，成功的话打印新插入元素的value。

```cpp
#include <map>
#include <iostream>
int main()
{
  std::map<int, int> m;
  auto r = m.insert(std::make_pair(4, 5));
  if (auto [position, success] = r; success)
  {
    std::cout << position->second << std::endl;
  }
}
```

### structure binding 中修饰符的作用
对以上代码应用C++ insights 工具分析：
```cpp
#include <map>
#include <iostream>
int main()
{
  std::map<int, int> m = std::map<int, int, std::less<int>, std::allocator<std::pair<const int, int> > >();
  std::pair<std::__map_iterator<std::__tree_iterator<std::__value_type<int, int>, std::__tree_node<std::__value_type<int, int>, void *> *, long> >, bool> r = m.insert<std::pair<int, int>, void>(std::make_pair(4, 5));
  {
    std::pair<std::__map_iterator<std::__tree_iterator<std::__value_type<int, int>, std::__tree_node<std::__value_type<int, int>, void *> *, long> >, bool> __r7 = std::pair<std::__map_iterator<std::__tree_iterator<std::__value_type<int, int>, std::__tree_node<std::__value_type<int, int>, void *> *, long> >, bool>(r);
    std::__map_iterator<std::__tree_iterator<std::__value_type<int, int>, std::__tree_node<std::__value_type<int, int>, void *> *, long> >& position = std::get<0UL>(__r7);
    std::tuple_element<1, std::pair<std::__map_iterator<std::__tree_iterator<std::__value_type<int, int>, std::__tree_node<std::__value_type<int, int>, void *> *, long> >, bool> >::type& success = std::get<1UL>(__r7);
    if(success) {
      std::cout.operator<<(position.operator->()->second).operator<<(std::endl);
    }
  }  
}
```
可以看到auto [position, success] = r 语句，编译器会首先对r做拷贝，这里编译器拷贝为变量__r7，auto的作用是指定__r7的类型，如果我们将上诉语句改写为auto &[position,success] = r,再跑C++ insights
```cpp
#include <map>
#include <iostream>
int main()
{
  std::map<int, int> m = std::map<int, int, std::less<int>, std::allocator<std::pair<const int, int> > >();
  std::pair<std::__map_iterator<std::__tree_iterator<std::__value_type<int, int>, std::__tree_node<std::__value_type<int, int>, void *> *, long> >, bool> r = m.insert<std::pair<int, int>, void>(std::make_pair(4, 5));
  {
    std::pair<std::__map_iterator<std::__tree_iterator<std::__value_type<int, int>, std::__tree_node<std::__value_type<int, int>, void *> *, long> >, bool> & __r7 = r;
    std::__map_iterator<std::__tree_iterator<std::__value_type<int, int>, std::__tree_node<std::__value_type<int, int>, void *> *, long> >& position = std::get<0UL>(__r7);
    std::tuple_element<1, std::pair<std::__map_iterator<std::__tree_iterator<std::__value_type<int, int>, std::__tree_node<std::__value_type<int, int>, void *> *, long> >, bool> >::type& success = std::get<1UL>(__r7);
    if(success) {
      std::cout.operator<<(position.operator->()->second).operator<<(std::endl);
    }
  }
}

```
可以看到__r7的类型为引用类型，也就是说structure binding修饰符是用来指定编译器内部创建的变量的类型，而不是绑定的各个元素的类型。
### structure binding中绑定的每个元素的类型
用一个例子来说明
```cpp
#include <string>
#include <iostream>
#include <utility>
#include <type_traits>

struct splitter {
  splitter(std::string const& s) :str_(s) { std::cout << "default constructor\n"; }
  std::string str_;
  char& front() { return str_.front(); }
  const char& front() const { return str_.front(); }
  splitter rest() const { return { str_.substr(1) }; }
  operator bool() const { return !str_.empty(); }
};
template<>
struct std::tuple_size<splitter> :
  public std::integral_constant<std::size_t, 2> {};
template<>
struct std::tuple_element<0, splitter> {
  using type = char;
};
template<>
struct std::tuple_element<1, splitter> {
  using type = splitter;
};
template<std::size_t I> decltype(auto) get(const splitter& s) {
  std::cout << "Get const ref\n";
  if constexpr (I == 0) { return s.front(); }
  else { return s.rest(); }
}
template<std::size_t I> decltype(auto) get(splitter& s) {
  std::cout << "Get ref\n";
  if constexpr (I == 0) { return s.front(); }
  else { return s.rest(); }
}
template<std::size_t I> decltype(auto) get(splitter&& s) {
  std::cout << "Get Rvalue ref\n";
  if constexpr (I == 0) { return s.front(); }
  else { return s.rest(); }
}
int main()
{
  auto str = splitter{ "alice" };

  auto& [front, rest] = str;
  static_assert(
      std::is_same_v<decltype(rest), splitter>);
  static_assert(
      std::is_same_v<decltype(front), char>);
  std::cout << (&front == &str.front()) << std::endl;;
}

```
以上我们自行实现了支持structure binding的类，要支持structure binding,需要特化tuple_element,tuple_size,和实现三个get函数。front的类型和tuple_element<0,splitter>::type相同，为char。另外需要注意的是front变量的地址和str.front()返回值的地址相同。这是因为structure binding时，编译器会为get函数的返回值创建一个左值变量front，其地址为get<0>返回值的地址；所以如果++front，str.front()将为'b'。请认真领会。

如果将auto& [front, rest] = str改写为const auto& [front, rest] = str，那么front的类型将为const tuple_element<0,splitter>::type，也就是const char。


更多资料请参考[structure bind uncovered](https://youtu.be/uZCvz-E1heA)，上述splitter的例子代码取自该演讲。