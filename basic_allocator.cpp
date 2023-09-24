#include "basic_allocator.h"

namespace TinySTL {
// init static value
char* BasicAllocator::_start_free = nullptr;
char* BasicAllocator::_end_free = nullptr;
size_t BasicAllocator::_heap_size = 0L;


inline void* BasicAllocator::allocate(size_t bytes) {
    if (bytes > static_cast<size_t>(ESmallObjectSize::ESmallObjectBytes)) {
        return std::malloc(bytes);
    }

    FreeList my_free_list;
    FreeList result;

    my_free_list = _free_list[memory_freelist_index(bytes)];
    result = my_free_list;
    if (result.is_null()) {
        return memory_refill(memory_round_up(bytes));
    }

    result.next = &my_free_list;
    return result.data;
}

inline void* BasicAllocator::deallocate(void* first_address, size_t bytes) {
    if (bytes > static_cast<size_t>(ESmallObjectSize::ESmallObjectBytes)) {
        std::free(first_address);
    }

    FreeList* target_memspace = static_cast<FreeList*>(first_address);
    FreeList my_free_list;

    my_free_list = _free_list[memory_freelist_index(bytes)];
    target_memspace->next = &my_free_list;
    
    target_memspace = &my_free_list;
    return first_address;
}

inline void* BasicAllocator::reallocate(void* first_address, size_t pre_size, size_t new_size) {
    deallocate(first_address, pre_size);
    first_address = allocate(new_size);
    return first_address;
}

size_t BasicAllocator::memory_align(size_t bytes) {
    if (bytes < 512) {
        return bytes <= 256 ? (bytes <= 128 ? EAlign128 : EAlign256) : EAlign512;
    } 
    bytes <= 2048 ? (bytes <= 1024 ? EAlign1024 : EAlign2048) : EAlign4096;
}

size_t BasicAllocator::memory_round_up(size_t bytes) {
    return ((bytes + memory_align(bytes) - 1) & ~(memory_align(bytes) - 1));
}

size_t BasicAllocator::memory_freelist_index(size_t bytes) {
    if (bytes <= 512) {
        return bytes <= 256
            ? bytes <= 128 
            ? ((bytes + EAlign128 - 1) / EAlign128 - 1) 
            : (15 + (bytes + EAlign256 - 129) / EAlign256)
            : (23 + (bytes + EAlign512 - 257) / EAlign512);
    }

    return bytes <= 2048
        ? bytes <= 1024 
        ? (31 + (bytes + EAlign1024 - 513) / EAlign1024)
        : (39 + (bytes + EAlign2048 - 1025) / EAlign2048)
        : (47 + (bytes + EAlign4096 - 2049) / EAlign4096);
}
} // end namespace TinySTL