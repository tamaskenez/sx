template <typename ValueType, int Rank = 1>
class strided_array_view : public details::any_array_view_base<ValueType, Rank>
{
    using Base = details::any_array_view_base<ValueType, Rank>;
    friend strided_array_view<const ValueType, Rank>;

public:
    using Base::rank;
    using index_type  = typename Base::index_type;
    using bounds_type = typename Base::bounds_type;
    using size_type   = typename Base::size_type;
    using value_type  = typename Base::value_type;
    using pointer     = typename Base::pointer;
    using reference   = typename Base::reference;

    _CONSTEXPR strided_array_view() _NOEXCEPT
        : Base{ {}, {}, nullptr }
    {
    }

    // copy ctor
    template <typename ViewValueType,
        typename = std::enable_if_t<std::is_convertible<std::add_pointer_t<ViewValueType>, pointer>::value
            && std::is_same<std::remove_cv_t<ViewValueType>, std::remove_cv_t<value_type>>::value>>
    _CONSTEXPR strided_array_view(const strided_array_view<ViewValueType, rank>& rhs) _NOEXCEPT
        : Base{ rhs.bnd, rhs.srd, rhs.data_ptr }
    {
    }

    // fundamental explicit ctor
    // Preconditions:
    //   - for any index idx, if bounds().contains(idx),
    //     for i = [0,rank), idx[i] * stride[i] must be representable as ptrdiff_t
    //   - for any index idx, if bounds().contains(idx),
    //     (*this)[idx] must refer to a valid memory location
    _CONSTEXPR strided_array_view(bounds_type bounds, index_type stride, pointer data) _NOEXCEPT
        : Base{ std::move(bounds), std::move(stride), data }
    {
    }

    // from Viewable (has size, data) which is not array_view to rank = 1
    template <typename Viewable,
        typename = std::enable_if_t<rank == 1
            && details::is_viewable<Viewable, value_type>::value
            && !details::is_strided_array_view<Viewable>::value>>
    _CONSTEXPR strided_array_view(Viewable&& cont)
        : Base{ static_cast<typename bounds_type::value_type>(cont.size()), 1, cont.data() }
    {
    }

    // from ArrayType
    // Preconditions: product of the ArrayType extents must be <= ptrdiff_t max.
    template <typename ArrayType,
        typename = std::enable_if_t<std::is_convertible<std::add_pointer_t<std::remove_all_extents_t<ArrayType>>, pointer>::value
            && std::is_same<std::remove_cv_t<std::remove_all_extents_t<ArrayType>>, std::remove_cv_t<value_type>>::value
            && std::rank<ArrayType>::value == rank>>
    _CONSTEXPR strided_array_view(ArrayType& data) _NOEXCEPT
        : Base{ details::make_bounds<ArrayType>(), details::make_stride(details::make_bounds<ArrayType>()),
                details::to_pointer(data) }
    {
    }

/* shallow assign
    template <typename ViewValueType,
        typename = std::enable_if_t<std::is_convertible<std::add_pointer_t<ViewValueType>, pointer>::value
            && std::is_same<std::remove_cv_t<ViewValueType>, std::remove_cv_t<value_type>>::value>>
    _CONSTEXPR strided_array_view& operator=(const strided_array_view<ViewValueType, rank>& rhs) _NOEXCEPT
    {
        Base::bnd = rhs.bnd;
        Base::srd = rhs.srd;
        Base::data_ptr = rhs.data_ptr;
        return *this;
    }
*/
    using Base::operator[];

    // Returns a slice of the view.
    // Preconditions: slice < (*this).bounds()[0]
    template <int _dummy_rank = rank>
    _CONSTEXPR details::slice_return_type_t<SX_ARRAY_VIEW_NAMESPACE::strided_array_view, value_type, Rank>
        operator[](typename std::enable_if<_dummy_rank != 1, typename index_type::value_type>::type slice) const _NOEXCEPT
    {
        static_assert(_dummy_rank == rank, "_dummy_rank must have the default value!");
        assert(slice < Base::bnd[0]);

        index_type idx;
        idx[0] = slice;

        SX_ARRAY_VIEW_NAMESPACE::bounds<rank - 1> bound;
        SX_ARRAY_VIEW_NAMESPACE::index<rank - 1> stride;
        for (int i = 1; i < rank; ++i)
        {
            bound[i - 1] = Base::bnd[i];
            stride[i - 1] = Base::srd[i];
        }

        return{ bound, stride, &operator[](idx) };
    }
};
