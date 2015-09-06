#ifndef _IMPL_ARRAY_VIEW_H_
#define _IMPL_ARRAY_VIEW_H_ 1

#include <stdexcept>
#include <type_traits>
#include <utility>
#include "coordinate.h"

namespace ARRAY_VIEW_NAMESPACE {

using rank_type = std::size_t;
using index_type = std::size_t;
using size_type = std::size_t;
using extent_type = std::size_t;

template <typename ValueType, rank_type Rank>
class strided_array_view;

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
    struct is_strided_array_view_oracle : std::false_type {
    };
    template <typename T, rank_type N>
    struct is_strided_array_view_oracle<strided_array_view<T, N> > : std::true_type {
    };

    template <typename T>
    struct is_strided_array_view : is_strided_array_view_oracle<std::decay_t<T> > {
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

    // like std::array but allows more constructors
    template <typename T, rank_type Rank>
    struct arraylike
        : public std::array<T, Rank> {
        using base_type = std::array<T, Rank>;
        using typename base_type::value_type;

        arraylike() = default;
        arraylike(const arraylike&) = default;
        template<typename U>
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

        template <typename U>
        _CONSTEXPR arraylike(std::initializer_list<U> il)
        {
            assert(il.size() == Rank);
            std::copy_n(il.begin(), Rank, base_type::begin());
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

    template <typename ValueType, rank_type Rank>
    class any_array_view_base {
    public:
        static const rank_type rank = Rank;
        using rank_type = ::sx::rank_type;
        using index_type = ::sx::index_type;
        using size_type = ::sx::size_type;
        using extent_type = ::sx::extent_type;
        using indices_type = indices_template<Rank>;
        using extents_type = extents_template<Rank>;
        using value_type = ValueType;
        using pointer = ValueType*;
        using reference = ValueType&;

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
#if 0
        // Preconditions: for any index idx, if section_bounds.contains(idx),
        // extents().contains(origin + idx) must be true
        _CONSTEXPR strided_array_view<value_type, rank> section(
            const index_type& origin, const bounds_type& section_bnd) const
        {
            assert(bnd.contains(origin));
            assert(check_section_correct(origin, section_bnd));
            return { section_bnd, srd, &operator[](origin) };
        }

        // Preconditions: for any index idx, if section_bounds.contains(idx),
        // extents().contains(origin + idx) must be true
        _CONSTEXPR strided_array_view<value_type, rank> section(
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

template <typename ValueType, rank_type Rank = 1>
class strided_array_view : public details::any_array_view_base<ValueType, Rank> {
    using Base = details::any_array_view_base<ValueType, Rank>;

public:
    using Base::rank;
    using index_type = typename Base::index_type;
    using indices_type = typename Base::indices_type;
    using extents_type = typename Base::extents_type;
    using size_type = typename Base::size_type;
    using value_type = typename Base::value_type;
    using pointer = typename Base::pointer;
    using reference = typename Base::reference;

    _CONSTEXPR strided_array_view() _NOEXCEPT : Base{ nullptr, {}, {} } {}

    // copy ctor
    template <typename ViewValueType,
        typename = std::enable_if_t<std::is_convertible<std::add_pointer_t<ViewValueType>,
                                        pointer>::value
            && std::is_same<std::remove_cv_t<ViewValueType>,
                                        std::remove_cv_t<value_type> >::value> >
    _CONSTEXPR strided_array_view(
        const strided_array_view<ViewValueType, rank>& rhs) _NOEXCEPT
        : Base{ rhs.data_ptr, rhs.bnd, rhs.srd }
    {
    }

    template <typename ViewValueType,
        typename = std::enable_if_t<std::is_convertible<std::add_pointer_t<ViewValueType>,
                                        pointer>::value
            && std::is_same<std::remove_cv_t<ViewValueType>,
                                        std::remove_cv_t<value_type> >::value> >
    _CONSTEXPR strided_array_view& assign_view(
        const strided_array_view<ViewValueType, rank>& rhs) _NOEXCEPT
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
    _CONSTEXPR strided_array_view(pointer data, extents_type bounds, indices_type stride) _NOEXCEPT
        : Base{ data, std::move(bounds), std::move(stride) }
    {
    }

    // from Viewable (has size, data) which is not array_view, only if
    // this->rank
    // == 1
    template <
        typename Viewable,
        typename = std::enable_if_t<rank == 1 && details::is_viewable<Viewable, value_type>::value && !details::is_strided_array_view<Viewable>::value> >
    _CONSTEXPR strided_array_view(Viewable&& cont)
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
    _CONSTEXPR strided_array_view(ArrayType& data) _NOEXCEPT
        : Base{
            details::to_pointer(data), details::make_bounds<ArrayType>(),
            details::make_stride(details::make_bounds<ArrayType>()),
        }
    {
    }

    using Base::operator[];

#if 0
    // Returns a slice of the view.
    // Preconditions: slice < (*this).extents()[0]
    template <rank_type _dummy_rank = rank>
    _CONSTEXPR details::slice_return_type_t<ARRAY_VIEW_NAMESPACE::strided_array_view, value_type, Rank>
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
};

template <typename T, typename U, rank_type Rank,
    typename = std::enable_if_t<std::is_same<std::remove_cv_t<T>, std::remove_cv_t<U> >::value> >
bool is_same_view(const strided_array_view<T, Rank>& x,
    const strided_array_view<U, Rank>& y)
{
    return x.data() == y.data() && x.extents() == y.extents() && x.strides() == y.strides();
}

struct end {};
template<
smart_index operator-(end, )
struct length {};
struct smart_index {
    index_type idx;
    enum kind_t { SI_ABSOLUTE, SI_FROM_END, SI_LENGTH } kind;
};
template<typename T, rank_type Rank>
array_view<T> subvector(array_view<T, Rank> v,
    array_view<T, Rank>::indices_type idx,
    rank_type dim,
    size_type size = end,
    index_type stride = 1
    ){
    return ;
}

make_array_view(
    &X[idx], X.extents(dim), X.strides(dim)
    );

}

#endif // _IMPL_ARRAY_VIEW_H_
