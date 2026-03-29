#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_bit.hpp>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, hicpp-vararg)

namespace zeta::core {

template <typename Integral>
struct integral_bit::Traits<Integral> {
    static constexpr long long PopCount(Integral num) {
        return BasicPopCount(num);
    }

    static constexpr long long CLZ(Integral num) { return BasicCLZ(num); }

    static constexpr long long CTZ(Integral num) { return BasicCTZ(num); }
};

#pragma push_macro("F")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(type)                                                          \
    template <>                                                          \
    struct integral_bit::Traits<type> {                                  \
        static constexpr long long PopCount(type num) {                  \
            ZETA_Core_DebugAssert(static_cast<type>(0) <= num);          \
                                                                         \
            return __builtin_popcountg(integral::UnsignedOf<type>(num)); \
        }                                                                \
                                                                         \
        static constexpr long long CLZ(type num) {                       \
            ZETA_Core_DebugAssert(static_cast<type>(0) < num);           \
                                                                         \
            return __builtin_clzg(integral::UnsignedOf<type>(num));      \
        }                                                                \
                                                                         \
        static constexpr long long CTZ(type num) {                       \
            ZETA_Core_DebugAssert(static_cast<type>(0) < num);           \
                                                                         \
            return __builtin_ctzg(integral::UnsignedOf<type>(num));      \
        }                                                                \
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
struct integral_bit::Traits<unsigned _BitInt(N)> {
    static constexpr long long PopCount(unsigned _BitInt(N) num) {
        ZETA_Core_DebugAssert(static_cast<unsigned _BitInt(N)>(0) <= num);

        return __builtin_popcountg(num);
    }

    static constexpr long long CLZ(unsigned _BitInt(N) num) {
        ZETA_Core_DebugAssert(static_cast<unsigned _BitInt(N)>(0) < num);

        return __builtin_clzg(num);
    }

    static constexpr long long CTZ(unsigned _BitInt(N) num) {
        ZETA_Core_DebugAssert(static_cast<unsigned _BitInt(N)>(0) < num);

        return __builtin_ctzg(num);
    }
};

template <size_t N>
struct integral_bit::Traits<signed _BitInt(N)> {
    static constexpr long long PopCount(signed _BitInt(N) num) {
        ZETA_Core_DebugAssert(static_cast<signed _BitInt(N)>(0) <= num);

        return __builtin_popcountg(static_cast<unsigned _BitInt(N)>(num));
    }

    static constexpr long long CLZ(signed _BitInt(N) num) {
        ZETA_Core_DebugAssert(static_cast<signed _BitInt(N)>(0) < num);

        return __builtin_clzg(static_cast<unsigned _BitInt(N)>(num));
    }

    static constexpr long long CTZ(signed _BitInt(N) num) {
        ZETA_Core_DebugAssert(static_cast<signed _BitInt(N)>(0) < num);

        return __builtin_ctzg(static_cast<unsigned _BitInt(N)>(num));
    }
};

template <typename Num>
constexpr long long integral_bit::BasicPopCount(Num num) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Num>);

    ZETA_Core_DebugAssert(static_cast<Num>(0) <= num);

    return __builtin_popcountg(
        static_cast<unsigned _BitInt(integral::WidthOf<Num>)>(num));
}

template <typename Num>
constexpr long long integral_bit::PopCount(Num num) {
    return Traits<Num>::PopCount(num);
}

template <typename Num>
constexpr long long integral_bit::BasicCLZ(Num num) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Num>);

    ZETA_Core_DebugAssert(static_cast<Num>(0) < num);

    return __builtin_clzg(
        static_cast<unsigned _BitInt(integral::WidthOf<Num>)>(num));
}

template <typename Num>
constexpr long long integral_bit::CLZ(Num num) {
    return Traits<Num>::CLZ(num);
}

template <typename Num>
constexpr long long integral_bit::BasicCTZ(Num num) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Num>);

    ZETA_Core_DebugAssert(static_cast<Num>(0) < num);

    return __builtin_ctzg(
        static_cast<unsigned _BitInt(integral::WidthOf<Num>)>(num));
}

template <typename Num>
constexpr long long integral_bit::CTZ(Num num) {
    return Traits<Num>::CTZ(num);
}

template <typename Num>
constexpr long long integral_bit::FindPrevBit(Num num_, long long pos) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Num>);

    constexpr long long w{ static_cast<long long>(integral::WidthOf<Num>) };

    ZETA_Core_DebugAssert(static_cast<Num>(0) <= num_);

    ZETA_Core_DebugAssert(-1 <= pos);
    ZETA_Core_DebugAssert(pos <= w - 1);

    if (pos == -1) { return -1; }

    using UnsignedNum = integral::UnsignedOf<Num>;

    UnsignedNum num{ static_cast<UnsignedNum>(num_) };

    num <<= w - 1 - pos;

    return num == static_cast<UnsignedNum>(0)
               ? -1
               : (CLZ)(static_cast<UnsignedNum>(1)) - w + 1 + pos - (CLZ)(num);
}

template <typename Num>
constexpr long long integral_bit::FindNextBit(Num num_, long long pos) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Num>);

    constexpr long long w{ static_cast<long long>(integral::WidthOf<Num>) };

    ZETA_Core_DebugAssert(static_cast<Num>(0) <= num_);

    ZETA_Core_DebugAssert(0 <= pos);
    ZETA_Core_DebugAssert(pos <= w);

    if (pos == w) { return w; }

    using UnsignedNum = integral::UnsignedOf<Num>;

    UnsignedNum num{ static_cast<UnsignedNum>(num_) };

    num >>= pos;

    return num == static_cast<UnsignedNum>(0) ? w : pos + (CTZ)(num);
}

}  // namespace zeta::core

// NOLINTEND(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
