---
layout: post
title:  "stl vector的现代演进"
date:   2020-08-09 09:23:58 +0800
categories: jekyll update
tags: container
---
vector是stl中最常用最重要的容器，那么vector有那些地方还可以做的更佳呢?
### 支持push_front成员函数
vector只能push_back而不能push_front，原因是push_front时需要将现有所有元素向后拷贝，然后将插入的值拷贝到第一个元素，很耗时。2015年google summer of code的一个项目[dvector](https://github.com/erenon/double_ended)，试图解决这一问题，dvector支持push_front，方法是元素不再存储在自由存储数组的开始，而是存储在中间:

|&nbsp;|&nbsp; | 1 | 2 | 3 |&nbsp;|&nbsp;|
|:-:| :-: |:-:| :-: |:-:| :-: |:-:|
|&nbsp;|0 | 1 | 2 | 3 |&nbsp;|&nbsp;|

假定dvector中有三个元素{1,2,3},dvector会在开始有两个分配但未初始化的空位，这样push_front就是复杂度O(1)的操作，push_front(0)后数组中有四个初始化的元素{0,1,2,3}，表格第一二行分别表示了push_front(0)操作前和操作后元素在自由数组中的存储方式。

### 快速删除非尾端的元素

如下代码想删除vector的第5个元素，可是该操作很耗时，因为删除后需要将第五个元素后边的所有元素依次向前拷贝。

```cpp
#include <vector>
int main()
{
    std::vector<int>v(5000);
    v.erase(v.begin()+5);

}
```
有个简单的技巧，如果不需要保证删除前后的相对顺序，可以使用先将要删的元素和最后的元素交换，然后删除最后的元素，上述代码可改写为:

```cpp
#include <vector>
#include <algorithm>
int main()
{
    std::vector<int>v(5000);
    std::iter_swap(v.begin()+5,v.end()-1);
    v.erase(v.end()-1);
}
```
还有一种办法是将被删除的元素打标记，表示已被删除，而不将后边的元素向前搬动，参见[colony](https://plflib.org/colony.htm)。注意colony的vector相对标准vector而言，迭代器是random_iterator而不是continuable_iterator。

### 小对象优化
现在的主流的stl实现都使用小对象优化，在vector的capacity小于一定数值时，元素存储在对象内部的buffer而不是自由存储的数组中。能极大的提升小容量时vector的性能。更有甚者，将vector的capacity作为模板参数，如[boost static_vector](https://www.boost.org/doc/libs/1_57_0/doc/html/container/non_standard_containers.html#container.non_standard_containers.static_vector),static_vector的元素只能保存在对象内部的buffer中，当需要存储的元素个数超过capacity时，不会在自由存储中申请新存储而是直接抛异常。

### 支持relocation
如果vector容器中的元素的move操作不抛异常，那么vector在push_back时如果元素个数要超过capacity时，需要
1. 先在自由存储中申请一个原来capacity一定倍数的数组，
2. 再对所有元素做uninitialized_move操作，
3. 然后对原始数组中的所有元素调用析构函数，
4. 释放原始数组的存储。

而如果支持relocation，则可以合并第2、第3步，改为一步：直接调用memcopy。而memcopy的性能自不待言。使用relocation可显著提升性能，减少编译生成的指令数量。relocation正在由arthur dwyer提案以期成为标准，arthur在C++ Now上有做relocation的原理的详细精彩演讲：[“The Best Type Traits that C++ Doesn't Have”](https://youtu.be/MWBfmmg8-Yo)和[ “Trivially Relocatable”](https://youtu.be/SGdfPextuAU)。

### pmr vector
pmr vector在C++17时引入，pmr vector使用pmr alloator，其分配器是多态分配器。使用strategy 设计模式，pmr allocator可以使用不同的memory_resource，jason turner在c\++ weekly 中演示了pmr 容器能提升容器性能3.5倍以上。


### 支持递归
boost.container.vector支持递归容器，启发来自于Matt Austern的大作[The Standard Librarian: Containers of Incomplete Types](https://www.drdobbs.com/the-standard-librarian-containers-of-inc/184403814)。支持递归使得我们可以很容易的实现树状数据结构。
以下代码演示了用递归 vector实现二叉树，[godbolt](https://godbolt.org/z/oGTv17)。
```cpp

#include <boost/container/vector.hpp>
#include <iostream>
#include <algorithm>

using namespace boost::container;

struct data
{
   int               i_;
   //A vector holding still undefined class 'data'
   vector<data>      v_;
};

void tranverseData(const data& d)

{
  std::cout<<"{";
  std::cout<<d.i_;
  std::cout<<":";
  std::for_each(d.v_.begin(),d.v_.end(),[](auto&& da){
    tranverseData(da);

  });
  std::cout<<"}";


}



int main()
{
   //a container holding a recursive data type
   data d;
   d.i_=0;
   vector<data> &sv = d.v_;
   sv.resize(2);
   sv[0].i_=1;
   sv[1].i_=2;
   sv[0].v_.resize(2);
   sv[0].v_[0].i_=3;
   sv[0].v_[1].i_=4;
   sv[1].v_.resize(2);
   sv[1].v_[0].i_=5;
   sv[1].v_[1].i_=6;
   tranverseData(d);

}

```
