#ifndef _IMPL_ARRAY_VIEW_H_
#define _IMPL_ARRAY_VIEW_H_ 1

#include <stdexcept>
#include <type_traits>
#include <utility>
#include "coordinate.h"
#include "static_const.h"
#include "sx/random_access_iterator_pair.h"

namespace sx {

using rank_type = std::size_t;
using index_type = std::size_t;
using size_type = std::size_t;
using extent_type = std::size_t;

template <typename T, rank_type Rank>
class array_view;

struct array_layout_t {
    constexpr explicit array_layout_t(int value)
        : value(value)
    {
    }
    constexpr bool operator==(const array_layout_t& x) const { return value == x.value; }
    constexpr bool operator!=(const array_layout_t& x) const { return value != x.value; }
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
    constexpr void copy_head_and_advance(T* b, H h)
    {
        *b = h;
    }
    template <typename T, typename H, typename... Tail>
    constexpr void copy_head_and_advance(T* b, H h, Tail... tail)
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

    // like std::array but allows more constructors
    template <typename T, rank_type Rank>
    struct arraylike
        : public std::array<T, Rank> {
        using base_type = std::array<T, Rank>;
        using typename base_type::value_type;

        arraylike() = default;
        arraylike(const arraylike&) = default;
        template <typename U>
        arraylike(const std::array<U, Rank>& x)
            : base_type(x)
        {
        }
        arraylike& operator=(const arraylike&) = default;
        arraylike& operator=(const base_type& x)
        {
            base_type::operator=(x);
            return *this;
        }

        _CONSTEXPR arraylike(value_type il)
        {
            static_assert(Rank == 1, "Single-value constructor can be used only if Rank == 1");
        }

        template <typename... Ints, typename = std::enable_if_t<sizeof...(Ints) == Rank
                                        && all_integrals<Ints...>::value> >
        constexpr arraylike(Ints... ints)
        {
            copy_head_and_advance(base_type::data(), ints...);
        }

        base_type& base() { return *this; }
        const base_type& base() const { return *this; }
    };

    template <rank_type Rank>
    using extents_template = arraylike<extent_type, Rank>;
    template <rank_type Rank>
    using indices_template = arraylike<index_type, Rank>;

    template <typename T, typename U, rank_type Rank>
    _CONSTEXPR bool is_within_extents(const std::array<T, Rank>& idx,
        const std::array<U, Rank>& extents) _NOEXCEPT
    {
        for (rank_type i = 0; i < Rank; ++i) {
            if (idx[i] < 0 || idx[i] >= extents[i])
                return false;
        }
        return true;
    }

