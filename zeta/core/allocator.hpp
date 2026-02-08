#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::allocator {

template <typename AllocatorLike>
void CheckContract(AllocatorLike&& alctr);

struct VTable {
    template <typename Allocator>
    static constexpr VTable const& Build();

    void* (*Allocate)(void* alctr, size_t size);

    void (*Deallocate)(void* alctr, void* ptr);
};

template <typename Allocator>
struct BasicVTableBuilder {
    static constexpr VTable Build();

    static void* Allocate(void* alctr, size_t size);

    static void Deallocate(void* alctr, void* ptr);
};

template <typename Allocator>
struct VTableBuilder {
    static constexpr VTable Build();
};

template <typename Allocator>
constexpr VTable const& GetVTable();

struct Ref {
    unsigned short align;

    VTable const* vtable;

    void* alctr;

    static size_t GetAlign(Ref const* ref_view);

    static void* Allocate(Ref* ref_view, size_t size);

    static void Deallocate(Ref* ref_view, void* ptr);
};

namespace ops {

template <typename AllocatorLike>
size_t GetAlign(AllocatorLike&& alctr);

template <typename AllocatorLike>
void* Allocate(AllocatorLike&& alctr, size_t size);

template <typename AllocatorLike>
void Deallocate(AllocatorLike&& alctr, void* ptr);

template <typename AllocatorLike>
void* SafeAllocate(AllocatorLike&& alctr, size_t align, size_t size);

template <typename AllocatorLike>
Ref MakeRef(AllocatorLike&& alctr);

}  // namespace ops

extern Ref weak_lifo_allocator;

}  // namespace zeta::core::allocator
