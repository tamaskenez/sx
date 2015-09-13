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

template <typename T, rank_type Rank>
class multi_array : public array_view<T, Rank> {
private:
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
    static constexpr rank_type rank = Rank;
    using reference = T&;
    using const_reference = const T&;

    multi_array() = default;
    multi_array(const multi_array& x) = default;
    multi_array(multi_array&& x)
        : base_type(x.d.data(), x.extents(), x.strides())
        , d(std::move(x.d))
    {
        update_base();
    }
    explicit multi_array(const extents_type& e, const T& value = T())
        : base_type(nullptr, e, array_layout::c_order)
        , d(linear_index(e, base_type::srd), value)
    {
        update_base();
    }
    explicit multi_array(const extents_type& e, array_layout_t layout, const T& value = T())
        : base_type(nullptr, e, layout)
        , d(linear_index(e, base_type::srd), value)
    {
        update_base();
    }
    template <typename U>
    multi_array& operator=(const array_view<U, Rank>& x); //todo
    multi_array& operator=(multi_array&& x); //todo

    constexpr operator const array_view<T, Rank>&()
    {
        return *static_cast<base_type*>(this);
    }
    constexpr operator const array_view<const T, Rank>&() const
    {
        return *reinterpret_cast<array_view<const T, Rank>*>(static_cast<base_type*>(this));
    }
    base_type* base_ptr() { return this; }
    const base_type* const_base_ptr() const { return this; }
    constexpr const array_view<T, Rank>& view()
    {
        //casting to identical type, written this way
        //to mimic the view() const member function
        return *static_cast<array_view<T, Rank>*>(base_ptr());
    }
    constexpr const array_view<const T, Rank>& view() const
    {
        return *reinterpret_cast<const array_view<const T, Rank>*>(const_base_ptr());
    }

#if SX_MULTI_ARRAY_PASS_INDICES_BY_VALUE
    //todo op[](size_type ) if rank = 1
    T& operator[](const indices_type& offset);
    const T& operator[](const indices_type& offset) const;
#else
    T& operator[](indices_type offset);
    const T& operator[](indices_type offset) const;
#endif

    template <typename... Ts>
    auto
    operator()(Ts... v) const -> decltype(this->view()(v...))
    {
        return view()(v...);
    }
    template <typename... Ts>
    auto
    operator()(Ts... v) -> decltype(this->view()(v...))
    {
        return view()(v...);
    }

    using base_type::empty;
};

template <typename T>
using matrix = multi_array<T, (rank_type)2>;
}

#endif
