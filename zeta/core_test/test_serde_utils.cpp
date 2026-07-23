#include <zeta/core/integral_math.ipp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/serde_utils.ipp>
#include <zeta/core_test/random.hpp>

template <typename Integral>
inline Integral GenIntegral() {
    using OpUnsignedIntegral = zeta::core::integral::MakeUnsignedOf<Integral>;

    constexpr unsigned long long width{
        zeta::core::integral::WidthOf<Integral>
    };

    OpUnsignedIntegral ret{ 0 };

    if constexpr (0 < width % 8) {
        unsigned char c{
            zeta::core_test::GetRandomInt<unsigned char, unsigned char>(
                0, (1U << (width % 8)) - 1U)
        };

        ret = static_cast<OpUnsignedIntegral>(c);
    }

    for (unsigned long long i{ 0 }; i < width / 8; ++i) {
        unsigned char c{
            zeta::core_test::GetRandomInt<unsigned char, unsigned char>(0, 255)
        };

        ret *= 256;
        ret += c;
    }

    return static_cast<Integral>(ret);
}

template <typename Integral>
inline Integral GenIntegralMax() {
    ZETA_Core_StaticAssert(zeta::core::integral::IsIntegral<Integral>);

    constexpr bool signedness{
        zeta::core::integral::IsSignedIntegral<Integral>
    };

    using OpUnsignedIntegral = zeta::core::integral::MakeUnsignedOf<Integral>;

    OpUnsignedIntegral ret{ static_cast<OpUnsignedIntegral>(-1) };

    if (signedness) { ret >>= 1; }

    return static_cast<Integral>(ret);
}

template <typename Integral>
inline Integral GenIntegralMin() {
    ZETA_Core_StaticAssert(zeta::core::integral::IsIntegral<Integral>);

    constexpr bool signedness{
        zeta::core::integral::IsSignedIntegral<Integral>
    };

    if constexpr (signedness) {
        return -GenIntegralMax<Integral>() - static_cast<Integral>(1);
    } else {
        return static_cast<Integral>(0);
    }
}

#define SPECIAL_VALUE_RANGE_MIN 0
#define SPECIAL_VALUE_RANGE_MIN_PLUS_ONE 1
#define SPECIAL_VALUE_NEG_TWO 2
#define SPECIAL_VALUE_NEG_ONE 3
#define SPECIAL_VALUE_ZERO 4
#define SPECIAL_VALUE_POS_ONE 5
#define SPECIAL_VALUE_POS_TWO 6
#define SPECIAL_VALUE_RANGE_MAX_MINUS_ONE 7
#define SPECIAL_VALUE_RANGE_MAX 8

#define SPECIAL_VALUE_RANGE_RANDOM 9

template <typename RangeIntegral, typename IOIntegral, typename OIIntegral,
          unsigned long long OctetCnt>
