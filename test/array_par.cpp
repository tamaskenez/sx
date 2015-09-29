#include "sx/array_par.h"
#include "simple_test.hpp"

using sx::array_par;

void f(array_par<int, 3> v, int d)
{
    for (int i = 0; i < 3; ++i)
        CHECK(v[i] == (i + d));
}

int main(int argc, const char* argv[])
{

    {
        array_par<int, 3> a;
        for (auto i : a)
            CHECK(i == 0);
    }

#define C(A, B)                 \
    for (int i = 0; i < 3; ++i) \
        CHECK(A[i] == B[i]);
    {
        array_par<int, 3> cai = { 4, 5, 6 };
        std::array<int, 3> cbc = { { 7, 8, 9 } };
        {
            array_par<int, 3> ai(cai);
            C(ai, cai)
        }
        {
            array_par<int, 3> ai(cbc);
            C(ai, cbc)
            CHECK(ai.base() == cbc);
            CHECK(const_cast<const array_par<int, 3>*>(&ai)->base() == cbc);
        }
        {
            array_par<int, 3> ai;
            ai = cai;
            C(ai, cai)
        }
        {
            array_par<int, 3> ai;
            ai = cbc;
            C(ai, cbc)
        }
        {
            array_par<int, 1> q(3);
            CHECK(q[0] == 3);
        }
        {
            array_par<int, 3> q(1, 2, 3);
            for (int i = 0; i < 3; ++i)
                CHECK(q[i] == (i + 1));
        }
        {
            f({ 1, 2, 3 }, 1);
            array_par<int, 3> cai = { 4, 5, 6 };
            f(cai, 4);
            std::array<int, 3> cbc = { { 7, 8, 9 } };
            f(cbc, 7);
        }
    }
    return test_result();
}
