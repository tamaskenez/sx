#include "sx/array_view.h"
#include <vector>

int main(int argc, char* argv[]) {

	std::vector<int> vi = { 1, 2, 3 };

	auto av = sx::array_view<int>(vi);

	for (size_t i = 0; i < av.size(); ++i)
		printf("%d ", av[i]);

    return 0;
}