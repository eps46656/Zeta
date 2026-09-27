#include <zeta/core/integral_endec.ipp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/lin_seq_endpoint.ipp>
#include <zeta/core_test/random.hpp>

#define SIGNED true
#define UNSIGNED false

#define LE 'L'
#define BE 'B'

#define FWC false
#define VWC true

template <typename Integral, typename DigitIntegral>
inline Integral GenIntegral() {
    using OpUnsignedIntegral = zeta::core::integral::MakeUnsignedOf<Integral>;

    constexpr size_t width{ zeta::core::integral::WidthOf<Integral> };

    constexpr size_t digit_width{
        zeta::core::integral::WidthOf<DigitIntegral>
    };

    OpUnsignedIntegral ret{ 0 };

    if constexpr (0 < width % digit_width) {
        DigitIntegral c{
            zeta::core_test::GenUniformRandomInt<DigitIntegral, DigitIntegral>(
                0, static_cast<DigitIntegral>(1) << (width % digit_width) - 1)
        };

        ret = static_cast<OpUnsignedIntegral>(c);
    }

    for (size_t i{ 0 }; i < width / digit_width; ++i) {
        DigitIntegral c{
            zeta::core_test::GenUniformRandomInt<DigitIntegral, DigitIntegral>(
                0, zeta::core::integral::RangeMaxOf<DigitIntegral>)
        };

        ret *= 256;
        ret += c;
    }

    return static_cast<Integral>(ret);
}

