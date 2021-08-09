#include <benchmark/benchmark.h>

#include <string>
#include <string_view>

unsigned const_ref(const std::string& s)
{
  unsigned result = 0;
  int n = s.size();
  for (int i=0; i < n; ++i) {
    result += s[i];
  }
  return result;
}

unsigned const_ref_iters(const std::string& s)
{
  unsigned result = 0;
  for (char ch : s) {
    result += ch;
  }
  return result;
}

unsigned view(std::string_view sv)
{
  unsigned result = 0;
  int n = sv.size();
  for (int i=0; i < n; ++i) {
    result += sv[i];
  }
  return result;
}

static void ShortConstRef(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  std::string s = "short but fits in SBO";
  for (auto _ : state) {
    benchmark::ClobberMemory();
    int result = const_ref(s);
    benchmark::DoNotOptimize(result);
  }
}

static void LongConstRef(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  std::string s = "short but fits in SBO";
  s.reserve(100);
  for (auto _ : state) {
    benchmark::ClobberMemory();
    int result = const_ref(s);
    benchmark::DoNotOptimize(result);
  }
}

static void ShortConstRefIters(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  std::string s = "short but fits in SBO";
  for (auto _ : state) {
    benchmark::ClobberMemory();
    int result = const_ref_iters(s);
    benchmark::DoNotOptimize(result);
  }
}

static void LongConstRefIters(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  std::string s = "short but fits in SBO";
  s.reserve(100);
  for (auto _ : state) {
    benchmark::ClobberMemory();
    int result = const_ref_iters(s);
    benchmark::DoNotOptimize(result);
  }
}

static void ShortView(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  std::string s = "short";
  for (auto _ : state) {
    benchmark::ClobberMemory();
    int result = view(s);
    benchmark::DoNotOptimize(result);
  }
}

static void LongView(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  std::string s = "short";
  s.reserve(100);
  for (auto _ : state) {
    benchmark::ClobberMemory();
    int result = view(s);
    benchmark::DoNotOptimize(result);
  }
}

BENCHMARK(ShortConstRef);
BENCHMARK(LongConstRef);
BENCHMARK(ShortConstRefIters);
BENCHMARK(LongConstRefIters);
BENCHMARK(ShortView);
BENCHMARK(LongView);

BENCHMARK_MAIN();
