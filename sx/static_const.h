#ifndef STATIC_CONST_INCLUDED
#define STATIC_CONST_INCLUDED

namespace sx {
template <typename T>
struct static_const {
    static constexpr T value{};
};

template <typename T>
constexpr T static_const<T>::value;
}

#endif
