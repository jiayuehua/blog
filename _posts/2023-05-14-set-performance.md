---
layout: post
title:  "比较三种ordered set的性能"
date:   2023-05-14 09:24:58 +0800
categories: jekyll update
tags:
  container
---
标准库提供了有序的std::set,folly中新增了两种有序set,sorted_vector_set和heap_vector_set。我们比较下它们的查找和插入的性能。

benchmark代码:

    #include <algorithm>
    #include <functional>
    #include <vector>
    #include <folly/Benchmark.h>
    #include <vector>
    #include <set>
    #include <folly/sorted_vector_types.h>
    #include <folly/container/heap_vector_types.h>
    #include <folly/Random.h>
    static inline constexpr int N=1000000;
    BENCHMARK_DRAW_LINE();
    BENCHMARK(set_search, n)
    {
      folly::BenchmarkSuspender s;
      folly::Random::DefaultGenerator rng;
      rng.seed(0xdeadbeaf);
      std::vector<int>v(n);
      std::iota(v.begin(),v.end(),0);
      std::set<int> m;
      m.insert(v.begin(),v.end());
      auto it=m.begin();
      s.dismiss();
      for (int i = 0; i < n; ++i)
      {
        it=m.find(folly::Random::rand32(0,N));
      }
      folly::doNotOptimizeAway(it);
    }
    BENCHMARK_RELATIVE (sorted_vector_set_search_tag, n)
    {
      folly::BenchmarkSuspender s;
      folly::Random::DefaultGenerator rng;
      rng.seed(0xdeadbeaf);
      std::vector<int>v(n);
      std::iota(v.begin(),v.end(),0);
      folly::sorted_vector_set<int> m;
      m.insert(folly::sorted_unique, v.begin(), v.end()) ;
      auto it=m.begin();
      s.dismiss();
      for (int i = 0; i < n; ++i)
      {
        it=m.find(folly::Random::rand32(0,N));
      }
      folly::doNotOptimizeAway(it);
    }
    BENCHMARK_RELATIVE (heap_vector_set_search, n)
    {
      folly::BenchmarkSuspender s;
      folly::Random::DefaultGenerator rng;
      rng.seed(0xdeadbeaf);
      std::vector<int>v(n);
      std::iota(v.begin(),v.end(),0);
      folly::heap_vector_set<int> m;
      m.insert( v.begin(), v.end()) ;
      auto it=m.begin();
      s.dismiss();
      for (int i = 0; i < n; ++i)
      {
        it=m.find(folly::Random::rand32(0,N));
      }
      folly::doNotOptimizeAway(it);
    }
    BENCHMARK_DRAW_LINE();
    BENCHMARK(set_insert, n)
    {
      folly::BenchmarkSuspender s;
      std::vector<int>v(n);
      std::iota(v.begin(),v.end(),0);
      std::set<int> m;
      s.dismiss();
      m.insert(v.begin(),v.end());
      folly::doNotOptimizeAway(m);
    }
    BENCHMARK_RELATIVE (sorted_vector_set_insert_tag, n)
    {
      folly::BenchmarkSuspender s;
      std::vector<int>v(n);
      std::iota(v.begin(),v.end(),0);
      folly::sorted_vector_set<int> m;
      s.dismiss();
      m.insert(folly::sorted_unique, v.begin(), v.end()) ;
      folly::doNotOptimizeAway(m);
    }
    BENCHMARK_RELATIVE (sorted_vector_set_insert, n)
    {
      folly::BenchmarkSuspender s;
      std::vector<int>v(n);
      std::iota(v.begin(),v.end(),0);
      folly::sorted_vector_set<int> m;
      s.dismiss();
      m.insert( v.begin(), v.end()) ;
      folly::doNotOptimizeAway(m);
    }
    BENCHMARK_RELATIVE (heap_vector_set_insert, n)
    {
      folly::BenchmarkSuspender s;
      std::vector<int>v(n);
      std::iota(v.begin(),v.end(),0);
      folly::heap_vector_set<int> m;
      s.dismiss();
      m.insert( v.begin(), v.end()) ;
      folly::doNotOptimizeAway(m);
    }
    BENCHMARK_DRAW_LINE();
    BENCHMARK(set_insert_random, n)
    {
      folly::BenchmarkSuspender s;
      std::vector<int>v(n);
      std::iota(v.begin(),v.end(),0);

      std::random_device rd;
      std::mt19937       g(rd());
      std::shuffle(v.begin(), v.end(), g);

      std::set<int> m;
      s.dismiss();
      m.insert(v.begin(),v.end());
      folly::doNotOptimizeAway(m);
    }
    BENCHMARK_RELATIVE (sorted_vector_set_insert_random, n)
    {
      folly::BenchmarkSuspender s;
      std::vector<int>v(n);
      std::iota(v.begin(),v.end(),0);
      folly::sorted_vector_set<int> m;

      std::random_device rd;
      std::mt19937       g(rd());
      std::shuffle(v.begin(), v.end(), g);

      s.dismiss();
      m.insert( v.begin(), v.end()) ;
      folly::doNotOptimizeAway(m);
    }
    BENCHMARK_RELATIVE (heap_vector_set_insert_random, n)
    {
      folly::BenchmarkSuspender s;
      std::vector<int>v(n);
      std::iota(v.begin(),v.end(),0);
      folly::heap_vector_set<int> m;

      std::random_device rd;
      std::mt19937       g(rd());
      std::shuffle(v.begin(), v.end(), g);

      s.dismiss();
      m.insert( v.begin(), v.end()) ;
      folly::doNotOptimizeAway(m);
    }
    BENCHMARK_DRAW_LINE();
    BENCHMARK(set_insert_one_by_one, n)
    {
      folly::BenchmarkSuspender s;
      std::vector<int>v(n);
      std::iota(v.begin(),v.end(),0);
      std::set<int> m;

      std::random_device rd;
      std::mt19937       g(rd());
      std::shuffle(v.begin(), v.end(), g);
      s.dismiss();
      for (int i = 0; i < n; ++i)
      {
        m.insert(v[i]);
      }
      folly::doNotOptimizeAway(m);
    }
    BENCHMARK_RELATIVE (sorted_vector_insert_one_by_one, n)
    {
      folly::BenchmarkSuspender s;
      folly::sorted_vector_set<int> m;
      std::vector<int>v(n);
      std::iota(v.begin(),v.end(),0);

      std::random_device rd;
      std::mt19937       g(rd());
      std::shuffle(v.begin(), v.end(), g);
      s.dismiss();
      for (int i = 0; i < n; ++i)
      {
        m.insert(v[i]);
      }
      folly::doNotOptimizeAway(m);
    }
    BENCHMARK_RELATIVE (heap_vector_insert_one_by_one, n)
    {
      folly::BenchmarkSuspender s;
      folly::heap_vector_set<int> m;
      std::vector<int>v(n);
      std::iota(v.begin(),v.end(),0);

      std::random_device rd;
      std::mt19937       g(rd());
      std::shuffle(v.begin(), v.end(), g);
      s.dismiss();
      for (int i = 0; i < n; ++i)
      {
        m.insert(v[i]);
      }
      folly::doNotOptimizeAway(m);
    }
    int main(int argc, char** argv)
    {
      folly::runBenchmarks();
    }


