#ifndef _IMPL_ARRAY_VIEW_H_
#define _IMPL_ARRAY_VIEW_H_ 1

#include <stdexcept>
#include "sx/type_traits.h"
#include <utility>
#include <numeric>
#include <vector>
#include <cmath>
#include <string>

#include "range/utility/static_const.hpp"
#include "range/range_traits.hpp"
#include "sx/coordinate.h"
#include "sx/random_access_iterator_pair.h"
#include "sx/array_par.h"

namespace sx {

using rank_type = std::size_t;
using size_t = std::size_t;

template <typename T, rank_type Rank>
class array_view;

struct array_layout_t {
    constexpr explicit array_layout_t(int value) noexcept
        : value(value)
    {
    }
    constexpr bool operator==(const array_layout_t& x) const noexcept { return value == x.value; }
    constexpr bool operator!=(const array_layout_t& x) const noexcept { return value != x.value; }
    const int value;
};

namespace array_layout {
    static constexpr array_layout_t c_order{ 0 };
    static constexpr array_layout_t fortran_order{ 1 };
};

namespace details {
    // Note: This should be replaced with std::index_sequence if available.
    template <size_t... I>
    struct index_seq {
    };

    template <typename T>
    struct append_1;
    template <size_t... I>
    struct append_1<index_seq<I...> > {
        using type = index_seq<I..., sizeof...(I)>;
    };
    template <typename T>
    using append_1_t = typename append_1<T>::type;

    template <size_t N>
    struct make_seq;
    template <size_t N>
    using make_seq_t = typename make_seq<N>::type;

    template <size_t N>
    struct make_seq {
        using type = append_1_t<make_seq_t<N - 1> >;
    };
    template <>
    struct make_seq<0> {
        using type = index_seq<>;
    };

    template <typename T, size_t... I>
    _CONSTEXPR bounds<sizeof...(I)> make_bounds_inner(index_seq<I...>) _NOEXCEPT
    {
        return { static_cast<ptrdiff_t>(std::extent<T, I>::value)... };
    }

    // Make bounds from an array type extents.
    template <typename T>
    _CONSTEXPR auto make_bounds() _NOEXCEPT
        -> decltype(make_bounds_inner<T>(make_seq_t<std::rank<T>::value>{}))
    {
        return make_bounds_inner<T>(make_seq_t<std::rank<T>::value>{});
    }

    // Make a stride vector from bounds, assuming continugous memory.
    template <rank_type Rank>
    _CONSTEXPR index<Rank>
    make_stride(const bounds<Rank>& bnd) _NOEXCEPT
    {
        index<Rank> stride;
        stride[Rank - 1] = 1;
        for (int i = Rank - 1; i-- > 0;) {
            stride[i] = stride[i + 1] * bnd[i + 1];
        }
        return stride;
    }

    template <typename T>
    _CONSTEXPR T*
    to_pointer(T& t) _NOEXCEPT
    {
        return &t;
    }

    template <typename T, rank_type N>
    _CONSTEXPR std::remove_all_extents_t<T>* to_pointer(T(&t)[N]) _NOEXCEPT
    {
        return to_pointer(t[0]);
    }

    template <typename T, typename ValueType>

    struct is_viewable
        : std::integral_constant<bool, std::is_convertible<decltype(std::declval<T>().size()),
                                           ptrdiff_t>::value
                  && std::is_convertible<decltype(std::declval<T>().data()),
                                           ValueType*>::value
                  && std::is_same<std::remove_cv_t<std::remove_pointer_t<decltype(
                                      std::declval<T>().data())> >,
                                           std::remove_cv_t<ValueType> >::value> {
    };

    template <typename T>
    struct is_array_view_oracle : std::false_type {
    };
    template <typename T, rank_type N>
    struct is_array_view_oracle<array_view<T, N> > : std::true_type {
    };

    template <typename T>
    struct is_array_view : is_array_view_oracle<std::decay_t<T> > {
    };

    template <template <typename, rank_type> class ViewType, typename ValueType,
        rank_type Rank>
    struct slice_return_type {
        using type = ViewType<ValueType, Rank - 1>;
    };

