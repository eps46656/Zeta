#pragma once

#include <zeta/core/debug_utils/lifecycle_sanity.ipp>
#include <zeta/core/debug_utils/mem_recording.ipp>
#include <zeta/core/debug_utils/memory.hpp>

namespace zeta::core::debug_utils {

namespace memory::detail {

inline mem_recording::MemRecorder records_;

}

template <typename T>
constexpr T* memory::Malloc(size_t size) {
    if (size == 0) { return nullptr; }

    void* ptr{ std::malloc(size) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(ptr != nullptr);

    detail::records_.Add(ptr, size);

    lifecycle_sanity::region_server.DeclareMem(ptr, size);

    return static_cast<T*>(ptr);
}

constexpr void memory::Free(void* ptr) {
    if (ptr == nullptr) { return; }

    size_t size{ detail::records_.GetSize(ptr) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < size);

    zeta::core::debug_utils::lifecycle_sanity::region_server.UndeclareMem(ptr,
                                                                          size);

    detail::records_.Remove(ptr);

    std::free(ptr);
}

}  // namespace zeta::core::debug_utils
