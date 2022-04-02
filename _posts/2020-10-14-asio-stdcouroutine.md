---
layout: post
title:  "使用asio和C++20协程实现echo服务器"
date:   2020-10-15 09:23:58 +0800
categories: jekyll update
---
这次我们改用C++20的无栈协程实现echo服务。需要针对读和写编写两个awaitable类型，封装异步读写操作。

```cpp
//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (c) 2020-2020 jiayuehua
//

#include <coroutine>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <cstdlib>

template <class T>
struct resumable_thing {
  template <class U>
  struct promise_type_impl_base;

  template <template <class> class F, class G>
  struct promise_type_impl_base<F<G>> {
    using U = F<G>;

    using myresumeable = resumable_thing<G>;
    constexpr myresumeable get_return_object() noexcept
    {
      return myresumeable(std::coroutine_handle<U>::from_promise(*(static_cast<U*>(this))));
    }
    constexpr auto initial_suspend() const noexcept { return std::suspend_never{}; }
    auto           final_suspend() const noexcept { return std::suspend_always{}; }
  };

  template <class U>
  struct promise_type_impl : promise_type_impl_base<promise_type_impl<U>> {
    using type = U;
    std::optional<U> _value;
    constexpr void   return_value(U&& value) noexcept { _value = std::move(value); }
  };

  template <>
  struct promise_type_impl<void> : promise_type_impl_base<promise_type_impl<void>> {
    using type = void;
    constexpr void return_void() noexcept {}
  };

  using promise_type = promise_type_impl<T>;

  constexpr explicit resumable_thing(std::coroutine_handle<promise_type> coroutine) noexcept : _coroutine(coroutine) {}

  constexpr const auto& get() const noexcept requires !std::is_void_v<T> { return _coroutine.promise()._value; }

  // constexpr void get() const noexcept requires std::is_void_v<T> {}

  ~resumable_thing() noexcept {}

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

using boost::asio::ip::tcp;

struct ReadTask {
  constexpr bool await_ready() const noexcept { return false; }
  void           await_suspend(std::coroutine_handle<> h) noexcept
  {
    s->async_read_some(boost::asio::buffer(data_, n_),
                       [this, h](boost::system::error_code ec, std::size_t length) mutable {
                         this->value_error_ = std::make_tuple(length, ec);
                         h.resume();
                       });
  }
  auto                                       await_resume() const noexcept { return value_error_; }
  tcp::socket*                               s;
  char*                                      data_;
  int                                        n_;
  std::tuple<int, boost::system::error_code> value_error_{};
};
struct WriteTask {
  constexpr bool await_ready() const noexcept { return false; }
  void           await_suspend(std::coroutine_handle<> h) const noexcept
  {
    boost::asio::async_write(*s, boost::asio::buffer(data_, n_), [h](boost::system::error_code ec, std::size_t length) {
      if (!ec) {
        h.resume();
      } else {
        std::cout << "error:" << ec.value() << std::endl;
      }
    });
  }
  void         await_resume() const noexcept {}
  tcp::socket* s;
  char*        data_;
  int          n_;
};

resumable_thing<void> mysession(tcp::socket s)
{
  const int BUFLEN = 128;
  char      buf[BUFLEN];
  for (;;) {
    auto n = co_await ReadTask{&s, buf, BUFLEN};
    if (!std::get<0>(n)) {
      s.close();
      break;
    }
    if (std::get<1>(n)) {
      std::cerr << "error: " << std::get<1>(n);
      s.close();
      break;
    }
    co_await WriteTask{&s, buf, std::get<0>(n)};
  }
}
class server
{
 public:
  server(boost::asio::io_context& io_context, short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
  {
    do_accept();
  }

 private:
  void do_accept()
  {
    acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
      if (!ec) {
        auto r = mysession(std::move(socket));
      }
      do_accept();
    });
  }

  tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
  try {
    if (argc != 2) {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;

    server s(io_context, std::atoi(argv[1]));

    io_context.run();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
```