    template <template <typename, rank_type> class ViewType, typename ValueType>
    struct slice_return_type<ViewType, ValueType, 1> {
        using type = void;
    };

    template <template <typename, rank_type> class ViewType, typename ValueType,
        rank_type Rank>
    using slice_return_type_t =
        typename slice_return_type<ViewType, ValueType, Rank>::type;

    template <int N, typename T, typename... Ts>
    struct all_integrals_helper {
        static constexpr bool value = std::is_integral<T>::value
            && all_integrals_helper<N - 1, Ts...>::value;
    };

    template <typename T, typename... Ts>
    struct all_integrals_helper<1, T, Ts...> {
        static constexpr bool value = std::is_integral<T>::value;
    };

    template <typename... Ts>
    struct all_integrals {
        static constexpr bool value = all_integrals_helper<sizeof...(Ts) + 1, int, Ts...>::value;
    };

    template <rank_type Rank>
    using extents_template = array_par<size_t, Rank>;
    template <rank_type Rank>
    using indices_template = array_par<size_t, Rank>;

    template <rank_type Rank, typename T, typename U>
    constexpr bool is_within_extents(T&& idx,
        U&& extents) noexcept
    {
        for (rank_type i = 0; i < Rank; ++i) {
            if (idx[i] < 0 || idx[i] >= extents[i])
                return false;
        }
        return true;
    }
}

// end_fn is the type of the `end` global static object
struct end_fn {
};

namespace {
    // `end` global static object, used when specifying slices the matlab way:
    // e.g. {2, end-3}
    constexpr auto&& end = ranges::static_const<end_fn>::value;
}

//`fromend_value` is the type of the expression `end - <integral>`
//it encapsulates the integral value
template <typename I, typename = std::enable_if_t<std::is_integral<I>::value> >
struct fromend_value {
    const I value;
    constexpr fromend_value(I i)
        : value(i)
    {
    }
};

//`length_value` is the type of the expression `length = <integral>`
//it encapsulates the integral value
template <typename I>
struct length_value {
    const I value;
    constexpr length_value(I i)
        : value(i)
    {
    }
};

//creates the expression `end - <integral>`
template <typename I,
    typename = std::enable_if<std::is_integral<I>::value> >
constexpr fromend_value<I> operator-(end_fn, I i)
{
    return fromend_value<I>(i);
}

//type of static global `length` object for the
//expression `length = <integral>`
//also provides alternative syntax for range|container.size()
struct length_fn {
    template <typename I,
        typename = std::enable_if<std::is_integral<I>::value> >
    constexpr length_value<I> operator=(I i) const
    {
        return length_value<I>(i);
    }
    template <typename Rng>
    ranges::range_size_t<Rng> operator()(Rng&& rng) const { return rng.size(); }
};

namespace {
    //static global `length` object
    constexpr auto&& length = ranges::static_const<length_fn>::value;
}

//type of static global `all` object for the
//expression `all` (= the entire slice)
struct all_fn {
};

namespace {
    //static global `all` object
    constexpr auto&& all = ranges::static_const<all_fn>::value;
}

//base type for the union type of the expressions
// - absolute index: `<integral>`
// - end-index: `end`
// - end-relative index: `end - <integral>`
// - length-expression: `length = <integral>`
struct smart_index_base {

    enum kind_t { K_ABSOLUTE,
        K_FROM_END,
        K_LENGTH };

    const size_t value;
    const kind_t kind;

    template <typename I>
    constexpr smart_index_base(I value, kind_t k)
        : value(value)
        , kind(k)
    {
    }

    //returns the absolute, resolved index
    constexpr size_t index(size_t begin_idx, size_t extent) const
    {
        if (kind == K_ABSOLUTE)
            return value;
        else if (kind == K_FROM_END)
            return extent - value;
        else {
            assert(kind == K_LENGTH);
            return begin_idx + value;
        }
    }

    //returns the absolute, resolved index, assumes the kind is not length
    constexpr size_t index_when_not_length(size_t extent) const
    {
        if (kind == K_ABSOLUTE)
            return value;
        else {
            assert(kind == K_FROM_END);
            return extent - value;
        }
    }
};

//the union type of the expressions, provides appropriate constructors over
//smart_index_base
// - absolute index: `<integral>`
// - end-index: `end`
// - end-relative index: `end - <integral>`
struct index_or_fromend : protected smart_index_base {
public:
    using smart_index_base::kind;
    using smart_index_base::value;
    using smart_index_base::kind_t;

