#include "sx/array_view.h"
#include "simple_test.hpp"

int main(int argc, const char* argv[])
{
    sx::array_view<int> a;
    return test_result();
}

# if 0

these as start position:
    int
    end
    end - int

these as end positions:
    int
    end
    end - int
    length = int

or all:
    all


    constexpr array_view() noexcept
    constexpr array_view(pointer data, extents_type extents, indices_type strides) noexcept
    constexpr array_view(pointer data, extents_type extents, array_layout_t layout) noexcept

    // relaxed copy ctor
    constexpr array_view(
        const array_view<U, Rank>& rhs) noexcept

    // from Viewable (a non-array_view type that has size() and data())
    // enabled only if this->rank == 1
    _CONSTEXPR array_view(Viewable&& cont)

    // from ArrayType
    constexpr array_view(ArrayType& data) noexcept

    // assigment, shallow copy
    constexpr array_view& operator=(
        const array_view<U, rank>& rhs) noexcept

    // assigment from same-shape array_view (deep copy)
    operator<<=(array_view<U, Rank> x) const

    const array_view&
    operator<<=(Rng&& x) const

    constexpr pointer data() const noexcept { return data_ptr; }

    constexpr const std::array<size_t, Rank>& extents() const noexcept { return bnd; }
    constexpr size_t extents(rank_type i) const noexcept { return bnd[i]; }

    constexpr const std::array<size_t, Rank>& strides() const noexcept { return srd; }
    constexpr size_t strides(rank_type i) const noexcept { return srd[i]; }

    constexpr size_type size() const noexcept
    constexpr bool empty() const noexcept
    constexpr reference operator[](const indices_type& idx) const noexcept

    constexpr reference operator()(size_t x) const noexcept
    constexpr array_view<T, 1> operator()(slice_bounds x) const noexcept
    constexpr reference operator()(size_t x, size_t y) const noexcept
    constexpr array_view<T, 1> operator()(slice_bounds x, size_t y) const noexcept
    constexpr array_view<T, 1> operator()(size_t x, slice_bounds y) const noexcept
    constexpr array_view<T, 2> operator()(slice_bounds x, slice_bounds y) const noexcept

    iterator begin() const
    iterator end() const

    struct iterator {
        constexpr reference operator*() const noexcept { return (*that)[idx]; }
        constexpr pointer operator->() const noexcept { return &(operator*()); }
        constexpr const indices_type& indices() const noexcept { return idx; }
        this_type& operator++()
        this_type operator++(int)
        this_type& operator--()
        this_type operator--(int) const
        void swap(this_type& y)
        this_type& operator+=(difference_type n)
        this_type& operator-=(difference_type y)
        bool operator==(const this_type& x) const
        bool operator!=(const this_type& y) const { return !(*this == y); }
        bool operator<(const this_type& x) const
        bool operator>(const this_type& y) const { return y < *this; }
        bool operator>=(const this_type& y) const { return !(*this < y); }
        bool operator<=(const this_type& y) const { return !(*this > y); }
        this_type operator+(difference_type y) const
        this_type operator-(difference_type y) const
        difference_type operator-(const this_type& y)
    };
};

constexpr array_view<T, Rank> make_array_view(T* data, details::extents_template<Rank> e,
constexpr array_view<T> make_array_view(std::vector<T>& v) noexcept
constexpr array_view<const T> make_array_view(const std::vector<T>& v) noexcept

inline std::string mat2str(double d)
inline std::string mat2str(int i)
std::string mat2str(array_view<T, 1> X)
std::string mat2str(array_view<T, 2> x)
std::string mat2str(const std::vector<T>& x)

#endif