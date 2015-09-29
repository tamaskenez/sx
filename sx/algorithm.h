#ifndef ALGORITHM_INCLUDED_23047234
#define ALGORITHM_INCLUDED_23047234

#include <algorithm>
#include <vector>
#include <cmath>
#include <cassert>

#include "sx/abbrev.h"
#include "range/range_traits.hpp"

namespace sx {

/* Many mutating functions here follow to pattern I found in Niebler's range-v3:

       The same function is implemented in 2 flavors:

           void foo(T& range, ...) {
               //modifies 'range' in-place
               ...
           }

        and

           T foo(T&& range, ...) {
               //modifies 'range'
               ...
               return std::move(range); // and moves it out
           }

        The two use cases:

            std::vector<int> bar;
            ... // work on 'bar'
            foo(bar);

        and

            auto bar = foo(another_foo_returning_vector(...));

    */
template <typename Range>
void unique(Range& rng)
{
    rng.erase(std::unique(BEGINEND(rng)), rng.end());
}

template <typename Range>
Range unique(Range&& rng)
{
    rng.erase(std::unique(BEGINEND(rng)), rng.end());
    return std::move(rng);
}

template <typename Range>
void sort_unique(Range& rng)
{
    std::sort(BEGINEND(rng));
    unique(rng);
}

template <typename Range>
Range sort_unique(Range&& rng)
{
    std::sort(BEGINEND(rng));
    return unique(std::move(rng));
}

template <typename Container, typename First, typename Last>
Container& insert_at_end(Container& c, First first, Last last)
{
    c.insert(c.end(), first, last);
    return c;
}

template <typename Container, typename Rng>
Container& insert_at_end(Container& c, Rng&& rng)
{
    c.insert(c.end(), BEGINEND(rng));
    return c;
}

template <typename R1, typename R2, typename Cont>
void set_difference(R1&& r1, R2&& r2, Cont& cont)
{
    cont.erase(std::set_difference(BEGINEND(r1), BEGINEND(r2), cont.begin()),
        cont.end());
}

template <typename R1, typename R2, typename Cont>
Cont set_difference(R1&& r1, R2&& r2, Cont&& cont)
{
    set_difference(r1, r2, cont);
    return std::move(cont);
}

template <typename R1, typename R2>
std::vector<ranges::range_value_t<R1> > set_difference(R1&& r1, R2&& r2)
{
    return set_difference(r1, r2,
        std::vector<ranges::range_value_t<R1> >(r1.size()));
}

template <typename R1, typename R2, typename Cont>
void set_intersection(R1&& r1, R2&& r2, Cont& cont)
{
    cont.erase(std::set_intersection(BEGINEND(r1), BEGINEND(r2), cont.begin()),
        cont.end());
}

template <typename R1, typename R2, typename Cont>
Cont set_intersection(R1&& r1, R2&& r2, Cont&& cont)
{
    set_intersection(r1, r2, cont);
    return std::move(cont);
}

template <typename R1, typename R2>
std::vector<ranges::range_value_t<R1> > set_intersection(R1&& r1, R2&& r2)
{
    return set_intersection(r1, r2,
        std::vector<ranges::range_value_t<R1> >(r2.size()));
}

// return x \ y, that is y / x
template <typename X, typename Y>
X scalar_rdiv_range(X&& x, const Y& y)
{
    for (auto& xi : x)
        xi = y / xi;
    return std::move(x);
}

// performs x \= y, that is x = x \ y, that is x = y / x
template <typename X, typename Y>
void scalar_rdiv_range(X& x, const Y& y)
{
    for (auto& xi : x)
        xi = y / xi;
}

// return x / y
template <typename X, typename Y>
X range_div_scalar(X&& x, const Y& y)
{
    for (auto& xi : x)
        xi /= y;
    return std::move(x);
}

// perform x /= y
template <typename X, typename Y>
void range_div_scalar(X& x, const Y& y)
{
    for (auto& xi : x)
        xi /= y;
}

// return x * y
template <typename X, typename Y>
X range_mul_scalar(X&& x, const Y& y)
{
    for (auto& xi : x)
        xi *= y;
    return std::move(x);
}

// perform x *= y
template <typename X, typename Y>
void range_mul_scalar(X& x, const Y& y)
{
    for (auto& xi : x)
        xi *= y;
}

// like numpy searchsorted
// it's like a vectorized lower_bound returning indices instead of iterators
template <typename SizeT, typename RangeA, typename RangeV>
std::vector<SizeT> searchsorted(RangeA&& a, RangeV&& v)
{
    std::vector<SizeT> result;
    result.reserve(v.size());
    for (auto& x : v)
        result.push_back(std::lower_bound(BEGINEND(a), x) - a.begin());
    return result;
}

template <typename T, typename Rng>
T sum(Rng&& rng)
{
    T s = T{};
    for (auto v : rng)
        s += v;
    return s;
}

template <typename Rng>
ranges::range_value_t<Rng> sum(Rng&& rng)
{
    return sum<ranges::range_value_t<Rng>, Rng>(rng);
}

template <typename Rng,
    typename = std::enable_if_t<!std::is_fundamental<Rng>::value> >
void log(Rng& rng)
{
    for (auto& v : rng)
        v = std::log(v);
}

template <typename Rng,
    typename = std::enable_if_t<!std::is_fundamental<Rng>::value> >
Rng log(Rng&& rng)
{
    log(rng);
    return std::move(rng);
}

template <typename T, typename Rng>
T mean(Rng&& rng)
{
    assert(!rng.empty());
    return static_cast<T>(sum<T, Rng>(rng) / rng.size());
}

template <typename Rng>
ranges::range_value_t<Rng> mean(Rng&& rng)
{
    return mean<ranges::range_value_t<Rng>, Rng>(rng);
}

template <typename X, typename Y, typename Z>
bool leq_and_leq(const X& x, const Y& y, const Z& z)
{
    return x <= y && y <= z;
}

template <typename X, typename Y, typename Z>
bool less_and_leq(const X& x, const Y& y, const Z& z)
{
    return x < y && y <= z;
}

template <typename X, typename Y, typename Z>
bool less_and_less(const X& x, const Y& y, const Z& z)
{
    return x < y && y < z;
}

template <typename Rng>
bool isempty(Rng&& rng) { return rng.empty(); }

// bincount(<rvalue-for-result>, range) -> <result>
// bincount(<lvalue-for-result>, range) -> void
// bincount<T>(range) -> std::vector<T>
// works like numpy.bincount
template <typename ResultType, typename Rng,
    typename = std::enable_if_t<std::is_integral<ranges::range_value_t<Rng> >::value> >
void bincount(std::vector<ResultType>& result, Rng&& rng)
{
    if (rng.empty())
        result.clear();
    else {
        auto mm = std::minmax_element(BEGINEND(rng));
        assert(*mm.first >= 0);
        result.assign(*mm.second + 1, 0);
        for (auto& x : rng)
            ++result[x];
    }
}

template <typename ResultType, typename Rng,
    typename = std::enable_if_t<std::is_integral<ranges::range_value_t<Rng> >::value> >
std::vector<ResultType> bincount(std::vector<ResultType>&& result, Rng&& rng)
{
    bincount(result, rng);
    return std::move(result);
}

template <typename T, typename Rng,
    typename = std::enable_if_t<std::is_integral<ranges::range_value_t<Rng> >::value> >
std::vector<T> bincount(Rng&& x)
{
    return bincount(std::vector<T>(), x);
}
}

#endif