    //absolute index
    template <typename I>
    constexpr index_or_fromend(I idx)
        : smart_index_base(idx, K_ABSOLUTE)
    {
    }

    //`end - <integral>`
    template <typename I>
    constexpr index_or_fromend(fromend_value<I> e)
        : smart_index_base(e.value, K_FROM_END)
    {
    }

    //`end`
    constexpr index_or_fromend(end_fn)
        : smart_index_base(0, K_FROM_END)
    {
    }

    //resolves absolute index
    constexpr size_t index(size_t extent) const
    {
        return smart_index_base::index_when_not_length(extent);
    }

protected:
    template <typename I>
    constexpr index_or_fromend(I value, kind_t k)
        : smart_index_base(value, k)
    {
    }
};

//the union type of the expressions, provides further constructor over
//index_or_fromend
// - absolute index: `<integral>`
// - end-index: `end`
// - end-relative index: `end - <integral>`
// - length-expression: `length = <integral>`
struct index_or_fromend_or_length : protected index_or_fromend {
    using index_or_fromend::index_or_fromend;
    using smart_index_base::index;
    using smart_index_base::index_when_not_length;
    using smart_index_base::kind;
    using smart_index_base::value;
    using smart_index_base::kind_t;

    //`length = <integral>`
    template <typename I>
    constexpr index_or_fromend_or_length(length_value<I> l)
        : index_or_fromend(l.value, K_LENGTH)
    {
    }
};

//encapsulates a slice between a `index_or_fromend` to
//`index_or_fromend_or_length` values
struct slice_bounds {
    const index_or_fromend from;
    const index_or_fromend_or_length to_or_length;

    template <typename F, typename T>
    constexpr slice_bounds(F from, T to_or_length)
        : from(from)
        , to_or_length(to_or_length)
    {
    }

    constexpr slice_bounds(all_fn)
        : from(0)
        , to_or_length(sx::end)
    {
    }

    //resolves the length of the slice given the extent
    //of the entire dimension (`end` resolves to `extent`)
    constexpr size_t length(size_t extent) const
    {
        return to_or_length.kind == ::sx::smart_index_base::K_LENGTH
            ? to_or_length.value
            : to_or_length.index_when_not_length(extent) - from.index(extent);
    }
};

template <typename T, rank_type Rank = 1>
class array_view {
    static_assert(Rank > 0, "rank must be > 0");

public:
    // types, constants
    using rank_type = ::sx::rank_type;
    using size_type = ::sx::size_t;
    using indices_type = details::indices_template<Rank>;
    using extents_type = details::extents_template<Rank>;
    using value_type = typename std::remove_const<T>::type;
    using pointer = T*;
    using reference = T&;

    constexpr rank_type rank() const { return Rank; }

protected:
    // state
    pointer data_ptr;
    extents_type bnd;
    indices_type srd;

public:
    // construction
    constexpr array_view() noexcept
        : data_ptr(nullptr) //bnd, src initializes to zero
    {
    }

    // fundamental ctor
    constexpr array_view(pointer data, extents_type extents, indices_type strides) noexcept
        : data_ptr(data),
          bnd(extents),
          srd(strides)
    {
    }

    // this ctor works only if rank == 1
    constexpr array_view(pointer data, extents_type extents) noexcept
        : data_ptr(data),
          bnd(extents),
          srd(1)
    {
    }
    constexpr array_view(pointer data, extents_type extents, array_layout_t layout) noexcept
        : data_ptr(data),
          bnd(extents)
    {
        if (layout == array_layout::c_order) {
            srd[Rank - 1] = 1;
            for (int i = (int)Rank - 2; i >= 0; --i)
                srd[i] = srd[i + 1] * bnd[i + 1];
        }
        else {
            assert(layout == array_layout::fortran_order);
            srd[0] = 1;
            for (rank_type i = 1; i < Rank; ++i)
                srd[i] = srd[i - 1] * bnd[i - 1];
        }
    }

