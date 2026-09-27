#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/memory.ipp>
#include <zeta/core/debug_utils/recording_allocator.hpp>
#include <zeta/core/debug_utils/sanity.ipp>

namespace zeta::core::debug_utils {

template <allocator::IsAllocator TargetAllocator>
constexpr recording_allocator::Allocator<TargetAllocator>::Allocator(
    std::shared_ptr<TargetAllocator> target_alctr,
    debug_utils::memory::MemRecorderClient const& mrc)
    : target_alctr{ target_alctr }, mrc{ mrc } {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(this->target_alctr != nullptr);

    debug_utils::sanity::RegisterSanityCheckFunc(
        this, debug_utils::sanity::DummySanityCheckFunc);
}

template <allocator::IsAllocator TargetAllocator>
constexpr recording_allocator::Allocator<TargetAllocator>::~Allocator() {
    debug_utils::sanity::UnregisterSanityCheckFunc(this);
}

template <allocator::IsAllocator TargetAllocator>
constexpr void*
recording_allocator::Allocator<TargetAllocator>::GetReferedInstPtr(
    this Allocator const& self, allocator::Tag) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.target_alctr != nullptr);

    return allocator::GetReferedInstPtr(*self.target_alctr);
}

template <allocator::IsAllocator TargetAllocator>
constexpr size_t recording_allocator::Allocator<TargetAllocator>::GetAlign(
    this Allocator const& self, allocator::Tag) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.target_alctr != nullptr);

    return allocator::GetAlign(*self.target_alctr);
}

template <allocator::IsAllocator TargetAllocator>
constexpr void* recording_allocator::Allocator<TargetAllocator>::Allocate(
    this Allocator& self, allocator::Tag, size_t size) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.target_alctr != nullptr);

    void* ret{ allocator::Allocate(*self.target_alctr, size) };

    if (ret == nullptr) { return nullptr; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        __builtin_is_aligned(ret, allocator::GetAlign(*self.target_alctr)));

    self.mrc.Add(ret, size);

    return ret;
}

template <allocator::IsAllocator TargetAllocator>
constexpr void recording_allocator::Allocator<TargetAllocator>::Deallocate(
    this Allocator& self, allocator::Tag, void* ptr) {
    if (ptr != nullptr) { self.mrc.Remove(ptr); }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.target_alctr != nullptr);

    allocator::Deallocate(*self.target_alctr, ptr);
}

template <allocator::IsAllocator TargetAllocator>
constexpr debug_utils::memory::MemRecorder const&
recording_allocator::Allocator<TargetAllocator>::GetMemRecorder(
    this Allocator const& self) {
    return self.mrc.GetMemRecorder();
}

template <allocator::IsAllocator TargetAllocator>
constexpr debug_utils::memory::MemRecorderClient const&
recording_allocator::Allocator<TargetAllocator>::GetMemRecorderClient(
    this Allocator const& self) {
    return self.mrc;
}

template <allocator::IsAllocator TargetAllocator>
constexpr void recording_allocator::Allocator<TargetAllocator>::SanityCheck(
    void const* self_, debug_utils::sanity::SanityCheckScope scope) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self_ != nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        scope == debug_utils::sanity::SanityCheckScope::Basic ||
        scope == debug_utils::sanity::SanityCheckScope::Complete);

    auto& self{ *static_cast<Allocator<TargetAllocator> const*>(self_) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.target_alctr != nullptr);

    sanity::ExpandFinishedSanityCheckScope(
        self_, debug_utils::sanity::SanityCheckScope::Basic);

    if (scope == debug_utils::sanity::SanityCheckScope::Basic) { return; }

    debug_utils::sanity::SanityCheck(self.target_alctr.get(), scope);
}

template <allocator::IsAllocator InAllocator>
constexpr decltype(auto) recording_allocator::TryMakeSubGroupAllocator(
    InAllocator& in_alctr, std::string const& sub_group_name) {
    if constexpr (recording_allocator::IsRecordingAllocator<InAllocator>) {
        return Allocator<InAllocator>{
            in_alctr.target_alctr,
            in_alctr.mrc.GetSubGroupClient(sub_group_name),
        };
    } else {
        return in_alctr;
    }
}

}  // namespace zeta::core::debug_utils
