#include "sx/algorithm.h"

#include <numeric>
#include "simple_test.hpp"
#include "v.h"

struct S {
    S() = default;
    S(const S&) = delete;
    S(S&&) {}
};

void f2(S&& s)
{
}

void f(S&& s)
{
    using Q = decltype(s);
    using W = S&&;
    static_assert(std::is_same<Q, W>::value, "");
    f2((Q)(s));
}

int main()
{

    // unique
    {
        const V v1 = { 1, 2, 3, 3, 4, 4, 5 };
        const V v2 = { 5, 3, 4, 4, 2, 3, 1 };
        V vt = { 1, 2, 3, 4, 5 };
        V w;
        w = v1;
        sx::unique(w);
        CHECK(w == vt);

        w = v1;
        auto r = sx::unique(std::move(w));
        CHECK(r == vt);

        w = v2;
        sx::sort_unique(w);
        CHECK(w == vt);

        w = v2;
        r = sx::sort_unique(std::move(w));
        CHECK(r == vt);
    }

    // insert_at_end
    {
        V v1 = { 1, 2, 3 };
        const V v2 = { 4, 5, 6 };
        const V vt = { 1, 2, 3, 4, 5, 6 };

        V v;
        v = v1;
        sx::insert_at_end(v, v2.begin(), v2.end());
        CHECK(v == vt);

        v = v1;
        sx::insert_at_end(v, v2);
        CHECK(v == vt);
    }

    // set_difference
    {
        const V v1 = { 1, 2, 3, 4, 5 };
        const V v2 = { 2, 4, 5 };
        const V vt = { 1, 3 };
        V c(v1.size());
        sx::set_difference(v1, v2, c);
        CHECK(c == vt);

        c = sx::set_difference(v1, v2, V(v1.size()));
        CHECK(c == vt);

        auto d = sx::set_difference(v1, v2);
        static_assert(std::is_same<decltype(d), std::vector<int> >::value, "");
        CHECK(d == vt);
    }

    // set_intersection
    {
        const V v1 = { 1, 2, 4, 5 };
        const V v2 = { 2, 3, 5 };
        const V vt = { 2, 5 };
        V c(v1.size());
        sx::set_intersection(v1, v2, c);
        CHECK(c == vt);

        c.assign(10, 10);
        c = sx::set_intersection(v1, v2, V(v1.size()));
        CHECK(c == vt);

        auto d = sx::set_intersection(v1, v2);
        static_assert(std::is_same<decltype(d), std::vector<int> >::value, "");
        CHECK(d == vt);
    }

    // scalar_rdiv_range
    {
        const V v1 = { 3, 4, 6 };
        const V vt = { 4, 3, 2 };
        V v2;
        v2 = v1;
        CHECK(vt == sx::scalar_rdiv_range(std::move(v2), 12));

        v2 = v1;
        sx::scalar_rdiv_range(v2, 12);
        CHECK(vt == v2);
    }

    // range_div_scalar
    {
        const V v1 = { 3, 6, 9 };
        const V vt = { 1, 2, 3 };
        V v2;
        v2 = v1;
        CHECK(vt == sx::range_div_scalar(std::move(v2), 3));

        v2 = v1;
        sx::range_div_scalar(v2, 3);
        CHECK(vt == v2);
    }

    // range_mul_scalar
    {
        const V v1 = { 3, 6, 9 };
        const V vt = { 9, 18, 27 };
        V v2;
        v2 = v1;
        CHECK(vt == sx::range_mul_scalar(std::move(v2), 3));

        v2 = v1;
        sx::range_mul_scalar(v2, 3);
        CHECK(vt == v2);
    }

    {
        const V a = { 10, 20, 30, 40, 50, 60, 70, 80, 90 };
        const V v = { 35, 10, 5, 100, 90, 65, 70, 90 };

        auto r = sx::searchsorted<char>(a, v);

        static_assert(
            std::is_same<std::remove_reference_t<decltype(*begin(r))>, char>::value, "");

        CHECK(r.size() == v.size());

        for (int i = 0; i < v.size(); ++i) {
            auto it = std::lower_bound(a.begin(), a.end(), v[i]);
            CHECK((it - a.begin()) == r[i]);
        }
    }

    {
        V a = { 10, 4, 5, 23, 54 };
        CHECK(std::accumulate(BEGINEND(a), 0) == sx::sum(a));
    }
    return test_result();
}
