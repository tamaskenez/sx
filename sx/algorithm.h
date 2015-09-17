#ifndef ALGORITHM_INCLUDED_23047234
#define ALGORITHM_INCLUDED_23047234

#include <algorithm>
#include <vector>

#include "sx/abbrev.h"
#include "range/range_traits.hpp"

namespace sx {

	template<typename Range>
	void unique_range(Range& rng) {
		rng.erase(std::unique(BEGINEND(rng)), rng.end());
	}

	template<typename Range>
	Range unique_range(Range&& rng) {
		rng.erase(std::unique(BEGINEND(rng)), rng.end());
        return std::move(rng);
	}

    template<typename Range>
	void sort_unique_range(Range& rng) {
		std::sort(BEGINEND(rng));
		unique_range(rng);
	}

	template<typename Range>
	Range sort_unique_range(Range&& rng) {
		std::sort(BEGINEND(rng));
		return unique_range(std::move(rng));
	}

	template<typename Container, typename First, typename Last>
	void insert_at_end(Container& c, First first, Last last) {
		c.insert(c.end(), first, last);
		return c;
	}

    template<typename R1, typename R2>
    void set_difference_range(R1& r1, R2&& r2) {
        r1.erase(std::set_difference(BEGINEND(r1), BEGINEND(r2)), ranges::end(r1));
    }

    template<typename R1, typename R2>
    R1 set_difference_range(R1&& r1, R2&& r2) {
        r1.erase(std::set_difference(BEGINEND(r1), BEGINEND(r2)), ranges::end(r1));
        return std::move(r1);
    }

    template<typename X, typename Y>
    Y scalar_div_range(const X& x, Y&& y) {
        for(auto&yi: y) yi = x / yi;
        return std::move(y);
    }

    template<typename X, typename Y>
    void scalar_div_range(const X& x, Y& y) {
        for(auto&yi: y) yi = x / yi;

    }

    template<typename X, typename Y>
    X range_div_scalar(X&& x, const Y& y) {
        for(auto&xi: x) xi /= y;
        return std::move(x);
    }

    template<typename X, typename Y>
    void range_div_scalar(X&& x, const Y& y) {
        for(auto&xi: x) xi /= y;
    }

	//like numpy searchsorted
	//it's like a vectorized lower_bound returning indices instead of iterators
	template<typename SizeT, typename RangeA, typename RangeV>
	std::vector<SizeT> searchsorted(RangeA&& a, RangeV&& v) {
		std::vector<SizeT> result;
		result.reserve(v.size());
		for(auto&x: v)
			result.push_back(std::lower_bound(BEGINEND(a), x) - a.begin());
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
		return sum<ranges::range_value_t<Rng>, Rng>(rng);
	}

	template<typename T, typename Rng>
	T mean(Rng&& rng) {
		assert(!rng.empty());
		return static_cast<T>(sum<T, Rng>(rng) / rng.size());
	}

	template<typename Rng>
	ranges::range_value_t<Rng> mean(Rng&& rng) {
		return mean<ranges::range_value_t<Rng>, Rng>(rng);
	}


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
}

#endif
