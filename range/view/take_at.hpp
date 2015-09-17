#ifndef RANGES_SX_VIEW_TAKE_AT_HPP
#define RANGES_SX_VIEW_TAKE_AT_HPP

#include <cassert>
//#include <atomic>
//#include <utility>
//#include <type_traits>
//#include <meta/meta.hpp>
#include <range/range_fwd.hpp>
//#include <range/v3/size.hpp>
//#include <range/v3/distance.hpp>
#include <range/begin_end.hpp>
#include <range/range_traits.hpp>
#include <range/range_concepts.hpp>
//#include <range/v3/view_adaptor.hpp>
//#include <range/v3/utility/box.hpp>
//#include <range/v3/utility/functional.hpp>
//#include <range/v3/utility/iterator.hpp>
//#include <range/v3/utility/static_const.hpp>
//#include <range/v3/view/view.hpp>
//#include <range/v3/view/all.hpp>
#include "range/range.hpp"

namespace ranges
{
    inline namespace sx
    {
        namespace view
        {
			template<typename T>
			struct is_random_access_iterator_category {
				static const bool value = std::is_base_of<std::random_access_iterator_tag, T>::value;
			};

			template<typename T>
			struct is_random_access_iterator {
				static const bool value = is_random_access_iterator_category<typename std::iterator_traits<T>::iterator_category>::value;
			};

			template<typename T>
			struct has_random_access_iterator {
				static const bool value = is_random_access_iterator<
					range_iterator_t<T>
				>::value;
			};

            //todo certain memfns should be enabled depending
            //on iterator_category
			template<typename D, typename I,
				typename = std::enable_if_t<
					is_random_access_iterator<D>::value
				>
			>
			struct take_at_iterator
			: public std::iterator<
					typename I::iterator_category
					, typename D::value_type
					, typename I::difference_type
					, typename D::pointer
					, typename D::reference
				>
			{
                using iterator = std::iterator<
                typename I::iterator_category
                , typename D::value_type
                , typename I::difference_type
                , typename D::pointer
                , typename D::reference
                >;

            public:
                using typename iterator::iterator_category;
                using typename iterator::value_type;
                using typename iterator::difference_type;
                using typename iterator::pointer;
                using typename iterator::reference;
			private:
				const D data_begin;
				const difference_type data_size;
				I base_it;
			public:
				using this_type = take_at_iterator;

				// construction, assignment
				take_at_iterator() = default;
				take_at_iterator(const this_type&) = default;
				take_at_iterator& operator=(const this_type&) = default;

				take_at_iterator(D data_begin, difference_type data_size, I base_it)
				: data_begin(std::move(data_begin))
				, data_size(data_size)
				, base_it(std::move(base_it))
				{}

				// observers
				reference operator*() const
				{
					assert(0 <= *base_it && *base_it < data_size);
					return data_begin[*base_it];
				}
				pointer operator->() const
				{
					assert(0 <= *base_it && *base_it < data_size);
					return (data_begin + *base_it).operator->();
				}
				decltype(data_begin[*base_it]) operator[](difference_type y) const {
					const auto idx = *base_it[y];
					assert(0 <= idx && idx < data_size);
					return data_begin[idx];
				}

				// modifiers
				this_type& operator++() { ++base_it; return *this; }
				this_type operator++(int) { this_type x(*this); ++(*this); return x; }
				this_type& operator--() { --base_it; return *this; }
				this_type operator--(int) const { this_type x(*this); --(*this); return x; }
				void swap(this_type& y) {
					using std::swap;
					swap(data_begin, y.data_begin);
					swap(base_it, y.base_it);
				}
				this_type& operator+=(difference_type y) { base_it += y; return *this; }
				this_type& operator-=(difference_type y) { (*this) += (-y); return *this; }

				// comparison
				bool operator==(const this_type& y) const
				{ assert(data_begin == y.data_begin); return base_it == y.base_it; }
				bool operator!=(const this_type& y) const { return !(*this==y); }
				bool operator<(const this_type& y) const
				{ assert(data_begin == y.data_begin); return base_it < y.base_it; }
				bool operator>(const this_type& y) const { return y < *this; }
				bool operator>=(const this_type& y) const { return !(*this < y); }
				bool operator<=(const this_type& y) const { return !(*this > y); }

                //operations
                this_type operator+(difference_type y) const { this_type x(*this); x += y; return x; }
                this_type operator-(difference_type y) const { this_type x(*this); x -= y; return x; }
                difference_type operator-(const this_type& y) {
                    return base_it - y.base_it;
                }
			};

			template<typename D, typename I>
			inline void swap(take_at_iterator<D, I>& x, take_at_iterator<D, I>& y) { x.swap(y); }

			template<typename D, typename I>
            inline take_at_iterator<D, I> operator+(typename take_at_iterator<D, I>::difference_type x, const take_at_iterator<D, I>& y) { return y + x; }

			//take_at(data, indices): return data[indices]
            struct take_at_fn
            {
            public:
                template<typename RngData, typename RngIdcs,
					typename = std::enable_if_t<
						has_random_access_iterator<RngData>::value
					>
				>
                range_default_sentinel<
                    take_at_iterator<
                        range_iterator_t<RngData>
                        , range_iterator_t<RngIdcs>
                    >
                >
				operator()(RngData && rngdata, RngIdcs && rngidcs) const
                {
					using it_t = take_at_iterator<range_iterator_t<RngData>, range_iterator_t<RngIdcs>>;
                    using idcs_iterator_cat_t = typename range_iterator_t<RngIdcs>::iterator_category;
                    return {
						it_t(begin(rngdata), rngdata.size(), begin(rngidcs))
						, it_t(begin(rngdata), rngdata.size(), end(rngidcs))
					};
                }
            };

            /// \relates stride_fn
            /// \ingroup group-views
            namespace
            {
                constexpr auto&& take_at = static_const<take_at_fn>::value;
            }
        }
        /// @}
    }
}

#endif
