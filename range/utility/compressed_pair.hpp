#ifndef COMPRESSED_PAIR_INCLUDED_269348234
#define COMPRESSED_PAIR_INCLUDED_269348234

#include <utility>

namespace ranges {
	inline namespace sx {
		template<typename F, typename S>
		using compressed_pair = std::pair<F, S>;
	}
}


#endif
