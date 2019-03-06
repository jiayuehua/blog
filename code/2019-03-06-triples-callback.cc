#include <iostream>
#include <tuple>
#include <type_traits>

template<class F>
auto boolify(F& f) {
    return [&](auto&&... args) {
        using R = decltype(f(std::forward<decltype(args)>(args)...));
        if constexpr (std::is_void_v<R>) {
            f(std::forward<decltype(args)>(args)...);
            return false;
        } else {
            return f(std::forward<decltype(args)>(args)...);
        }
    };
}

template<class F>
void generate_triples(F f) { 
    for (int z = 1; true; ++z) {
        for (int x = 1; x < z; ++x) {
            for (int y = x; y < z; ++y) {
                if (x*x + y*y == z*z) {
                    bool stop = boolify(f)(std::make_tuple(x, y, z));
                    if (stop) return;
                }
            }
        }
    }
}

template<class F>
auto take(int k, F f) {
    return [f, i=k](auto x) mutable -> bool {
        return (i-- == 0) || boolify(f)(x);
    };
}

int main() {
    generate_triples(
        take(10,
            [&](auto triple) {
                std::cout << '('
                    << std::get<0>(triple) << ','
                    << std::get<1>(triple) << ','
                    << std::get<2>(triple) << ')' << '\n';
            }
        )
    );
}
