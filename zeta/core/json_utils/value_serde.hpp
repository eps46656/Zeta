#pragma once

#include <zeta/core/json_utils/utils.hpp>
#include <zeta/core/pair.hpp>

namespace zeta::core::json_utils::value_serde {

enum struct DeserializerStateEnum : BaseDeserializerStateCodeTable::Type {
    SendingNull = BaseDeserializerStateCodeTable::SendingNull,

    SendingBoolean = BaseDeserializerStateCodeTable::SendingBoolean,

    SendingNumericStart = BaseDeserializerStateCodeTable::SendingNumericStart,

    SendingNumericSign = BaseDeserializerStateCodeTable::SendingNumericSign,

    SendingNumericIntPartDigitLead =
        BaseDeserializerStateCodeTable::SendingNumericIntPartDigitLead,

    SendingNumericIntPartDigitTail =
        BaseDeserializerStateCodeTable::SendingNumericIntPartDigitTail,

    SendingNumericFracPartDigit =
        BaseDeserializerStateCodeTable::SendingNumericFracPartDigit,

    SendingNumericExpPartE =
        BaseDeserializerStateCodeTable::SendingNumericExpPartE,

    SendingNumericExpPartSign =
        BaseDeserializerStateCodeTable::SendingNumericExpPartSign,

    SendingNumericExpPartDigit =
        BaseDeserializerStateCodeTable::SendingNumericExpPartDigit,

    SendingNumericFinish = BaseDeserializerStateCodeTable::SendingNumericFinish,

    SendingStringStart = BaseDeserializerStateCodeTable::SendingStringStart,

    SendingStringChar = BaseDeserializerStateCodeTable::SendingStringChar,

    SendingStringFinish = BaseDeserializerStateCodeTable::SendingStringFinish,

    SendingArrayStart = BaseDeserializerStateCodeTable::SendingArrayStart,

    SendingArrayElem = BaseDeserializerStateCodeTable::SendingArrayElem,

    SendingArrayFinish = BaseDeserializerStateCodeTable::SendingArrayFinish,

    SendingObjectStart = BaseDeserializerStateCodeTable::SendingObjectStart,

    SendingObjectKey = BaseDeserializerStateCodeTable::SendingObjectKey,

    SendingObjectValue = BaseDeserializerStateCodeTable::SendingObjectValue,

    SendingObjectFinish = BaseDeserializerStateCodeTable::SendingObjectFinish,

    Finished = BaseDeserializerStateCodeTable::Finished,

    Corrupted = BaseDeserializerStateCodeTable::Corrupted,
};

template <elem_stream::provider::IsProvider CodepointProvider>
struct Deserializer {
    static constexpr size_t max_depth{ 64 };

    DeserializerStateEnum states[max_depth];

    size_t depth;

    BufferedCodepointProvider<CodepointProvider>& bcpp;

    constexpr Deserializer(BufferedCodepointProvider<CodepointProvider>& bcpp);

    constexpr void ReceiveNull(this Deserializer& self);

    constexpr bool ReceiveBoolean(this Deserializer& self);

    constexpr void ReceiveNumericStart(this Deserializer& self);

    constexpr bool ReceiveNumericSign(this Deserializer& self);

    constexpr unsigned ReceiveNumericIntPartDigit(this Deserializer& self);

    constexpr unsigned ReceiveNumericFracPartDigit(this Deserializer& self);

    constexpr unsigned char ReceiveNumericExpPartE(this Deserializer& self);

    constexpr pair::Pair<unsigned char, bool> ReceiveNumericExpPartSign(
        this Deserializer& self);

    constexpr unsigned ReceiveNumericExpPartDigit(this Deserializer& self);

    constexpr void ReceiveNumericFinish(this Deserializer& self);

    constexpr void ReceiveStringStart(this Deserializer& self);

    constexpr unicode::unichar_t ReceiveStringChar(this Deserializer& self);

    constexpr void ReceiveStringFinish(this Deserializer& self);

    constexpr void ReceiveArrayStart(this Deserializer& self);

    constexpr void ReceiveArrayElem(this Deserializer& self);

    constexpr void ReceiveArrayFinish(this Deserializer& self);

    constexpr void ReceiveObjectStart(this Deserializer& self);

    constexpr void ReceiveObjectKey(this Deserializer& self);

    constexpr void ReceiveObjectValue(this Deserializer& self);

    constexpr void ReceiveObjectFinish(this Deserializer& self);
};

/*

L = L_null | L_boolean | L_numeric | L_string | L_array | L_object

L_null = ReceiveNull

L_boolean = ReceiveBoolean

L_numeric =
    ReceiveNumericStart
    ReceiveNumericSign
    ReceiveNumericIntPartDigit+
    ReceiveNumericFracPartDigit*
    (
        ReceiveNumericExpPartE
        ReceiveNumericExpPartSign?
        ReceiveNumericExpPartDigit+
    )?
    ReceiveNumericFinish

L_string = ReceiveStringStart ReceiveStringChar* ReceiveStringFinish

L_array = ReceiveArrayStart (ReceiveArrayElem L)* ReceiveArrayFinish

L_object =
    ReceiveObjectStart
    (ReceiveObjectKey L_string ReceiveObjectValue L)*
    ReceiveObjectFinish

*/

}  // namespace zeta::core::json_utils::value_serde
