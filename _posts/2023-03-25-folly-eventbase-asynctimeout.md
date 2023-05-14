---
layout: post
title:  "借助folly eventbase实现简单的异步周期回调"
date:   2023-03-25 09:24:58 +0800
categories: jekyll update
tags:
  async
---
folly/io/async/EventBase.h 以libevent为基础，实现了强大的异步io网络库。这里只是简单的介绍一下如何使用EventBase实现简单的异步周期回调。

    class MyPrinter
    {
      int                                  n_;
      EventBase*                           pmanager;
      std::unique_ptr<folly::AsyncTimeout> ob_;

     public:
      MyPrinter(EventBase* manager, int n) : n_(n), pmanager(manager) { print(); }
      void print()
      {
        if (n_) {
          ob_ = AsyncTimeout::schedule(1s, *pmanager, [this]() noexcept {
            std::cout << "hello world" << std::endl;
            this->print();
          });
          --n_;
        }
      }
    };

    int main()
    {
       EventBase manager;
       MyPrinter m(&manager, 5);
       manager.loop();
    }

这里将每隔一秒打印一次"hello world"，共打印五次。print内的AsyncTimeOut::schedule用于发起异步操作，并且指定过期后要回调的函数对象，schedule的回调的lambda内又调用print本身。这样只要n_不为零，就会调用schedule。每次print都将要回调的计数n_减一。而main函数中的manage.loop用于检查当前 EventBase 是否有事件需要处理，如果有则处理，如果没有则退出。所以每次lambda被调用后，如果调用了schedule,manager.loop检查发现有事件需要处理，就会继续执行事件处理，而如果没有调用schedule，manager.loop检查发现没有事件需要处理，就会退出事件处理，程序结束。可以看到实现了我们的希望的异步周期回调功能。


