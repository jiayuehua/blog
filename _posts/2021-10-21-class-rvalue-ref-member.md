---
layout: post
title:  "Do Runtime polymorphism correctly "
date:   2021-10-21 09:23:58 +0800
categories: jekyll update
---
C++的运行时多态历史上使用继承和虚函数的方法，可是继承有很多缺点，比如基类和派生类紧密耦合，基类的成员变量和成员函数都介入到了派生类。sean parent在其better code系列演讲中对继承做了强烈批判。louis的dyno库使用boost hana为基础，并使用type erase技术对运行时多态做了非常杰出的改进。这里我们介绍google 工程师john bandela 的另外一个方案[metaprogrammed_polymorphism](https://github.com/google/cpp-from-the-sky-down/tree/master/metaprogrammed_polymorphism)，使用type erase和函数重载技术，不过非常小巧，不到270行。

基本思想是过去的虚函数, 这里改为使用自由函数，虚函数的名字改为自由函数的第一个参数的类别名称。 比如接口
```cpp
class Base{
public:
virtual void draw(std::ostream&)const;
void x2(std::unique_ptr<int>);
};
```
改为

```cpp
class draw {};
class x2 {};

// Use types instead of names
// void draw(std::ostream&) -> void(draw, std::ostream&)
void call_draw(polymorphic::ref<void(draw, std::ostream&) const> d) {
	std::cout << "in call_draw\n";
	d.call<draw>(std::cout);
}


template <typename T>
void poly_extend(draw, const T& t, std::ostream& os) {
	os << t << "\n";
}

template <typename T>
void poly_extend(x2, T& t, std::unique_ptr<int>) {
	t = t + t;
}


	polymorphic::object<
		void(x2,std::unique_ptr<int>),
		void(draw, std::ostream & os) const
		> object;

```
poly_extend的第二个参数T可以看作以前的派生类。第一个参数是和以前虚函数名字相同的类型。
如果要实现针对某种类型特定的操作，只要重载poly_extend就可，比照以前派生类中覆盖虚函数。
如
```cpp
void poly_extend(draw, const int& t, std::ostream& os) {
	os<<"int:" << t << "\n";
}
```
polymorphic::object提供多态的value语义，也就是可以方便的放在容器里，polymorphic::ref提供多态的ref语义，用作函数参数，类似view。

具体使用方法，object.emplace用于创建的满足object的模板参数的某种特定对象，如object.emplace(9)代表object是int 对象的wrapper。object.call<>用于调用具体的poly_extend函数，如object.call \<draw>(3)将调用draw类型为第一个参数类型的如下函数
```cpp
template <typename T>
void poly_extend(draw, const T& t, std::ostream& os) {
	os << t << "\n";
}
```

完整的例子如下，







```cpp
// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// limitations under the License.

#include "polymorphic.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <memory>

class draw {};
class x2 {};

// Use types instead of names
// void draw(std::ostream&) -> void(draw, std::ostream&)
void call_draw(polymorphic::ref<void(draw, std::ostream&) const> d) {
	std::cout << "in call_draw\n";
	d.call<draw>(std::cout);
}


template <typename T>
void poly_extend(draw, const T& t, std::ostream& os) {
	os << t << "\n";
}

class x2;
template <typename T>
void poly_extend(x2, T& t, std::unique_ptr<int>) {
	t = t + t;
}

int main() {
	std::vector<polymorphic::object<
		void(x2,std::unique_ptr<int>),
		void(draw, std::ostream & os) const
		>> objects;
	for (int i = 0; i < 30; ++i) {
		switch (i % 3) {
		case 0:
			objects.emplace_back(i);
			break;
		case 1:
			objects.emplace_back(double(i) + double(i) / 10.0);
			break;
		case 2:
			objects.emplace_back(std::to_string(i) + " string");
			break;
		}
	}
	auto o = objects.front();
	polymorphic::object<void(draw, std::ostream&)const> co(10);
	auto co1 = co;
	call_draw(co);
	for (const auto& o : objects) call_draw(o);
	for (auto& o : objects) o.call<x2>(nullptr);
	for (auto& o : objects) call_draw(o);
}
```