template <typename Integral>
inline Integral GenIntegralMax() {
    static_assert(zeta::core::integral::IsIntegral<Integral>);

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
    static_assert(zeta::core::integral::IsIntegral<Integral>);

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

template <typename RangeIntegral, char Endianness, typename IOIntegral,
          typename OIIntegral, typename DigitIntegral, size_t DigitWidth,
          bool VariantDigitCnt, size_t DigitCnt>
inline void test_IOI(int special_value) {
    ZETA_Core_DebugUtils_Diag_LogVar(
        zeta::core::debug_utils::logging::GetTypeStr<RangeIntegral>());
    ZETA_Core_DebugUtils_Diag_LogVar(
        zeta::core::debug_utils::logging::GetTypeStr<IOIntegral>());
    ZETA_Core_DebugUtils_Diag_LogVar(
        zeta::core::debug_utils::logging::GetTypeStr<OIIntegral>());
    ZETA_Core_DebugUtils_Diag_LogVar(DigitCnt);

    constexpr size_t buffer_cnt{ 256 };
    DigitIntegral buffer[buffer_cnt];

    static_assert(zeta::core::integral::IsIntegral<RangeIntegral>);
    static_assert(zeta::core::integral::IsIntegral<IOIntegral>);
    static_assert(zeta::core::integral::IsIntegral<OIIntegral>);

    static_assert(zeta::core::integral::IsUnsignedIntegral<RangeIntegral> ==
                  zeta::core::integral::IsUnsignedIntegral<IOIntegral>);

    static_assert(zeta::core::integral::IsUnsignedIntegral<RangeIntegral> ==
                  zeta::core::integral::IsUnsignedIntegral<OIIntegral>);

    static_assert(zeta::core::integral::WidthOf<RangeIntegral> <=
                  zeta::core::integral::WidthOf<IOIntegral>);

    constexpr bool is_signed{
        zeta::core::integral::IsSignedIntegral<RangeIntegral>
    };

    zeta::core::integral_endec::Endianness endianness;

    switch (Endianness) {
    case LE: endianness = zeta::core::integral_endec::Endianness::Little; break;
    case BE: endianness = zeta::core::integral_endec::Endianness::Big; break;
    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }

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
        io_val = static_cast<IOIntegral>(
            GenIntegral<RangeIntegral, DigitIntegral>());
        break;

    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }

    using DigitRangeIntegral =
        zeta::core::meta::Conditional<is_signed,
                                      signed _BitInt(DigitWidth * DigitCnt),
                                      unsigned _BitInt(DigitWidth * DigitCnt)>;

    DigitRangeIntegral should_o_val{ static_cast<DigitRangeIntegral>(io_val) };
    OIIntegral should_oi_val{ static_cast<OIIntegral>(should_o_val) };

    ZETA_Core_DebugUtils_Diag_LogVar(io_val);
    ZETA_Core_DebugUtils_Diag_LogVar(
        static_cast<zeta::core::integral::MakeUnsignedOf<decltype(io_val)>>(
            io_val));
    ZETA_Core_DebugUtils_Diag_LogVar(should_o_val);
    ZETA_Core_DebugUtils_Diag_LogVar(
        static_cast<
            zeta::core::integral::MakeUnsignedOf<decltype(should_o_val)>>(
            should_o_val));
    ZETA_Core_DebugUtils_Diag_LogVar(should_oi_val);
    ZETA_Core_DebugUtils_Diag_LogVar(
        static_cast<
            zeta::core::integral::MakeUnsignedOf<decltype(should_oi_val)>>(
            should_oi_val));

    bool no_lossy_io;
    bool no_lossy_oi;

    zeta::core::integral_endec::EncodeResult encode_result;
    zeta::core::integral_endec::DecodeResult<OIIntegral> decode_result;

    {
        auto mem_reader{ zeta::core::lin_seq_endpoint::acceptor::Acceptor{
            .data = buffer,
            .elem_size = sizeof(DigitIntegral),
            .elem_stride = sizeof(DigitIntegral),
            .elem_cnt = DigitCnt,
        } };

        /*

        Because
        'meta::IsSame<meta::RemoveCVRef<decltype(AcceptorTraits<meta::RemoveCVRef<Acceptor>>::GetElemSize(acceptor))>,
        size_t>' would be invalid: implicit instantiation of undefined template
        'zeta::core::elem_stream::AcceptorTraits<zeta::core::lin_seq_endpoint::acceptor::Acceptor>'

        */

        encode_result = zeta::core::integral_endec::Encode(
            mem_reader,
            // acceptor

            endianness,
            // endianness_like

            zeta::core::meta::TypeWrapper<DigitIntegral>{},
            // digit_integral

            zeta::core::meta::ValueWrapper<size_t, DigitWidth>{},
            // digit_width

            DigitCnt,
            // digit_cnt_like

            io_val
            // src_value
        );

        no_lossy_io = !encode_result.value_out_of_range;

        ZETA_Core_DebugUtils_Diag_LogVar(
            static_cast<DigitIntegral const*>(mem_reader.data) - buffer);

        ZETA_Core_DebugUtils_Diag_LogVar(DigitCnt);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            static_cast<DigitIntegral*>(mem_reader.data) - buffer ==
            static_cast<long long>(DigitCnt));

        ZETA_Core_DebugUtils_Diag_LogVar(io_val);
        ZETA_Core_DebugUtils_Diag_LogVar(should_o_val);

        if (io_val == should_o_val) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(no_lossy_io);
        } else {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(!no_lossy_io);
        }
    }

    ZETA_Core_DebugUtils_Diag_LogVar(buffer[0]);
    ZETA_Core_DebugUtils_Diag_LogVar(buffer[1]);
    ZETA_Core_DebugUtils_Diag_LogVar(buffer[2]);
    ZETA_Core_DebugUtils_Diag_LogVar(buffer[3]);
    ZETA_Core_DebugUtils_Diag_LogVar(buffer[4]);
    ZETA_Core_DebugUtils_Diag_LogVar(buffer[5]);
    ZETA_Core_DebugUtils_Diag_LogVar(buffer[6]);
    ZETA_Core_DebugUtils_Diag_LogVar(buffer[7]);
    ZETA_Core_DebugUtils_Diag_LogVar(buffer[8]);
    ZETA_Core_DebugUtils_Diag_LogVar(buffer[9]);
    ZETA_Core_DebugUtils_Diag_LogVar(buffer[10]);
    ZETA_Core_DebugUtils_Diag_LogVar(buffer[11]);
    ZETA_Core_DebugUtils_Diag_LogVar(buffer[12]);
    ZETA_Core_DebugUtils_Diag_LogVar(buffer[13]);
    ZETA_Core_DebugUtils_Diag_LogVar(buffer[14]);
    ZETA_Core_DebugUtils_Diag_LogVar(buffer[15]);

    {
        auto mem_writer{ zeta::core::lin_seq_endpoint::provider::Provider{
            .data = buffer,
            .elem_size = sizeof(DigitIntegral),
            .elem_stride = sizeof(DigitIntegral),
            .elem_cnt = DigitCnt,
        } };

        decode_result = zeta::core::integral_endec::Decode(
            mem_writer,
            // provider

            endianness,
            // endianness_like

            zeta::core::meta::TypeWrapper<DigitIntegral>{},
            // digit_integral

            zeta::core::meta::ValueWrapper<size_t, DigitWidth>{},
            // digit_width

            []() {
                if constexpr (VariantDigitCnt) {
                    return zeta::core::integral_endec::VariableOctetCntTag{};
                } else {
                    return DigitCnt;
                }
            }(),
            // digit_cnt_like

            zeta::core::meta::TypeWrapper<OIIntegral>{}  // dst_integral
        );

        no_lossy_oi = !decode_result.value_out_of_range;
        oi_val = decode_result.value;

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            static_cast<DigitIntegral const*>(mem_writer.data) - buffer ==
            static_cast<long long>(DigitCnt));

        ZETA_Core_DebugUtils_Diag_LogVar(oi_val);
        ZETA_Core_DebugUtils_Diag_LogVar(should_oi_val);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(oi_val == should_oi_val);

        if (should_o_val == should_oi_val) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(no_lossy_oi);
        } else {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(!no_lossy_oi);
        }
    }

    zeta::core::debug_utils::diag::diag_logger.Clear();
}

template <bool Signedness, size_t RangeWidth, char Endianness, size_t IOWidth,
          size_t OIWidth, size_t DigitWidth, bool VariantDigitCnt,
          size_t DigitCnt>
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

    test_IOI<RangeIntegral,                                 // RangeIntegral
             Endianness,                                    // Endianness
             IOIntegral,                                    // IOIntegral
             OIIntegral,                                    // OIIntegral
             unsigned _BitInt(((DigitWidth + 7) / 8) * 8),  // DigitIntegral
             DigitWidth,                                    // DigitWidth
             VariantDigitCnt,                               // VariantDigitCnt
             DigitCnt                                       // DigitCnt
             >(special_value);
}

