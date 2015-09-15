#ifndef ALGORITHM_INCLUDED_23047234
#define ALGORITHM_INCLUDED_23047234

#include <algorithm>
#include <vector>

#include "sx/abbrev.h"
#include "range/range_traits.hpp"

namespace sx {

	template<typename Range>
	void unique_inplace(Range& rng) {
		rng.erase(std::unique(BEGINEND(rng)), rng.end());
	}

    template<typename Range>
	void sort_unique_inplace(Range& rng) {
		std::sort(BEGINEND(rng));
		unique_inplace(rng);
	}

	template<typename Container, typename First, typename Last>
	void append_inplace(Container& c, First first, Last last) {
		c.insert(c.end(), first, last);
		return c;
	}

	//like numpy searchsorted
	//it's like a vectorized lower_bound returning indices instead of iterators
	template<typename SizeT, typename RangeA, typename RangeV>
	std::vector<SizeT> searchsorted(RangeA&& a, RangeV&& v) {
		std::vector<SizeT> result;
		result.reserve(v.size());
		for(auto&x: v)
			result.push_back(std::lower_bound(BEGINEND(a), v) - a.begin());
		return result;
	}

	template<typename T, typename Rng>
	T sum(Rng&& rng) {
		T s = T{};
		for(auto v: rng)
			s += v;
		return s;
	}

	template<typename Rng>
	ranges::range_value_t<Rng> sum(Rng&& rng) {
		return sum<ranges::range_value_t<Rng>, Rng>(std::forward(rng));
	}

	template<typename T, typename Rng>
	T mean(Rng&& rng) {
		assert(!rng.empty());
		return static_cast<T>(sum<T, Rng>(std::forward(rng)) / rng.size());
	}

	//template<typename Rng>
	//ranges::range_value_t<Rng> mean(Rng&& rng) {
	//	return mean<ranges::range_value_t<Rng>, Rng>(std::forward(rng));
	//}


	template<typename X, typename Y, typename Z>
	bool leq_and_leq(const X& x, const Y& y, const Z& z) {
		return x <= y && y <= z;
	}

	template<typename X, typename Y, typename Z>
	bool less_and_leq(const X& x, const Y& y, const Z& z) {
		return x < y && y <= z;
	}

	template<typename X, typename Y, typename Z>
	bool less_and_less(const X& x, const Y& y, const Z& z) {
		return x < y && y < z;
	}

	template<typename Rng>
	bool isempty(Rng&& rng) { return rng.empty(); }

	template<typename Rng>
	bool length(Rng&& rng) { return rng.size(); }
}

#endif
