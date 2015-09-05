#ifndef RANDOM_ACCESS_ITERATOR_PAIR_INCLUDED_742023423
#define RANDOM_ACCESS_ITERATOR_PAIR_INCLUDED_742023423

namespace sx {

// defines:

// template<typename I1, typename I2>
// struct random_access_iterator_pair;

// which can be constructed using:

// template<typename I1, typename I2>
// random_access_iterator_pair<I1, I2> make_random_access_iterator_pair(I1 it1, I2 it2);

// which is similar to boost::zip_iterator except
// - not as sophisticated and of lower quality and less complete
// - but aims to be a RandomAccess + Output iterator so
//   can be used for std::sort:

// This is not a symmetrical iterator pair, 'first' leads the operations
//'second' only follows

// These helper structs can be used for sorting:

struct less_by_first;
struct less_by_first_and_second;

// like this:

// auto b = make_random_access_iterator_pair(v1.begin(), v2.begin());
// auto e = make_random_access_iterator_pair(v1.end(), v2.end());
// std::sort(b, e, less_by_first());

// the two iterators must have
// - same difference_type
// - both random access iterator
template <typename I1, typename I2>
struct is_good_for_random_access_iterator_pair {
    static const bool value = std::is_same<
                                  typename std::iterator_traits<I1>::difference_type,
                                  typename std::iterator_traits<I2>::difference_type>::value
        && std::is_same<
                                  typename std::iterator_traits<I1>::iterator_category,
                                  std::random_access_iterator_tag>::value
        && std::is_same<
                                  typename std::iterator_traits<I1>::iterator_category,
                                  std::random_access_iterator_tag>::value;
};

template <typename I1, typename I2>
struct random_access_iterator_pair_reference;

template <typename I1, typename I2>
struct random_access_iterator_pair_pointer;

// helper class for common derived types
template <typename I1, typename I2>
struct random_access_iterator_pair_traits {
    using traits1 = std::iterator_traits<I1>;
    using traits2 = std::iterator_traits<I2>;

    using value_type1 = typename traits1::value_type;
    using value_type2 = typename traits2::value_type;
    using reference1 = typename traits1::reference;
    using reference2 = typename traits2::reference;

    using difference_type = typename traits1::difference_type;
    using reference = random_access_iterator_pair_reference<I1, I2>;
    using pointer = random_access_iterator_pair_pointer<I1, I2>;
    using value_type = std::pair<value_type1, value_type2>;
};

//the proxy of 'reference', models std::pair<I1::reference, I2::reference>
template <typename I1, typename I2>
struct random_access_iterator_pair_reference {
    using traits = random_access_iterator_pair_traits<I1, I2>;
    using value_type = typename traits::value_type;
    using reference1 = typename traits::reference1;
    using reference2 = typename traits::reference2;

    using this_type = random_access_iterator_pair_reference;

    reference1 first;
    reference2 second;

    random_access_iterator_pair_reference(const this_type&) = default;
    random_access_iterator_pair_reference(reference1 r1, reference2 r2)
        : first(r1)
        , second(r2)
    {
    }

    this_type& operator=(const this_type& x)
    {
        first = x.first;
        second = x.second;
        return *this;
    }

    this_type& operator=(this_type&& x)
    {
        first = std::move(x.first);
        second = std::move(x.second);
        return *this;
    }

    this_type& operator=(const value_type& x)
    {
        first = x.first;
        second = x.second;
        return *this;
    }

    this_type& operator=(value_type&& x)
    {
        first = std::move(x.first);
        second = std::move(x.second);
        return *this;
    }

    operator value_type() const
    {
        return { first, second };
    }
};

//corresponding free swap functions
//todo: the other two combinations could also be provided
//or implemented in a single template swap with strict enable_if to
//constrain it to random_access_iterator_pair_reference
template <typename I1, typename I2>
void swap(random_access_iterator_pair_reference<I1, I2>& x, random_access_iterator_pair_reference<I1, I2>& y)
{
    std::swap(x.first, y.first);
    std::swap(x.second, y.second);
}

template <typename I1, typename I2>
void swap(random_access_iterator_pair_reference<I1, I2>&& x, random_access_iterator_pair_reference<I1, I2>&& y)
{
    std::swap(x.first, y.first);
    std::swap(x.second, y.second);
}

// proxy for 'pointer'
template <typename I1, typename I2>
struct random_access_iterator_pair_pointer {
    using traits = random_access_iterator_pair_traits<I1, I2>;
    using reference = typename traits::reference;
    using reference1 = typename traits::reference1;
    using reference2 = typename traits::reference2;

    using this_type = random_access_iterator_pair_pointer<I1, I2>;

    reference r;

    random_access_iterator_pair_pointer(reference1 r1, reference2 r2)
        : r(r1, r2)
    {
    }
    random_access_iterator_pair_pointer(const this_type&) = default;

    //a normal pointer can be changed, this object can't be since it's a reference
    this_type& operator=(const this_type&) = delete;

