#include <cstdlib>
#include "sx/abbrev.h"
#include <vector>
#include <numeric>
#include "simple_test.hpp"
#include "v.h"

struct S1 {
    V v;
    typename V::const_iterator begin() const { return v.begin(); }
    typename V::const_iterator end() const { return v.end(); }
    typename V::iterator begin() { return v.begin(); }
    typename V::iterator end() { return v.end(); }
};

struct S2 {
    V v;
};

typename V::const_iterator begin(const S2& s) { return s.v.begin(); }
typename V::const_iterator end(const S2& s) { return s.v.end(); }
typename V::iterator begin(S2& s) { return s.v.begin(); }
typename V::iterator end(S2& s) { return s.v.end(); }

int main(int argc, const char* argv[]) {

    // test BEGINEND
    S1 s1, s2;
    const int N = 10;

    s1.v.assign(N, 0);
    s2.v.assign(N, 0);

    std::fill(BEGINEND(s1), 1);
    CHECK(std::accumulate(BEGINEND(s1), 0) == N);
    std::fill(BEGINEND(s2), 1);
    CHECK(std::accumulate(BEGINEND(s2), 0) == N);

    const auto& cs1 = s1;
    const auto& cs2 = s2;

    CHECK(std::accumulate(BEGINEND(cs1), 0) == N);
    CHECK(std::accumulate(BEGINEND(cs2), 0) == N);

    return test_result();
}
