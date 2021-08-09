#include <iostream>
#include <tuple>
#include <experimental/coroutine>

template<class T>
struct generator {
  struct promise_type;
  using handle = std::experimental::coroutine_handle<promise_type>;
  struct promise_type {
    T current_value;
    auto get_return_object() { return generator{handle::from_promise(*this)}; }
    auto initial_suspend() { return std::experimental::suspend_always{}; }
    auto final_suspend() { return std::experimental::suspend_always{}; }
    void unhandled_exception() { std::terminate(); }
    void return_void() {}
    auto yield_value(T value) {
      current_value = value;
      return std::experimental::suspend_always{};
    }
  };
  bool move_next() { return coro ? (coro.resume(), !coro.done()) : false; }
  T current_value() { return coro.promise().current_value; }
  generator(generator const&) = delete;
  generator(generator && rhs) : coro(rhs.coro) { rhs.coro = nullptr; }
  ~generator() { if (coro) coro.destroy(); }
private:
  generator(handle h) : coro(h) {}
  handle coro;
};

auto triples() -> generator<std::tuple<int, int, int>> { 
    for (int z = 1; true; ++z) {
        for (int x = 1; x < z; ++x) {
            for (int y = x; y < z; ++y) {
                if (x*x + y*y == z*z) {
                    co_yield std::make_tuple(x, y, z);
                }
            }
        }
    }
}

int main() {
    auto g = triples();
    for (int i = 0; i < 10; ++i) {
        g.move_next();
        auto triple = g.current_value();
        std::cout << '('
                  << std::get<0>(triple) << ','
                  << std::get<1>(triple) << ','
                  << std::get<2>(triple) << ')' << '\n';
    }
}
