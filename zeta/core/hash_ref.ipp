#pragma once

#include <zeta/core/hash_ref.hpp>

namespace zeta::core {

template <typename TargetHasher, typename Value>
    requires hash::CanHash<TargetHasher, Value>
constexpr hash_ref::Hasher::Hasher(TargetHasher const& target_hasher,
                                   meta::TypeWrapper<Value>) {
    this->Set(target_hasher, meta::TypeWrapper<Value>{});
}

constexpr void hash_ref::Hasher::Init(this Hasher& hash_refer,
                                      Hasher const& other_hasher) {
    hash_refer = other_hasher;
}

template <typename TargetHasher, typename Value>
    requires hash::CanHash<TargetHasher, Value>
constexpr void hash_ref::Hasher::Init(this Hasher& hash_refer,
                                      TargetHasher const& target_hasher,
                                      meta::TypeWrapper<Value>) {
    hash_refer.Set(target_hasher, meta::TypeWrapper<Value>{});
}

constexpr void hash_ref::Hasher::Set(this Hasher& hash_refer,
                                     Hasher const& other_hasher) {
    hash_refer = other_hasher;
}

template <typename TargetHasher, typename Value>
    requires hash::CanHash<TargetHasher, Value>
constexpr void hash_ref::Hasher::Set(this Hasher& hash_refer,
                                     TargetHasher const& target_hasher,
                                     meta::TypeWrapper<Value>) {
    hash_refer.target_hasher = &target_hasher;

    hash_refer.hash_func = [](void const* hasher, void const* value,
                              unsigned long long salt) {
        return (hash::Hash)(*static_cast<TargetHasher const*>(hasher),
                            *static_cast<Value const*>(value), salt);
    };
}

constexpr unsigned long long hash_ref::Hasher::Hash(
    this Hasher const& hash_refer, void const* value, unsigned long long salt) {
    return hash_refer.hash_func(hash_refer.target_hasher, value, salt);
}

}  // namespace zeta::core
