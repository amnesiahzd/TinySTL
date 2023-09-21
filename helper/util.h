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

template <class T1, class T2>
struct pair {
    T1 first;
    T2 second;

    // 1.T1 is the defualt value of Other1, if not specify the Other1, will use the T1 as the defualt 
    // 2.std::enable_if in C++11, able/disable template function according to the bool value of the first element
    //   if true, return void(the second element)
    //   if false, return illeagle type 
    // 3.std::enable_if will return a type rather not a value
    // 4. typename = ... is not a necessary part, only to be clear to the struct of the code

    template <class Other1 = T1, 
              class Other2 = T2, 
              typename = typename std::enable_if<std::is_default_constructible<Other1>::value && 
                                                 std::is_default_constructible<Other2>::value, void>::type>
    constexpr pair() : first(), second() {}

    // 1.std::is_convertible check the type A can trans to type B
    // 2.= 0 is the default value
    // TODO: get clear why its int to be the third param of the template

    template <class U1 = T1, 
              class U2 = T2,
              typename = typename std::enable_if<std::is_copy_constructible<U1>::value &&
                                                 std::is_copy_constructible<U2>::value &&
                                                 std::is_convertible<const U1&, T1>::value &&
                                                 std::is_convertible<const U2&, T2>::value, int>::type = 0>
    constexpr pair(const U1& a, const U2& b) : first(a), second(b) {} // Original code here is T1, T2, i think U1 and U2 is the correct

    template <class U1 = T1, 
              class U2 = T2,
              typename = typename std::enable_if<std::is_copy_constructible<U1>::value &&
                                                 std::is_copy_constructible<U2>::value &&
                                                 (!std::is_convertible<const U1&, T1>::value || 
                                                 !std::is_convertible<const U2&, T2>::value), int>::type = 0>
    explicit constexpr pair(const U1& a, const U2& b) : first(a), second(b) {} // Original code here is T1, T2, i think U1 and U2 is the correct

    pair(const pair& rhs) = default;
    pair(pair&& rhs) = default;

    template <class Other1, 
              class Other2,
              typename = typename std::enable_if<std::is_constructible<T1, Other1>::value &&
                                                 std::is_constructible<T2, Other2>::value &&
                                                 std::is_convertible<Other1&&, T1>::value &&
                                                 std::is_convertible<Other2&&, T2>::value, int>::type = 0>
    constexpr pair(Other1&& a, Other2&& b) : first(TinySTL::forward<Other1>(a)), second(TinySTL::forward<Other2>(b)) {}

    template <class Other1, 
              class Other2,
              typename = typename std::enable_if<std::is_constructible<T1, Other1>::value && 
                                                 std::is_constructible<T2, Other2>::value &&
                                                 (!std::is_convertible<Other1, T1>::value ||
                                                 !std::is_convertible<Other2, T2>::value), int>::type = 0>
    explicit constexpr pair(Other1&& a, Other2&& b) : first(mystl::forward<Other1>(a)), second(mystl::forward<Other2>(b)) {}

    template <class Other1, 
              class Other2,
              typename = typename std::enable_if<std::is_constructible<T1, const Other1&>::value &&
                                                 std::is_constructible<T2, const Other2&>::value &&
                                                 std::is_convertible<const Other1&, T1>::value &&
                                                 std::is_convertible<const Other2&, T2>::value, int>::type = 0>
    constexpr pair(const pair<Other1, Other2>& other) : first(other.first),  second(other.second) {}

    template <class Other1, 
              class Other2,
              typename = typename std::enable_if<std::is_constructible<T1, const Other1&>::value &&
                                                 std::is_constructible<T2, const Other2&>::value &&
                                                 (!std::is_convertible<const Other1&, T1>::value ||
                                                 !std::is_convertible<const Other2&, T2>::value), int>::type = 0>
    explicit constexpr pair(const pair<Other1, Other2>& other) : first(other.first), second(other.second) {}

    template <class Other1, 
              class Other2,
              typename = typename std::enable_if<std::is_constructible<T1, Other1>::value &&
                                                 std::is_constructible<T2, Other2>::value &&
                                                 std::is_convertible<Other1, T1>::value &&
                                                 std::is_convertible<Other2, T2>::value, int>::type = 0>
    constexpr pair(pair<Other1, Other2>&& other) : first(mystl::forward<Other1>(other.first)), 
                                                   second(mystl::forward<Other2>(other.second)) {}

    template <class Other1, 
              class Other2,
              typename = typename std::enable_if<std::is_constructible<T1, Other1>::value &&
                                                 std::is_constructible<T2, Other2>::value &&
                                                 (!std::is_convertible<Other1, T1>::value ||
                                                 !std::is_convertible<Other2, T2>::value), int>::type = 0>
    explicit constexpr pair(pair<Other1, Other2>&& other) : first(mystl::forward<Other1>(other.first)),
                                                            second(mystl::forward<Other2>(other.second)) {}

    pair& operator=(const pair& rhs) {
        if (this != &rhs) { 
            first = rhs.first;
            second = rhs.second;
        }

        return *this;
    }

    pair& operator=(pair&& rhs) {
        if (this != &rhs) {
            first = TinySTL::move(first);
            second = TinySTL::move(second);
        }

        return *this;
    }

    template <class Other1, class Other2>
    pair& operator=(const pair<Other1, Other2>& other) {
        first = other.first;
        second = other.second;
        return *this;
    }

    template <class Other1, class Other2>
    pair& operator=(pair<Other1, Other2>&& other) {
        first = TinySTL::move(other.first);
        second = TinySTL::move(other.second);
        return *this;
    }

    ~pair() = default;

    void swap(pair& other) {
        if (this != &other) {
            TinySTL::swap(first, other.first);
            TinySTL::swap(second, other.second);
        }
    }

    template <class T1, class T2>
    bool operator==(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs) {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }

    template <class T1, class T2>
    bool operator<(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs) {
        return lhs.first < rhs.first || (lhs.first == rhs.first && lhs.second < rhs.second);
    }

    template <class T1, class T2>
    bool operator!=(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs) {
        return !(lhs == rhs);
    }

    template <class T1, class T2>
    bool operator>(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs) {
        return rhs < lhs;
    }

    template <class T1, class T2>
    bool operator<=(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs) {
        return !(rhs < lhs);
    }

    template <class T1, class T2>
    bool operator>=(const pair<T1, T2>& lhs, const pair<T1, T2>& rhs) {
        return !(lhs < rhs);
    }

    template <class T1, class T2>
    void swap(pair<T1, T2>& lhs, pair<T1, T2>& rhs) {
        lhs.swap(rhs);
    }

    // 全局函数，让两个数据成为一个 pair
    template <class T1, class T2>
    pair<T1, T2> make_pair(T1&& first, T2&& second) {
        return pair<T1, T2>(mystl::forward<T1>(first), mystl::forward<T2>(second));
    }
};


} // end namespace TinySTL