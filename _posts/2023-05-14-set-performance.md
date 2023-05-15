---
layout: post
title:  "比较四种ordered set的性能"
date:   2023-05-14 09:24:58 +0800
categories: jekyll update
tags:
  container
---
标准库提供了有序的std::set,abseil中新增了btree_set,folly中新增了两种有序set,sorted_vector_set和heap_vector_set。我们比较下它们的查找和插入的性能。

benchmark代码:


    #include <vector>
    #include <folly/Benchmark.h>
    #include <vector>
    #include <set>
    #include <folly/sorted_vector_types.h>
    #include <folly/container/heap_vector_types.h>
    #include <folly/Random.h>
    #include <absl/container/btree_set.h>
    static inline constexpr int N = 1000000;
    BENCHMARK_DRAW_LINE();
    template<class T>
    void ordered_search(unsigned int n)
    {
      folly::BenchmarkSuspender       s;
      folly::Random::DefaultGenerator rng;
      rng.seed(0xdeadbeaf);
      std::vector<int> v(N);
      std::iota(v.begin(), v.end(), 0);
      T m;
      m.insert(v.begin(), v.end());
      auto it = m.begin();
      s.dismiss();
      for (int i = 0; i < n; ++i) {
        it = m.find(folly::Random::rand32(0, N));
      }
      folly::doNotOptimizeAway(it);
    }
    BENCHMARK(set_search, n)
    { ordered_search<std::set<int>>(n); }
    BENCHMARK_RELATIVE(btree_set_search, n)
    {ordered_search < absl::btree_set<int>>(n);
    }
    BENCHMARK_RELATIVE (sorted_vector_set_search, n)
    {ordered_search<folly::sorted_vector_set<int>>(n);}
    BENCHMARK_RELATIVE (heap_vector_set_search, n)
    {ordered_search<folly::heap_vector_set<int>>(n);}
    BENCHMARK_DRAW_LINE();
    template<class T, bool sorted_unique =false >
    void ordered_insert()
    {
      folly::BenchmarkSuspender s;
      std::vector<int>          v(N);
      std::iota(v.begin(), v.end(), 0);
      T m;
      s.dismiss();
      m.insert(v.begin(), v.end());
      if constexpr (sorted_unique)
      {
        m.insert(folly::sorted_unique,v.begin(), v.end());
      } else {
        m.insert(v.begin(), v.end());
      }
      folly::doNotOptimizeAway(m);
    }
    BENCHMARK(set_insert )
    { ordered_insert<std::set<int>>(); }
    BENCHMARK_RELATIVE(btree_set_insert)
    { ordered_insert<absl::btree_set<int>>(); }
    BENCHMARK_RELATIVE (sorted_vector_set_insert_with_tag)
    { ordered_insert<folly::sorted_vector_set<int>,true>(); }
    BENCHMARK_RELATIVE (sorted_vector_set_insert)
    { ordered_insert<folly::sorted_vector_set<int>>(); }
    BENCHMARK_RELATIVE (heap_vector_set_insert)
    { ordered_insert<folly::heap_vector_set<int>>(); }
    BENCHMARK_DRAW_LINE();
    template<class T>
    void ordered_insert_random()
    {
      folly::BenchmarkSuspender s;
      std::vector<int>          v(N);
      std::iota(v.begin(), v.end(), 0);

      std::random_device rd;
      std::mt19937       g(rd());
      std::shuffle(v.begin(), v.end(), g);

      T m;
      s.dismiss();
      m.insert(v.begin(), v.end());
      folly::doNotOptimizeAway(m);
    }
    BENCHMARK(set_insert_random)
    { ordered_insert_random<std::set<int>>(); }
    BENCHMARK_RELATIVE(btree_set_insert_random)
    { ordered_insert_random<absl::btree_set<int>>(); }
    BENCHMARK_RELATIVE (sorted_vector_set_insert_random )
    { ordered_insert_random<folly::sorted_vector_set<int>>(); }
    BENCHMARK_RELATIVE (heap_vector_set_insert_random )
    { ordered_insert_random<folly::heap_vector_set<int>>(); }
    BENCHMARK_DRAW_LINE();
    template<class T>
    void ordered_insert_one_by_one()
    {
      folly::BenchmarkSuspender s;
      auto n =N/10;
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
    BENCHMARK(set_insert_one_by_one)
    { ordered_insert_one_by_one<std::set<int>>(); }
    BENCHMARK_RELATIVE(btree_set_insert_one_by_one )
    { ordered_insert_one_by_one<absl::btree_set<int>>(); }
    BENCHMARK_RELATIVE (sorted_vector_insert_one_by_one )
    { ordered_insert_one_by_one<folly::sorted_vector_set<int>>(); }
    BENCHMARK_RELATIVE (heap_vector_insert_one_by_one )
    { ordered_insert_one_by_one<folly::heap_vector_set<int>>(); }
    int main(int argc, char** argv)
    {
      folly::runBenchmarks();
    }


![ordered_benchmark](/images/ordered.jpg)

结论
- 当容器中有1000000个元素时，heap_vector_set的查询性能最佳,set最差。
- 如果插入的是有序的range，range中有1000000个元素时，sorted_vector_set使用folly::sorted_unique tag时插入性能最佳,set最差。
- 如果插入的是随机的range，range中有1000000个元素时，sorted_vector_set插入性能依然最佳,set最差。
- 如果是一个一个插入随机元素，插入1000000个时，btree_set插入性能依然最佳，而heap_vector_set的插入性能最差，只有set的0.2%,sorted_vector_set的性能只有set的13% 。
 
