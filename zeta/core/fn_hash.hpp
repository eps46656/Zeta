#pragma once

#include <zeta/core/hash.hpp>

namespace zeta::core::fn_hash {

struct FnHasher {
    void const* hasher;
    unsigned long long (*hash)(void const* hasher, void const* value,
                               unsigned long long salt);

    constexpr FnHasher() = default;

    constexpr FnHasher(FnHasher const&) = default;

    constexpr FnHasher(FnHasher&&) = default;

    template <typename Hasher, typename Value>
        requires hash::CanHash<Hasher, Value>
    constexpr FnHasher(Hasher const& hasher, meta::TypeWrapper<Value>);

    constexpr unsigned long long Hash(this FnHasher const& fn_hash,
                                      void const* value,
                                      unsigned long long salt);
};

}  // namespace zeta::core::fn_hash

template <>
struct zeta::core::hash::HasherTraits<zeta::core::fn_hash::FnHasher>
    : public MemberFuncHasherTraitsAdapter<fn_hash::FnHasher> {};
