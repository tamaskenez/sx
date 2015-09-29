#ifndef ARRAY_PAR_INCLUDED_27394234
#define ARRAY_PAR_INCLUDED_27394234

#include <array>
#include <cstddef>

#include "sx/type_traits.h"

namespace sx {

namespace detail {
    template <typename T, typename H>
    constexpr void copy_head_and_advance(T* b, H h) noexcept
    {
        *b = h;
    }
    template <typename T, typename H, typename... Tail>
    constexpr void copy_head_and_advance(T* b, H h, Tail... tail) noexcept
    {
        *b = h;
        copy_head_and_advance(b + 1, tail...);
    }
}
// array_par is like std::array but allows more constructors
// It can be used to for function arguments receiving a fixed-length list
// in a single parameter:
//
//     foo(int i, array_par<int, 2> v)
//
// The main difference is that it provides constructor that
// accepts a list of arguments
//
//     array_par<int, 2>(2, 3) // holds the list {2, 3}
//
// Thanks to these constructors the function with array_par parameters can
// be called like this:
//
//     foo(0, {2, 3}) // v = {2, 3}
//
// The reason this is implemented by variadic templates instead of
// initializer_list that the initializer_list can take only a homogenous
// list which is inconvenient at times:
//
// This does not work with initializer_list:
//
//     foo(0, {4, myvector.size()})
//
// because the first item is int, the second is size_t
//
// Note: The default ctor initializes to zero.
template <typename T, size_t Rank>
struct array_par
    : public std::array<T, Rank> {
    using base_type = std::array<T, Rank>;
    using typename base_type::value_type;

    // default constructor
    constexpr array_par() noexcept
    {
        this->fill(0);
    }

    // copy constructor
    constexpr array_par(const array_par&) noexcept = default;

    // implicit construction from std::array of same type
    constexpr array_par(const base_type& x) noexcept
        : base_type(x)
    {
    }

    // op= from array_par of same type
    constexpr array_par& operator=(const array_par&) noexcept = default;

    //  op= from std::array of same type
    constexpr array_par& operator=(const base_type& x) noexcept
    {
        base_type::operator=(x);
        return *this;
    }

    // contruct from value of list of values
    constexpr array_par(value_type v) noexcept
    {
        static_assert(Rank == 1, "Single-value constructor can be used only if Rank == 1");
        (*this)[0] = v;
    }
    template <typename... Ts,
        typename = std::enable_if_t<sizeof...(Ts) == Rank> >
    constexpr array_par(Ts... ts) noexcept
    {
        detail::copy_head_and_advance(base_type::data(), ts...);
    }

    // base observers
    constexpr base_type& base() noexcept { return *this; }
    constexpr const base_type& base() const noexcept { return *this; }
};
}

#endif
