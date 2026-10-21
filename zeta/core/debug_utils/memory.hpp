#pragma once

#include <zeta/core/meta.hpp>

namespace zeta::core::debug_utils::memory {

template <typename T>
constexpr T* Malloc(size_t size);

constexpr void Free(void* ptr);

}  // namespace zeta::core::debug_utils::memory
