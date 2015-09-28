#ifndef TYPE_TRAITS_INCLUDED_79238234
#define TYPE_TRAITS_INCLUDED_79238234

// include "sx/type_traits.h" and if current cxx standard < 14
// then add some missing features

#include <type_traits>

#if 201100 < __cplusplus && __cplusplus < 201400

namespace std {

template <bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <class T>
using remove_cv_t = typename remove_cv<T>::type;

template <class T>
using remove_const_t = typename remove_const<T>::type;

template <class T>
using remove_volatile_t = typename remove_volatile<T>::type;

template <class T>
using remove_reference_t = typename remove_reference<T>::type;
}

#endif

#endif
