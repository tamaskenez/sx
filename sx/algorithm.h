#ifndef ALGORITHM_INCLUDED_23047234
#define ALGORITHM_INCLUDED_23047234

#include <algorithm>
#include <vector>

#include "sx/abbrev.h"

namespace sx {

	template<typename Range>
	Range unique_inplace(Range&& rng) {
		rng.erase(std::unique(BEGINEND(rng)), rng.end());
		return rng;
	}

	template<typename Range>
	Range sort_unique_inplace(Range&& rng) {
		std::sort(BEGINEND(rng));
		return unique_inplace(std::move(rng));
	}

	template<typename Container, typename First, typename Last>
	Container& append(Container& c, First first, Last last) {
		c.insert(c.end(), first, last);
		return c;
	}

	//like numpy searchsorted
	//it's like a vectorized lower_bound returning indices instead of iterators
	template<typename SizeT, typename RangeA, typename RangeV>
	std::vector<SizeT> searchsorted(RangeA a, RangeV v) {
		std::vector<SizeT> result;
		result.reserve(v.size());
		for(auto&x: v)
			result.push_back(std::lower_bound(BEGINEND(a), v) - a.begin());
		return result;
	}
}

#endif
