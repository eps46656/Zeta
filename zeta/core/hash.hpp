#pragma once

#include <zeta/core/define.hpp>

namespace zeta::core::hash {

namespace ops {

unsigned long long BasicMemHash(void const* data, size_t size,
                                unsigned long long salt);

unsigned long long BasicElemHash(void const* data, size_t width, size_t stride,
                                 size_t cnt, unsigned long long salt);

template <typename T, typename _ = void>
struct BasicHashImpl;

template <typename T>
unsigned long long BasicHash(T const& x, unsigned long long salt);

template <typename T>
unsigned long long TypeErasedBasicHash(void const* x, unsigned long long salt);

}  // namespace ops

template <typename T>
struct CppStdBasicHash {
    size_t operator()(T const& x) const;
};

}  // namespace zeta::core::hash
