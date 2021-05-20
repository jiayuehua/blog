#include <type_traits>

template<class, class, class = void>
struct enable_xor : std::false_type {};

template<class T, class U, std::enable_if_t<enable_xor<T, U>::value, int> = 0>
int operator^(T t, U u) {
    return t.value() ^ u.value();
}

namespace Foolib {
    struct Foo {
#ifdef AFTER_YOU
        static constexpr bool barlib_compatible = true;
#endif
        int value() const;
    };
}
template<class U>  // Foo can be xor'ed with anything foolib-compatible.
struct enable_xor<Foolib::Foo, U, std::enable_if_t<U::foolib_compatible>> : std::true_type {};

namespace Barlib {
    struct Bar {
#ifdef NO_AFTER_YOU
        static constexpr bool foolib_compatible = true;
#endif
        int value() const;
    };
}
template<class T>  // Bar can be xor'ed with anything barlib-compatible.
struct enable_xor<T, Barlib::Bar, std::enable_if_t<T::barlib_compatible>> : std::true_type {};

int main() {
    Foolib::Foo foo;
    Barlib::Bar bar;
    return (foo ^ bar);
}
