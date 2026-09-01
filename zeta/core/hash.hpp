#pragma once

#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core::hash {

struct Tag {};

template <typename Hasher, typename Value>
concept CanHash = requires(Hasher const& hasher, Tag tag, Value const& value,
                           unsigned long long salt) {
    requires meta::IsSame<
        meta::RemoveCVRef<decltype(hasher.Hash(tag, value, salt))>,
        unsigned long long>;
};

template <typename Hasher, typename Value>
    requires CanHash<Hasher, Value>
constexpr unsigned long long Hash(Hasher const& hasher, Value const& value,
                                  unsigned long long salt);

struct EmptyHasher {
    template <typename Value>
    static constexpr unsigned long long Hash(Value const&, unsigned long long);
};

using ArchetypeHasher = EmptyHasher;

template <typename Value>
struct BasicHasher {};

template <integral::IsIntegral Integral>
struct BasicHasher<Integral> {
    static constexpr unsigned long long Hash(Tag, Integral integral,
                                             unsigned long long salt);
};

template <meta::IsPointer Pointer>
struct BasicHasher<Pointer> {
    static constexpr unsigned long long Hash(Tag, void* const pointer,
                                             unsigned long long salt);
};

template <typename Value>
unsigned long long BasicHash(Value const& value, unsigned long long salt);

struct UniversalBasicHasher {
    template <typename Value>
    static constexpr unsigned long long Hash(Tag, Value const& value,
                                             unsigned long long salt);
};

template <typename Value>
unsigned long long TypeErasedBasicHash(void const* value,
                                       unsigned long long salt);

template <typename Value>
struct CppStdBasicHash {
    size_t operator()(Value const& value) const;
};

}  // namespace zeta::core::hash
