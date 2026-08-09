#pragma once

#include <zeta/core/fn_hash.hpp>

namespace zeta::core {

template <typename Hasher, typename Value>
    requires hash::CanHash<Hasher, Value>
constexpr fn_hash::FnHasher::FnHasher(Hasher const& hasher,
                                      meta::TypeWrapper<Value>)
    : hasher{ &hasher },
      hash{ [](void const* hasher, void const* value, unsigned long long salt) {
          return (hash::Hash)(*static_cast<Hasher const*>(hasher),
                              *static_cast<Value const*>(value), salt);
      } } {}

constexpr unsigned long long fn_hash::FnHasher::Hash(
    this FnHasher const& fn_hasher, void const* value,
    unsigned long long salt) {
    return fn_hasher.hash(fn_hasher.hasher, value, salt);
}

}  // namespace zeta::core
