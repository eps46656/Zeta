#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::allocator {

template <typename Allocator, typename = void>
struct AllocatorTraits;

template <typename AllocatorLike>
void* GetReferedInst(AllocatorLike&& alctr);

template <typename AllocatorLike>
size_t GetAlign(AllocatorLike&& alctr);

template <typename AllocatorLike>
void* Allocate(AllocatorLike&& alctr, size_t size);

template <typename AllocatorLike>
void Deallocate(AllocatorLike&& alctr, void* ptr);

template <typename AllocatorLike>
void* SafeAllocate(AllocatorLike&& alctr, size_t align, size_t size);

template <typename AllocatorLike>
void CheckContract(AllocatorLike&& alctr);

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