    // relaxed copy ctor
    template <typename U,
        typename = std::enable_if_t<std::is_convertible<std::add_pointer_t<U>,
                                        pointer>::value
            && std::is_same<U, value_type>::value> >
    constexpr array_view(
        const array_view<U, Rank>& rhs) noexcept
        : array_view(rhs.data(), rhs.extents(), rhs.strides())
    {
    }

    // from Viewable (a non-array_view type that has size() and data())
    // enabled only if this->rank == 1
    template <
        typename Viewable,
        typename = std::enable_if_t<Rank == 1
            && details::is_viewable<Viewable, value_type>::value
            && !details::is_array_view<Viewable>::value> >
    _CONSTEXPR array_view(Viewable&& cont)
        : array_view(cont.data(), static_cast<size_t>(cont.size()), 1)
    {
    }

    // from ArrayType
    template <
        typename ArrayType,
        typename = std::enable_if_t<std::is_convertible<std::add_pointer_t<std::remove_all_extents_t<ArrayType> >,
                                        pointer>::value
            && std::is_same<std::remove_cv_t<std::remove_all_extents_t<ArrayType> >,
                                        std::remove_cv_t<value_type> >::value
            && std::rank<ArrayType>::value == Rank> >
    constexpr array_view(ArrayType& data) noexcept
        : array_view(
              details::to_pointer(data), details::make_bounds<ArrayType>(),
              details::make_stride(details::make_bounds<ArrayType>()))
    {
    }

    // assigment, shallow copy
    template <typename U,
        typename = std::enable_if_t<std::is_convertible<std::add_pointer_t<U>,
                                        pointer>::value
            && std::is_same<std::remove_cv_t<U>,
                                        std::remove_cv_t<value_type> >::value> >
    constexpr array_view& operator=(
        const array_view<U, Rank>& rhs) noexcept
    {
        bnd = rhs.extents();
        srd = rhs.strides();
        data_ptr = rhs.data();
        return *this;
    }

    // assigment from same-shape array_view (deep copy)
    template <typename U,
        typename = std::enable_if_t<!std::is_const<T>::value
            && std::is_convertible<U, T>::value> >
    const array_view&
    operator<<=(array_view<U, Rank> x) const
    {
        copy_from_same_shape_array_view(x);
        return *this;
    }

    template <typename Rng,
        typename = std::enable_if_t<!std::is_const<T>::value
            && std::is_convertible<ranges::range_value_t<Rng>, T>::value
            && Rank == 1> >
    const array_view&
    operator<<=(Rng&& x) const
    {
        copy_from_range_to_1d(x);
        return *this;
    }

    // observers
    constexpr pointer data() const noexcept { return data_ptr; }

    constexpr const std::array<size_t, Rank>& extents() const noexcept { return bnd; }
    constexpr size_t extents(rank_type i) const noexcept { return bnd[i]; }

    constexpr const std::array<size_t, Rank>& strides() const noexcept { return srd; }
    constexpr size_t strides(rank_type i) const noexcept { return srd[i]; }

    constexpr size_type size() const noexcept
    {
        size_type ret = bnd[0];
        for (rank_type i = 1; i < Rank; ++i)
            ret *= bnd[i];
        return ret;
    }

    constexpr bool empty() const noexcept
    {
        for (rank_type i = 0; i < Rank; ++i) {
            if (bnd[i] == 0)
                return true;
        }
        return false;
    }

    // element access
    constexpr reference operator[](const indices_type& idx) const noexcept
    {
        assert(details::is_within_extents<Rank>(idx, bnd));
        auto ptr = data_ptr;
        for (int i = 0; i < Rank; i++) {
            ptr += idx[i] * srd[i];
        }
        return *ptr;
    }

