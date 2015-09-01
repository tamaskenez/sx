#include "sx/array_view.h"
#include "sx/range.h"
#include "sx/random_access_iterator_pair.h"
#include <vector>
#include <string>
#include <random>
#include <list>

#include <utility>

const int N = 10;

std::vector<int> vi(N);
std::vector<double> vd(N);

auto report = []() {
    printf("REPORT ");
    for(int i = 0; i < N; ++i) {
        printf("%d -> %f\n", vi[i], vd[i]);
        printf("%d ", vi[i]);
    }
//    printf("\n");
};



void f() {
    for(int i = 0; i < N; ++i) {
        vi[i] = (i*57)%10;
        vd[i] = vi[i];
//        vi[i] = 0;
    }


    report();

    printf("---magic---\n");

    std::list<int> li;
    typedef decltype(li.begin()) I1;
    typedef decltype(vd.begin()) I2;
    bool b = sx::is_good_for_random_access_iterator_pair<I1, I2>::value;
    printf("%s\n",b?"yes":"no");

    auto zbegin = sx::make_random_access_iterator_pair(vi.begin(), vd.begin());
    auto zend = sx::make_random_access_iterator_pair(vi.end(), vd.end());

    //std::sort(vi.begin(), vi.end(), [](auto x, auto y)->bool{report(); return x < y;});
    std::sort(zbegin, zend, sx::less_by_first());

    report();
}

int main(int argc, char* argv[]) {
    f();
    return 0;
}
