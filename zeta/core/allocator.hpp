#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::allocator {

template <typename Allocator, typename = void>
struct AllocatorTraits;

template <typename Allocator>
void* GetReferedInst(Allocator& alctr);

template <typename Allocator>
size_t GetAlign(Allocator& alctr);

template <typename Allocator>
void* Allocate(Allocator& alctr, size_t size);

template <typename Allocator>
void Deallocate(Allocator& alctr, void* ptr);

template <typename Allocator>
void* SafeAllocate(Allocator& alctr, size_t align, size_t size);

template <typename Allocator>
void CheckContract(Allocator& alctr);

struct VTable {
    void* (*Allocate)(void* alctr, size_t size);

    void (*Deallocate)(void* alctr, void* ptr);
};

template <typename Allocator>
constexpr VTable BuildVTableBasic();

template <typename Allocator, typename = void>
struct BuildVTableImpl {
    static constexpr VTable Call();
};

template <typename Allocator>
constexpr VTable const& GetVTable();

}  // namespace zeta::core::allocator
