#ifndef _IMPL_ARRAY_VIEW_H_
#define _IMPL_ARRAY_VIEW_H_ 1

#include <stdexcept>
#include <type_traits>
#include <utility>

#include "sx/coordinate.h"
#include "sx/static_const.h"
#include "sx/random_access_iterator_pair.h"

namespace sx {

using rank_type = std::size_t;
using index_type = std::size_t;
using size_type = std::size_t;
using extent_type = std::size_t;

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
    static constexpr array_layout_t fortran_order{ 0 };
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
    _CONSTEXPR auto
    make_bounds() _NOEXCEPT
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

    // like std::array but allows more constructors,
    // default ctor initializes to zero
    template <typename T, rank_type Rank>
    struct arraylike
        : public std::array<T, Rank> {
        using base_type = std::array<T, Rank>;
        using typename base_type::value_type;

        constexpr arraylike() noexcept
        {
            this->fill(0);
        }
        constexpr arraylike(const arraylike&) noexcept = default;
        template <typename U>
        constexpr arraylike(const std::array<U, Rank>& x) noexcept
            : base_type(x)
        {
        }
        constexpr arraylike& operator=(const arraylike&) noexcept = default;
        constexpr arraylike& operator=(const base_type& x) noexcept
        {
            base_type::operator=(x);
            return *this;
        }

        constexpr arraylike(value_type v) noexcept
        {
            static_assert(Rank == 1, "Single-value constructor can be used only if Rank == 1");
            (*this)[0] = v;
        }

        template <typename... Ints, typename = std::enable_if_t<sizeof...(Ints) == Rank
                                        && all_integrals<Ints...>::value> >
        constexpr arraylike(Ints... ints) noexcept
        {
            copy_head_and_advance(base_type::data(), ints...);
        }

        constexpr base_type& base() noexcept { return *this; }
        constexpr const base_type& base() const noexcept { return *this; }
    };

    template <rank_type Rank>
    using extents_template = arraylike<extent_type, Rank>;
    template <rank_type Rank>
    using indices_template = arraylike<index_type, Rank>;

    template <typename T, typename U, rank_type Rank>
    constexpr bool is_within_extents(const std::array<T, Rank>& idx,
        const std::array<U, Rank>& extents) noexcept
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
    constexpr auto&& end = static_const<end_fn>::value;
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
struct length_fn {
    template <typename I,
        typename = std::enable_if<std::is_integral<I>::value> >
    constexpr length_value<I> operator=(I i) const
    {
        return length_value<I>(i);
    }
};

namespace {
    //static global `length` object
    constexpr auto&& length = static_const<length_fn>::value;
}

//type of static global `all` object for the
//expression `all` (= the entire slice)
struct all_fn {
};

namespace {
    //static global `all` object
    constexpr auto&& all = static_const<all_fn>::value;
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

    const index_type value;
    const kind_t kind;

    template <typename I>
    constexpr smart_index_base(I value, kind_t k)
        : value(value)
        , kind(k)
    {
    }

    //returns the absolute, resolved index
    constexpr index_type index(index_type begin_idx, extent_type extent) const
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
    constexpr index_type index_when_not_length(extent_type extent) const
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
    constexpr index_type index(extent_type extent) const
    {
        return smart_index_base::index_when_not_length(extent);
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
        : smart_index_base(l.value, K_LENGTH)
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
    constexpr extent_type length(extent_type extent) const
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
    using index_type = ::sx::index_type;
    using size_type = ::sx::size_type;
    using extent_type = ::sx::extent_type;
    using indices_type = details::indices_template<Rank>;
    using extents_type = details::extents_template<Rank>;
    using value_type = typename std::remove_const<T>::type;
    using pointer = T*;
    using reference = T&;

    static constexpr rank_type rank = Rank;

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
    constexpr array_view(pointer data, extents_type extents, indices_type stride) noexcept
        : data_ptr(data),
          bnd(extents),
          srd(stride)
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
        } else {
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
            && std::is_same<std::remove_cv_t<U>,
                                        std::remove_cv_t<value_type> >::value> >
    constexpr array_view(
        const array_view<U, Rank>& rhs) noexcept
        : array_view(rhs.data_ptr, rhs.bnd, rhs.srd)
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
        : array_view(cont.data(), static_cast<extent_type>(cont.size()), 1)
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
        const array_view<U, rank>& rhs) noexcept
    {
        bnd = rhs.bnd;
        srd = rhs.srd;
        data_ptr = rhs.data_ptr;
        return *this;
    }

    // assigment, deep copy
    template <typename U,
        typename = std::enable_if_t<!std::is_const<T>::value
            && std::is_convertible<U, T>::value> >
    const array_view&
    operator<<=(array_view<U, Rank> x) const
    {
        copy_from_same_shape_array_view(x);
        return *this;
    }

