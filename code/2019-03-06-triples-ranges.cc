// A sample standard C++20 program that prints
// the first N Pythagorean triples.
#include <iostream>
#include <optional>
#include <range/v3/all.hpp>
 
using std::get;
using std::optional;
using std::make_tuple;
using std::cout;
using ranges::view_interface;
using ranges::iterator_t;
namespace view = ranges::v3::view;

// maybe_view defines a view over zero or one
// objects.
template<class T>
struct maybe_view : view_interface<maybe_view<T>> {
  maybe_view() = default;
  maybe_view(T t) : data_(std::move(t)) {
  }
  T const *begin() const noexcept {
    return data_ ? &*data_ : nullptr;
  }
  T const *end() const noexcept {
    return data_ ? &*data_ + 1 : nullptr;
  }
private:
  optional<T> data_{};
};
 
// "for_each" creates a new view by applying a
// transformation to each element in an input
// range, and flattening the resulting range of
// ranges.
// (This uses one syntax for constrained lambdas
// in C++20.)
inline constexpr auto for_each =
  [](auto&& r, auto fun) {
      return decltype(r)(r)
        | view::transform(std::move(fun))
        | view::join;
  };
 
// "yield_if" takes a bool and a value and
// returns a view of zero or one elements.
inline constexpr auto yield_if =
  [](bool b, auto x) {
    return b ? maybe_view{std::move(x)}
             : maybe_view<decltype(x)>{};
  };
 
int main() {
  // Define an infinite range of all the
  // Pythagorean triples:
  using view::iota;
  auto triples =
    for_each(iota(1), [](int z) {
      return for_each(iota(1, z+1), [=](int x) {
        return for_each(iota(x, z+1), [=](int y) {
          return yield_if(x*x + y*y == z*z,
            make_tuple(x, y, z));
        });
      });
    });
 
    // Display the first 10 triples
    for(auto triple : triples | view::take(10)) {
      cout << '('
           << get<0>(triple) << ','
           << get<1>(triple) << ','
           << get<2>(triple) << ')' << '\n';
  }
}
