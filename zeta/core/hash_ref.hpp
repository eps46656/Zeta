#pragma once

#include <zeta/core/hash.hpp>
#include <zeta/core/lifecycle.hpp>

namespace zeta::core::hash_ref {

struct Hasher {
    void const* target_hasher;

    unsigned long long (*hash_func)(void const* hasher, void const* value,
                                    unsigned long long salt);

    constexpr Hasher() = default;

    constexpr Hasher(Hasher const&) = default;

    template <typename TargetHasher, typename Value>
        requires hash::CanHash<TargetHasher, Value>
    constexpr Hasher(TargetHasher const& target_hasher,
                     meta::TypeWrapper<Value>);

    constexpr void Init(this Hasher& hasher, Hasher const& other_hasher);

    template <typename TargetHasher, typename Value>
        requires hash::CanHash<TargetHasher, Value>
    constexpr void Init(this Hasher& hasher, TargetHasher const& target_hasher,
                        meta::TypeWrapper<Value>);

    constexpr Hasher& operator=(Hasher const&) = default;

    constexpr void Set(this Hasher& hasher, Hasher const& other_hasher);

    template <typename TargetHasher, typename Value>
        requires hash::CanHash<TargetHasher, Value>
    constexpr void Set(this Hasher& hasher, TargetHasher const& target_hasher,
                       meta::TypeWrapper<Value>);

    constexpr unsigned long long Hash(this Hasher const& hasher,
                                      void const* value,
                                      unsigned long long salt);
};

}  // namespace zeta::core::hash_ref

namespace zeta::core {

template <>
struct lifecycle::Traits<hash_ref::Hasher>
    : public lifecycle::MemberFuncTraitsAdapter<hash_ref::Hasher> {};

template <>
struct hash::HasherTraits<hash_ref::Hasher>
    : public MemberFuncHasherTraitsAdapter<hash_ref::Hasher> {};

}  // namespace zeta::core
