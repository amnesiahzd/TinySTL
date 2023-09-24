/**
 * 
 * 
*/

#pragma once
#include <cstddef>

#include "type_trais.h"

namespace TinySTL {

// define five iterator base struct
struct input_iterator_base {};
struct output_iterator_base {};
struct forward_iterator_base : public input_iterator_base {};
struct bidirectional_iterator_base : public forward_iterator_base {};
struct random_access_iterator_base : public bidirectional_iterator_base {};

// ptrdiff_t only used on liner space
// in template coding, "class Distance = ptrdiff_t" means ptrdiff_t is the defualt value of Distance

template <class Category, class T, class Distance = ptrdiff_t, class Pointer = T*, class Reference = T&>
struct iterator {
    using iterator_category = Category;
    using value_type = T;
    using pointer = Pointer;
    using reference = Reference;
    using liner_distance = Distance;
};

template <class T>
struct has_iterator_cat {
private:
    struct two { char a; char b; };
    template <class U> static two test(...);
    template <class U> static char test(typename U::iterator_category* = 0);
public:
    static const bool value = sizeof(test<T>(0)) == sizeof(char);
};

template <class Iterator, bool>
struct iterator_traits_impl {};

template <class Iterator, bool>
struct iterator_traits_helper {};

template <class Iterator>
struct iterator_traits_helper<Iterator, true> : public iterator_traits_impl<Iterator,
            std::is_convertible<typename Iterator::iterator_category, input_iterator_base>::value ||
            std::is_convertible<typename Iterator::iterator_category, output_iterator_base>::value> {};

template <class Iterator>
struct iterator_traits : public iterator_traits_helper<Iterator, has_iterator_cat<Iterator>::value> {};

// Partial Template Specialization

template <class T>
struct iterator_traits<T*> {
    using iterator_category = random_access_iterator_base;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using liner_distance = ptrdiff_t;
};

template <class T>
struct iterator_traits<const T*> {
    using iterator_category = random_access_iterator_base;
    using value_type = T;
    using pointer = const T*;
    using reference = const T&;
    using liner_distance = ptrdiff_t;
};

template <class T, class U, bool b = has_iterator_cat<iterator_traits<T>>::value>
struct has_iterator_cat_of 
    : public m_bool_constant<std::is_convertible<typename iterator_traits<T>::iterator_category, U>::value> // typename is to tell compiler its a name of type
{};

// Partial Template Specialization, if T and U didnt match some condition
// has_iterator_cat_of<T, U, false> is the Partial Template Specialization, set by coder

template <class T, class U>
struct has_iterator_cat_of<T, U, false> : public m_false_type {};

template <class Iter>
struct is_input_iterator : public has_iterator_cat_of<Iter, input_iterator_base> {};

template <class Iter>
struct is_output_iterator : public has_iterator_cat_of<Iter, output_iterator_base> {};

template <class Iter>
struct is_forward_iterator  : public has_iterator_cat_of<Iter, forward_iterator_base> {};

template <class Iter>
struct is_bidirectional_iterator  : public has_iterator_cat_of<Iter, bidirectional_iterator_base> {};

template <class Iter>
struct is_random_access_iterator  : public has_iterator_cat_of<Iter, random_access_iterator_base> {};

template <class Iterator>
struct is_iterator : public m_bool_constant<is_input_iterator<Iterator>::value || is_output_iterator<Iterator>::value> {};


// category
template <class Iterator>
typename iterator_traits<Iterator>::iterator_category
iterator_category(const Iterator&) {
    return iterator_traits<Iterator>::iterator_category;
}


// distance_type
template <class Iterator>
typename iterator_traits<Iterator>::difference_type*
distance_type(const Iterator&) {
    return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
}

// value_type
template <class Iterator>
typename iterator_traits<Iterator>::value_type*
value_type(const Iterator) {
    return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
}

// caculate the distance between iterator

template <class InputIterator>
typename iterator_traits<InputIterator>::difference_type
distance_dispatch(InputIterator first, InputIterator last, input_iterator_base) {
    typename iterator_traits<InputIterator>::difference_type n = 0;
    while (first != last) {
        ++first;
        ++n;
    }

    return n;
}

template <class RandomIter>
typename iterator_traits<RandomIter>::difference_type
distance_dispatch(RandomIter first, RandomIter last, random_access_iterator_base) {
    return last - first;
}

template <class InputIterator>
typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last) {
    return distance_dispatch(first, last, iterator_category(first));
}

