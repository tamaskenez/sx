#ifndef MATRIX_INCLUDED_7204384234
#define MATRIX_INCLUDED_7204384234

#include <vector>
#include <array>

#include <cassert>

//todo const indices& or by value, is there a difference? 0 or 1
#define SX_MULTI_ARRAY_PASS_INDICES_BY_VALUE 0

namespace sx {

    namespace detail {
        template<typename T, std::size_t N>
        struct offset
        : public std::array<T, N> {
            using super_type = std::array<T, N>;
            using typename super_type::value_type;

            offset() = default;
            offset(const offset&) = default;
            offset& operator=(const offset&) = default;

            _CONSTEXPR offset(value_type il) {
                static_assert(N == 1, "Single-value constructor can be used only if N == 1");
            }

            template<typename U>
            _CONSTEXPR offset(std::initializer_list<U> il) {
                assert(il.size() == N);
                std::copy_n(il.begin(), N, super_type::begin());
            }
        };
    }
template<typename T, std::size_t Rank>
class multi_array {
    std::vector<T> d;
public:
    using size_type              = std::size_t;
    using indices_type           = std::array<size_type, Rank>;
    using value_type             = T;
    using offset_type            = detail::offset<size_type, Rank>;
    static constexpr size_t rank = Rank;

    multi_array() = default;
    multi_array(const multi_array& x) : d(x.d) {}
    multi_array(multi_array&& x) : d(std::move(x.d)) {}
    multi_array& operator=(const multi_array& x); //todo
    multi_array& operator=(multi_array&& x); //todo

#if SX_MULTI_ARRAY_PASS_INDICES_BY_VALUE
    //todo op[](size_type ) if rank = 1
    T& operator[](const offset_type& offset);
    const T& operator[](const offset_type& offset) const;
#else
    T& operator[](offset_type offset);
    const T& operator[](offset_type offset) const;
#endif
};

template<typename T> using matrix = multi_array<T, 2>;

}

#endif