inline void test_IOI(int special_value) {
    ZETA_Core_Debug_PrintVar(
        zeta::core::debug_utils::GetTypeStr<RangeIntegral>());
    ZETA_Core_Debug_PrintVar(zeta::core::debug_utils::GetTypeStr<IOIntegral>());
    ZETA_Core_Debug_PrintVar(zeta::core::debug_utils::GetTypeStr<OIIntegral>());
    ZETA_Core_Debug_PrintVar(OctetCnt);

    unsigned char buffer[128];

    ZETA_Core_StaticAssert(zeta::core::integral::IsIntegral<RangeIntegral>);
    ZETA_Core_StaticAssert(zeta::core::integral::IsIntegral<IOIntegral>);
    ZETA_Core_StaticAssert(zeta::core::integral::IsIntegral<OIIntegral>);

    ZETA_Core_StaticAssert(
        zeta::core::integral::IsUnsignedIntegral<RangeIntegral> ==
        zeta::core::integral::IsUnsignedIntegral<IOIntegral>);

    ZETA_Core_StaticAssert(
        zeta::core::integral::IsUnsignedIntegral<RangeIntegral> ==
        zeta::core::integral::IsUnsignedIntegral<OIIntegral>);

    ZETA_Core_StaticAssert(zeta::core::integral::WidthOf<RangeIntegral> <=
                           zeta::core::integral::WidthOf<IOIntegral>);

    constexpr bool is_signed{
        zeta::core::integral::IsSignedIntegral<RangeIntegral>
    };

    IOIntegral io_val;
    OIIntegral oi_val;

    switch (special_value) {
    case SPECIAL_VALUE_RANGE_MIN:
        io_val = static_cast<IOIntegral>(GenIntegralMin<RangeIntegral>());
        break;

    case SPECIAL_VALUE_RANGE_MIN_PLUS_ONE:
        io_val = static_cast<IOIntegral>(GenIntegralMin<RangeIntegral>() +
                                         static_cast<RangeIntegral>(1));
        break;

    case SPECIAL_VALUE_NEG_TWO: io_val = static_cast<IOIntegral>(-2); break;

    case SPECIAL_VALUE_NEG_ONE: io_val = static_cast<IOIntegral>(-1); break;

    case SPECIAL_VALUE_ZERO: io_val = static_cast<IOIntegral>(0); break;

    case SPECIAL_VALUE_POS_ONE: io_val = static_cast<IOIntegral>(1); break;

    case SPECIAL_VALUE_POS_TWO: io_val = static_cast<IOIntegral>(2); break;

    case SPECIAL_VALUE_RANGE_MAX_MINUS_ONE:
        io_val = static_cast<IOIntegral>(GenIntegralMax<RangeIntegral>() -
                                         static_cast<RangeIntegral>(1));
        break;

    case SPECIAL_VALUE_RANGE_MAX:
        io_val = static_cast<IOIntegral>(GenIntegralMax<RangeIntegral>());
        break;

    case SPECIAL_VALUE_RANGE_RANDOM:
        io_val = static_cast<IOIntegral>(GenIntegral<RangeIntegral>());
        break;

    default: ZETA_Core_Unreachable();
    }

    using OctetRangeIntegral =
        zeta::core::meta::Conditional<is_signed, signed _BitInt(OctetCnt * 8),
                                      unsigned _BitInt(OctetCnt * 8)>;

    OctetRangeIntegral should_o_val{ static_cast<OctetRangeIntegral>(io_val) };
    OIIntegral should_oi_val{ static_cast<OIIntegral>(should_o_val) };

    ZETA_Core_Debug_PrintVar(io_val);
    ZETA_Core_Debug_PrintVar(
        static_cast<zeta::core::integral::MakeUnsignedOf<decltype(io_val)>>(
            io_val));
    ZETA_Core_Debug_PrintVar(should_o_val);
    ZETA_Core_Debug_PrintVar(
        static_cast<
            zeta::core::integral::MakeUnsignedOf<decltype(should_o_val)>>(
            should_o_val));
    ZETA_Core_Debug_PrintVar(should_oi_val);
    ZETA_Core_Debug_PrintVar(
        static_cast<
            zeta::core::integral::MakeUnsignedOf<decltype(should_oi_val)>>(
            should_oi_val));

    bool no_lossy_io;
    bool no_lossy_oi;

    {
        auto mem_reader{ zeta::core::lin_seq_elem_stream::Acceptor{
            .data = buffer, .elem_size = 1, .elem_stride = 1 } };

        no_lossy_io = zeta::core::serde_utils::SerializeIntegral(
            mem_reader, io_val,
            zeta::core::value_wrapper::DynamicValueWrapper<unsigned long long>{
                OctetCnt },
            zeta::core::serde_utils::EndiannessEnum::Little{}, false, nullptr);

        ZETA_Core_DebugAssert(static_cast<unsigned char*>(mem_reader.data) -
                                  buffer ==
                              static_cast<long long>(OctetCnt));

        if (io_val == should_o_val) {
            ZETA_Core_DebugAssert(no_lossy_io);
        } else {
            ZETA_Core_DebugAssert(!no_lossy_io);
        }
    }

    ZETA_Core_Debug_PrintVar(buffer[0]);
    ZETA_Core_Debug_PrintVar(buffer[1]);
    ZETA_Core_Debug_PrintVar(buffer[2]);
    ZETA_Core_Debug_PrintVar(buffer[3]);
    ZETA_Core_Debug_PrintVar(buffer[4]);
    ZETA_Core_Debug_PrintVar(buffer[5]);
    ZETA_Core_Debug_PrintVar(buffer[6]);
    ZETA_Core_Debug_PrintVar(buffer[7]);

    {
        auto mem_writer{ zeta::core::lin_seq_elem_stream::Provider{
            .data = buffer, .elem_size = 1, .elem_stride = 1 } };

        no_lossy_oi = zeta::core::serde_utils::DeserializeIntegral(
            mem_writer, oi_val,
            zeta::core::value_wrapper::DynamicValueWrapper<unsigned long long>{
                OctetCnt },
            zeta::core::serde_utils::EndiannessEnum::Little{}, false, nullptr);

        ZETA_Core_DebugAssert(
            static_cast<unsigned char const*>(mem_writer.data) - buffer ==
            static_cast<long long>(OctetCnt));

        ZETA_Core_DebugAssert(ZETA_Core_Debug_PrintVar(oi_val) ==
                              ZETA_Core_Debug_PrintVar(should_oi_val));

        if (should_o_val == should_oi_val) {
            ZETA_Core_DebugAssert(no_lossy_oi);
        } else {
            ZETA_Core_DebugAssert(!no_lossy_oi);
        }
    }

    zeta::core::debug_utils::ClearDebugStrStream();
}

