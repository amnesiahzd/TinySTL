#ifndef _BASIC_ALLOCATOR_H
#define _BASIC_ALLOCATOR_H

#include <cstdlib>
#include <cstddef>
#include <cstdio>
#include <new>

/**
 *  
 * 
*/

namespace TinySTL {
struct FreeList {
    FreeList* next;     // next memory block
    char* data;         // this node
    FreeList() {next = nullptr;}
};

enum {
  EAlign128 = 8, 
  EAlign256 = 16, 
  EAlign512 = 32,
  EAlign1024 = 64, 
  EAlign2048 = 128,
  EAlign4096 = 256
};

enum { ESmallObjectBytes = 4096 };

enum { EFreeListsNumber = 64 };


class BasicAllocator {
private:
    static char* _start_free;
    static char* _end_free;
    static size_t _heap_size;
    static FreeList _free_list[EFreeListsNumber];  // consider FreeList* 

    static size_t memory_align(size_t bytes);
    static size_t memory_round_up(size_t bytes);
    static size_t memory_freelist_index(size_t bytes);
    static void* memory_refill(size_t size);
    static char* memory_chunk_alloc(size_t size, size_t &nobj);

public:
    void* allocate(size_t bytes);
    void* deallocate(void* first_address, size_t bytes);
    void* reallocate(void* first_address, size_t pre_size, size_t new_size);


};
} // end namespace TinySTL 

#endif _BASIC_ALLOCATOR_H