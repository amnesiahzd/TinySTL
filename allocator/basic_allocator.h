#ifndef _BASIC_ALLOCATOR_H
#define _BASIC_ALLOCATOR_H

#include <cstdlib>

namespace TinySTL {
class basic_allocator {
public:
    static void* allocate(size_t bytes);
private:
    enum _B_Align {
        ALIGN = 8
    };
};
} // end namespace TinySTL 
#endif _BASIC_ALLOCATOR_H