首先看看有1000000个元素时的查询性能,heap_vector_set的性能是最好的,set的性能是最差的,sorted_vector_set的性能是std::set的1.87倍多,heap_vector_set的性能是std::set的2.59倍多,这个性能差距还是很明显的. 

|[...]/map_benchmark.cpp     |relative  | time/iter   | iters/s
|---|---|---|---|
|set_search|   |                                                 24.21ns    |41.30M|
|sorted_vector_set_search_tag  |                  187.38%  |  12.92ns |   77.39M |
|heap_vector_set_search                           |259.1%    | 9.35ns |  107.00M|

然后看看有向空容器中一次性插入1000000个元素时的性能,假定这1000000个元素是单调递增的，可以看到使用folly::sorted_unique tag插入时sorted_vecotr_set性能是set插入性能的42倍，没带folly::sorted_unique tag时sorted_vector_set插入性能是set插入性能的22倍，heap_vector_set插入时性能是set插入性能的3.4倍，这个性能差距还是很明显的.

|set_insert                                     |         |  27.44ns  | 36.45M|
|---|---|---|---|
|sorted_vector_set_insert_tag                   |4209.7%  | 651.74ps  |  1.53G|
|sorted_vector_set_insert                       |2188.4%  |   1.25ns  |797.64M|
|heap_vector_set_insert                         |340.76%  |   8.05ns  |124.20M|

然后看看有向空容器中一次性插入1000000个元素时的性能,假定这1000000个元素是随机排列的，可以看到sorted_vecotr_set性能是set插入性能的2.37倍, heap_vector_set插入时性能是set插入性能的2.08倍，这个性能差距还是很明显的.

|set_insert_random                          |             |   67.77ns |   14.76M|
|---|---|---|---|
|sorted_vector_set_insert_random             |    237.09%  |  28.58ns  |  34.98M|
|heap_vector_set_insert_random               |    208.27%  |  32.54ns  |  30.73M|

然后看看有向空容器中每次插入1个元素,共插入1000000次时的性能,每次插入的元素都是随机的，可以看到sorted_vecotr_set性能是set插入性能的1.19倍, heap_vector_set插入时性能是set插入性能的0.16倍。

|set_insert_one_by_one                   |               |  63.62ns  | 15.72M|
|---|---|---|---|
|sorted_vector_insert_one_by_one         |          119% |  53.46ns  | 18.70M|
|heap_vector_insert_one_by_one           |       16.295% | 390.42ns  |  2.56M|

结论
- 当容器中有1000000个元素时，heap_vector_set的查询性能最佳,set最差。
- 如果插入的是有序的range，sorted_vector_set使用folly::sorted_unique tag时插入性能非常突出,set最差。
- 如果插入的是随机的range，sorted_vector_set插入性能依然最佳,set最差。
- 如果是一个一个插入随机元素，sorted_vector_set插入性能依然最佳，而heap_vector_set的插入性能最差，只有set的16%。
 
