#ifndef RANGE_INCLUDED_207920893423
#define RANGE_INCLUDED_207920893423

#include <cassert>
#include <type_traits>
#include <iterator>

namespace sx {

template <typename T>
struct range_view_iterator
: public std::iterator<
    std::random_access_iterator_tag,
    T,
    std::ptrdiff_t,
    const T*,
    T
>
{
private:
    T value_;
    using iterator = std::iterator<std::random_access_iterator_tag, T, std::ptrdiff_t, const T*, T>;
public:
    using this_type = range_view_iterator;
    using typename iterator::iterator_category;
    using typename iterator::value_type;
    using typename iterator::difference_type;
    using typename iterator::pointer;
    using typename iterator::reference;

    // construction, assignment
	range_view_iterator() : value_(0) {}
    range_view_iterator(const this_type&) = default;
	this_type& operator=(const this_type&) = default;

    explicit range_view_iterator(T value)
        : value_(value)
    {}

    // observers
	reference operator*() const { return value_; }
private:
    pointer operator->() const = delete; //could return an array proxy, implement if needed
public:
    reference operator[](difference_type x) const { return value_ + x; }

    // modifiers
    this_type& operator++() { ++value_; return *this; }
	this_type operator++(int) { this_type x(*this); ++(*this); return x; }
    this_type& operator--() { --value_; }
	this_type operator--(int) const { this_type x(*this); --(*this); return x; }
    void swap(this_type& y) { using std::swap; std::swap(value_, y.value_); }
	this_type& operator+=(difference_type y) { value_ += y; return *this; }
	this_type& operator-=(difference_type y) { (*this) += (-y); return *this; }

    // comparison
    bool operator==(const this_type& y) const { return value_ == y.value_; }
    bool operator!=(const this_type& y) const { return !(*this==y); }
    bool operator<(const this_type& y) const { return value_ < y.value_; }
    bool operator>(const this_type& y) const { return y < *this; }
    bool operator>=(const this_type& y) const { return !(*this < y); }
    bool operator<=(const this_type& y) const { return !(*this > y); }

    //operations
    this_type operator+(difference_type y) const { this_type x(*this); x += y; return x; }
    this_type operator-(difference_type y) const { this_type x(*this); x -= y; return x; }
    difference_type operator-(const this_type& y) { return value_ - y.value_; }
};

template<typename T>
inline void swap(range_view_iterator<T>& x, range_view_iterator<T>& y) { x.swap(y); }

template<typename T>
inline range_view_iterator<T> operator+(
    typename range_view_iterator<T>::difference_type x,
    const range_view_iterator<T>& y)
{ return y + x; }

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

} //namespace sx

#endif
