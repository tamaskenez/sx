#include "sx/array_view.h"
#include "sx/range.h"
#include <vector>

int main(int argc, char* argv[]) {
	std::vector<int> vi = { 1, 2, 3 };

    sx::bounds<2> b;
    sx::array_view<int, 2> ize;

	//for (size_t i = 0; i < av.size(); ++i)
	//	printf("%d ", av[i]);
    printf("\n");

    for(auto i: sx::range(2, 5)) {
        printf("%d ", i);
    }

    return 0;
}