inline void main1() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 1'787'582'349 };

    // unsigned seed{ random_seed };
    unsigned seed{ fixed_seed };

    ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

    ZETA_Core_DebugUtils_Logging_ImmLogVar(random_seed);
    ZETA_Core_DebugUtils_Logging_ImmLogVar(fixed_seed);
    ZETA_Core_DebugUtils_Logging_ImmLogVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    for (size_t i{ 0 }; i < 1024 * 128; ++i) {
        if (i % 1024 == 0) { ZETA_Core_DebugUtils_Logging_ImmLogVar(i / 1024); }

        for (int special_value{ i == 0 ? SPECIAL_VALUE_RANGE_MIN
                                       : SPECIAL_VALUE_RANGE_RANDOM };
             special_value <= SPECIAL_VALUE_RANGE_RANDOM; ++special_value) {
            /*
            FINISHED


            F<SIGNED, 66, LE, 101, 138, 2, FWC, 2>(special_value);
            F<UNSIGNED, 16, LE, 101, 62, 7, VWC, 18>(special_value);

             F<SIGNED, 33, BE, 60, 88, 2, VWC, 5>(special_value);
            F<SIGNED, 61, BE, 153, 33, 4, FWC, 8>(special_value);

            F<SIGNED, 39, BE, 142, 65, 8, VWC, 15>(special_value);

            F<UNSIGNED, 90, LE, 168, 92, 4, VWC, 14>(special_value);

            F<UNSIGNED, 92, LE, 104, 34, 12, VWC, 20>(special_value);
            F<SIGNED, 115, LE, 161, 157, 12, VWC, 15>(special_value);
            F<UNSIGNED, 51, LE, 167, 21, 5, FWC, 3>(special_value);

            F<UNSIGNED, 46, BE, 104, 45, 5, VWC, 18>(special_value);
            F<SIGNED, 38, BE, 77, 58, 9, FWC, 20>(special_value);
            F<UNSIGNED, 30, LE, 88, 165, 11, VWC, 20>(special_value);
            F<UNSIGNED, 21, BE, 130, 151, 12, VWC, 3>(special_value);
            F<SIGNED, 97, BE, 107, 37, 8, FWC, 20>(special_value);
            F<SIGNED, 7, BE, 166, 48, 12, VWC, 19>(special_value);
            F<UNSIGNED, 119, BE, 129, 116, 8, VWC, 3>(special_value);
            F<UNSIGNED, 13, BE, 98, 107, 8, FWC, 14>(special_value);
            F<UNSIGNED, 27, BE, 33, 129, 4, FWC, 19>(special_value);
            F<SIGNED, 48, BE, 134, 131, 7, FWC, 13>(special_value);

            */

            F<SIGNED, 119, BE, 146, 27, 5, VWC, 9>(special_value);
            F<UNSIGNED, 87, BE, 155, 116, 9, FWC, 5>(special_value);
            F<SIGNED, 6, BE, 167, 79, 7, FWC, 12>(special_value);
            F<UNSIGNED, 87, BE, 127, 149, 6, VWC, 13>(special_value);
            F<SIGNED, 82, LE, 166, 72, 11, VWC, 6>(special_value);
            F<UNSIGNED, 35, BE, 149, 118, 8, VWC, 8>(special_value);
            F<SIGNED, 10, BE, 143, 44, 4, VWC, 14>(special_value);
            F<UNSIGNED, 83, BE, 98, 56, 7, FWC, 16>(special_value);
            F<SIGNED, 95, LE, 144, 134, 12, VWC, 18>(special_value);
            F<UNSIGNED, 99, LE, 164, 162, 12, VWC, 20>(special_value);

            F<UNSIGNED, 54, BE, 144, 78, 8, FWC, 20>(special_value);
            F<SIGNED, 34, BE, 102, 32, 7, FWC, 10>(special_value);
            F<SIGNED, 104, LE, 124, 56, 9, FWC, 20>(special_value);
            F<UNSIGNED, 45, BE, 46, 96, 2, FWC, 3>(special_value);
            F<SIGNED, 71, BE, 80, 151, 4, VWC, 9>(special_value);
            F<UNSIGNED, 51, BE, 157, 45, 7, VWC, 15>(special_value);
            F<SIGNED, 25, BE, 53, 79, 8, FWC, 15>(special_value);
            F<UNSIGNED, 98, BE, 119, 12, 2, FWC, 8>(special_value);
            F<UNSIGNED, 61, LE, 163, 97, 6, VWC, 11>(special_value);
            F<SIGNED, 9, BE, 148, 104, 4, VWC, 5>(special_value);

            /*
            WAITING



            */

            zeta::core::debug_utils::diag::diag_logger.Clear();
        }
    }
}

int main() {
    main1();
    ZETA_Core_DebugUtils_Logging_ImmLogVar("ok");

    return 0;
}