template <bool Signedness, unsigned long long RangeWidth,
          unsigned long long IOWidth, unsigned long long OIWidth,
          unsigned long long OctetCnt>
void F(int special_value) {
    using RangeIntegral =
        zeta::core::meta::Conditional<Signedness, signed _BitInt(RangeWidth),
                                      unsigned _BitInt(RangeWidth)>;

    using IOIntegral =
        zeta::core::meta::Conditional<Signedness, signed _BitInt(IOWidth),
                                      unsigned _BitInt(IOWidth)>;

    using OIIntegral =
        zeta::core::meta::Conditional<Signedness, signed _BitInt(OIWidth),
                                      unsigned _BitInt(OIWidth)>;

    test_IOI<RangeIntegral,  // RangeIntegral
             IOIntegral,     // IOIntegral
             OIIntegral,     // OIIntegral
             OctetCnt        // OctetCnt
             >(special_value);
}

inline void main1() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 1'784'375'523 };

    unsigned seed{ random_seed };
    // unsigned seed{ fixed_seed };

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    for (size_t i{ 0 }; i < 1024 * 128; ++i) {
        if (i % 1024 == 0) { ZETA_Core_PrintVar(i / 1024); }

        /*
        PASS

        F<true, 34, 39, 61, 11>();
        F<true, 34, 39, 25, 7>();
        */

        for (int special_value{ i == 0 ? SPECIAL_VALUE_RANGE_MIN
                                       : SPECIAL_VALUE_RANGE_RANDOM };
             special_value <= SPECIAL_VALUE_RANGE_RANDOM; ++special_value) {
            /*
            FINISHED

            F<true, 7, 8, 4, 1>(special_value);
            F<true, 8, 9, 5, 2>(special_value);
            F<true, 9, 15, 7, 2>(special_value);
            F<true, 15, 16, 8, 2>(special_value);
            F<true, 16, 17, 9, 3>(special_value);
            F<true, 17, 24, 12, 3>(special_value);
            F<true, 24, 25, 13, 4>(special_value);
            F<true, 25, 31, 17, 5>(special_value);
            F<true, 31, 32, 16, 4>(special_value);
            F<true, 32, 33, 17, 5>(special_value);
            F<true, 33, 39, 25, 7>(special_value);
            F<true, 34, 39, 25, 7>(special_value);
            F<true, 39, 40, 24, 5>(special_value);
            F<true, 40, 41, 25, 6>(special_value);
            F<true, 40, 63, 31, 8>(special_value);
            F<true, 63, 64, 32, 8>(special_value);
            F<true, 64, 65, 33, 9>(special_value);

            F<true, 2, 3, 2, 1>(special_value);
            F<true, 33, 34, 17, 5>(special_value);
            F<true, 65, 80, 40, 10>(special_value);

            F<false, 2, 3, 2, 1>(special_value);
            F<false, 7, 8, 4, 1>(special_value);
            F<false, 8, 9, 5, 2>(special_value);
            F<false, 9, 15, 7, 2>(special_value);
            F<false, 15, 16, 8, 2>(special_value);
            F<false, 16, 17, 9, 3>(special_value);
            F<false, 17, 24, 12, 3>(special_value);
            F<false, 24, 25, 13, 4>(special_value);
            F<false, 25, 31, 17, 5>(special_value);
            F<false, 31, 32, 16, 4>(special_value);
            F<false, 32, 33, 17, 5>(special_value);
            F<false, 33, 34, 17, 5>(special_value);
            F<false, 34, 39, 25, 7>(special_value);
            F<false, 39, 40, 24, 5>(special_value);
            F<false, 40, 41, 25, 6>(special_value);
            F<false, 40, 63, 31, 8>(special_value);
            F<false, 63, 64, 32, 8>(special_value);
            F<false, 64, 65, 33, 9>(special_value);
            F<false, 65, 80, 40, 10>(special_value);

            F<true, 23, 24, 12, 3>(special_value);
            F<true, 24, 32, 16, 4>(special_value);
            F<true, 35, 40, 20, 5>(special_value);
            F<true, 47, 48, 24, 6>(special_value);
            F<true, 55, 56, 28, 7>(special_value);
            F<true, 71, 72, 36, 9>(special_value);

            F<false, 23, 24, 12, 3>(special_value);
            F<false, 24, 32, 16, 4>(special_value);
            F<false, 35, 40, 20, 5>(special_value);
            F<false, 47, 48, 24, 6>(special_value);
            F<false, 55, 56, 28, 7>(special_value);
            F<false, 71, 72, 36, 9>(special_value);

            F<true, 3, 4, 2, 1>(special_value);
            F<true, 4, 7, 3, 1>(special_value);
            F<true, 8, 9, 7, 2>(special_value);
            F<true, 9, 10, 8, 2>(special_value);
            F<true, 10, 15, 7, 2>(special_value);
            F<true, 17, 18, 10, 3>(special_value);
            F<true, 18, 23, 11, 3>(special_value);
            F<true, 33, 34, 18, 5>(special_value);
            F<true, 41, 47, 23, 6>(special_value);
            F<true, 48, 49, 25, 7>(special_value);
            F<true, 49, 55, 27, 7>(special_value);
            F<true, 56, 57, 29, 8>(special_value);
            F<true, 57, 63, 31, 8>(special_value);
            F<false, 8, 9, 7, 2>(special_value);

            F<false, 33, 39, 25, 7>(special_value);
            F<false, 34, 40, 24, 5>(special_value);
            F<false, 48, 63, 31, 8>(special_value);
            */

            F<true, 8, 64, 7, 1>(special_value);
            F<true, 9, 64, 8, 2>(special_value);

            F<true, 33, 64, 32, 5>(special_value);
            F<true, 34, 64, 33, 5>(special_value);

            F<true, 64, 64, 63, 8>(special_value);

            F<true, 5, 6, 3, 1>(special_value);
            F<true, 6, 7, 4, 1>(special_value);
            F<true, 10, 17, 9, 3>(special_value);
            F<true, 11, 18, 10, 3>(special_value);
            F<true, 12, 19, 11, 3>(special_value);
            F<true, 13, 20, 12, 3>(special_value);
            F<true, 14, 21, 13, 3>(special_value);

            F<true, 19, 26, 13, 4>(special_value);
            F<true, 20, 27, 14, 4>(special_value);
            F<true, 21, 28, 15, 4>(special_value);
            F<true, 22, 29, 16, 4>(special_value);
            F<true, 26, 33, 17, 5>(special_value);
            F<true, 27, 34, 18, 5>(special_value);
            F<true, 28, 35, 19, 5>(special_value);
            F<true, 29, 36, 20, 5>(special_value);
            F<true, 30, 37, 21, 5>(special_value);

            F<true, 35, 42, 21, 6>(special_value);
            F<true, 36, 43, 22, 6>(special_value);
            F<true, 37, 44, 23, 6>(special_value);
            F<true, 38, 45, 24, 6>(special_value);
            F<true, 42, 49, 25, 7>(special_value);
            F<true, 43, 50, 26, 7>(special_value);
            F<true, 44, 51, 27, 7>(special_value);
            F<true, 45, 52, 28, 7>(special_value);

            F<true, 50, 57, 29, 8>(special_value);
            F<true, 51, 58, 30, 8>(special_value);
            F<true, 52, 59, 31, 8>(special_value);
            F<true, 53, 60, 32, 8>(special_value);
            F<true, 58, 65, 33, 9>(special_value);
            F<true, 59, 66, 34, 9>(special_value);
            F<true, 60, 67, 35, 9>(special_value);
            F<true, 61, 68, 36, 9>(special_value);

            /*
            WAITING



            */
        }
    }
}

int main() {
    main1();
    ZETA_Core_PrintVar("ok");

    return 0;
}
