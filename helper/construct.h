#pragma once

#include <new>

#include "iterator.h"
#include "type_trais.h"

namespace TinySTL {
/**
 * Constructs an object at the specified memory location
 * 
 * @param ptr Memory address initial location
 * 
*/
template <class T>
void construct(T* ptr) {
    // TODO: consider add a staticast to judge if T can be construct
    ::new ((void*)ptr) T(); // base namespace new
}

/**
 * Constructs an object in the specified area with the specified value
 * 
 * @param ptr   Memory address initial location
 * @param value initial value
 * 
*/
template <class T, class V>
void construct(T* ptr, V& value) {
    // TODO: consider add a staticast to judge if T can be construct
    ::new ((void*)ptr) T(value);
}

/**
 * Constructs an object on allocated memory and can pass a construction parameter package to the object's constructor
 * 
 * @param ptr  Memory address initial location
 * @param args Initialization parameter package
*/
template <class T, class... Args>
void construct(T* ptr, Args&&... args) {
    ::new ((void*)ptr) T(std::forward<Args>(args)...);
}

/**
 * Destruct the corresponding object
 * 
 * @param ptr Pointer to destructed object
 * 
*/
template <class T>
void destroy_one(T* ptr, std::true_type) {
    // without any processing
}

/**
 * Destruct the corresponding object
 * 
 * @param ptr Pointer to destructed object
 * 
*/
template <class T>
void destroy_one(T* ptr, std::false_type) {
    if (ptr != nullptr) { // TODO: consider add a staticast to judge if T can be desstruct
        ptr->~T();
    }
}

/**
 * Used to destroy a pointer to an object of type T 
 * and select the destruction method based on the result of std::is_trivially_destructible<T>{}
 * 
 * @param pointer The memory location to be destroyed
*/
template <class T>
void destroy(T* pointer) {
  destroy_one(pointer, std::is_trivially_destructible<T>{});
}

/**
 * Destruction operation in the specified container
 * 
 * @param first Destruction starting position
 * @param last  Destruction end position
*/
template <class ForwardIter>
void destroy_category(ForwardIter first, ForwardIter last, std::true_type) {
    // without any processing
}

/**
 * Destruction operation in the specified container
 * 
 * @param first Destruction starting position
 * @param last  Destruction end position
*/
template <class ForwardIter>
void destroy_category(ForwardIter first, ForwardIter last, std::false_type) {
    for (; first != last; first++) {
        // first:   iterator itself
        // *first:  the actual value;
        // &*first: because destroy function need a pointer.
        destroy(&*first); 
    }
}

/**
 * Destruction operation in the specified container
 * 
 * @param first Destruction starting position
 * @param last  Destruction end position
*/
template <class ForwardIter>
void destroy(ForwardIter first, ForwardIter last) {
    destroy_cat(first, last, std::is_trivially_destructible<typename iterator_traits<ForwardIter>::value_type>{});
}
} // end namespace TinySTL