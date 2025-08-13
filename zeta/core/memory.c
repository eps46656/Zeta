#include <zeta/core/memory.h>

Zeta_Core_Allocator Zeta_Core_Memory_weak_lifo_allocator = {
    .vtable = NULL,
    .context = NULL,
};

Zeta_Core_Allocator Zeta_Core_Memory_general_allocator = {
    .vtable = NULL,
    .context = NULL,
};
