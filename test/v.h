#ifndef V_INCLUDED_273049234
#define V_INCLUDED_273049234

#include <vector>

// like std::vector<int> without copy constructor
// handy for testing if a parameter takes by value
// because of a typo
struct V : std::vector<int> {
    using super = std::vector<int>;
    using super::super;
    V() = default;
    V(const V&) = delete;
    V(V&& x) : super(std::move(x)) {}
    V& operator=(const V& x) = default;
    V& operator==(V&&x) {
        V y(std::move(x));
        swap(y);
        return *this;
    }
};



#endif
