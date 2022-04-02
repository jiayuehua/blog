---
layout: post
title:  "使用堆栈遍历二叉树"
date:   2020-10-02 09:23:58 +0800
categories: jekyll update
---
以下我们使用堆栈遍历二叉树，支持前序中序后序，使用智能指针管理内存。遍历时支持定制化的动作。
```cpp
#include <iostream>
#include <stack>
#include <type_traits>
#include <utility>
enum { ZeroTouch, FirstTouch, SecondTouch };
template <class T>
struct BinNode {
  using value_type = T;
  T                        data_{};
  std::unique_ptr<BinNode> left_{};
  std::unique_ptr<BinNode> right_{};
  int                      state_ = ZeroTouch;
  BinNode(T data) noexcept : data_(data) {}
};
template <class DerivedAction>
struct Action {
  template <class T>
  void zero(BinNode<T>* pNode) const noexcept
  {
    pNode->state_ = FirstTouch;
    (static_cast<const DerivedAction*>(this))->zeroState(pNode->data_);
  }
  template <class T>
  void first(BinNode<T>* pNode) const noexcept
  {
    pNode->state_ = SecondTouch;
    (static_cast<const DerivedAction*>(this))->firstState(pNode->data_);
  }
  template <class T>
  void second(BinNode<T>* pNode) const noexcept
  {
    pNode->state_ = ZeroTouch;
    (static_cast<const DerivedAction*>(this))->secondState(pNode->data_);
  }

 protected:
  template <class T>
  void zeroState(T data) const noexcept
  {
  }
  template <class T>
  void firstState(T data) const noexcept
  {
  }
  template <class T>
  void secondState(T data) const noexcept
  {
  }
};
template <class Fun>
struct PreAction : public Action<PreAction<Fun>> {
  PreAction(Fun f) : f_(f) {}

  template <class U>
  void zeroState(U&& data) const noexcept
  {
    f_(std::forward<U>(data));
  }

 private:
  Fun f_;
};
template <class Fun>
struct CenterAction : public Action<CenterAction<Fun>> {
  CenterAction(Fun f) : f_(f) {}

  template <class U>
  void firstState(U&& data) const noexcept
  {
    f_(std::forward<U>(data));
  }

 private:
  Fun f_;
};
template <class Fun>
struct PostAction : public Action<PostAction<Fun>> {
  PostAction(Fun f) : f_(f) {}

  template <class U>
  void secondState(U&& data) const noexcept
  {
    f_(std::forward<U>(data));
  }

 private:
  Fun f_;
};
template <class T, class Act>
void TranverseBinTree(BinNode<T>* head, Act a) noexcept
{
  std::stack<BinNode<T>*> s;
  if (head) {
    s.push(head);
  }
  while (!s.empty()) {
    if (s.top()->state_ == ZeroTouch) {
      a.zero(s.top());
      if (s.top()->left_) {
        s.push(s.top()->left_.get());
      }
    } else if (s.top()->state_ == FirstTouch) {
      a.first(s.top());
      if (s.top()->right_) {
        s.push(s.top()->right_.get());
      }
    } else if (s.top()->state_ == SecondTouch) {
      a.second(s.top());
      s.pop();
    }
  }
}
template <class T, class Func>
void PreTranverseBinTree(BinNode<T>* head, Func f) noexcept
{
  PreAction<Func> action(std::move(f));
  TranverseBinTree(head, action);
}
template <class T, class Func>
void CenterTranverseBinTree(BinNode<T>* head, Func f) noexcept
{
  CenterAction<Func> action(std::move(f));
  TranverseBinTree(head, action);
}
template <class T, class Func>
void PostTranverseBinTree(BinNode<T>* head, Func f) noexcept
{
  PostAction<Func> action(std::move(f));
  TranverseBinTree(head, action);
}

int main()
{
  using BinNodeInt              = BinNode<int>;
  std::unique_ptr<BinNodeInt> d = std::make_unique<BinNodeInt>(4);
  std::unique_ptr<BinNodeInt> e = std::make_unique<BinNodeInt>(5);
  std::unique_ptr<BinNodeInt> b = std::make_unique<BinNodeInt>(1);
  b->left_                      = std::move(d);
  b->right_                     = std::move(e);
  std::unique_ptr<BinNodeInt> c = std::make_unique<BinNodeInt>(2);
  std::unique_ptr<BinNodeInt> a = std::make_unique<BinNodeInt>(0);
  a->left_                      = std::move(b);
  a->right_                     = std::move(c);
  {
    std::cout << "PreTranverse\n";
    auto l = [](int& i) {
      std::cout << i << ' ';
      ++i;
    };
    PreTranverseBinTree(a.get(), std::move(l));
    std::cout << "\n"
              << "--------\n";
  }
  {
    std::cout << "\nCenterTranverse\n";
    auto l = [](int i) {
      std::cout << i << " ";
      ++i;
    };
    CenterTranverseBinTree(a.get(), std::move(l));
    std::cout << "\n"
              << "--------\n";
  }
  {
    std::cout << "\nPostTranverse\n";
    auto l = [](const int& i) { std::cout << i << " "; };
    PostTranverseBinTree(a.get(), std::move(l));
    std::cout << "\n"
              << "--------\n";
  }
}


```