#pragma once

#include <zeta/core/allocator.hpp>

namespace zeta::core::allocator_ref {

struct Ref {
    size_t align;

    allocator::VTable const* vtable;

    void* alctr;
};

size_t GetAlign(Ref const& ref);

void* Allocate(Ref& ref, size_t size);

void Deallocate(Ref& ref, void* ptr);

void CheckRef(Ref& ref);

template <typename Allocator>
Ref MakeRef(Allocator& alctr);

}  // namespace zeta::core::allocator_ref

namespace zeta::core {

template <>
struct allocator::AllocatorTraits<allocator_ref::Ref const> {
    static void* GetReferedInstPtr(allocator_ref::Ref const& ref);

    static size_t GetAlign(allocator_ref::Ref const& ref);
};

template <>
struct allocator::AllocatorTraits<allocator_ref::Ref>
    : public allocator::AllocatorTraits<allocator_ref::Ref const> {
    static void* Allocate(allocator_ref::Ref& ref, size_t size);

    static void Deallocate(allocator_ref::Ref& ref, void* ptr);
};

}  // namespace zeta::core
