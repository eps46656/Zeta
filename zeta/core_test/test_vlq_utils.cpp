#include <zeta/core/integral_math.ipp>
#include <zeta/core/lin_seq_elem_stream.ipp>
#include <zeta/core/serde_utils.ipp>
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

    zeta::core::serde_utils::EndiannessEnum::Value endianness_value;

    switch (Endianness) {
    case LE:
        endianness_value =
            zeta::core::serde_utils::EndiannessEnum::Little::value;
        break;
    case BE:
        endianness_value = zeta::core::serde_utils::EndiannessEnum::Big::value;
        break;
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

    bool no_lossy_io;
    bool no_lossy_oi;

    {
        auto mem_reader{ zeta::core::lin_seq_elem_stream::Acceptor{
            .data = buffer,
            .elem_size = sizeof(DigitIntegral),
            .elem_stride = sizeof(DigitIntegral),
            .elem_cnt = buffer_cnt,
        } };

        /*

        Because
        'meta::IsSame<meta::RemoveCVRef<decltype(AcceptorTraits<meta::RemoveCVRef<Acceptor>>::GetElemSize(acceptor))>,
        size_t>' would be invalid: implicit instantiation of undefined template
        'zeta::core::elem_stream::AcceptorTraits<zeta::core::lin_seq_elem_stream::Acceptor>'

        */

        no_lossy_io = zeta::core::vlq_utils::SerializeIntegral(
            io_val,
            // src_value

            zeta::core::value_wrapper::DynamicValueWrapper<
                zeta::core::serde_utils::EndiannessEnum::Value>{
                endianness_value },
            // endianness_like

            zeta::core::meta::TypeWrapper<DigitIntegral>{},
            // digit_integral

            zeta::core::value_wrapper::StaticValueWrapper<size_t, DigitWidth>{},
            // digit_width

            false,
            // allow_lossy

            mem_reader,
            // acceptor

            nullptr
            // dst_error
        );

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

        no_lossy_oi = zeta::core::vlq_utils::DeserializeIntegral(
            oi_val,
            // dst_value

            zeta::core::value_wrapper::DynamicValueWrapper<
                zeta::core::serde_utils::EndiannessEnum::Value>{
                endianness_value },
            // endianness_like

            zeta::core::meta::TypeWrapper<DigitIntegral>{},
            // digit_integral

            zeta::core::value_wrapper::StaticValueWrapper<size_t, DigitWidth>{},
            // digit_width

            false,
            // allow_lossy

            mem_writer,
            // provider

            nullptr
            // dst_error
        );

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

            F<SIGNED, 39, BE, 142, 65, 8, VWC, 15>(special_value);

            */

            F<SIGNED, 66, LE, 101, 138, 8>(special_value);
            F<UNSIGNED, 16, LE, 101, 62, 7>(special_value);
            // F<SIGNED, 33, BE, 60, 88, 2, VWC, 5>(special_value);
            // F<SIGNED, 61, BE, 153, 33, 4, FWC, 8>(special_value);

            /*
            WAITING


            F<UNSIGNED, 90, LE, 168, 92, 4, VWC, 14>(special_value);

            F<SIGNED, 1, LE, 28, 54, 10, VWC, 3>(special_value);
            F<UNSIGNED, 92, LE, 104, 34, 12, VWC, 20>(special_value);
            F<SIGNED, 115, LE, 161, 157, 12, VWC, 15>(special_value);
            F<UNSIGNED, 51, LE, 167, 21, 5, FWC, 3>(special_value);

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
