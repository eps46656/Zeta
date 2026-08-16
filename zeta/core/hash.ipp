#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/hash.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

template <typename Hasher, typename Value>
    requires hash::CanHash<Hasher, Value>
constexpr unsigned long long hash::Hash(Hasher const& hasher,
                                        Value const& value,
                                        unsigned long long salt) {
    return HasherTraits<meta::RemoveCVRef<Hasher>>::Hash(hasher, value, salt);
}

template <typename Hasher>
template <typename Value>
constexpr decltype(auto) hash::MemberFuncHasherTraitsAdapter<Hasher>::Hash(
    Hasher const& hasher, Value const& value, unsigned long long salt) {
    return hasher.Hash(value, salt);
}

template <typename Value>
constexpr unsigned long long hash::EmptyHasher::Hash(Value const&,
                                                     unsigned long long) {
    return 0;
}

template <typename Value>
unsigned long long hash::BasicHash(Value const& value,
                                   unsigned long long salt) {
    return (Hash)(BasicHasher<Value>{}, value, salt);
}

template <integral::IsIntegral Integral>
struct hash::HasherTraits<hash::BasicHasher<Integral>> {
    static unsigned long long Hash(hash::BasicHasher<Integral>,
                                   Integral integral, unsigned long long salt) {
        auto unsigned_integral{ integral::MakeUnsignedOf<Integral>{
            integral } };

        unsigned long long value{ salt ^ static_cast<unsigned long long>(
                                             unsigned_integral) };

        if constexpr (integral::WidthOf<unsigned long long> <
                      integral::WidthOf<Integral>) {
            for (; 0 < unsigned_integral;
                 unsigned_integral >>= integral::WidthOf<unsigned long long>) {
                value *= 23;
                value += static_cast<unsigned long long>(integral);
            }
        }

#if ZETA_Core_ullong_width == 32
        value = (value ^ (value >> 16)) * 0x45d9f3bULL;
        value = (value ^ (value >> 16)) * 0x45d9f3bULL;
        value = value ^ (value >> 16);
#elif ZETA_Core_ullong_width == 64
        value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
        value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
        value = value ^ (value >> 31);
#else
#error "Unsupported architecture."
#endif

        value ^= salt;

        return value;
    }
};

template <meta::IsPointer Pointer>
struct hash::HasherTraits<hash::BasicHasher<Pointer>> {
    static unsigned long long Hash(hash::BasicHasher<Pointer>,
                                   void* const pointer,
                                   unsigned long long salt) {
        return (BasicHash)(reinterpret_cast<uintptr_t>(pointer), salt);
    }
};

template <>
struct hash::HasherTraits<hash::UniversalBasicHasher> {
    template <typename Value>
    static constexpr unsigned long long Hash(hash::UniversalBasicHasher,
                                             Value const& value,
                                             unsigned long long salt) {
        return (BasicHash)(value, salt);
    }
};

template <typename Value>
unsigned long long hash::TypeErasedBasicHash(void const* value,
                                             unsigned long long salt) {
    return (BasicHash<Value>)(*static_cast<Value const*>(value), salt);
}

template <typename Value>
size_t hash::CppStdBasicHash<Value>::operator()(Value const& value) const {
    return static_cast<size_t>((BasicHash<Value>)(value, 0));
}

}  // namespace zeta::core
