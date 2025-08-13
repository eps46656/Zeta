#include <zeta/core/cascade_allocator.h>
#include <zeta/core/memory.h>
#include <zeta/core_test/std_allocator.h>

#if ZETA_Core_EnableDebug

Zeta_CoreTest_StdAllocator std_allocator;

#else

Zeta_CascadeAllocator cascade_allocator;

unsigned char cascade_mem[512 * 1024 * 1024];

#endif

void Zeta_CoreTest_InitCascadeAllocator() {
#if ZETA_Core_EnableDebug
    // use standard malloc to be sanitized by asan

    Zeta_Core_Memory_weak_lifo_allocator = {
        .vtable = &Zeta_CoreTest_StdAllocator_allocator_vtable,
        .context = &std_allocator,
    };
#else

    cascade_allocator.align = alignof(max_align_t);
    cascade_allocator.mem = cascade_mem;
    cascade_allocator.size = sizeof(cascade_mem);

    Zeta_CascadeAllocator_Init(&cascade_allocator);

    Zeta_Core_Memory_weak_lifo_allocator = {
        .vtable = &Zeta_Core_CascadeAllocator_allocator_vtable,
        .context = &cascade_allocator,
    };
#endif
}
