#ifndef STATIC_CONST_INCLUDED
#define STATIC_CONST_INCLUDED

namespace sx {
template <typename T>
struct static_const {
    static constexpr T value{};
};

template <typename T>
constexpr T static_const<T>::value;

template <typename T, typename V, V v>
struct static_const_with_value {
    static constexpr T value{v};
};

template <typename T, typename V, V v>
constexpr T static_const_with_value<T, V, v>::value;

}

#endif
