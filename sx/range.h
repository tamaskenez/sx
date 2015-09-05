#ifndef RANGE_INCLUDED_207920893423
#define RANGE_INCLUDED_207920893423

#include <type_traits>
#include <iterator>

namespace sx {

template <typename T>
struct range_view_iterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using reference = T;
    using pointer = T*;
    using difference_type = std::ptrdiff_t;

    explicit range_view_iterator(T value)
        : value_(value)
    {
    }

    bool operator==(const range_view_iterator& x) const
    {
        return value_ == x.value_;
    }

    bool operator!=(const range_view_iterator& x) const
    {
        return value_ != x.value_;
    }

    reference operator*() const
    {
        return value_;
    }

    range_view_iterator& operator++()
    {
        ++value_;
        return *this;
    }

    range_view_iterator operator++(int)
    {
        range_view_iterator r(*this);
        ++(*this);
        return r;
    }

    range_view_iterator& operator--()
    {
        --value_;
        return *this;
    }

    range_view_iterator operator--(int)
    {
        range_view_iterator r(*this);
        --(*this);
        return r;
    }

    range_view_iterator& operator+=(ptrdiff_t n)
    {
        value_ += n;
        return *this;
    }

    range_view_iterator& operator-=(ptrdiff_t n)
    {
        value_ -= n;
        return *this;
    }

    ptrdiff_t operator-(const range_view_iterator& y) const
    {
        return value_ - y.value_;
    }

    reference operator[](ptrdiff_t x) const
    {
        return value_ + x;
    }

#define SX_DEF(OP) \
    bool operator OP(const range_view_iterator& y) const { return value_ OP y.value_; }
    SX_DEF(< )
    SX_DEF(> )
    SX_DEF(<= )
    SX_DEF(>= )
#undef SX_DEF
private:
    T value_;
};

//like python range
template <typename T, typename = std::enable_if_t<std::is_integral<T>::value> >
struct range_view {
public:
    using iterator = range_view_iterator<T>;

    explicit range_view(T to)
        : from_(0)
        , to_(to)
    {
        assert(0 <= to);
    }

    range_view(T from, T to)
        : from_(from)
        , to_(to)
    {
        assert(from <= to);
    }

    iterator begin() const
    {
        return iterator(from_);
    }

    iterator end() const
    {
        return iterator(to_);
    }

private:
    T from_, to_;
};

template <typename T>
range_view<T> range(T to)
{
    return range_view<T>(to);
}

template <typename T>
range_view<T> range(T from, T to)
{
    return range_view<T>(from, to);
}

template <typename T>
range_view_iterator<T> operator+(const range_view_iterator<T>& x, ptrdiff_t y)
{
    return range_view_iterator<T>(x) += y;
}

template <typename T>
range_view_iterator<T> operator+(ptrdiff_t y, const range_view_iterator<T>& x)
{
    return range_view_iterator<T>(x) += y;
}

} //namespace sx

namespace std {
template <typename T>
struct iterator_traits< ::sx::range_view_iterator<T> > {
private:
    using iterator = ::sx::range_view_iterator<T>;

public:
    using difference_type = typename iterator::difference_type;
    using value_type = typename iterator::value_type;
    using reference = typename iterator::reference;
    using iterator_category = typename iterator::iterator_category;
    using pointer = typename iterator::pointer;
};
}

#endif
