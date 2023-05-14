---
layout: post
title:  "比较四种unordered set的性能"
date:   2023-05-14 09:34:58 +0800
categories: jekyll update
tags:
  container
---
标准库提供了std::unordered_set,abseil新增了flat_hash_set,boost新增了unordered_flat_set,folly中新增了F14FastSet。我们比较下它们的查找和插入的性能。

 benchmark代码:

    #include <algorithm>
    #include <vector>
    #include <gtest/gtest.h>
    #include <folly/Benchmark.h>
    #include <vector>
    #include <unordered_set>
    #include <folly/Random.h>
    #include <absl/container/flat_hash_set.h>
    #include <boost/unordered/unordered_flat_set.hpp>
    #include <folly/container/F14Set.h>
    #include "unordered_map_benchmark.h"
    static inline constexpr int N = 5000000;

    template<class T>
    void unordered_search(unsigned int n,int begin,int end)
    {
      folly::BenchmarkSuspender       s;
      folly::Random::DefaultGenerator rng;
      rng.seed(0xdeadbeaf);
      std::vector<int> v(n);
      std::iota(v.begin(), v.end(), 0);
      T m;
      m.insert(v.begin(), v.end());
      auto it = m.begin();
      s.dismiss();
      for (int i = 0; i < n; ++i) {
        it = m.find(folly::Random::rand32(begin, end));
      }
      folly::doNotOptimizeAway(it);
    }

    BENCHMARK(unordered_set_search_not_found, n)
    { unordered_search<std::unordered_set<int>>(n,N,2*N); }
    BENCHMARK_RELATIVE(absl_flat_set_search_not_found, n)
    {
      unordered_search<absl::flat_hash_set<int>>(n,N,2*N); }
    BENCHMARK_RELATIVE (boost_unordered_flat_set_search_not_found, n)
    {
      unordered_search<boost::unordered_flat_set<int>>(n,N,2*N);  
    }
    BENCHMARK_RELATIVE (folly_f14_set_search_not_found, n)
    {
      unordered_search<folly::F14FastSet<int>>(n,N,2*N);  
    }
    BENCHMARK_DRAW_LINE();
    BENCHMARK(unordered_set_search, n)
    { unordered_search<std::unordered_set<int>>(n,0,N); }
    BENCHMARK_RELATIVE(absl_flat_set_search, n)
    {
      unordered_search<absl::flat_hash_set<int>>(n,0,N);  
    }
    BENCHMARK_RELATIVE (boost_unordered_flat_set_search, n)
    {
      unordered_search<boost::unordered_flat_set<int>>(n,0,N);  
    }
    BENCHMARK_RELATIVE (folly_f14_set_search, n)
    {
      unordered_search<folly::F14FastSet<int>>(n,0,N);
    }
    BENCHMARK_DRAW_LINE();
    template<class T>
    void unordered_insert(unsigned int n)
    {
      folly::BenchmarkSuspender s;
      std::vector<int>          v(n);
      std::iota(v.begin(), v.end(), 0);
      T m;
      s.dismiss();
      m.insert(v.begin(), v.end());
      folly::doNotOptimizeAway(m);
    }
    BENCHMARK(unordered_set_insert, n)
    { unordered_insert<std::unordered_set<int>>(n); }
    BENCHMARK_RELATIVE(abseil_flat_set_insert, n)
    {
      unordered_insert<absl::flat_hash_set<int>>(n);
    }
    BENCHMARK_RELATIVE (boost_unordered_flat_set_insert, n)
    {
      unordered_insert<boost::unordered_flat_set<int>>(n);
      
    }
    BENCHMARK_RELATIVE (folly_f14_set_insert, n)
    {
      unordered_insert<folly::F14FastSet<int>>(n);
    }
    BENCHMARK_DRAW_LINE();
    template<class T>
    void unordered_insert_random(unsigned int n)
    {
      folly::BenchmarkSuspender s;
      std::vector<int>          v(n);
      std::iota(v.begin(), v.end(), 0);

      std::random_device rd;
      std::mt19937       g(rd());
      std::shuffle(v.begin(), v.end(), g);

      T m;
      s.dismiss();
      m.insert(v.begin(), v.end());
      folly::doNotOptimizeAway(m);
    }
    BENCHMARK(unordered_set_insert_random, n)
    { unordered_insert_random<std::unordered_set<int>>(n); }
    BENCHMARK_RELATIVE(abseil_flat_set_insert_random, n)
    {
      unordered_insert_random<absl::flat_hash_set<int>>(n);
    }
    BENCHMARK_RELATIVE (boost_unordered_flat_set_insert_random, n)
    {
      unordered_insert_random<boost::unordered_flat_set<int>>(n);
    }
    BENCHMARK_RELATIVE (folly_f14_set_insert_random, n)
    {
      unordered_insert_random<folly::F14FastSet<int>>(n);
    }
    BENCHMARK_DRAW_LINE();
    template<class T>
    void unordered_insert_one_by_one(unsigned int n)
    {
      folly::BenchmarkSuspender s;
      std::vector<int>          v(n);
      std::iota(v.begin(), v.end(), 0);
      T m;

      std::random_device rd;
      std::mt19937       g(rd());
      std::shuffle(v.begin(), v.end(), g);
      s.dismiss();
      for (int i = 0; i < n; ++i) {
        m.insert(v[i]);
      }
      folly::doNotOptimizeAway(m);
    }
    BENCHMARK(unordered_set_insert_one_by_one, n)
    { unordered_insert_one_by_one<std::unordered_set<int>>(n); }
    BENCHMARK_RELATIVE(abseil_unordered_set_insert_one_by_one, n)
    {
     unordered_insert_one_by_one<absl::flat_hash_set<int>>(n);
    }
    BENCHMARK_RELATIVE (boost_unordered_flat_set_insert_one_by_one, n)
    {
      unordered_insert_one_by_one<boost::unordered_flat_set<int>>(n);
    }
    BENCHMARK_RELATIVE (folly_f14_set_insert_one_by_one, n)
    {
      unordered_insert_one_by_one<folly::F14FastSet<int>>(n);
    }
    int main(int argc, char** argv)
    {
      folly::runBenchmarks();
    }

benchmark结果
查询时，容器中有5000000个元素。以及不同方式插入5000000个元素时结果

![benchmark_result](/images/unordered_set_benchmark.jpg)

结论:

- flat_hash_set,unordered_flat_set,F14FastSet的查询和插入性能都显著优于unordered_set
- 查询时，不管查询的元素在不在容器中，flat_hash_set和unordered_flat_set的性能要优于F14FastSet
- 插入时，一次性插入range中的所有元素时，F14FastSet的性能明显优于flat_hash_set和unordered_flat_set
- 不管是一次性插入所有元素还是一个个插入元素，flat_hash_set的性能都劣于unordered_flat_set和F14FastSet

 
