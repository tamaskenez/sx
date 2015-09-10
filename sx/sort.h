#ifndef SORT_INCLUDED_273409823434
#define SORT_INCLUDED_273409823434

namespace sx {

template <typename ForwardItLower, typename ForwardItValue, typename ForwardItUpper>
bool next_variation(
    ForwardItLower lower_bounds,
    ForwardItValue values,
    ForwardItUpper upper_bounds,
    std::size_t N)
{
    for (std::size_t i = 0; i < N; ++i) {
        if (++(*values) != *upper_bounds)
            return true;
        *values = *lower_bounds;
        ++lower_bounds;
        ++values;
        ++upper_bounds;
    }
    return false;
}

template <typename R, rank_type Rank,
    typename = std::enable_if_t<!std::is_const<R>::value> >
void sortperm_inplace(array_view<R, Rank> X, rank_type dim = 0)
{
    using ResultArray = array_view<R, Rank>;

    // iterate over X, leaving out the 'dim' dimension
    std::array<typename ResultArray::size_type, Rank> lower_bounds, it, e;
    lower_bounds.fill(0);
    it.fill(0);
    std::copy_n(X.extents().begin(), Rank, e.begin());
    e[dim] = 1;

    for (;;) {
        auto vbegin = &X[it];
        auto vend = vbegin + X.extents[dim] * X.strides(dim);
        std::sort(vbegin, vend);

        if (!next_variation(lower_bounds.begin(), it.begin(), e.begin()))
            break;
    }
}

template <typename T, rank_type Rank>
multi_array<typename std::remove_const<T>::type, Rank>
sort(array_view<T, Rank> X, rank_type dim = 0)
{
    using V = typename std::remove_const<T>::type;
    using ResultArray = multi_array<V, Rank>;

    ResultArray R(X.extents());
    sort_inplace(R, dim);
    return R;
}

// like Julia's sortperm
// sorts along 'dim' dimension
template <typename T, typename U, rank_type Rank>
multi_array<T, Rank> sortperm(array_view<U, Rank> X, int dim = 0)
{
    using ResultArray = multi_array<T, Rank>;
    ResultArray R(X.extents(), X.strides());

    std::vector<T> w(X.extents(dim));

    // iterate over X, fixing it[dim] to 0
    std::array<typename ResultArray::size_type, Rank> lower_bounds, it, e;
    lower_bounds.fill(0);
    it.fill(0);
    std::copy_n(X.extents().begin(), Rank, e.begin());
    e[dim] = 1;

    for (;;) {
        auto rbegin = &R[it];
        auto rend = rbegin + R.extents(dim) * R.strides(dim);
        std::iota(rbegin, rend, 0);
        auto xv = make_array_view<1>(&X[it], X.extents(dim), X.strides(dim));
        std::copy(BEGINEND(xv), w.begin());
        std::sort(
            make_random_access_iterator_pair(w.begin(), rbegin),
            make_random_access_iterator_pair(w.end(), rend));

        assert(R.extents(dim) == w.size());
        std::copy(BEGINEND(w),
            make_array_view<1>(&R[it], R.extents(dim), R.strides(dim)).begin());

        if (!next_variation(lower_bounds.begin(), it.begin(), e.begin(), Rank))
            break;
    }
    return R;
}
}

#endif
