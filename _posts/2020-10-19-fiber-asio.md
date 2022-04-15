---
layout: post
title:  "使用asio和boost fiber纤程实现echo服务器"
date:   2020-10-19 10:00:00 +0800
categories: jekyll update
tags:
  asio
   fiber
---
boost fiber提供了编写异步程序的另一种方式，发起异步调用时切换上下文，由调度器迁移到其他纤程，当异步操作完成时，将原来纤程加入就绪队列，最终不再wait，成功执行完成操作，下文我们使用纤程实现echo服务器，可以看到会话session的语法和过程式调用一样简单。

```cpp
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/write.hpp>
#include <iostream>
#include <memory>
#include <libs/fiber/examples/asio/yield.hpp>
#include <libs/fiber/examples/asio/detail/yield.hpp>
#include <libs/fiber/examples/asio/round_robin.hpp>

using boost::asio::ip::tcp;

void session(tcp::socket&& socket_)
{
  using boost::fibers::asio::yield;

  {
    try {
      boost::system::error_code ec;
      char                      data[16];
      for (;;) {
        std::size_t n = socket_.async_read_some(boost::asio::buffer(data), yield);

        if (!n) {
          break;
        }
        boost::asio::async_write(socket_, boost::asio::buffer(data, n), yield);
      }
    } catch (std::exception& e) {
      socket_.close();
    }
  }
}
void server(std::shared_ptr<boost::asio::io_context> io_ctx, int port);

int main(int argc, char* argv[])
{
  try {
    int port = 15999;
    if (argc == 2) {
      port = std::atoi(argv[1]);
    }

    std::shared_ptr<boost::asio::io_context> io_ctx = std::make_shared<boost::asio::io_context>();
                   boost::fibers::use_scheduling_algorithm<boost::fibers::asio::round_robin>(io_ctx);

    using boost::fibers::asio::yield;
    boost::fibers::fiber(server, io_ctx, port).detach();
    io_ctx->run();

  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}

void server(std::shared_ptr<boost::asio::io_context> io_ctx, int port)
{
  tcp::acceptor acceptor(*io_ctx, tcp::endpoint(tcp::v4(), port));
  using boost::fibers::asio::yield;

  for (;;) {
    boost::system::error_code ec;
    tcp::socket               socket(*io_ctx);
    acceptor.async_accept(socket, yield[ec]);
    if (!ec) {
      boost::fibers::fiber(session, std::move(socket)).detach();
    }
  }
}

```