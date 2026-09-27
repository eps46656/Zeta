#pragma once

#include <cstddef>
#include <cstdlib>
#include <vector>
#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/memory.ipp>
#include <zeta/core/debug_utils/sanity.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core_test::std_allocator {

struct Allocator {
    core::debug_utils::memory::MemRecorder mem_recorder;

    size_t usage;

    size_t buffered_size_{ 0 };
    size_t max_buffered_ptrs_num_{ 0 };
    std::vector<void*> buffered_ptrs_;

    constexpr Allocator() {
#if ZETA_Core_DebugUtils_Sanity_Enable
        zeta::core::debug_utils::sanity::RegisterSanityCheckFunc(
            static_cast<void const*>(this),
            zeta::core::debug_utils::sanity::DummySanityCheckFunc);
#endif
    }

    constexpr void* GetReferedInstPtr(this Allocator const& alctr,
                                      core::allocator::Tag) {
        return const_cast<void*>(static_cast<void const*>(&alctr));
    }

    constexpr size_t GetAlign(this Allocator const& alctr,
                              core::allocator::Tag) {
        alctr.Check();
        return alignof(max_align_t);
    }

    constexpr void* Allocate(this Allocator& alctr, core::allocator::Tag,
                             size_t size) {
        alctr.Check();

        if (size == 0) { return nullptr; }

        void* ptr{ std::malloc(size) };

#if ZETA_Core_DebugEnable
        alctr.mem_recorder.Add(ptr, size);
        alctr.usage += size;
#endif

        return ptr;
    }

    constexpr void Deallocate(this Allocator& alctr, core::allocator::Tag,
                              void* ptr) {
        alctr.Check();

        if (ptr == nullptr) { return; }

        alctr.mem_recorder.Remove(ptr);

        std::free(ptr);
    }

    constexpr void Check(this Allocator const&) {}
};

}  // namespace zeta::core_test::std_allocator
