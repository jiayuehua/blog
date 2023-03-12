---
layout: post
title:  "放宽对move from 对象的要求"
date:   2023-02-27 09:24:58 +0800
categories: jekyll update
tags:
  move-semantic 
---

现在C\++标准中要求move from 对象必须是合法，不确定的状态。比如对于std::vector\<int>move from对象，应能合法的调用所有没有precondition的成员函数，如push_back size empty等，但注意不能直接调用operator []，可能导致UB。sean parent认为这些要求太过严格，他观察到标准库对move from对象只执行四种操作：

* 析构
* 拷贝赋值
* 移动赋值
* 移动赋值给自身。

移动赋值给自身只有可能在std::swap(self,self) self-swap中出现，而self swap只可能在random_shuffle标准库算法中使用，self swap没啥价值。因此sean parent建议move from对象只需要支持前三种便可以，而不需要是合法不确定的状态。在新规范下 std::vector\<int> move from对象不能再调用push_back size empty等，只能支持源为非move from对象的拷贝或移动赋值和自身析构三种操作。
sean parent的move from对象规范能很大的简化支持move的类型的实现，并提供更加符合常理的语义。
比如我们想实现一个值语义的value wrapper，初始代码如下。

    class my_type {
      using implementation= int;
        std::unique_ptr<implementation> _remote;
    public:
        explicit my_type(int a) : _remote{std::make_unique<implementation>(a)} {}

        my_type(const my_type& a) : _remote{std::make_unique<implementation>(*a._remote)} {}

        my_type& operator=(const my_type& a) {
            *_remote = *a._remote;//UB, this->_remote may be is null.
            return *this;
        }

        my_type& operator=(my_type&& a) =default;
        my_type (my_type&& a) =default;
        
        friend bool operator==(const my_type& a, const my_type& b) {
            return *a._remote == *b._remote;
        }
        int value()const{
          return *_remote;
        }
    };

构造函数将_remote设置成非空指针。可以看到这时默认移动函数将move from对象的成员_remote设为空指针，但这是符合新规范的，因为新规范并不要求move from对象还满足不变式，因此可以_remote成员设为任何值，只要能通过赋值被覆盖和析构自身就可。
但这里的拷贝赋值操作还有bug，因为对于move from对象_remote为空，解引用将UB,需要改为：

        my_type& operator=(const my_type& a) {
            my_type tmp(a);
            using std::swap;
            swap(tmp._remote, _remote);
            return *this;
        }

也就是说对于新规格，只需要对拷贝赋值操作要考虑_remote为空的情形，其它成员函数都可以假定_remote非空，从而极大的简化实现，比如以上的value和operator==操作，不必要考虑_remote为空的情形。另外新规格还使得move构造标为noexcept成为可能。对于my_type newobject(std::move(other))只需要将move from对象other的_remote成员设为nullptr便可。可如果按照旧的规范，要保持不变式，move from对象other的_remote成员不能为空，还需管理内存，这意味着需要新分配存储，自然不能标为noexcept。    
新规范能简化支持move类型的代码实现，比旧规范易懂，语义也更正确。
