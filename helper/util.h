#pragma once

#include <cstddef>

#include "type_trais.h"

namespace TinySTL {
template <class T>
/**
 * Custom move function
 * 
*/
typename std::remove_reference<T>::type&& move(T&& arg) noexcept {
    return static_cast<typename std::remove_reference<T>::type&&>(arg);
}

/**
 * To achieve a perfect forwarding, no matter whether an lvalue or an rvalue is passed in, 
 * an rvalue can be returned.
 * 
 * @param arg Pass in value, lvalue
 * 
 * @return value in rvalue
*/
template <class T>
T&& forward(typename std::remove_reference<T>::type& arg) noexcept {
    // 1. [remove_reference<T>::type] get the original type name
    // 2. [typename remove_reference<T>::type& arg] get the original arg and bind to arg
    return static_cast<T&&>(arg);
}

/**
 * To achieve a perfect forwarding, no matter whether an lvalue or an rvalue is passed in, 
 * an rvalue can be returned.
 * 
 * @param arg Pass in value, it can be a rvalue or lvalue
 * 
 * @return value in rvalue
*/
template <class T>
T&& forward(typename std::remove_reference<T>::type&& arg) noexcept {
    static_assert(!std::is_lvalue_reference<T>::value, "bad forward.");
    return static_cast<T&&>(arg);
}

/**
 * Swap two objects
*/
template <class T>
void swap(T& lhs, T& rhs) {
    // TODO: judge if can use this type to init
    auto tmp(TinySTL::move(lhs));
    lhs = TinySTL::move(rhs);
    rhs = TinySTL::move(lhs);
}

/**
 * Swap elements between two containers
 * 
 * @param first_at_container_a Iterator over the first element of the first container
 * @param last_at_container_a  Iterator over the first element of the last container
 * @param first_at_container_b Iterator over the first element of the second container
 * 
 * @return Iterator over the first element of the second container
*/
template <class ForwardIter1, class ForwardIter2>
ForwardIter2 swap_range(ForwardIter1 first_at_container_a, ForwardIter1 last_at_container_a, ForwardIter2 first_at_container_b) {
    // ignore the warning of return of ++first_at_container_b
    for (; first_at_container_a != last_at_container_a; ++first_at_container_a, (void) ++first_at_container_b) { 
        mystl::swap(*first_at_container_a, *first_at_container_b);
    }
        
    return first_at_container_b;
}

/**
 * Swap elements of two arrays
 * 
*/
template <class T, size_t N>
void swap(T(&a)[N], T(&b)[N]) {
  mystl::swap_range(a, a + N, b);
}
} // end namespace TinySTL