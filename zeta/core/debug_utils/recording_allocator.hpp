#pragma once

#include <memory>
#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils/mem_recording.hpp>

namespace zeta::core::debug_utils::recording_allocator {

template <allocator::IsAllocator TargetAllocator>
struct Allocator {
    std::shared_ptr<TargetAllocator> target_alctr;
    debug_utils::mem_recording::MemRecorderClient mrc;

    constexpr Allocator(
        std::shared_ptr<TargetAllocator> target_alctr,
        debug_utils::mem_recording::MemRecorderClient const& mrc);

    constexpr Allocator(Allocator const& alctr) = default;

    constexpr void* GetReferedInstPtr(this Allocator const& self,
                                      allocator::Tag);

    constexpr size_t GetAlign(this Allocator const& alctr, allocator::Tag);

    constexpr void* Allocate(this Allocator& self, allocator::Tag, size_t size);

    constexpr void Deallocate(this Allocator& self, allocator::Tag, void* ptr);

    constexpr debug_utils::mem_recording::MemRecorder const& GetMemRecorder(
        this Allocator const& self);

    constexpr debug_utils::mem_recording::MemRecorderClient const&
    GetMemRecorderClient(this Allocator const& self);
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
