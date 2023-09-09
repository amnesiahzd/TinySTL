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
    bool is_null() {return data == nullptr;};
};

// TODO: to know the usage of enum class
// @author: AmnesiaHzd@gmail.com
// @date: 2023/09/09 
enum EAlignSize {
  EAlign128 = 8, 
  EAlign256 = 16, 
  EAlign512 = 32,
  EAlign1024 = 64, 
  EAlign2048 = 128,
  EAlign4096 = 256
};

enum ESmallObjectSize { ESmallObjectBytes = 4096 };

enum EFreeList { EFreeListsNumber = 64 };

class BasicAllocator {
private:
    static size_t memory_align(size_t bytes);
    static size_t memory_round_up(size_t bytes);
    static size_t memory_freelist_index(size_t bytes); // change name to get ...
    static void* memory_refill(size_t size);
    static char* memory_chunk_alloc(size_t size, size_t &nobj);

public:
    static char* _start_free;
    static char* _end_free;
    static size_t _heap_size;
    static FreeList _free_list[EFreeList::EFreeListsNumber];  // consider FreeList* 

    /**
     * allocte the memory, if the byte less than small memory block size, then use memory List
     * @param bytes User demand size
     * 
     * @return the first bytes of the memory block address
    */
    void* allocate(size_t bytes);
    void* deallocate(void* first_address, size_t bytes);
    void* reallocate(void* first_address, size_t pre_size, size_t new_size);
};
} // end namespace TinySTL 

#endif _BASIC_ALLOCATOR_H