---
layout: post
title:  "variadic template and typelist mutual conversion"
date:   2021-06-03 09:23:58 +0800
categories: jekyll update
---
C++11引入了variadic template，现在模板库都使用新的variadic template来实现模板元编程，可是C\++03使用typelist，旧有的模板元库的算法难道都要舍弃了，另起炉灶吗？旧有的算法有其价值，最好的方法是能将typelist和variadic template相互转换，这样我们就可以应用旧有的算法于variadic template了。
如何转换呢？我们这里使用boost mp11库实现。

先看如何将typelist转换为variadic template
```cpp
#include <boost/mp11.hpp>
#include <type_traits>
using namespace boost::mp11;
struct X1 {};
struct X2 {};
struct X3 {};
template<class T, class U> struct Typelist {};
struct Nulltype ;

using V2 =  Typelist<X1, Typelist<X2, Typelist<X3, Nulltype>>>;
using R2 = mp_iterate<V2, mp_first, mp_second>; // L1
using L1=mp_list<X1,X2,X3>;
static_assert(std::is_same_v<R2,L1>);

```
可以看到我们使用mp_iterate实现，mp_iterator 先使用mp_second将V2转换为 {Typelist<X1, Typelist<X2, Typelist<X3, Nulltype>>>,Typelist<X2, Typelist<X3, Nulltype>>>,Typelist<X3, Nulltype>},然后使用mp_first执行transform操作结果为mp_list<X1,X2,X3>。

然后我们看如何将variadic template转换为typelist
```cpp
#include <boost/mp11.hpp>
#include <type_traits>
using namespace boost::mp11;
struct X1 {};
struct X2 {};
struct X3 {};
template<class T, class U> struct Typelist {};
struct Nulltype ;

using V2 =  Typelist<X1, Typelist<X2, Typelist<X3, Nulltype>>>;

using L1=mp_list<X1,X2,X3>;

using V3 = mp_reverse_fold<L1, Nulltype, Typelist>;
static_assert(std::is_same_v<V3,V2>);

```

从V2我们可以看到传统的typelist实际上是对一组types 以Typelist为二元操作以Nulltype为初值执行right fold计算。mp_reverse_fold可以方便的实现rightfold操作。

如果对variadic template调用typelist相关的算法，可使用mp_reverse_fold先将variadic template转换为typelist，调用传统算法，如果结果是typelist使用mp_iterate转换回variadic template。

