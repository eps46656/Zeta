#pragma once

#include <memory>
#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils/memory.hpp>
#include <zeta/core/debug_utils/sanity.hpp>

namespace zeta::core::debug_utils::recording_allocator {

template <allocator::IsAllocator TargetAllocator>
struct Allocator {
    std::shared_ptr<TargetAllocator> target_alctr;
    debug_utils::memory::MemRecorderClient mrc;

    constexpr Allocator(std::shared_ptr<TargetAllocator> target_alctr,
                        debug_utils::memory::MemRecorderClient const& mrc);

    constexpr Allocator(Allocator const& alctr) = default;

    constexpr ~Allocator();

    constexpr void* GetReferedInstPtr(this Allocator const& self,
                                      allocator::Tag);

    constexpr size_t GetAlign(this Allocator const& alctr, allocator::Tag);

    constexpr void* Allocate(this Allocator& self, allocator::Tag, size_t size);

    constexpr void Deallocate(this Allocator& self, allocator::Tag, void* ptr);

    constexpr debug_utils::memory::MemRecorder const& GetMemRecorder(
        this Allocator const& self);

    constexpr debug_utils::memory::MemRecorderClient const&
    GetMemRecorderClient(this Allocator const& self);

    static constexpr void SanityCheck(
        void const* alctr, debug_utils::sanity::SanityCheckScope scope);
};

namespace detail {

template <typename T>
struct IsRecordingAllocator_ {
    static constexpr bool value{ false };
};

template <allocator::IsAllocator TargetAllocator>
struct IsRecordingAllocator_<Allocator<TargetAllocator>> {
    static constexpr bool value{ true };
};

}  // namespace detail

template <typename T>
constexpr bool IsRecordingAllocator{ detail::IsRecordingAllocator_<T>::value };

template <allocator::IsAllocator Allocator>
constexpr decltype(auto) TryMakeSubGroupAllocator(
    Allocator& alctr, std::string const& sub_group_name);

}  // namespace zeta::core::debug_utils::recording_allocator
