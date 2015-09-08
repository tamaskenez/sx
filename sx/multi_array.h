#ifndef MATRIX_INCLUDED_7204384234
#define MATRIX_INCLUDED_7204384234

#include <vector>
#include <array>

#include <cassert>
#include "sx/array_view.h"

//todo const indices& or by value, is there a difference? 0 or 1
#define SX_MULTI_ARRAY_PASS_INDICES_BY_VALUE 0

namespace sx {

template <typename T, typename U, rank_type Rank>
constexpr size_type linear_index(details::arraylike<T, Rank> x, details::arraylike<U, Rank> strides)
{
    size_type s = 0;
    for (int i = 0; i < Rank; ++i)
        s += x[i] * strides[i];
    return s;
}

template <typename T, std::size_t Rank>
class multi_array : protected array_view<T, Rank> {
    std::vector<T> d;

protected:
    using base_type = array_view<T, Rank>;
    void update_base()
    {
        base_type::data_ptr = d.data();
    }

public:
    using size_type = typename base_type::size_type;
    using indices_type = typename base_type::indices_type;
    using extents_type = typename base_type::extents_type;
    using value_type = T;
    using base_type::rank;
    using base_type::extents;
    using base_type::strides;

    multi_array() = default;
    multi_array(const multi_array& x) = default;
    multi_array(multi_array&& x)
        : base_type(x.d.data(), x.extents(), x.strides())
        , d(std::move(x.d))
    {
        update_base();
    }
    multi_array(const extents_type& e, const indices_type& s)
        : base_type(nullptr, e, s)
        , d(linear_index(e, s))
    {
    }
    template <typename U>
    multi_array& operator=(const array_view<U, Rank>& x); //todo
    multi_array& operator=(multi_array&& x); //todo

    operator const array_view<T, Rank>&() { return *static_cast<base_type*>(this); }
    const array_view<T, Rank>& ize() { return *static_cast<base_type*>(this); }
    operator const array_view<const T, Rank>&() const { return *reinterpret_cast<array_view<const T, Rank>*>(static_cast<base_type*>(this)); }
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
