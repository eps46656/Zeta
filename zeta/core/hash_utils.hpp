#pragma once

namespace zeta::core::hash_utils {

unsigned long long BasicMemHash(void const* data, size_t size,
                                unsigned long long salt);

unsigned long long BasicElemHash(void const* data, size_t width, size_t stride,
                                 size_t cnt, unsigned long long salt);

}  // namespace zeta::core::hash_utils
