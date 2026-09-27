#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils/sanity.hpp>

namespace zeta::core::poly_allocator {

struct Allocator {
    size_t align;

    allocator::VTable const* vtable;

    void* target_alctr;

    constexpr Allocator();

    constexpr Allocator(Allocator const&) = default;

    constexpr Allocator(Allocator&&) = default;

    template <allocator::IsAllocator TargetCntr>
    constexpr Allocator(TargetCntr& target_alctr);

    constexpr ~Allocator();

    constexpr Allocator& operator=(Allocator const&) = default;

    template <allocator::IsAllocator TargetAllocator>
    constexpr void Set(this Allocator& self, TargetAllocator& target_alctr);

    constexpr void* GetReferedInstPtr(this Allocator const& self,
                                      allocator::Tag);

    constexpr size_t GetAlign(this Allocator const& self, allocator::Tag);

    constexpr void* Allocate(this Allocator& self, allocator::Tag, size_t size);

    constexpr void Deallocate(this Allocator& self, allocator::Tag, void* ptr);

    constexpr void Check(this Allocator const& self);

    static constexpr void SanityCheck(
        void const* self, debug_utils::sanity::SanityCheckScope scope);
};

}  // namespace zeta::core::poly_allocator
