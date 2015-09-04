template <typename ValueType, int Rank = 1>
class array_view : public details::any_array_view_base<ValueType, Rank>
{
    using Base = details::any_array_view_base<ValueType, Rank>;
    template <typename AnyValueType, int AnyRank> friend class array_view;
    template <typename AnyValueType, int AnyRank> friend class strided_array_view;

public:
    using Base::rank;
    using index_type  = typename Base::index_type;
    using bounds_type = typename Base::bounds_type;
    using size_type   = typename Base::size_type;
    using value_type  = typename Base::value_type;
    using pointer     = typename Base::pointer;
    using reference   = typename Base::reference;

    _CONSTEXPR array_view() _NOEXCEPT
        : Base{ {}, {}, nullptr }
    {
    }

    // from Viewable (has size, data) which is not array_view to rank = 1
    template <typename Viewable,
        typename = std::enable_if_t<rank == 1
            && details::is_viewable<Viewable, value_type>::value
            && !details::is_array_view<Viewable>::value>>
    _CONSTEXPR array_view(Viewable&& cont)
        : Base{ static_cast<typename bounds_type::value_type>(cont.size()), 1, cont.data() }
    {
    }

    // from ArrayType
    // Preconditions: product of the ArrayType extents must be <= ptrdiff_t max.
    template <typename ArrayType,
        typename = std::enable_if_t<std::is_convertible<std::add_pointer_t<std::remove_all_extents_t<ArrayType>>, pointer>::value
            && std::is_same<std::remove_cv_t<std::remove_all_extents_t<ArrayType>>, std::remove_cv_t<value_type>>::value
            && std::rank<ArrayType>::value == rank>>
    _CONSTEXPR array_view(ArrayType& data) _NOEXCEPT
        : Base{ details::make_bounds<ArrayType>(), details::make_stride(details::make_bounds<ArrayType>()),
                details::to_pointer(data) }
    {
    }

    _CONSTEXPR array_view(bounds_type bounds, pointer data)
        : Base{ bounds, details::make_stride(bounds), data }
    {
    }

    template <typename ViewValueType,
        typename = std::enable_if_t<std::is_convertible<std::add_pointer_t<ViewValueType>, pointer>::value
            && std::is_same<std::remove_cv_t<ViewValueType>, std::remove_cv_t<value_type>>::value>>
    _CONSTEXPR array_view& operator=(const array_view<ViewValueType, rank>& rhs) _NOEXCEPT
    {
        Base::bnd = rhs.bnd;
        Base::srd = rhs.srd;
        Base::data_ptr = rhs.data_ptr;
        return *this;
    }

    using Base::operator[];

    // Returns a slice of the view.
    // Preconditions: slice < (*this).bounds()[0]
    template <int _dummy_rank = rank>
    _CONSTEXPR typename details::slice_return_type<std::experimental::D4087::array_view, value_type, Rank>::type
        operator[](typename std::enable_if<_dummy_rank != 1, typename index_type::value_type>::type slice) const
    {
        static_assert(_dummy_rank == rank, "_dummy_rank must have the default value!");
        assert(slice < Base::bnd[0]);

        index_type idx;
        idx[0] = slice;

        std::experimental::D4087::bounds<rank - 1> bound;
        for (int i = 1; i < rank; ++i)
        {
            bound[i - 1] = Base::bnd[i];
        }

        return{ bound, &operator[](idx) };
    }

    _CONSTEXPR pointer data() const _NOEXCEPT
    {
        return Base::data_ptr;
    }
};