template <class InputIterator, class Distance>
void advance_dispatch(InputIterator& iterator, Distance n, input_iterator_base) {
    while (n--) {
        ++iterator;
    }
}

template <class BidirectionalIterator, class Distance>
void advance_dispatch(BidirectionalIterator& iterator, Distance distance, bidirectional_iterator_base) {
    if (distance >= 0) {
        while (distance--) {
            ++iterator;
        }
    } else {
        while (distance++) {
            --iterator;
        }
    }
}

template <class RandomIter, class Distance>
void advance_dispatch(RandomIter& iterator, Distance distance, random_access_iterator_base) {
    iterator += distance;
}

template <class InputIterator, class Distance>
void advance(InputIterator& iterator, Distance distance) {
    advance_dispatch(iterator, distance, iterator_category(iterator));
}

/*******************************************************************************************************/

// reverse_iterator

template <class Iterator>
class reverse_iterator {
private:
    Iterator current;

public:
    using iterator_category = typename iterator_traits<Iterator>::iterator_category;
    using value_type = typename iterator_traits<Iterator>::value_type;
    using difference_type = typename iterator_traits<Iterator>::difference_type;
    using pointer = typename iterator_traits<Iterator>::pointer;
    using reference = typename iterator_traits<Iterator>::reference;
    // using iterator_type = Iterator;
    using self_type = reverse_iterator<Iterator>;

    // TODO: consider using &&
    reverse_iterator() {}
    explicit reverse_iterator(Iterator iter) : current(iter) {}
    reverse_iterator(const self_type& rhs) : current(rhs.current) {}

    Iterator base() const {
        return current;
    }

    reference operator*() const { // The previous position of the forward iterator
        auto forward = current;
        return *--forward;
    }

    pointer operator->() const {
        return &(operator*());
    }

    self_type& operator++() {
        --current;
        return *this;
    }

    self_type operator++(int) {
        self_type temp = *this;
        --current; // return --this.current ?
        return temp;
    }

    self_type& operator--() {
        ++current;
        return *this;
    }

    self_type operator--(int) {
        self_type temp = *this;
        ++current; // return ++this.current ?
        return temp;
    }

    self_type& operator+=(difference_type n) {
        current -= n;
        return *this;
    }

    self_type operator+(difference_type n) const {
        return self_type(current - n);
    }

    self_type& operator-=(difference_type n) {
        current += n;
        return *this;
    }

    // 为什么一会儿是引用一会儿是名字？

    self_type operator-(difference_type n) const {
        return self_type(current + n);
    }

    reference operator[](difference_type n) const {
        return *(*this + n);
    }

    
};

// make caculate operator out of the class
// 1.New operator overloads can be added to a class without modifying the class definition
// 2.Operator functions may not be suitable as member functions of a class, 
//   especially when you need to compare objects of two different classes. 
//   Additionally, they can accept different types of parameters, increasing their versatility.

template <class Iterator>
typename reverse_iterator<Iterator>::difference_type
operator-(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
    return rhs.base() - lhs.base();
}

template <class Iterator>
bool operator==(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
    return lhs.base() == rhs.base();
}

template <class Iterator>
bool operator<(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
    return rhs.base() < lhs.base();
}

template <class Iterator>
bool operator!=(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
    return !(lhs == rhs);
}

template <class Iterator>
bool operator>(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
    return rhs < lhs;
}

template <class Iterator>
bool operator<=(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
    return !(rhs < lhs);
}

template <class Iterator>
bool operator>=(const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
    return !(lhs < rhs);
}
} // end namespace TinySTL