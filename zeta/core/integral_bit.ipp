#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_bit.hpp>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, hicpp-vararg)

namespace zeta::core {

template <typename Integral>
struct integral_bit::IntegralTraits<Integral> {
    static constexpr unsigned long long PopCount(Integral num) {
        return (BasicPopCount)(num);
    }

    static constexpr unsigned long long CLZ(Integral num) {
        return (BasicCLZ)(num);
    }

    static constexpr unsigned long long CTZ(Integral num) {
        return (BasicCTZ)(num);
    }
};

#pragma push_macro("F")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(type)                                                            \
    template <>                                                            \
    struct integral_bit::IntegralTraits<type> {                            \
        static constexpr unsigned long long PopCount(type num) {           \
            ZETA_Core_DebugAssert(static_cast<type>(0) <= num);            \
                                                                           \
            return static_cast<unsigned long long>(                        \
                __builtin_popcountg(integral::MakeUnsignedOf<type>(num))); \
        }                                                                  \
                                                                           \
        static constexpr unsigned long long CLZ(type num) {                \
            ZETA_Core_DebugAssert(static_cast<type>(0) < num);             \
                                                                           \
            return static_cast<unsigned long long>(                        \
                __builtin_clzg(integral::MakeUnsignedOf<type>(num)));      \
        }                                                                  \
                                                                           \
        static constexpr unsigned long long CTZ(type num) {                \
            ZETA_Core_DebugAssert(static_cast<type>(0) < num);             \
                                                                           \
            return static_cast<unsigned long long>(                        \
                __builtin_ctzg(integral::MakeUnsignedOf<type>(num)));      \
        }                                                                  \
    };

F(char)
F(unsigned char)
F(signed char)
F(unsigned short)
F(signed short)
F(unsigned int)
F(signed int)
F(unsigned long)
F(signed long)
F(unsigned long long)
F(signed long long)

#pragma pop_macro("F")

template <size_t N>
struct integral_bit::IntegralTraits<unsigned _BitInt(N)> {
    static constexpr unsigned long long PopCount(unsigned _BitInt(N) num) {
        ZETA_Core_DebugAssert(static_cast<unsigned _BitInt(N)>(0) <= num);

        return static_cast<unsigned long long>(__builtin_popcountg(num));
    }

    static constexpr unsigned long long CLZ(unsigned _BitInt(N) num) {
        ZETA_Core_DebugAssert(static_cast<unsigned _BitInt(N)>(0) < num);

        return static_cast<unsigned long long>(__builtin_clzg(num));
    }

    static constexpr unsigned long long CTZ(unsigned _BitInt(N) num) {
        ZETA_Core_DebugAssert(static_cast<unsigned _BitInt(N)>(0) < num);

        return static_cast<unsigned long long>(__builtin_ctzg(num));
    }
};

template <size_t N>
struct integral_bit::IntegralTraits<signed _BitInt(N)> {
    static constexpr unsigned long long PopCount(signed _BitInt(N) num) {
        ZETA_Core_DebugAssert(static_cast<signed _BitInt(N)>(0) <= num);

        return static_cast<unsigned long long>(
            __builtin_popcountg(static_cast<unsigned _BitInt(N)>(num)));
    }

    static constexpr unsigned long long CLZ(signed _BitInt(N) num) {
        ZETA_Core_DebugAssert(static_cast<signed _BitInt(N)>(0) < num);

        return static_cast<unsigned long long>(
            __builtin_clzg(static_cast<unsigned _BitInt(N)>(num)));
    }

    static constexpr unsigned long long CTZ(signed _BitInt(N) num) {
        ZETA_Core_DebugAssert(static_cast<signed _BitInt(N)>(0) < num);

        return static_cast<unsigned long long>(
            __builtin_ctzg(static_cast<unsigned _BitInt(N)>(num)));
    }
};

template <typename Integral>
constexpr unsigned long long integral_bit::BasicPopCount(Integral num) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Integral>);

    ZETA_Core_DebugAssert(static_cast<Integral>(0) <= num);

    return __builtin_popcountg(
        static_cast<unsigned _BitInt(integral::WidthOf<Integral>)>(num));
}

template <typename Integral>
constexpr unsigned long long integral_bit::PopCount(Integral num) {
    return IntegralTraits<Integral>::PopCount(num);
}

template <typename Integral>
constexpr unsigned long long integral_bit::BasicCLZ(Integral num) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Integral>);

    ZETA_Core_DebugAssert(static_cast<Integral>(0) < num);

    return __builtin_clzg(
        static_cast<unsigned _BitInt(integral::WidthOf<Integral>)>(num));
}

template <typename Integral>
constexpr unsigned long long integral_bit::CLZ(Integral num) {
    return IntegralTraits<Integral>::CLZ(num);
}

template <typename Integral>
constexpr unsigned long long integral_bit::BasicCTZ(Integral num) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Integral>);

    ZETA_Core_DebugAssert(static_cast<Integral>(0) < num);

    return __builtin_ctzg(
        static_cast<unsigned _BitInt(integral::WidthOf<Integral>)>(num));
}

template <typename Integral>
constexpr unsigned long long integral_bit::CTZ(Integral num) {
    return IntegralTraits<Integral>::CTZ(num);
}

template <typename Integral>
constexpr unsigned long long integral_bit::FindPrevBit(Integral num_,
                                                       unsigned long long pos) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Integral>);

    constexpr unsigned long long w{ integral::WidthOf<Integral> };

    ZETA_Core_DebugAssert(static_cast<Integral>(0) <= num_);

    ZETA_Core_DebugAssert(0 <= pos);
    ZETA_Core_DebugAssert(pos < w);

    using UnsignedIntegral = integral::MakeUnsignedOf<Integral>;

    UnsignedIntegral num{ static_cast<UnsignedIntegral>(num_) };

    num <<= w - 1 - pos;

    return num == static_cast<UnsignedIntegral>(0)
               ? integral::RangeMaxOf<unsigned long long>
               : (CLZ)(static_cast<UnsignedIntegral>(1)) - w + 1 + pos -
                     (CLZ)(num);
}

template <typename Integral>
constexpr unsigned long long integral_bit::FindNextBit(Integral num_,
                                                       unsigned long long pos) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Integral>);

    constexpr unsigned long long w{ integral::WidthOf<Integral> };

    ZETA_Core_DebugAssert(static_cast<Integral>(0) <= num_);

    ZETA_Core_DebugAssert(0 <= pos);
    ZETA_Core_DebugAssert(pos < w);

    using UnsignedIntegral = integral::MakeUnsignedOf<Integral>;

    UnsignedIntegral num{ static_cast<UnsignedIntegral>(num_) };

    num >>= pos;

    return num == static_cast<UnsignedIntegral>(0)
               ? integral::RangeMaxOf<unsigned long long>
               : pos + (CTZ)(num);
}

}  // namespace zeta::core

// NOLINTEND(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
