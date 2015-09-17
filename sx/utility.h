#ifndef UTILITY_INCLUDED_298649283479238
#define UTILITY_INCLUDED_298649283479238

#include <vector>
#include "range/range_traits.hpp"

namespace sx {

	template<typename T, typename Rng>
	std::vector<T> make_vector(Rng&& rng) {
		return {ranges::begin(rng), ranges::end(rng)};
	}
	template<typename Rng>
	std::vector<ranges::range_value_t<Rng>> make_vector(Rng&& rng) {
		return {ranges::begin(rng), ranges::end(rng)};
	}
}

#endif
