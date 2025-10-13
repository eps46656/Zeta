#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/type_traits.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::allocator {

template <typename AllocatorImpl>
bool CheckAllocator(AllocatorImpl const* allocator_impl);

// -----------------------------------------------------------------------------

struct AllocatorVTable {
    void* (*Allocate)(void* allocator_inst, size_t size);

    void (*Deallocate)(void* allocator_inst, void* ptr);

    template <typename AllocatorImpl>
    static AllocatorVTable const& Make();
};

// -----------------------------------------------------------------------------

template <typename IsConst>
struct AllocatorRefTpl {
    ZETA_Core_StaticAssert(value_wrapper::IsStaticValueWrapper<IsConst>);
    ZETA_Core_StaticAssert(IsAnyOf<decltype(IsConst::value), bool const>);

    Conditional<IsConst::value, void const*, void*> inst;

    unsigned short align;

    AllocatorVTable const* vtable;

    // -------------------------------------------------------------------------

    static size_t GetAlign(void const* allocator_ref);

    template <typename _ = void>
    static EnableIf<!IsConst::value, void*, _> Allocate(void* allocator_ref,
                                                        size_t size);

    template <typename _ = void>
    static EnableIf<!IsConst::value, void, _> Deallocate(void* allocator_ref,
                                                         void* ptr);

    // -------------------------------------------------------------------------

    static bool CheckAllocator(void const* allocator_ref);
};

using AllocatorRef = AllocatorRefTpl<value_wrapper::StaticValueWrapper<false>>;

using ConstAllocatorRef =
    AllocatorRefTpl<value_wrapper::StaticValueWrapper<true>>;

// -----------------------------------------------------------------------------

template <typename AllocatorImpl>
void* SafeAllocate(AllocatorImpl* allocator_impl, size_t align, size_t size);

// -----------------------------------------------------------------------------

extern AllocatorRef weak_lifo_allocator;

}  // namespace zeta::core::allocator