    //todo: probably this could be solved in a general way with some metaprogramming
    //with variadic templates
    //however I'm not sure a variadic template could handle
    //the case where slice_bounds is initialized with {x, y} which has no
    //type on its own
    constexpr reference operator()(size_t x) const noexcept
    {
        static_assert(Rank == 1, "operator() must be called with Rank number of arguments");
        return data_ptr[x * srd[0]];
    }
    constexpr array_view<T, 1> operator()(slice_bounds x) const noexcept
    {
        static_assert(Rank == 1, "operator() must be called with Rank number of arguments");
        return array_view<T, 1>(data_ptr, x.length(bnd[0]), srd);
    }
    constexpr reference operator()(size_t x, size_t y) const noexcept
    {
        static_assert(Rank == 2, "operator() must be called with Rank number of arguments");
        return data_ptr[x * srd[0] + y * srd[1]];
    }
    constexpr array_view<T, 1> operator()(slice_bounds x, size_t y) const noexcept
    {
        static_assert(Rank == 2, "operator() must be called with Rank number of arguments");
        return array_view<T, 1>(data_ptr + x.from.index(bnd[0]) * srd[0] + y * srd[1], x.length(bnd[0]), srd[0]);
    }
    constexpr array_view<T, 1> operator()(size_t x, slice_bounds y) const noexcept
    {
        static_assert(Rank == 2, "operator() must be called with Rank number of arguments");
        return array_view<T, 1>(data_ptr + x * srd[0] + y.from.index(bnd[1]) * srd[1], y.length(bnd[1]), srd[1]);
    }
    constexpr array_view<T, 2> operator()(slice_bounds x, slice_bounds y) const noexcept
    {
        static_assert(Rank == 2, "operator() must be called with Rank number of arguments");
        return array_view<T, 2>(data_ptr + x.from.index(bnd[0]) * srd[0] + y.from.index(bnd[1]) * srd[1], { x.length(bnd[0]), y.length(bnd[1]) }, srd);
    }

    struct iterator;

