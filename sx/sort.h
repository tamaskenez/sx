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

	template<typename R, std::size_t Rank,
	typename = std::enable_if_t<!std::is_const<R>::value>>
	void sortperm_inplace(strided_array_view<R, Rank> X, std::size_t dim = 0) {
		using ResultArray = strided_array_view<R, Rank>;

		// iterate over X, leaving out the 'dim' dimension
		std::array<typename ResultArray::size_type, Rank> it;
		it.fill(0);

		for(;;) {
			auto vbegin = &X[it];
			auto vend = vbegin + X.extents[dim] * X.strides(dim);
			std::sort(vbegin, vend);

			int i = 0;
			for(; i < Rank; ++i) {
				if(i == dim) continue;
				++it[i];
				if(it[i] < X.extents[i]) break;
				it[i] = 0;
			}
			if (i == Rank) break;
		}
	}

	template<typename T, std::size_t Rank>
	multi_array<typename std::remove_const<T>::type, Rank>
	sort(strided_array_view<T, Rank> X, int std::size_t = 0) {
		using V = std::remove_const<T>::type;
		using ResultArray = multi_array<V, Rank>;

		ResultArray R(X.extents());
		sort_inplace(R, dim);
		return R;
	}

	// like Julia's sortperm
	// sorts along 'dim' dimension
	template<typename T, typename U, std::size_t Rank>
	sx::matrix<T> sortperm(sx::strided_array_view<U, 2> X, std::size_t dim) {

	}

}

#endif
