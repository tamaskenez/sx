#include "sx/algorithm.h"
#include "simple_test.hpp"
#include "v.h"


int main() {

	// unique
	{
		const V v1 = {1, 2, 3, 3, 4, 4, 5};
		const V v2 = {5, 3, 4, 4, 2, 3, 1};
		V vt = {1, 2, 3, 4, 5};
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
        V v1 = {1, 2, 3};
        const V v2 = {4, 5, 6};
        const V vt = {1, 2, 3, 4, 5, 6};

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
        const V v1 = {1, 2, 3, 4, 5};
        const V v2 = {2, 4, 5};
        const V vt = {1, 3};
        V c;
        sx::set_difference(v1, v2, c);
        CHECK(c==vt);

        c.assign(10,10);
        c = sx::set_difference(v1, v2, std::move(c));
        CHECK(c==vt);

        auto d = sx::set_difference(v1, v2);
        static_assert(std::is_same<decltype(d), std::vector<int>>::value, "");
        CHECK(d==vt);
    }
    return test_result();
}
