#pragma once

#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core::hash {

struct Tag {};

template <typename Hasher, typename Value>
concept CanHash = requires(Hasher const& hasher, Tag tag, Value const& value,
                           unsigned long long salt) {
    requires meta::IsNonReferenceType<Hasher>;

    requires meta::IsNonReferenceType<Value>;

    requires meta::IsSame<meta::RemoveConstVolatileReference<
                              decltype(hasher.Hash(tag, value, salt))>,
                          unsigned long long>;
};

template <meta::IsNonReferenceType Hasher, meta::IsNonReferenceType Value>
constexpr unsigned long long Hash(Hasher const& hasher, Value const& value,
                                  unsigned long long salt)
    requires CanHash<Hasher, Value>;

struct EmptyHasher {
    template <meta::IsNonReferenceType Value>
    static constexpr unsigned long long Hash(Tag, Value const&,
                                             unsigned long long);
};

using ArchetypeHasher = EmptyHasher;

template <typename Value>
struct BasicHasher {};

template <integral::IsIntegralType Integral>
struct BasicHasher<Integral> {
    static constexpr unsigned long long Hash(Tag, Integral integral,
                                             unsigned long long salt);
};

template <meta::IsPointerType Pointer>
struct BasicHasher<Pointer> {
    static constexpr unsigned long long Hash(Tag, void* const pointer,
                                             unsigned long long salt);
};

template <meta::IsNonReferenceType Value>
unsigned long long BasicHash(Value const& value, unsigned long long salt);

struct UniversalBasicHasher {
    template <meta::IsNonReferenceType Value>
    static constexpr unsigned long long Hash(Tag, Value const& value,
                                             unsigned long long salt);
};

template <meta::IsNonReferenceType Value>
unsigned long long TypeErasedBasicHash(void const* value,
                                       unsigned long long salt);

template <meta::IsNonReferenceType Value>
struct CppStdBasicHash {
    size_t operator()(Value const& value) const;
};

}  // namespace zeta::core::hash
