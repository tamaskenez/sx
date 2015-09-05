#ifndef MATRIX_INCLUDED_7204384234
#define MATRIX_INCLUDED_7204384234

#include <vector>
#include <array>

#include <cassert>
#include "sx/array_view.h"

//todo const indices& or by value, is there a difference? 0 or 1
#define SX_MULTI_ARRAY_PASS_INDICES_BY_VALUE 0

namespace sx {

template <typename T, std::size_t Rank>
class multi_array {
    std::vector<T> d;

public:
    using size_type = std::size_t;
    using indices_type = details::indices_template<Rank>;
    using extents_type = details::extents_template<Rank>;
    using value_type = T;
    static constexpr size_t rank = Rank;

    multi_array() = default;
    multi_array(const multi_array& x)
        : d(x.d)
    {
    }
    multi_array(multi_array&& x)
        : d(std::move(x.d))
    {
    }
    multi_array(const extents_type& e, const indices_type& s) {}
    multi_array& operator=(const multi_array& x); //todo
    multi_array& operator=(multi_array&& x); //todo

#if SX_MULTI_ARRAY_PASS_INDICES_BY_VALUE
    //todo op[](size_type ) if rank = 1
    T& operator[](const indices_type& offset);
    const T& operator[](const indices_type& offset) const;
#else
    T& operator[](indices_type offset);
    const T& operator[](indices_type offset) const;
#endif
};

template <typename T>
using matrix = multi_array<T, 2>;
}

#endif
