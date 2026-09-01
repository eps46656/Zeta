#pragma once

#include <zeta/core/hash.hpp>
#include <zeta/core/lifecycle.hpp>

namespace zeta::core::hash_deref {

struct OnlyDerefTag {};

template <typename TargetHasherLike, typename TargetValue>
struct Hasher {
    TargetHasherLike target_hasher;

    constexpr Hasher() = default;

    constexpr Hasher(Hasher const&) = default;

    template <typename... InnerHasherInitArgs>
    constexpr Hasher(InnerHasherInitArgs&&... target_hasher_init_args);

    constexpr void Init(this Hasher& hasher, Hasher const& other_hasher);

    template <typename... InnerHasherInitArgs>
    constexpr void Init(this Hasher& hasher,
                        InnerHasherInitArgs&&... target_hasher_init_args);

    constexpr Hasher& operator=(Hasher const&) = default;

    template <typename Value>
    constexpr unsigned long long Hash(this Hasher const& hasher, Value* value,
                                      unsigned long long salt);
};

}  // namespace zeta::core::hash_deref
