#ifndef V_INCLUDED_273049234
#define V_INCLUDED_273049234

#include <vector>

// like std::vector<int> without copy constructor
// handy for testing if a parameter takes by value
// because of a typo
template <typename T>
struct vector_nocopy : std::vector<T> {
    using super = std::vector<T>;
    using super::super;
    vector_nocopy() = default;
    vector_nocopy(const vector_nocopy&) = delete;
    vector_nocopy(vector_nocopy&& x)
        : super(std::move(x))
    {
    }
    vector_nocopy& operator=(const vector_nocopy& x) = default;
    vector_nocopy& operator==(vector_nocopy&& x)
    {
        vector_nocopy y(std::move(x));
        swap(y);
        return *this;
    }
};

using VI = vector_nocopy<int>;
using VD = vector_nocopy<double>;

#endif
