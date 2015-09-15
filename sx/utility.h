#ifndef UTILITY_INCLUDED_298649283479238
#define UTILITY_INCLUDED_298649283479238

#include <vector>
#include "range/range_traits.hpp"

namespace sx {

	template<typename Rng>
	ranges::range_value_t<Rng> make_vector(Rng&& rng) {
		return {begin(rng), end(rng)};
	}
}

#endif
