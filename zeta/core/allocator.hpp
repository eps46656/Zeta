#pragma once

#include <zeta/core/meta.hpp>

namespace zeta::core::allocator {

struct Tag {};

template <typename Allocator>
concept IsAllocator = requires(Allocator& alctr, size_t size, void* ptr) {
    requires meta::IsSame<meta::RemoveConstVolatileReference<
                              decltype(alctr.GetReferedInstPtr(Tag{}))>,
                          void*>;

    requires meta::IsSame<
        meta::RemoveConstVolatileReference<decltype(alctr.GetAlign(Tag{}))>,
        size_t>;

    requires meta::IsSame<meta::RemoveConstVolatileReference<
                              decltype(alctr.Allocate(Tag{}, size))>,
                          void*>;

    requires meta::IsSame<meta::RemoveConstVolatileReference<
                              decltype(alctr.Deallocate(Tag{}, ptr))>,
                          void>;
};

template <typename Allocator>
concept IsAllocatorWithLeftReference = IsAllocator<Allocator&>;

template <typename Allocator>
concept IsAllocatorWithRightReference = IsAllocator<Allocator&&>;

template <typename AllocatorLike>
concept IsAllocatorLike =
    IsAllocatorWithLeftReference<meta::RemovePointer<AllocatorLike>> &&
    IsAllocatorWithRightReference<meta::RemovePointer<AllocatorLike>>;

template <IsAllocatorWithRightReference Allocator>
constexpr decltype(auto) GetReferedInstPtr(Allocator&& alctr);

template <IsAllocatorWithRightReference Allocator>
constexpr decltype(auto) GetAlign(Allocator&& alctr);

template <IsAllocatorWithRightReference Allocator>
constexpr decltype(auto) Allocate(Allocator&& alctr, size_t size);

template <IsAllocatorWithRightReference Allocator>
constexpr decltype(auto) Deallocate(Allocator&& alctr, void* ptr);

template <IsAllocatorWithRightReference Allocator>
constexpr decltype(auto) SafeAllocate(Allocator&& alctr, size_t align,
                                      size_t size);

struct VTable {
    void* (*Allocate)(void* alctr, size_t size);

    void (*Deallocate)(void* alctr, void* ptr);
};

template <IsAllocator Allocator>
constexpr VTable const& GetVTable();

struct EmptyAllocator {
    constexpr EmptyAllocator() = default;

    static constexpr void* GetReferedInstPtr(Tag);

    static constexpr size_t GetAlign(Tag);

    static constexpr void* Allocate(Tag, size_t);

    static constexpr void Deallocate(Tag, void*);
};

}  // namespace zeta::core::allocator
