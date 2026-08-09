#pragma once

#include <zeta/core/meta.hpp>

namespace zeta::core::allocator {

template <typename Allocator>
struct AllocatorTraits;

template <typename Allocator>
concept IsAllocator = requires(Allocator& alctr, size_t size, void* ptr) {
    requires meta::IsSame<
        meta::RemoveCVRef<
            decltype(AllocatorTraits<Allocator>::GetReferedInstPtr(alctr))>,
        void*>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(AllocatorTraits<Allocator>::GetAlign(
            alctr))>,
        size_t>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(AllocatorTraits<Allocator>::Allocate(alctr,
                                                                        size))>,
        void*>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(AllocatorTraits<Allocator>::Deallocate(
            alctr, ptr))>,
        void>;
};

template <typename Allocator>
struct MemberFuncAllocatorTraitsAdapter {
    static constexpr decltype(auto) GetReferedInstPtr(Allocator& alctr);

    static constexpr decltype(auto) GetAlign(Allocator& alctr);

    static constexpr decltype(auto) Allocate(Allocator& alctr, size_t size);

    static constexpr decltype(auto) Deallocate(Allocator& alctr, void* ptr);
};

template <typename Allocator>
void* GetReferedInstPtr(Allocator& alctr);

template <typename Allocator>
size_t GetAlign(Allocator& alctr);

template <typename Allocator>
void* Allocate(Allocator& alctr, size_t size);

template <typename Allocator>
void Deallocate(Allocator& alctr, void* ptr);

template <typename Allocator>
void* SafeAllocate(Allocator& alctr, size_t align, size_t size);

struct VTable {
    void* (*Allocate)(void* alctr, size_t size);

    void (*Deallocate)(void* alctr, void* ptr);
};

template <typename Allocator>
constexpr VTable BuildVTableBasic();

template <typename Allocator>
struct BuildVTableImpl {
    static constexpr VTable Call();
};

template <typename Allocator>
constexpr VTable const& GetVTable();

}  // namespace zeta::core::allocator
