/// \file
// Range v3 library
//
//  Copyright Eric Niebler 2013-2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//

#ifndef RANGES_SX_RANGE_TRAITS_HPP
#define RANGES_SX_RANGE_TRAITS_HPP

//#include <utility>
//#include <iterator>
//#include "sx/type_traits.h"
//#include <meta/meta.hpp>
#include <range/range_fwd.hpp>
#include <range/begin_end.hpp>
#include <range/range_concepts.hpp>

namespace ranges {
inline namespace sx {
    /// \addtogroup group-core
    /// @{

    // Aliases (SFINAE-able)
    template <typename Rng>
    using range_iterator_t = concepts::Range::iterator_t<Rng>;

    //        template<typename Rng>
    //        using range_sentinel_t = concepts::Range::sentinel_t<Rng>;
    //
    template <typename Rng>
    using range_difference_t = typename range_iterator_t<Rng>::difference_type;
    //
    template <typename Rng>
    using range_size_t = typename std::make_unsigned<range_difference_t<Rng> >::type;
    //
    //        template<typename Rng>
    //        using range_value_t = concepts::InputRange::value_t<Rng>;

    template <typename Rng>
    using range_value_t = std::remove_cv_t<
        typename std::iterator_traits<range_iterator_t<Rng> >::value_type>;

    //        template<typename Rng>
    //        using range_reference_t = concepts::InputRange::reference_t<Rng>;
    //
    //        template<typename Rng>
    //        using range_rvalue_reference_t = concepts::InputRange::rvalue_reference_t<Rng>;
    //
    //        template<typename Rng>
    //        using range_common_reference_t = concepts::InputRange::common_reference_t<Rng>;
    //
    //        template<typename Rng>
    //        using range_category_t = concepts::InputRange::category_t<Rng>;
    //
    //        template<typename Rng>
    //        using range_common_iterator_t = common_iterator<range_iterator_t<Rng>, range_sentinel_t<Rng>>;
    //
    //        template<typename Rng>
    //        using range_safe_iterator_t = decltype(ranges::safe_begin(std::declval<Rng>()));
    //
    //        template<typename Rng>
    //        using range_safe_sentinel_t = decltype(ranges::safe_end(std::declval<Rng>()));
    //
    //        // Metafunctions
    //        template<typename Rng>
    //        using range_iterator = meta::defer<range_iterator_t, Rng>;
    //
    //        template<typename Rng>
    //        using range_sentinel = meta::defer<range_sentinel_t, Rng>;
    //
    //        template<typename Rng>
    //        using range_category = meta::defer<range_category_t, Rng>;
    //
    //        template<typename Rng>
    //        using range_value = meta::defer<range_value_t, Rng>;
    //
    //        template<typename Rng>
    //        using range_difference = meta::defer<range_difference_t, Rng>;
    //
    //        template<typename Rng>
    //        using range_reference = meta::defer<range_reference_t, Rng>;
    //
    //        template<typename Rng>
    //        using range_rvalue_reference = meta::defer<range_rvalue_reference_t, Rng>;
    //
    //        template<typename Rng>
    //        using range_common_reference = meta::defer<range_common_reference_t, Rng>;
    //
    //        template<typename Rng>
    //        using range_size = meta::defer<range_size_t, Rng>;
    //
    //        /// \cond
    //        namespace detail
    //        {
    //            std::integral_constant<cardinality, finite> test_cardinality(void *);
    //            template<cardinality Card>
    //            std::integral_constant<cardinality, Card> test_cardinality(basic_view<Card> *);
    //        }
    //        /// \endcond
    //
    //        // User customization point for specifying the cardinality of ranges:
    //        template<typename Rng, typename Void /*= void*/>
    //        struct range_cardinality
    //          : decltype(detail::test_cardinality(static_cast<Rng *>(nullptr)))
    //        {};
    //
    //        template<typename Rng>
    //        struct range_cardinality<Rng &>
    //          : range_cardinality<Rng>
    //        {};
    //
    //        template<typename Rng>
    //        struct range_cardinality<Rng const>
    //          : range_cardinality<Rng>
    //        {};
    //
    //        /// @}
}
}

#endif
