#pragma once

#include <zeta/core/elem_stream.hpp>
#include <zeta/core/unicode.hpp>

namespace zeta::core::json_utils {

unicode::unichar_t constexpr null_cp{ static_cast<unicode::unichar_t>(-1) };

constexpr bool IsInvisible(unicode::unichar_t cp);

constexpr bool IsTokenEnd(unicode::unichar_t cp);

template <elem_stream::provider::IsProvider CodepointProvider>
struct BufferedCodepointProvider {
    enum BufferStateEnum : unsigned char {
        Empty = 0,
        HasCodepointFetched = 1,
        HasCodepointUnfetched = 2,
    };

    BufferStateEnum buffer_state;
    unicode::unichar_t buffer_codepoint;

    CodepointProvider& cpp;

    constexpr BufferedCodepointProvider(CodepointProvider& cpp);

    constexpr bool IsEnd(this BufferedCodepointProvider& self);

    constexpr unicode::unichar_t Fetch(this BufferedCodepointProvider& self);

    constexpr unicode::unichar_t FetchSkipUnvisible(
        this BufferedCodepointProvider& self);

    constexpr void Revert(this BufferedCodepointProvider& self);
};

struct BaseSerializerStateCodeTable {
    using Type = unsigned char;

    static constexpr Type ReceivingValue{ 10 };

    static constexpr Type ReceivingArrayElemOrFinish{ 20 };

    static constexpr Type ReceivingObjectKeyOrFinish{ 10 };
    static constexpr Type ReceivingObjectValue{ 10 };

    static constexpr Type Finished{ 127 };
};

struct BaseDeserializerStateCodeTable {
    using Type = unsigned char;

    static constexpr Type SendingNull{ 10 };

    static constexpr Type SendingBoolean{ 20 };

    static constexpr Type SendingNumericStart{ 30 };
    static constexpr Type SendingNumericSign{ 31 };
    static constexpr Type SendingNumericIntPartDigitLead{ 32 };
    static constexpr Type SendingNumericIntPartDigitTail{ 33 };
    static constexpr Type SendingNumericFracPartDigit{ 34 };
    static constexpr Type SendingNumericExpPartE{ 35 };
    static constexpr Type SendingNumericExpPartSign{ 36 };
    static constexpr Type SendingNumericExpPartDigit{ 37 };
    static constexpr Type SendingNumericFinish{ 38 };

    static constexpr Type SendingStringStart{ 40 };
    static constexpr Type SendingStringChar{ 41 };
    static constexpr Type SendingStringFinish{ 42 };

    static constexpr Type SendingArrayStart{ 50 };
    static constexpr Type SendingArrayElem{ 51 };
    static constexpr Type SendingArrayFinish{ 52 };

    static constexpr Type SendingObjectStart{ 60 };
    static constexpr Type SendingObjectKey{ 61 };
    static constexpr Type SendingObjectValue{ 62 };
    static constexpr Type SendingObjectFinish{ 63 };

    static constexpr Type Finished{ 127 };

    static constexpr Type Corrupted{ 255 };
};

}  // namespace zeta::core::json_utils
