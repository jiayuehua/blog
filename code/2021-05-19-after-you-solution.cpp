#include <type_traits>

template<int K> struct priority_tag : priority_tag<K-1> {};
template<> struct priority_tag<0> {};

template<class, class>
void enable_xor(priority_tag<0>);

template<class T, class U,
         decltype(enable_xor<T, U>(priority_tag<99>{})) = 0>  // The max GUID is "99"
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
template<class T, class U,
     // Foo can be xor'ed with anything foolib-compatible.
    std::enable_if_t<std::is_same<T, Foolib::Foo>::value, int> = 0,
    std::enable_if_t<U::foolib_compatible, int> = 0>
int enable_xor(priority_tag<42>);  // Our GUID is "42"

namespace Barlib {
    struct Bar {
#ifdef NO_AFTER_YOU
        static constexpr bool foolib_compatible = true;
#endif
        int value() const;
    };
}
template<class T, class U,
     // Bar can be xor'ed with anything barlib-compatible.
    std::enable_if_t<std::is_same<U, Barlib::Bar>::value, int> = 0,
    std::enable_if_t<T::barlib_compatible, int> = 0>
int enable_xor(priority_tag<71>);  // Our GUID is "71"

int main() {
    Foolib::Foo foo;
    Barlib::Bar bar;
    return (foo ^ bar);
}
