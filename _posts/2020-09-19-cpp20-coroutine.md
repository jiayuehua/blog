---
layout: post
title:  "c++20 coroutine example"
date:   2020-09-19 09:23:58 +0800
categories: jekyll update
tags: coroutine
---
coroutine 作为C++20的新特性，在编程有着重要的应用。以下我们示范了如何使用coroutine，resumeable 类型和co_await able类型如何编写，以及coroutine中callee 和caller如何手动切换上下文。
```cpp
#include <coroutine>
#include <iostream>
#include <type_traits>
#include <utility>
#include <optional>
#include <string_view>
#include <type_traits>
template <class T>
struct resumable_thing {
  template <class U>
  struct promise_type_impl_base ;

  template <template<class >class  F,class G>
  struct promise_type_impl_base <F<G>>{
    using U = F<G>;

    using myresumeable =resumable_thing<G>;
    constexpr myresumeable get_return_object() noexcept
    {
      return myresumeable(std::coroutine_handle<U>::from_promise(*(static_cast<U*>(this))));
    }
    constexpr auto initial_suspend() const noexcept { return std::suspend_never{}; }
    auto           final_suspend() const noexcept
    {
      std::cout << "final syspend\n";
      return std::suspend_always{};
    }
  };

  template <class U>
  struct promise_type_impl : promise_type_impl_base<promise_type_impl<U>> {
    using  type = U;
    std::optional<U> _value;
    constexpr void   return_value(U&& value) noexcept { _value = std::move(value); }
  };

  template <>
  struct promise_type_impl<void> : promise_type_impl_base<promise_type_impl<void>> {
    using  type = void;
    constexpr void return_void() noexcept {}
  };

  using promise_type = promise_type_impl<T>;

  constexpr explicit resumable_thing(std::coroutine_handle<promise_type> coroutine) noexcept : _coroutine(coroutine) {}

  constexpr const auto& get() const noexcept requires !std::is_void_v<T> { return _coroutine.promise()._value; }

  // constexpr void get() const noexcept requires std::is_void_v<T> {}

  ~resumable_thing() noexcept
  {
    if (_coroutine) {
      _coroutine.destroy();
    }
  }

  constexpr void resume() const noexcept { _coroutine.resume(); }

  constexpr resumable_thing() = default;

  constexpr resumable_thing(resumable_thing const&) = delete;

  constexpr resumable_thing& operator=(resumable_thing const&) = delete;

  constexpr resumable_thing(resumable_thing&& other) noexcept : _coroutine(other._coroutine)
  {
    other._coroutine = nullptr;
  }

  constexpr resumable_thing& operator=(resumable_thing&& other) & noexcept
  {
    if (&other != this) {
      _coroutine       = other._coroutine;
      other._coroutine = nullptr;
    }
    return *this;
  }

  std::coroutine_handle<promise_type> _coroutine = nullptr;
};
struct Trace {
  std::string func_;
  Trace(const Trace& other)
  {
    func_ = other.func_;
    std::cout << func_ << " copy constructor\n";
  }
  Trace& operator=(const Trace& other)
  {
    func_ = other.func_;
    std::cout << func_ << " assign operator\n";
    return *this;
  }
  ~Trace() { std::cout << func_ << " Deconstruct\n"; }
  Trace(std::string_view f) : func_(f) { std::cout << f << " Construct\n"; }
};

struct Task {
  ~Task() { std::cout << "task descontructor\n"; }
  int            i = 0;
  constexpr bool await_ready() const noexcept { return false; }
  void           await_suspend(std::coroutine_handle<> h) const noexcept
  {
    h.resume();
    std::cout << "Task await_suspend \n";
  }
  int await_resume() const noexcept
  {
    std::cout << "Task await_resume \n";
    return 42;
  }
};

resumable_thing<void> f_void() noexcept
{
  Trace a("f_void");
  auto  i = co_await Task{};
  std::cout << i << std::endl;
  co_return;
}

resumable_thing<int> f_int() noexcept
{
  co_await Task{};
  co_return 1;
}

resumable_thing<int> f_suspend_always(Trace s) noexcept
{
  // Trace a("f_suspend_always ");
  co_await std::suspend_always{};
  // std::cout << "f_suspend_always Step A\n";
  co_return 500;
}

resumable_thing<int> f_suspend_never() noexcept
{
  Trace a("f_suspend_never");
  co_await std::suspend_never{};
  std::cout << "suspend_never 1\n";
  co_await std::suspend_never{};
  std::cout << "suspend_never 2\n";
  co_return 1;
}

int main()
{
  //{
  //  std::cout << "Test A:\n";
  //  auto r = f_int();
  //  r.resume();
  //  if (auto& o = r.get(); o) std::cout << *o << std::endl;
  //}
  {
    std::cout << "\nTest B:\n";
    auto r = f_void();
    std::cout << "\nTest B:\n";

    // r.resume();
  }
  {
    // std::cout << "\nTest C:\n";
    // auto r = f_suspend_always(Trace("f_susPendParameter"));
    // std::cout << "suspend_always get r before resume :";
    // if (auto& o = r.get(); o) std::cout << *o << std::endl;
    // r.resume();
    // std::cout << "suspend_always get r after resume  ";
    // std::cout << *r.get() << std::endl;
  }
  //{
  //  std::cout << "\nTest D:\n";
  //  auto r = f_suspend_never();
  //  if (auto& o = r.get(); o) std::cout << *o << std::endl;
  //}
}
```