    // traversal
    iterator begin() const
    {
        iterator it;
        prepare_iterator(it);
        it.idx.fill(0);
        return it;
    }
    iterator end() const
    {
        iterator it;
        prepare_iterator(it);
        int i = it.dim_permut.back();
        it.idx[i] = bnd[i];
        return it;
    }

private:
    // helper functions
    template <typename U,
        typename = std::enable_if_t<!std::is_const<T>::value
            && std::is_convertible<U, T>::value> >
    void copy_from_same_shape_array_view(const array_view<U, Rank>& x) const
    {
        //todo this could be optimized for special cases
        //(for example, two min(strides)=1 array_views, with same strides
        assert(bnd == x.extents());
        auto it_end = this->end();
        for (auto it = this->begin(); it != it_end; ++it) {
            *it = x[it.indices()];
        }
    }
    // helper functions
    template <typename Rng>
    void copy_from_range_to_1d(Rng&& x) const
    {
        static_assert(Rank == 1, "");
        assert(extents(0) == ranges::end(x) - ranges::begin(x));
        auto it = ranges::begin(x);
        //        auto e = ranges::end(x);
        for (size_t i = 0; i < extents(0); ++i, ++it)
            (*this)[i] = *it;
    }
    void prepare_iterator(iterator& it) const
    {
        it.that = this;
        std::iota(it.dim_permut.begin(), it.dim_permut.end(), 0);
        auto strides = srd;
        std::sort(
            make_random_access_iterator_pair(strides.begin(), it.dim_permut.begin()),
            make_random_access_iterator_pair(strides.end(), it.dim_permut.end()));
        size_t s = 1;
        for (int i = 0; i < Rank; ++i) {
            auto dpi = it.dim_permut[i];
            s *= extents(dpi);
            it.cumprod_extents[dpi] = s;
        }
    }

public:
    // linearizer iterator
    // traverses array_view in an efficient manner (no jumps)
    //todo treat rank==1 as special case (specialize template)
    struct iterator
        : public std::iterator<std::random_access_iterator_tag,
              value_type, std::ptrdiff_t, pointer, reference> {
        friend class array_view;

    private:
        array_view const* that = nullptr;
        indices_type idx;
        std::array<int, Rank> dim_permut; //strides[dim_permut[i]] is sorted
        std::array<size_t, Rank> cumprod_extents; //cumprod in the order of dim_permut
        // cumprod_extents[dim_permut[i]] = strides[dim_permut[0]] * .. * strides[dim_permut[i]]

        using iterator_base = std::iterator<std::random_access_iterator_tag,
            value_type, std::ptrdiff_t, pointer, reference>;

    public:
        using this_type = iterator;
        using iterator_category = typename iterator_base::iterator_category;
        using value_type = typename iterator_base::value_type;
        using difference_type = typename iterator_base::difference_type;
        using pointer = typename iterator_base::pointer;
        using reference = typename iterator_base::reference;

        // construction, assignment
        iterator()
        {
            idx.fill(0);
            dim_permut.fill(0);
            cumprod_extents.fill(0);
        }
        iterator(const this_type&) = default;
        iterator& operator=(const this_type&) = default;

        // observers
        constexpr reference operator*() const noexcept { return (*that)[idx]; }
        constexpr pointer operator->() const noexcept { return &(operator*()); }
        reference operator[](difference_type n) const
        {
            auto new_lin_idx = (difference_type)to_linear_idx() + n;
            assert(0 <= new_lin_idx && new_lin_idx <= cumprod_extents[dim_permut[Rank - 1]]);
            indices_type new_idx;
            from_linear_idx(n, new_idx);
            return that->operator[](new_idx);
        }
        constexpr const indices_type& indices() const noexcept { return idx; }

        // modifiers
        this_type& operator++()
        {
            rank_type i = 0;
            for (; i < Rank; ++i) {
                int j = dim_permut[i];
                if (++(idx[j]) != that->extents(j) || i + 1 == Rank)
                    break;
                idx[j] = 0;
            }
            return *this;
        }
        this_type operator++(int)
        {
            this_type x(*this);
            ++(*this);
            return x;
        }
        this_type& operator--()
        {
            rank_type i = 0;
            for (; i < Rank; ++i) {
                int j = dim_permut[i];
                if ((idx[j])-- != 0 || i + 1 == Rank)
                    break;
                idx[j] = that->extents(j) - 1;
            }
            return *this;
        }
        this_type operator--(int) const
        {
            this_type x(*this);
            --(*this);
            return x;
        }
        void swap(this_type& y)
        {
            using std::swap;
            swap(that, y.that);
            swap(idx, y.idx);
            swap(dim_permut, y.dim_permut);
            swap(cumprod_extents, y.cumprod_extents);
        }
        //like x % y but the handling of negative x is such that
        //the cyclic pattern of the remainder continues
        static inline size_t cyclic_remainder(difference_type x, size_t y)
        {
            assert(y > 0);
            if (x >= 0)
                return x % y;
            return (y - (-x % y)) % y;
        }
        size_t to_linear_idx() const
        {
            size_t r = 0;
            size_t cextj_prev = 1;
            for (rank_type i = 0; i < Rank; ++i) {
                auto j = dim_permut[i];
                r += cextj_prev * idx[j];
                cextj_prev = cumprod_extents[j];
            }
            return r;
        }
        void from_linear_idx(size_t n, indices_type& result) const
        {
            assert(0 <= n && n <= cumprod_extents[dim_permut[Rank - 1]]);
            size_t cextj_prev = 1;
            for (rank_type i = 0; n != 0 && i < Rank; ++i) {
                auto j = dim_permut[i];
                const size_t cextj = cumprod_extents[j];
                result[j] = cyclic_remainder(n, cextj) / cextj_prev;
                n -= result[j] * cextj_prev;
                cextj_prev = cextj;
            }
        }
        this_type& operator+=(difference_type n)
        {
            auto new_lin_idx = (difference_type)to_linear_idx() + n;
            assert(0 <= new_lin_idx && new_lin_idx <= cumprod_extents[dim_permut[Rank - 1]]);
            from_linear_idx(new_lin_idx, idx);
            return *this;
        }
        this_type& operator-=(difference_type y)
        {
            (*this) += (-y);
            return *this;
        }

        // comparison
        bool operator==(const this_type& x) const
        {
            assert(that == x.that);
            return that == x.that && idx == x.idx;
        }
        bool operator!=(const this_type& y) const { return !(*this == y); }
        bool operator<(const this_type& x) const
        {
            assert(that == x.that);
            assert(dim_permut == x.dim_permut);
            for (int i = Rank - 1; i >= 0; --i) {
                auto dpi = dim_permut[i];
                auto rhs = idx[dpi];
                auto lhs = x.idx[dpi];
                if (rhs < lhs)
                    return true;
                if (rhs > lhs)
                    return false;
            }
            return false; //they're equal
        }
        bool operator>(const this_type& y) const { return y < *this; }
        bool operator>=(const this_type& y) const { return !(*this < y); }
        bool operator<=(const this_type& y) const { return !(*this > y); }

        //operations
        this_type operator+(difference_type y) const
        {
            this_type x(*this);
            x += y;
            return x;
        }
        this_type operator-(difference_type y) const
        {
            this_type x(*this);
            x -= y;
            return x;
        }
        difference_type operator-(const this_type& y)
        {
            return static_cast<difference_type>(to_linear_idx()) - static_cast<difference_type>(y.to_linear_idx());
        }
    };
};

template <typename T, rank_type Rank>
inline void swap(
    typename array_view<T, Rank>::iterator& x,
    typename array_view<T, Rank>::iterator& y)
{
    x.swap(y);
}

template <typename T, rank_type Rank>
inline typename array_view<T, Rank>::iterator operator+(
    typename array_view<T, Rank>::iterator::difference_type x,
    const typename array_view<T, Rank>::iterator& y)
{
    return y + x;
}

template <typename T, typename U, rank_type Rank,
    typename = std::enable_if_t<std::is_same<std::remove_cv_t<T>, std::remove_cv_t<U> >::value> >
constexpr bool is_same_view(const array_view<T, Rank>& x,
    const array_view<U, Rank>& y) noexcept
{
    return x.data() == y.data() && x.extents() == y.extents() && x.strides() == y.strides();
}

template <rank_type Rank = 1, typename T>
constexpr array_view<T, Rank> make_array_view(T* data, details::extents_template<Rank> e,
    details::indices_template<Rank> s) noexcept
{
    return { data, e, s };
}

template <typename T>
constexpr array_view<T> make_array_view(std::vector<T>& v) noexcept
{
    return { v.data(), v.size(), 1 };
}

template <typename T>
constexpr array_view<const T> make_array_view(const std::vector<T>& v) noexcept
{
    return { v.data(), v.size(), 1 };
}

template <typename T>
using matrix_view = array_view<T, 2>;

inline std::string mat2str(double d)
{
    if (std::isnan(d))
        return "NaN";
    if (std::isinf(d))
        return d < 0 ? "-Inf" : "Inf";
    int i = 0;
    double d2 = d;
    const double kEps = 1e-15;
    for (; i <= 15; ++i, d2 *= 10.0) {
        if (fabs(round(d2) - d2) < kEps)
            break;
    }
    char buf[100];
    sprintf(buf, "%.*f", i, d);
    return buf;
}

inline std::string mat2str(int i)
{
    char buf[100];
    sprintf(buf, "%d", i);
    return buf;
}

template <typename T,
    typename = std::enable_if_t<std::is_arithmetic<T>::value> >
std::string mat2str(array_view<T, 1> X)
{
    std::string result;
    result = "[";
    bool first = true;
    for (auto x : X) {
        if (!first)
            result += " ";
        else
            first = false;
        result += mat2str(x);
    }
    result += "]";
    return result;
}

template <typename T,
    typename = std::enable_if_t<std::is_arithmetic<T>::value> >
std::string mat2str(array_view<T, 2> x)
{
    std::string result;
    result = "[";
    bool first_row = true;
    for (size_t i = 0; i < x.extents(0); ++i) {
        if (!first_row)
            result += ";";
        else
            first_row = false;
        bool first_col = true;
        for (size_t j = 0; j < x.extents(1); ++j) {
            if (!first_col)
                result += " ";
            else
                first_col = false;
            result += num2str(x(i, j));
        }
    }
    result += "]";
    return result;
}

template <typename T,
    typename = std::enable_if_t<std::is_arithmetic<T>::value> >
std::string mat2str(const std::vector<T>& x)
{
    return mat2str(make_array_view(x));
}

} //namespace sx

#endif // _IMPL_ARRAY_VIEW_H_
