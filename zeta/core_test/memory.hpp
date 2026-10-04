#pragma once

#include <cstdlib>
#include <unordered_map>
#include <zeta/core/debug_utils/lifecycle_sanity.ipp>
#include <zeta/core/debug_utils/memory.ipp>

namespace zeta::core_test::memory {

inline core::debug_utils::memory::MemRecorder records;

template <typename T = void>
constexpr T* Malloc(size_t size) {
    if (size == 0) { return nullptr; }

    T* ret{ static_cast<T*>(std::malloc(size)) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(ret != nullptr);

    records.Add(ret, size);

    return ret;
}

constexpr void Free(void* ptr) {
    if (ptr == nullptr) { return; }

    size_t size{ records.GetSize(ptr) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < size);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        !core::debug_utils::lifecycle_sanity::Contains(ptr, size));

    records.Remove(ptr);

    std::free(ptr);
}

}  // namespace zeta::core_test::memory
