#pragma once

#include <zeta/core/hash_deref.hpp>
#include <zeta/core/lifecycle.hpp>

namespace zeta::core {

template <typename TargetHasherLike, typename TargetValue>
template <typename... InnerHasherInitArgs>
constexpr hash_deref::Hasher<TargetHasherLike, TargetValue>::Hasher(
    InnerHasherInitArgs&&... target_hasher_init_args) {
    this->Init(meta::Forward<InnerHasherInitArgs>(target_hasher_init_args)...);
}

template <typename TargetHasherLike, typename TargetValue>
constexpr void hash_deref::Hasher<TargetHasherLike, TargetValue>::Init(
    this Hasher& hash_deref, Hasher const& other_hasher) {
    hash_deref = other_hasher;
}

template <typename TargetHasherLike, typename TargetValue>
template <typename... InnerHasherInitArgs>
constexpr void hash_deref::Hasher<TargetHasherLike, TargetValue>::Init(
    this Hasher& hash_deref, InnerHasherInitArgs&&... target_hasher_init_args) {
    lifecycle::Init(
        hash_deref.target_hasher,
        meta::Forward<InnerHasherInitArgs>(target_hasher_init_args)...);
}

template <typename TargetHasherLike, typename TargetValue>
template <typename Value>
constexpr unsigned long long
hash_deref::Hasher<TargetHasherLike, TargetValue>::Hash(
    this Hasher const& hash_deref, Value* value, unsigned long long salt) {
    if constexpr (meta::IsSame<TargetValue, OnlyDerefTag>) {
        return hash::Hash(hash_deref.target_hasher, *value, salt);
    } else {
        return hash::Hash(
            hash_deref.target_hasher,
            *static_cast<TargetValue*>(
                static_cast<meta::Conditional<meta::IsConst<Value>, void const*,
                                              void*>>(value)),
            salt);
    }
}

}  // namespace zeta::core
