#pragma once

#include <zeta/core/meta.hpp>

namespace zeta::core::allocator {

struct Tag {};

template <typename Allocator>
concept IsAllocator = requires(Allocator& alctr, size_t size, void* ptr) {
    requires meta::IsSame<
        meta::RemoveCVRef<decltype(alctr.GetReferedInstPtr(Tag{}))>, void*>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(alctr.GetAlign(Tag{}))>,
                          size_t>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(alctr.Allocate(Tag{}, size))>, void*>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(alctr.Deallocate(Tag{}, ptr))>, void>;
};

template <typename Allocator>
constexpr decltype(auto) GetReferedInstPtr(Allocator& alctr);

template <typename Allocator>
constexpr decltype(auto) GetAlign(Allocator& alctr);

template <typename Allocator>
constexpr decltype(auto) Allocate(Allocator& alctr, size_t size);

template <typename Allocator>
constexpr decltype(auto) Deallocate(Allocator& alctr, void* ptr);

template <typename Allocator>
constexpr decltype(auto) SafeAllocate(Allocator& alctr, size_t align,
                                      size_t size);

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