    template <typename U, rank_type Rank>
    _CONSTEXPR bool has_zero_extent(const std::array<U, Rank>& extents) _NOEXCEPT
    {
        for (rank_type i = 0; i < Rank; ++i) {
            if (extents[i] == 0)
                return true;
        }
        return false;
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

namespace details {
    template <typename T, rank_type Rank>
    class any_array_view_base {
    public:
        static const rank_type rank = Rank;
        using rank_type = ::sx::rank_type;
        using index_type = ::sx::index_type;
        using size_type = ::sx::size_type;
        using extent_type = ::sx::extent_type;
        using indices_type = indices_template<Rank>;
        using extents_type = extents_template<Rank>;
        using value_type = typename std::remove_const<T>::type;
        using pointer = T*;
        using reference = T&;

        _CONSTEXPR const typename extents_type::base_type& extents() const _NOEXCEPT { return bnd; }
        _CONSTEXPR const typename extents_type::value_type extents(std::size_t i) const _NOEXCEPT { return bnd[i]; }

        _CONSTEXPR size_type size() const _NOEXCEPT
        {
            size_type ret = bnd[0];
            for (rank_type i = 1; i < rank; ++i)
                ret *= bnd[i];
            return ret;
        }

        _CONSTEXPR const typename indices_type::base_type& strides() const _NOEXCEPT { return srd; }
        _CONSTEXPR const typename indices_type::value_type strides(std::size_t i) const _NOEXCEPT { return srd[i]; }

        // Preconditions: (*this).extents().contains(idx)
        _CONSTEXPR reference operator[](const indices_type& idx) const
        {
            assert(is_within_extents(idx, bnd));
            auto ptr = data_ptr;
            for (int i = 0; i < rank; i++) {
                ptr += idx[i] * srd[i];
            }
            return *ptr;
        }

        constexpr reference operator()(index_type x) const
        {
            static_assert(Rank == 1, "operator() must be called with Rank number of arguments");
            return data_ptr[x * srd[0]];
        }
        constexpr array_view<T, 1> operator()(slice_bounds x) const
        {
            static_assert(Rank == 1, "operator() must be called with Rank number of arguments");
            return array_view<T, 1>(data_ptr, x.length(bnd[0]), srd);
        }
        constexpr reference operator()(index_type x, index_type y) const
        {
            static_assert(Rank == 2, "operator() must be called with Rank number of arguments");
            return data_ptr[x * srd[0] + y * srd[1]];
        }
        constexpr array_view<T, 1> operator()(slice_bounds x, index_type y) const
        {
            static_assert(Rank == 2, "operator() must be called with Rank number of arguments");
            return array_view<T, 1>(data_ptr + x.from.index(bnd[0]) * srd[0] + y * srd[1], x.length(bnd[0]), srd[0]);
        }
        constexpr array_view<T, 1> operator()(index_type x, slice_bounds y) const
        {
            static_assert(Rank == 2, "operator() must be called with Rank number of arguments");
            return array_view<T, 1>(data_ptr + x * srd[0] + y.from.index(bnd[1]) * srd[1], y.length(bnd[1]), srd[1]);
        }
        constexpr array_view<T, 2> operator()(slice_bounds x, slice_bounds y) const
        {
            static_assert(Rank == 2, "operator() must be called with Rank number of arguments");
            return array_view<T, 1>(data_ptr + x.from.index(bnd[0]) * srd[0] + y.from.index(bnd[1]) * srd[1], { x.length(bnd[0]), y.length(bnd[1]) }, srd);
        }

#if 0
        // Preconditions: for any index idx, if section_bounds.contains(idx),
        // extents().contains(origin + idx) must be true
        _CONSTEXPR array_view<value_type, rank> section(
            const index_type& origin, const bounds_type& section_bnd) const
        {
            assert(bnd.contains(origin));
            assert(check_section_correct(origin, section_bnd));
            return { section_bnd, srd, &operator[](origin) };
        }

        // Preconditions: for any index idx, if section_bounds.contains(idx),
        // extents().contains(origin + idx) must be true
        _CONSTEXPR array_view<value_type, rank> section(
            const index_type& origin) const
        {
            assert(bnd.contains(origin));
            bounds_type section_bnd = bnd - origin;
            return section(origin, section_bnd);
        }
#endif
        _CONSTEXPR bool empty() const _NOEXCEPT
        {
            return has_zero_extent(bnd);
        }

    protected:
        _CONSTEXPR any_array_view_base(pointer data, extents_type bnd, indices_type stride) _NOEXCEPT
            : data_ptr(std::move(data)),
              bnd(std::move(bnd)),
              srd(std::move(stride))
        {
        }
        _CONSTEXPR any_array_view_base(pointer data, extents_type bnd, array_layout_t layout) _NOEXCEPT
            : data_ptr(std::move(data)),
              bnd(std::move(bnd))
        {
            if (layout == array_layout::c_order) {
                srd[Rank - 1] = 1;
                for (int i = Rank - 2; i >= 0; --i)
                    srd[i] = srd[i + 1] * bnd[i + 1];
            } else {
                assert(layout == array_layout::fortran_order);
                srd[0] = 1;
                for (int i = 1; i < Rank; ++i)
                    srd[i] = srd[i - 1] * bnd[i - 1];
            }
        }
#if 0
        _CONSTEXPR bool check_section_correct(
            const index_type& origin, const bounds_type& section_bnd) const _NOEXCEPT
        {
            for (int i = 0; i < rank; ++i) {
                if (origin[i] > bnd[i] - section_bnd[i]) {
                    return false;
                }
            }
            return true;
        }
#endif
        pointer data_ptr;
        extents_type bnd;
        indices_type srd;
        // Note: for non-strided array view, stride can be computed on-the-fly
        // thus saving couple of bytes. It should be measured whether it's
        // beneficial.
    };

} // namespace details

template <typename T, rank_type Rank = 1>
class array_view : public details::any_array_view_base<T, Rank> {
    using Base = details::any_array_view_base<T, Rank>;

public:
    using this_type = array_view;
    using Base::rank;
    using index_type = typename Base::index_type;
    using indices_type = typename Base::indices_type;
    using extents_type = typename Base::extents_type;
    using size_type = typename Base::size_type;
    using value_type = typename Base::value_type;
    using pointer = typename Base::pointer;
    using reference = typename Base::reference;

    _CONSTEXPR array_view() _NOEXCEPT : Base{ nullptr, {}, {} } {}

    // copy ctor
    template <typename ViewValueType,
        typename = std::enable_if_t<std::is_convertible<std::add_pointer_t<ViewValueType>,
                                        pointer>::value
            && std::is_same<std::remove_cv_t<ViewValueType>,
                                        std::remove_cv_t<value_type> >::value> >
    _CONSTEXPR array_view(
        const array_view<ViewValueType, rank>& rhs) _NOEXCEPT
        : Base{ rhs.data_ptr, rhs.bnd, rhs.srd }
    {
    }

    template <typename ViewValueType,
        typename = std::enable_if_t<std::is_convertible<std::add_pointer_t<ViewValueType>,
                                        pointer>::value
            && std::is_same<std::remove_cv_t<ViewValueType>,
                                        std::remove_cv_t<value_type> >::value> >
    _CONSTEXPR array_view& assign_view(
        const array_view<ViewValueType, rank>& rhs) _NOEXCEPT
    {
        Base::bnd = rhs.bnd;
        Base::srd = rhs.srd;
        Base::data_ptr = rhs.data_ptr;
        return *this;
    }

    // fundamental explicit ctor
    // Preconditions:
    //   - for any index idx, if extents().contains(idx),
    //     for i = [0,rank), idx[i] * stride[i] must be representable as
    //     ptrdiff_t
    //   - for any index idx, if extents().contains(idx),
    //     (*this)[idx] must refer to a valid memory location
    _CONSTEXPR array_view(pointer data, extents_type bounds, indices_type stride) _NOEXCEPT
        : Base{ data, std::move(bounds), std::move(stride) }
    {
    }

    _CONSTEXPR array_view(pointer data, extents_type bounds, array_layout_t layout) _NOEXCEPT
        : Base{ data, std::move(bounds), layout }
    {
    }

    // from Viewable (has size, data) which is not array_view, only if
    // this->rank
    // == 1
    template <
        typename Viewable,
        typename = std::enable_if_t<rank == 1 && details::is_viewable<Viewable, value_type>::value && !details::is_array_view<Viewable>::value> >
    _CONSTEXPR array_view(Viewable&& cont)
        : Base{ cont.data(), static_cast<typename extents_type::value_type>(cont.size()), 1 }
    {
    }

    // from ArrayType
    // Preconditions: product of the ArrayType extents must be <= ptrdiff_t max.
    template <
        typename ArrayType,
        typename = std::enable_if_t<std::is_convertible<std::add_pointer_t<std::remove_all_extents_t<ArrayType> >,
                                        pointer>::value
            && std::is_same<std::remove_cv_t<std::remove_all_extents_t<ArrayType> >,
                                        std::remove_cv_t<value_type> >::value
            && std::rank<ArrayType>::value == rank> >
    _CONSTEXPR array_view(ArrayType& data) _NOEXCEPT
        : Base{
            details::to_pointer(data), details::make_bounds<ArrayType>(),
            details::make_stride(details::make_bounds<ArrayType>()),
        }
    {
    }

    using Base::operator[];
    using Base::operator();

#if 0
    // Returns a slice of the view.
    // Preconditions: slice < (*this).extents()[0]
    template <rank_type _dummy_rank = rank>
    _CONSTEXPR details::slice_return_type_t<ARRAY_VIEW_NAMESPACE::array_view, value_type, Rank>
    operator[](typename std::enable_if<_dummy_rank != 1,
        typename index_type::value_type>::type
            slice) const _NOEXCEPT
    {
        static_assert(_dummy_rank == rank,
            "_dummy_rank must have the default value!");
        assert(slice < Base::bnd[0]);

        index_type idx;
        idx[0] = slice;

        ARRAY_VIEW_NAMESPACE::bounds<rank - 1> bound;
        ARRAY_VIEW_NAMESPACE::index<rank - 1> stride;
        for (int i = 1; i < rank; ++i) {
            bound[i - 1] = Base::bnd[i];
            stride[i - 1] = Base::srd[i];
        }

        return { bound, stride, &operator[](idx) };
    }
#endif
    _CONSTEXPR pointer data() const _NOEXCEPT
    {
        return Base::data_ptr;
    }
    //todo treat rank==1 as special case (specialize template)
    struct iterator : public std::iterator<std::bidirectional_iterator_tag,
                          value_type, std::ptrdiff_t, pointer, reference> {
        const this_type* that;
        indices_type idx;
        std::array<int, Rank> dim_permut; //strides[dim_permut[i]] is sorted

        reference operator*() const { return (*that)[idx]; }
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
        bool operator==(const iterator& x) const
        {
            assert(that == x.that);
            return that == x.that && idx == x.idx;
        }
        bool operator!=(const iterator& x) const { return !((*this) == x); }
        pointer operator->() const { return &(operator*()); }
    };
    void prepare_iterator(iterator& it) const
    {
        it.that = this;
        std::iota(it.dim_permut.begin(), it.dim_permut.end(), 0);
        auto strides = Base::srd;
        std::sort(
            make_random_access_iterator_pair(strides.begin(), it.dim_permut.begin()),
            make_random_access_iterator_pair(strides.end(), it.dim_permut.end()));
    }
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
        it.idx = Base::bnd;
        return it;
    }
};

template <typename T, typename U, rank_type Rank,
    typename = std::enable_if_t<std::is_same<std::remove_cv_t<T>, std::remove_cv_t<U> >::value> >
bool is_same_view(const array_view<T, Rank>& x,
    const array_view<U, Rank>& y)
{
    return x.data() == y.data() && x.extents() == y.extents() && x.strides() == y.strides();
}

template <typename T, rank_type Rank>
array_view<T> subvector(array_view<T, Rank> av,
    typename array_view<T, Rank>::indices_type idx,
    rank_type dim,
    index_or_fromend_or_length ifl = ::sx::end,
    index_type stride = 1)
{
    return array_view<T>(&av[idx], ifl.index(idx[dim], av.extents(dim)) - av.extents(dim), av.strides(dim));
}

template <typename T, rank_type Rank = 1>
array_view<T, Rank> make_array_view(T* data, details::extents_template<Rank> e,
    details::indices_template<Rank> s)
{
    return { data, std::move(e), std::move(s) };
}
} //namespace sx

#endif // _IMPL_ARRAY_VIEW_H_
