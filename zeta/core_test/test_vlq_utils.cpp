#include <zeta/core/integral_endec.ipp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/lin_seq_elem_stream.ipp>
#include <zeta/core/vlq_utils.ipp>
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
            zeta::core_test::GetRandomInt<DigitIntegral, DigitIntegral>(
                0, static_cast<DigitIntegral>(1) << (width % digit_width) - 1)
        };

        ret = static_cast<OpUnsignedIntegral>(c);
    }

    for (size_t i{ 0 }; i < width / digit_width; ++i) {
        DigitIntegral c{
            zeta::core_test::GetRandomInt<DigitIntegral, DigitIntegral>(
                0, zeta::core::integral::RangeMaxOf<DigitIntegral>)
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

template <typename RangeIntegral, char Endianness, typename IOIntegral,
          typename OIIntegral, typename DigitIntegral, size_t DigitWidth>
inline void test_IOI(int special_value) {
    ZETA_Core_Debug_PrintVar(
        zeta::core::debug_utils::GetTypeStr<RangeIntegral>());
    ZETA_Core_Debug_PrintVar(zeta::core::debug_utils::GetTypeStr<IOIntegral>());
    ZETA_Core_Debug_PrintVar(zeta::core::debug_utils::GetTypeStr<OIIntegral>());

    constexpr size_t buffer_cnt{ 256 };
    DigitIntegral buffer[buffer_cnt];

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

    zeta::core::integral_endec::Endianness endianness;

    switch (Endianness) {
    case LE: endianness = zeta::core::integral_endec::Endianness::Little; break;
    case BE: endianness = zeta::core::integral_endec::Endianness::Big; break;
    default: ZETA_Core_Unreachable();
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

    default: ZETA_Core_Unreachable();
    }

    OIIntegral should_oi_val{ static_cast<OIIntegral>(io_val) };

    ZETA_Core_Debug_PrintVar(io_val);
    ZETA_Core_Debug_PrintVar(
        static_cast<zeta::core::integral::MakeUnsignedOf<decltype(io_val)>>(
            io_val));
    ZETA_Core_Debug_PrintVar(should_oi_val);
    ZETA_Core_Debug_PrintVar(
        static_cast<
            zeta::core::integral::MakeUnsignedOf<decltype(should_oi_val)>>(
            should_oi_val));

    zeta::core::vlq_utils::DecodeResult<OIIntegral> decode_result;

    bool no_lossy_io;
    bool no_lossy_oi;

    {
        auto mem_reader{ zeta::core::lin_seq_elem_stream::Acceptor{
            .data = buffer,
            .elem_size = sizeof(DigitIntegral),
            .elem_stride = sizeof(DigitIntegral),
            .elem_cnt = buffer_cnt,
        } };

        zeta::core::vlq_utils::Encode(
            mem_reader,
            // acceptor

            endianness,
            // endianness_like

            zeta::core::meta::TypeWrapper<DigitIntegral>{},
            // digit_integral

            zeta::core::meta::ValueWrapper<size_t, DigitWidth>{},
            // digit_width

            io_val
            // src_value
        );

        no_lossy_io = true;

        ZETA_Core_Debug_PrintVar(
            static_cast<DigitIntegral const*>(mem_reader.data) - buffer);

        ZETA_Core_DebugAssert(no_lossy_io);
    }

    ZETA_Core_Debug_PrintVar(buffer[0]);
    ZETA_Core_Debug_PrintVar(buffer[1]);
    ZETA_Core_Debug_PrintVar(buffer[2]);
    ZETA_Core_Debug_PrintVar(buffer[3]);
    ZETA_Core_Debug_PrintVar(buffer[4]);
    ZETA_Core_Debug_PrintVar(buffer[5]);
    ZETA_Core_Debug_PrintVar(buffer[6]);
    ZETA_Core_Debug_PrintVar(buffer[7]);
    ZETA_Core_Debug_PrintVar(buffer[8]);
    ZETA_Core_Debug_PrintVar(buffer[9]);
    ZETA_Core_Debug_PrintVar(buffer[10]);
    ZETA_Core_Debug_PrintVar(buffer[11]);
    ZETA_Core_Debug_PrintVar(buffer[12]);
    ZETA_Core_Debug_PrintVar(buffer[13]);
    ZETA_Core_Debug_PrintVar(buffer[14]);
    ZETA_Core_Debug_PrintVar(buffer[15]);

    {
        auto mem_writer{ zeta::core::lin_seq_elem_stream::Provider{
            .data = buffer,
            .elem_size = sizeof(DigitIntegral),
            .elem_stride = sizeof(DigitIntegral),
            .elem_cnt = buffer_cnt,
        } };

        decode_result = zeta::core::vlq_utils::Decode(
            mem_writer,
            // provider

            endianness,
            // endianness_like

            zeta::core::meta::TypeWrapper<DigitIntegral>{},
            // digit_integral

            zeta::core::meta::ValueWrapper<size_t, DigitWidth>{},
            // digit_width

            zeta::core::meta::TypeWrapper<OIIntegral>{});

        no_lossy_oi = !decode_result.value_out_of_range;
        oi_val = decode_result.value;

        ZETA_Core_Debug_PrintVar(oi_val);
        ZETA_Core_Debug_PrintVar(should_oi_val);

        ZETA_Core_DebugAssert(ZETA_Core_Debug_PrintVar(oi_val) ==
                              ZETA_Core_Debug_PrintVar(should_oi_val));

        if (io_val == oi_val) {
            ZETA_Core_DebugAssert(no_lossy_oi);
        } else {
            ZETA_Core_DebugAssert(!no_lossy_oi);
        }
    }

    zeta::core::debug_utils::ClearDebugStrStream();
}

template <bool Signedness, size_t RangeWidth, char Endianness, size_t IOWidth,
          size_t OIWidth, size_t DigitWidth>
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
             DigitWidth                                     // DigitWidth
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

        for (int special_value{ i == 0 ? SPECIAL_VALUE_RANGE_MIN
                                       : SPECIAL_VALUE_RANGE_RANDOM };
             special_value <= SPECIAL_VALUE_RANGE_RANDOM; ++special_value) {
            /*
            FINISHED

            F<SIGNED, 66, LE, 101, 138, 8>(special_value);
            F<UNSIGNED, 16, LE, 101, 62, 7>(special_value);


            F<SIGNED, 33, BE, 60, 88, 5>(special_value);
            F<SIGNED, 61, BE, 153, 33, 8>(special_value);

            F<SIGNED, 7, LE, 16, 32, 2>(special_value);
            F<UNSIGNED, 8, BE, 24, 17, 3>(special_value);
            F<SIGNED, 15, BE, 64, 40, 4>(special_value);
            F<UNSIGNED, 16, LE, 19, 91, 5>(special_value);
            F<SIGNED, 23, LE, 88, 60, 6>(special_value);
            F<UNSIGNED, 24, BE, 127, 55, 7>(special_value);
            F<SIGNED, 31, BE, 153, 33, 8>(special_value);
            F<UNSIGNED, 32, LE, 72, 145, 2>(special_value);
            F<SIGNED, 33, BE, 60, 88, 5>(special_value);
            F<UNSIGNED, 34, LE, 101, 23, 6>(special_value);
            F<SIGNED, 39, LE, 45, 166, 7>(special_value);
            F<UNSIGNED, 40, BE, 180, 52, 8>(special_value);
            F<SIGNED, 47, BE, 99, 127, 3>(special_value);
            F<SIGNED, 55, LE, 192, 41, 5>(special_value);
            F<UNSIGNED, 56, BE, 137, 78, 6>(special_value);
            F<SIGNED, 61, BE, 153, 33, 8>(special_value);
            F<UNSIGNED, 62, LE, 255, 64, 7>(special_value);
            F<SIGNED, 63, LE, 128, 191, 4>(special_value);
            F<UNSIGNED, 64, BE, 256, 256, 8>(special_value);

            */

            F<SIGNED, 3, LE, 8, 11, 2>(special_value);
            F<UNSIGNED, 4, BE, 13, 7, 3>(special_value);
            F<SIGNED, 5, BE, 19, 24, 4>(special_value);
            F<UNSIGNED, 6, LE, 27, 15, 5>(special_value);
            F<SIGNED, 7, LE, 31, 42, 6>(special_value);
            F<UNSIGNED, 8, BE, 47, 29, 7>(special_value);
            F<SIGNED, 9, BE, 56, 63, 8>(special_value);
            F<UNSIGNED, 10, LE, 73, 18, 2>(special_value);
            F<SIGNED, 11, LE, 81, 96, 3>(special_value);
            F<UNSIGNED, 12, BE, 94, 37, 4>(special_value);
            F<SIGNED, 13, BE, 101, 108, 5>(special_value);
            F<UNSIGNED, 14, LE, 117, 51, 6>(special_value);
            F<SIGNED, 15, LE, 126, 133, 7>(special_value);

            F<UNSIGNED, 16, BE, 144, 66, 8>(special_value);
            F<SIGNED, 17, BE, 155, 149, 2>(special_value);
            F<UNSIGNED, 18, LE, 169, 79, 3>(special_value);
            F<SIGNED, 19, LE, 177, 188, 4>(special_value);
            F<UNSIGNED, 20, BE, 191, 92, 5>(special_value);
            F<SIGNED, 21, BE, 208, 201, 6>(special_value);
            F<UNSIGNED, 22, LE, 223, 104, 7>(special_value);
            F<SIGNED, 23, LE, 239, 217, 8>(special_value);
            F<UNSIGNED, 24, BE, 251, 118, 2>(special_value);
            F<SIGNED, 25, BE, 264, 233, 3>(special_value);

            F<UNSIGNED, 26, LE, 278, 127, 4>(special_value);
            F<SIGNED, 27, LE, 291, 245, 5>(special_value);
            F<UNSIGNED, 28, BE, 307, 136, 6>(special_value);
            F<SIGNED, 29, BE, 319, 261, 7>(special_value);
            F<UNSIGNED, 30, LE, 333, 148, 8>(special_value);
            F<SIGNED, 31, LE, 347, 277, 2>(special_value);
            F<UNSIGNED, 32, BE, 361, 159, 3>(special_value);
            F<SIGNED, 33, BE, 377, 289, 4>(special_value);
            F<UNSIGNED, 34, LE, 389, 171, 5>(special_value);
            F<SIGNED, 35, LE, 403, 305, 6>(special_value);
            F<UNSIGNED, 36, BE, 419, 183, 7>(special_value);
            F<SIGNED, 37, BE, 431, 317, 8>(special_value);

            /*
            WAITING

            F<UNSIGNED, 38, LE, 447, 194, 2>(special_value);
            F<SIGNED,   39, LE, 461, 333, 3>(special_value);
            F<UNSIGNED, 40, BE, 479, 205, 4>(special_value);
            F<SIGNED,   41, BE, 491, 349, 5>(special_value);
            F<UNSIGNED, 42, LE, 503, 217, 6>(special_value);
            F<SIGNED,   43, LE, 521, 365, 7>(special_value);
            F<UNSIGNED, 44, BE, 533, 228, 8>(special_value);
            F<SIGNED,   45, BE, 547, 381, 2>(special_value);
            F<UNSIGNED, 46, LE, 563, 239, 3>(special_value);
            F<SIGNED,   47, LE, 577, 397, 4>(special_value);
            F<UNSIGNED, 48, BE, 591, 251, 5>(special_value);
            F<SIGNED,   49, BE, 607, 413, 6>(special_value);
            F<UNSIGNED, 50, LE, 619, 262, 7>(special_value);
            F<SIGNED,   51, LE, 633, 429, 8>(special_value);
            F<UNSIGNED, 52, BE, 647, 274, 2>(special_value);
            F<SIGNED,   53, BE, 661, 445, 3>(special_value);
            F<UNSIGNED, 54, LE, 677, 285, 4>(special_value);
            F<SIGNED,   55, LE, 691, 461, 5>(special_value);
            F<UNSIGNED, 56, BE, 707, 297, 6>(special_value);
            F<SIGNED,   57, BE, 719, 477, 7>(special_value);
            F<UNSIGNED, 58, LE, 733, 308, 8>(special_value);
            F<SIGNED,   59, LE, 749, 493, 2>(special_value);
            F<UNSIGNED, 60, BE, 761, 320, 3>(special_value);
            F<SIGNED,   61, BE, 777, 509, 4>(special_value);
            F<UNSIGNED, 62, LE, 791, 331, 5>(special_value);
            F<SIGNED,   63, LE, 803, 525, 6>(special_value);
            F<UNSIGNED, 64, BE, 819, 343, 7>(special_value);



            */

            zeta::core::debug_utils::ClearDebugStrStream();
        }
    }
}

int main() {
    main1();
    ZETA_Core_PrintVar("ok");

    return 0;
}