    reference* operator->() const { return &r; }
    reference operator*() const { return r; }
};

// the actual random access iterator pair, models random access + input + output
// iterator
template <typename I1, typename I2, typename = std::enable_if_t<is_good_for_random_access_iterator_pair<I1, I2>::value> >
struct random_access_iterator_pair {

    using traits = random_access_iterator_pair_traits<I1, I2>;
    using value_type = typename traits::value_type;
    using reference = typename traits::reference;
    using pointer = typename traits::pointer;
    using difference_type = typename traits::difference_type;

    using iterator_category = std::random_access_iterator_tag;

    using this_type = random_access_iterator_pair<I1, I2>;

    I1 it1;
    I2 it2;

    random_access_iterator_pair() = default;
    random_access_iterator_pair(const this_type& x) = default;
    random_access_iterator_pair(this_type&& x)
        : it1(std::move(x.it1))
        , it2(std::move(x.it2))
    {
    }

    random_access_iterator_pair(const I1& it1, const I2& it2)
        : it1(it1)
        , it2(it2)
    {
    }
    random_access_iterator_pair(I1&& it1, I2&& it2)
        : it1(std::move(it1))
        , it2(std::move(it2))
    {
    }
    this_type& operator=(const this_type& x)
    {
        it1 = x.it1;
        it2 = x.it2;
        return *this;
    }
    this_type& operator==(this_type&& x)
    {
        it1 = std::move(x.it1);
        it2 = std::move(x.it2);
        return *this;
    }

    reference operator*() const
    {
        return { *it1, *it2 };
    }

    this_type& operator++()
    {
        ++it1;
        ++it2;
        return *this;
    }

    bool operator==(const this_type& x) const
    {
        return it1 == x.it1;
    }
    bool operator!=(const this_type& x) const
    {
        return !(*this == x);
    }

    pointer operator->() const
    {
        return { *it1, *it2 };
    }

    this_type operator++(int)
    {
        this_type r(*this);
        ++it1;
        ++it2;
        return r;
    }

    this_type& operator--()
    {
        --it1;
        --it2;
        return *this;
    }
    this_type operator--(int)
    {
        this_type r(*this);
        --it1;
        --it2;
        return r;
    }

    this_type& operator+=(difference_type d)
    {
        it1 += d;
        it2 += d;
        return *this;
    }
    this_type& operator-=(difference_type d)
    {
        it1 -= d;
        it2 -= d;
        return *this;
    }
    reference operator[](difference_type d) const
    {
        return reference(it1[d], it2[d]);
    }
    bool operator<(const this_type& x) const
    {
        return it1 < x.it1;
    }
    bool operator<=(const this_type& x) const
    {
        return it1 <= x.it1;
    }
    bool operator>(const this_type& x) const
    {
        return it1 > x.it1;
    }
    bool operator>=(const this_type& x) const
    {
        return it1 >= x.it1;
    }
};

// corresponding free functions

template <typename I1, typename I2>
random_access_iterator_pair<I1, I2> operator+(const random_access_iterator_pair<I1, I2>& x, typename random_access_iterator_pair<I1, I2>::difference_type d)
{
    return random_access_iterator_pair<I1, I2>(x.it1 + d, x.it2 + d);
}

template <typename I1, typename I2>
random_access_iterator_pair<I1, I2> operator+(typename random_access_iterator_pair<I1, I2>::difference_type d, const random_access_iterator_pair<I1, I2>& x)
{
    return random_access_iterator_pair<I1, I2>(d + x.it1, d + x.it2);
}

template <typename I1, typename I2>
random_access_iterator_pair<I1, I2> operator-(const random_access_iterator_pair<I1, I2>& x, typename random_access_iterator_pair<I1, I2>::difference_type d)
{
    return random_access_iterator_pair<I1, I2>(x.it1 - d, x.it2 - d);
}

template <typename I1, typename I2>
typename random_access_iterator_pair<I1, I2>::difference_type operator-(const random_access_iterator_pair<I1, I2>& x, const random_access_iterator_pair<I1, I2>& y)
{
    auto r = x.it1 - y.it1;
    assert(r == x.it2 - y.it2);
    return r;
}

template <typename I1, typename I2>
void swap(random_access_iterator_pair<I1, I2>& x, random_access_iterator_pair<I2, I2>& y)
{
    std::swap(x.it1, y.it1);
    std::swap(x.it2, y.it2);
}

//create random_access_iterator_pair
template <typename I1, typename I2>
random_access_iterator_pair<I1, I2> make_random_access_iterator_pair(I1 it1, I2 it2)
{
    return random_access_iterator_pair<I1, I2>(std::move(it1), std::move(it2));
}

//helper classes for sort

struct less_by_first {
    template <typename X, typename Y>
    bool operator()(X&& x, Y&& y) const
    {
        return x.first < y.first;
    }
};

struct less_by_first_and_second {
    template <typename X, typename Y>
    bool operator()(X&& x, Y&& y) const
    {
        if (x.first < y.first)
            return true;
        if (y.first < x.first)
            return false;
        return x.second < y.second;
    }
};
}

#endif