    template <typename U,
        typename = std::enable_if_t<!std::is_const<T>::value
            && std::is_convertible<U, T>::value> >
    array_view&
    operator<<=(array_view<U, Rank> x)
    {
        copy_from_same_shape_array_view(x);
        return *this;
    }

    // observers
    constexpr pointer data() const noexcept { return data_ptr; }

    constexpr const std::array<extent_type, Rank>& extents() const noexcept { return bnd; }
    constexpr extent_type extents(rank_type i) const noexcept { return bnd[i]; }

    constexpr const std::array<index_type, Rank>& strides() const noexcept { return srd; }
    constexpr index_type strides(rank_type i) const noexcept { return srd[i]; }

    constexpr size_type size() const noexcept
    {
        size_type ret = bnd[0];
        for (rank_type i = 1; i < rank; ++i)
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
        assert(is_within_extents(idx, bnd));
        auto ptr = data_ptr;
        for (int i = 0; i < rank; i++) {
            ptr += idx[i] * srd[i];
        }
        return *ptr;
    }

    constexpr reference operator()(index_type x) const noexcept
    {
        static_assert(Rank == 1, "operator() must be called with Rank number of arguments");
        return data_ptr[x * srd[0]];
    }
    constexpr array_view<T, 1> operator()(slice_bounds x) const noexcept
    {
        static_assert(Rank == 1, "operator() must be called with Rank number of arguments");
        return array_view<T, 1>(data_ptr, x.length(bnd[0]), srd);
    }
    constexpr reference operator()(index_type x, index_type y) const noexcept
    {
        static_assert(Rank == 2, "operator() must be called with Rank number of arguments");
        return data_ptr[x * srd[0] + y * srd[1]];
    }
    constexpr array_view<T, 1> operator()(slice_bounds x, index_type y) const noexcept
    {
        static_assert(Rank == 2, "operator() must be called with Rank number of arguments");
        return array_view<T, 1>(data_ptr + x.from.index(bnd[0]) * srd[0] + y * srd[1], x.length(bnd[0]), srd[0]);
    }
    constexpr array_view<T, 1> operator()(index_type x, slice_bounds y) const noexcept
    {
        static_assert(Rank == 2, "operator() must be called with Rank number of arguments");
        return array_view<T, 1>(data_ptr + x * srd[0] + y.from.index(bnd[1]) * srd[1], y.length(bnd[1]), srd[1]);
    }
    constexpr array_view<T, 2> operator()(slice_bounds x, slice_bounds y) const noexcept
    {
        static_assert(Rank == 2, "operator() must be called with Rank number of arguments");
        return array_view<T, 1>(data_ptr + x.from.index(bnd[0]) * srd[0] + y.from.index(bnd[1]) * srd[1], { x.length(bnd[0]), y.length(bnd[1]) }, srd);
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
        it.idx = bnd;
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
        assert(bnd == x.extents());
        auto it_end = this->end();
        for (auto it = this->begin(); it != it_end; ++it) {
            *it = x[it.indices()];
        }
    }
    void prepare_iterator(iterator& it) const
    {
        it.that = this;
        std::iota(it.dim_permut.begin(), it.dim_permut.end(), 0);
        auto strides = srd;
        std::sort(
            make_random_access_iterator_pair(strides.begin(), it.dim_permut.begin()),
            make_random_access_iterator_pair(strides.end(), it.dim_permut.end()));
    }

public:
    // linearizer iterator
    // traverses array_view in an efficient manner (no jumps)
    //todo treat rank==1 as special case (specialize template)
    struct iterator
        : public std::iterator<std::bidirectional_iterator_tag,
              value_type, std::ptrdiff_t, pointer, reference> {
        friend class array_view;

    private:
        const array_view* that;
        indices_type idx;
        std::array<int, Rank> dim_permut; //strides[dim_permut[i]] is sorted
    public:
        // comparison
        constexpr bool operator==(const iterator& x) const noexcept
        {
            assert(that == x.that);
            return that == x.that && idx == x.idx;
        }
        constexpr bool operator!=(const iterator& x) const noexcept { return !((*this) == x); }

        // observers
        constexpr reference operator*() const noexcept { return (*that)[idx]; }
        constexpr pointer operator->() const noexcept { return &(operator*()); }
        constexpr indices_type indices() const noexcept { return idx; }

        // modifiers
        iterator& operator++()
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
        iterator& operator--()
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
        iterator operator++(int)
        {
            iterator x(*this);
            operator++();
            return x;
        }
        iterator operator--(int)
        {
            iterator x(*this);
            operator--();
            return x;
        }
    };
};

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

} //namespace sx

#endif // _IMPL_ARRAY_VIEW_H_
