#ifndef SORT_INCLUDED_273409823434
#define SORT_INCLUDED_273409823434


namespace sx {

	namespace detail {
		template<typename SizeType, std::size_t Rank>
		struct enumerate_elems_except_along_one_dim {
			std::array<SizeType, Rank>& extents;
			std::size_t dim;
			enumerate_elems_except_along_one_dim(
				const std::array<SizeType, Rank>& extents, std::size_t dim)
				: extents(extents)
				, dim(dim)
				{}
			)
		}
	}
  
  template<typename ForwardItLower, typename ForwardItValue, typename ForwardItUpper>
  bool next_variation(
                 ForwardItLower lower_bounds,
                 ForwardItValue values,
                 ForwardItUpper upper_bounds,
                      std::size_t N) {
    for(std::size_t i = 0; i < N; ++i) {
      ;
      if(++(*values) != *upper_bounds) return true;
      *values = *lower_bounds;
      ++lower_bounds;
      ++values;
      ++upper_bounds;
    }
    return false;
  }

	template<typename R, std::size_t Rank,
	typename = std::enable_if_t<!std::is_const<R>::value>>
	void sortperm_inplace(strided_array_view<R, Rank> X, std::size_t dim = 0) {
		using ResultArray = strided_array_view<R, Rank>;

		// iterate over X, leaving out the 'dim' dimension
		std::array<typename ResultArray::size_type, Rank> lower_bounds, it, e;
    lower_bounds.fill(0);
		it.fill(0);
    std::copy_n(X.extents().begin(), Rank, e.begin());
    e[dim] = 1;

		for(;;) {
			auto vbegin = &X[it];
			auto vend = vbegin + X.extents[dim] * X.strides(dim);
			std::sort(vbegin, vend);

      if(!next_variation(lower_bounds.begin(), it.begin(), X.extents().begin()))
        break;
		}
	}

	template<typename T, std::size_t Rank>
	multi_array<typename std::remove_const<T>::type, Rank>
	sort(strided_array_view<T, Rank> X, std::size_t dim = 0) {
		using V = std::remove_const<T>::type;
		using ResultArray = multi_array<V, Rank>;

		ResultArray R(X.extents());
		sort_inplace(R, dim);
		return R;
	}

	// like Julia's sortperm
	// sorts along 'dim' dimension
	template<typename T, typename U, std::size_t Rank>
  multi_array<T, Rank> sortperm(strided_array_view<U, Rank> X, std::size_t dim = 0) {
    using ResultArray = multi_array<T, Rank>;
    ResultArray R(X.extents());
    
    std::vector<T> w(v.extents(dim));

    // iterate over X, leaving out the 'dim' dimension
    std::array<typename ResultArray::size_type, Rank> lower_bounds, it, e;
    lower_bounds.fill(0);
    it.fill(0);
    std::copy_n(X.extents().begin(), Rank, e.begin());
    e[dim] = 1;
    
    for(;;) {
      auto rbegin = &R[it];
      auto rend = rbegin + R.extents(dim) * R.strides(dim);
      std::iota(rbegin, rend, 0);
      auto xbegin = ;
      std::copy_n(&X[it], X.extents(dim), w.begin());
      std::sort(
              make_random_access_iterator_pair(w.begin(), rbegin),
              make_random_access_iterator_pair(w.end(), rend));
      }
      return ix;
	}
}

#endif
