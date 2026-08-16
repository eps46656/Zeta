#pragma once

#include <zeta/core/meta.hpp>

namespace zeta::core::hash {

template <typename Hasher>
struct HasherTraits;

template <typename Hasher, typename Value>
concept CanHash = requires(Hasher const& hasher, Value const& value,
                           unsigned long long salt) {
    requires meta::IsSame<
        meta::RemoveCVRef<decltype(HasherTraits<meta::RemoveCVRef<Hasher>>::
                                       Hash(hasher, value, salt))>,
        unsigned long long>;
};

template <typename Hasher, typename Value>
    requires CanHash<Hasher, Value>
constexpr unsigned long long Hash(Hasher const& hasher, Value const& value,
                                  unsigned long long salt);

template <typename Hasher>
struct MemberFuncHasherTraitsAdapter {
    template <typename Value>
    static constexpr decltype(auto) Hash(Hasher const& hasher,
                                         Value const& value,
                                         unsigned long long salt);
};

struct EmptyHasher {
    template <typename Value>
    static constexpr unsigned long long Hash(Value const&, unsigned long long);
};

using ArchetHasher = EmptyHasher;

template <>
struct HasherTraits<hash::EmptyHasher>
    : public hash::MemberFuncHasherTraitsAdapter<hash::EmptyHasher> {};

template <typename Value>
struct BasicHasher {};

template <typename Value>
unsigned long long BasicHash(Value const& value, unsigned long long salt);

struct UniversalBasicHasher {};

template <typename Value>
unsigned long long TypeErasedBasicHash(void const* value,
                                       unsigned long long salt);

template <typename Value>
struct CppStdBasicHash {
    size_t operator()(Value const& value) const;
};

}  // namespace zeta::core::hash
