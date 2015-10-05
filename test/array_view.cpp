#include "sx/array_view.h"
#include "simple_test.hpp"

int main(int argc, const char* argv[])
{
    using sx::array_view;
    {
        array_view<int> a;
        CHECK(a.empty() == true);
        CHECK(a.size() == 0);
        std::array<int, 3> ab = { { 1, 2, 3 } };
        auto vb = ab.data();
        array_view<int> b0(ab);
        array_view<int> b1(vb, 3);
        array_view<int> b2(vb, 3, 1);
        array_view<int> b3(vb, 3, sx::array_layout::c_order);
        array_view<int> b4(vb, 3, sx::array_layout::fortran_order);
        std::array<int, 3> v1;
        v1.fill(0);
        array_view<int> b5(v1);
        CHECK(b5.size() == 3);
        CHECK(b5[0] == 0);
        b5 <<= ab;
        std::array<array_view<int>*, 6> b;
        b[0] = &b0;
        b[1] = &b1;
        b[2] = &b2;
        b[3] = &b3;
        b[4] = &b4;
        b[5] = &b5;
        for (const auto x : b) {
            if (x != &b5)
                CHECK(x->data() == vb);
            CHECK(x->size() == 3);
            CHECK(x->rank() == 1);
            CHECK(x->extents().size() == 1);
            CHECK(x->strides().size() == 1);
            CHECK(x->extents(0) == 3);
            CHECK(x->strides(0) == 1);
            for (int i = 0; i < 3; ++i)
                CHECK((*x)[i] == i + 1);
            CHECK(x->empty() == false);
        }
        array_view<const int> cb(b1);
        CHECK(cb.data() == vb);
        CHECK(cb.size() == 3);
        CHECK(cb.rank() == 1);
        CHECK(cb.extents().size() == 1);
        CHECK(cb.strides().size() == 1);
        CHECK(cb.extents(0) == 3);
        CHECK(cb.strides(0) == 1);
        for (int i = 0; i < 3; ++i)
            CHECK(cb[i] == i + 1);
    }
    {
        std::array<int, 6> ab = { { 1, 2, 3, 4, 5, 6 } };
        std::array<int, 6> af = { { 1, 4, 2, 5, 3, 6 } };
        auto vb = ab.data();
        auto vf = af.data();
        array_view<int, 2> b0(vb, { 2, 3 }, { 3, 1 });
        array_view<int, 2> b1;
        b1 = b0;
        CHECK(b0.strides(0) == 3);
        CHECK(b0.strides(1) == 1);
        CHECK(b1.strides(0) == 3);
        CHECK(b1.strides(1) == 1);
        array_view<int, 2> b2(vb, { 2, 3 }, sx::array_layout::c_order);
        CHECK(b2.strides(0) == 3);
        CHECK(b2.strides(1) == 1);
        array_view<int, 2> b3(vf, { 2, 3 }, sx::array_layout::fortran_order);
        CHECK(b3.strides(0) == 1);
        CHECK(b3.strides(1) == 2);

        std::array<int, 6> ae, aee;
        ae.fill(0);
        aee.fill(0);
        array_view<int, 2> b4(ae.data(), { 2, 3 }, sx::array_layout::c_order);
        array_view<int, 2> b5(aee.data(), { 2, 3 }, sx::array_layout::fortran_order);
        b4 <<= b3;
        b5 <<= b4;
        CHECK(b4.data() == ae.data());
        CHECK(b5.data() == aee.data());

        std::array<array_view<int, 2>*, 6> b;
        b[0] = &b0;
        b[1] = &b1;
        b[2] = &b2;
        b[3] = &b3;
        b[4] = &b4;
        b[5] = &b5;
        for (auto& x : b) {
            CHECK(x->size() == 6);
            CHECK(x->rank() == 2);
            CHECK(x->extents().size() == 2);
            CHECK(x->strides().size() == 2);
            CHECK(x->extents(0) == 2);
            CHECK(x->extents(1) == 3);
            for (int i = 0; i < 2; ++i)
                for (int j = 0; j < 3; ++j) {
                    auto v = i * 3 + j + 1;
                    CHECK((*x)[{ i, j }] == v);
                    CHECK((*x)(i, j) == v);
                }
            CHECK(x->empty() == false);
        }
    }
    printf("\n");
    return test_result();
}

#if 0
//todo these should be included in the test

    // from ArrayType
    constexpr array_view(ArrayType& data) noexcept

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