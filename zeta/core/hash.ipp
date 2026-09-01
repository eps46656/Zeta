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
    return hasher.Hash(Tag{}, value, salt);
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
constexpr unsigned long long hash::BasicHasher<Integral>::Hash(
    Tag, Integral integral, unsigned long long salt) {
    auto unsigned_integral{ integral::MakeUnsignedOf<Integral>{ integral } };

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

template <meta::IsPointer Pointer>
constexpr unsigned long long hash::BasicHasher<Pointer>::Hash(
    Tag, void* const pointer, unsigned long long salt) {
    return (BasicHash)(reinterpret_cast<uintptr_t>(pointer), salt);
}

template <typename Value>
constexpr unsigned long long hash::UniversalBasicHasher::Hash(
    Tag, Value const& value, unsigned long long salt) {
    return (BasicHash)(value, salt);
}

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
