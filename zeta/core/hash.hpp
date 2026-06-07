#pragma once

#include <zeta/core/define.hpp>

namespace zeta::core::hash {

template <typename Hasher, typename Value>
struct HasherTraits {
    static constexpr unsigned long long Hash(Hasher const& hasher,
                                             Value const& value,
                                             unsigned long long salt);
};

template <typename Hasher, typename Value>
constexpr unsigned long long Hash(Hasher const& hasher, Value const& value,
                                  unsigned long long salt);

template <typename Hasher, typename Value>
void CheckContract();

template <typename Value, typename = void>
struct BasicHasher;  // IWYU pragma: export

template <typename Value>
unsigned long long BasicHash(Value const& value, unsigned long long salt);

struct UniversalBasicHasher {
    template <typename Value>
    constexpr unsigned long long operator()(Value const& value,
                                            unsigned long long salt) const;
};

template <typename Value>
unsigned long long TypeErasedBasicHash(void const* value,
                                       unsigned long long salt);

template <typename Value>
struct CppStdBasicHash {
    size_t operator()(Value const& value) const;
};

}  // namespace zeta::core::hash
