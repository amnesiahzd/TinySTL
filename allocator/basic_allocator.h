#ifndef _BASIC_ALLOCATOR_H
#define _BASIC_ALLOCATOR_H

#include <cstdlib>

namespace TinySTL {
class basic_allocator {
public:
    /**
     * 
     * 
    */
    static void* allocate(size_t bytes);

    /**
     * 
     * 
    */
    static void deallocate(void* ptr, size_t bytes);

    /**
     * 
     * 
    */
    static void* reallocate(void* ptr, size_t old_size, size_t new_size);
private:
    enum _B_Align { ALIGN = 8 };
    enum _B_MaxBytes {  MAX_BYTES = 128 };
    enum _B_N_FreeList { N_FREELISTS = (_B_MaxBytes::MAX_BYTES / _B_Align::ALIGN) };
    enum _B_N_Objects { N_OBJECTS = 20};

};
} // end namespace TinySTL 

#endif _BASIC_ALLOCATOR_H