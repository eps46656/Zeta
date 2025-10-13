#pragma once

namespace zeta::core::hash {

template <typename T>
unsigned long long Hash(T const& x, unsigned long long salt);

template <typename T>
unsigned long long TypeErasedHash(void const* x, unsigned long long salt);

template <typename T>
struct CppStdHash {
    size_t operator()(T const& x) const;
};

template <typename T, typename _ = void>
struct HashCore;

}  // namespace zeta::core::hash
