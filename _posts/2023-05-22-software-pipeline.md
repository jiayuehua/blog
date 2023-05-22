---
layout: post
title:  "使用mpmcpipeline和jthread实现软流水"
date:   2023-05-22 08:00:03 +0800
categories: jekyll update
tags:
  template-metaprogramming, concurrency 
---

多个Jthread线程，每个线程完成mpmcpipeline中一个stage的处理，实现软件流水。第一个stage收到sentinal value时，所有work线程都退出。

    #include <gtest/gtest.h>
    #include <folly/MPMCPipeline.h>
    #include <vector>
    #include <thread>
    #include <boost/mp11.hpp>
    #include <fmt/format.h>
    #include <folly/Function.h>
    #include <type_traits>
    #include <boost/hana.hpp>
    namespace mp11 = boost::mp11;
    namespace hana = boost::hana;

    template <class T>
    struct getNumber {
      constexpr int operator()(int n) const noexcept { return n; }
    };
    template <class T>
    constexpr auto getnumber = getNumber<T>{};
     //n queuesize, T...: sizeof...(T) ie (N+1) queues' type
    template <int n, typename... T>
      requires(sizeof...(T) >= 2)
    class SoftPipeline
    {

      //N+1 queues' sentinal
      boost::hana::tuple<T...> sentinals_;
      //N stage piepeline
      folly::MPMCPipeline<T...> pipeline_;
      //N stage,so N threads
      std::vector<std::jthread> threads_;
      //every stage work function
      template <int N>
      struct StageCall {
        SoftPipeline* softpipeline_;

        using Input  = mp11::mp_at_c<mp11::mp_list<T...>, N>;
        using Output = mp11::mp_at_c<mp11::mp_list<T...>, N + 1>;
        using Func   = folly::Function<Output(const Input&)>;
        Func func_;
        //if receive sentinal, break
        void operator()()
        {
          for (;;)
          {
          Input val;
          auto  ticket = softpipeline_->pipeline_.template blockingReadStage<N>(val);
          softpipeline_->pipeline_.template blockingWriteStage<N>(ticket, func_(val));
          if (val == hana::at(softpipeline_->sentinals_, hana::size_t<N>{}) )
            break;
          }
        }
      };

     public:
       using HeadType= mp11::mp_front<mp11::mp_list<T...>>;
       using TailType= mp11::mp_back<mp11::mp_list<T...>>;

      template <class... U>
        requires((sizeof...(U) == sizeof...(T) - 1) && !std::is_same_v<std::remove_cv_t< mp11::mp_at_c<mp11::mp_list<U...>,0>   >, SoftPipeline>)
      SoftPipeline(const HeadType& sentinalvalue, U&&... arg) : pipeline_(getnumber<T>(n)...)
      {
      //make sentinals tuple
        auto val   = hana::make_tuple(sentinalvalue);
        auto f     = []<class Seq, class Func>(Seq s, Func func) { return hana::append(s, func(hana::back(s))
        ); };
        auto funcs = hana::make_tuple(arg...);
        sentinals_ = hana::fold(funcs, val, f);
       
       //initional N threads
        [this] <std::size_t... I, class... F>(std::index_sequence<I...>, F&&... f) {
          (..., this->threads_.emplace_back(std::jthread(StageCall<I>{this, std::forward<F>(f)})));
        }(std::index_sequence_for<U...>{}, std::forward<U>(arg)...);
      }
      void blockingWrite(const HeadType &h)
      {
        pipeline_.blockingWrite(h);
      }
      TailType blockingRead()
      {
        TailType val;
        pipeline_.blockingRead(val);
        return val;
      }
    };

    TEST(mpmcpipeline, basic) {
      SoftPipeline<2,int,int,int> pipeline(3,[](int n) { return n + 1; },[](int n) { return n + 2; });
      pipeline.blockingWrite(1);
      auto n =pipeline.blockingRead();
      fmt::print("n:{}\n",n);
      pipeline.blockingWrite(3);
      n =pipeline.blockingRead();
      fmt::print("n:{}\n",n);
    }

value category 和noexcept规范作为练习留给读者。
