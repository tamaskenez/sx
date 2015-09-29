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
    static_assert(std::is_same<Q, S&&>::value, ""); //proves that `s` is indeed `S&&`
    //f2(s); //compiler error: no matching f2()
    f2((Q)s); //casting `s` to *its own type* works
}

int main()
{

    // unique
    {
        const VI v1 = { 1, 2, 3, 3, 4, 4, 5 };
        const VI v2 = { 5, 3, 4, 4, 2, 3, 1 };
        VI vt = { 1, 2, 3, 4, 5 };
        VI w;
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
        VI v1 = { 1, 2, 3 };
        const VI v2 = { 4, 5, 6 };
        const VI vt = { 1, 2, 3, 4, 5, 6 };

        VI v;
        v = v1;
        sx::insert_at_end(v, v2.begin(), v2.end());
        CHECK(v == vt);

        v = v1;
        sx::insert_at_end(v, v2);
        CHECK(v == vt);
    }

    // set_difference
    {
        const VI v1 = { 1, 2, 3, 4, 5 };
        const VI v2 = { 2, 4, 5 };
        const VI vt = { 1, 3 };
        VI c(v1.size());
        sx::set_difference(v1, v2, c);
        CHECK(c == vt);

        c = sx::set_difference(v1, v2, VI(v1.size()));
        CHECK(c == vt);

        auto d = sx::set_difference(v1, v2);
        static_assert(std::is_same<decltype(d), std::vector<int> >::value, "");
        CHECK(d == vt);
    }

    // set_intersection
    {
        const VI v1 = { 1, 2, 4, 5 };
        const VI v2 = { 2, 3, 5 };
        const VI vt = { 2, 5 };
        VI c(v1.size());
        sx::set_intersection(v1, v2, c);
        CHECK(c == vt);

        c.assign(10, 10);
        c = sx::set_intersection(v1, v2, VI(v1.size()));
        CHECK(c == vt);

        auto d = sx::set_intersection(v1, v2);
        static_assert(std::is_same<decltype(d), std::vector<int> >::value, "");
        CHECK(d == vt);
    }

    // scalar_rdiv_range
    {
        const VI v1 = { 3, 4, 6 };
        const VI vt = { 4, 3, 2 };
        VI v2;
        v2 = v1;
        CHECK(vt == sx::scalar_rdiv_range(std::move(v2), 12));

        v2 = v1;
        sx::scalar_rdiv_range(v2, 12);
        CHECK(vt == v2);
    }

    // range_div_scalar
    {
        const VI v1 = { 3, 6, 9 };
        const VI vt = { 1, 2, 3 };
        VI v2;
        v2 = v1;
        CHECK(vt == sx::range_div_scalar(std::move(v2), 3));

        v2 = v1;
        sx::range_div_scalar(v2, 3);
        CHECK(vt == v2);
    }

    // range_mul_scalar
    {
        const VI v1 = { 3, 6, 9 };
        const VI vt = { 9, 18, 27 };
        VI v2;
        v2 = v1;
        CHECK(vt == sx::range_mul_scalar(std::move(v2), 3));

        v2 = v1;
        sx::range_mul_scalar(v2, 3);
        CHECK(vt == v2);
    }

    {
        const VI a = { 10, 20, 30, 40, 50, 60, 70, 80, 90 };
        const VI v = { 35, 10, 5, 100, 90, 65, 70, 90 };

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
        const VI a = { 10, 4, 5, 23, 54 };
        auto ds = sx::sum<double>(a);
        static_assert(std::is_same<decltype(ds), double>::value, "");
        CHECK(std::accumulate(BEGINEND(a), 0) == sx::sum(a));
        CHECK(std::accumulate(BEGINEND(a), 0) == ds);
    }
    {
        const VD a = { 10, 4, 5, 23, 54 };
        VD vt;
        for (auto d : a)
            vt.push_back(log(d));
        VD b;
        b = a;
        sx::log(b);
        VD c;
        c = a;
        auto d = sx::log(std::move(c));
        CHECK(vt == b);
        CHECK(vt == d);
    }
    {
        const VI a = { 10, 4, 5, 23, 54 };
        auto ds = sx::mean<double>(a);
        static_assert(std::is_same<decltype(ds), double>::value, "");
        CHECK((std::accumulate(BEGINEND(a), 0) / a.size()) == sx::mean(a));
        CHECK((std::accumulate(BEGINEND(a), 0) / (double)a.size()) == ds);
    }
    {
        const int x = 10;
        const int z = 12;
        for (int y = x - 1; y <= z + 1; ++y) {
            const bool qq = x <= y && y <= z;
            const bool sq = x < y && y <= z;
            const bool ss = x < y && y < z;
            CHECK(sx::leq_and_leq(x, y, z) == qq);
            CHECK(sx::less_and_leq(x, y, z) == sq);
            CHECK(sx::less_and_less(x, y, z) == ss);
        }
    }
    {
        VI a(1), b;
        CHECK(sx::isempty(a) == a.empty());
        CHECK(sx::isempty(b) == b.empty());
    }

    {
        const VI v1 = { 6, 10, 3, 2, 3, 8, 7, 4, 1, 9,
            3, 1, 3, 4, 6, 8, 10, 7, 2, 10 };
        const VI vt = { 0, 2, 2, 4, 2, 0, 2, 2, 2, 1, 3 };
        auto va = sx::bincount(std::vector<int>(100), v1);
        VI v2(50);
        sx::bincount(v2, v1);
        auto v3 = sx::bincount<int>(v1);
        CHECK(va == vt);
        CHECK(v2 == vt);
        CHECK(v3 == vt);
    }

    printf("finished.\n"); //trigger xcode console
    return test_result();
}
