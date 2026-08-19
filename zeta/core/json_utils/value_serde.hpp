#pragma once

#include <zeta/core/json_utils/format_utils.hpp>
#include <zeta/core/json_utils/utils.hpp>
#include <zeta/core/pair.hpp>

namespace zeta::core::json_utils::value_serde {

enum struct SerializerStateEnum : BaseSerializeStateCodeTable::Type {
    ReceivingValue = BaseSerializeStateCodeTable::ReceivingValue,

    ReceivingNumericSign = BaseSerializeStateCodeTable::ReceivingNumericSign,

    ReceivingNumericIntPartDigit =
        BaseSerializeStateCodeTable::ReceivingNumericIntPartDigit,

    ReceivingNumericIntPartDigitOrNext =
        BaseSerializeStateCodeTable::ReceivingNumericIntPartDigitOrNext,

    ReceivingNumericFracPartDigitOrNext =
        BaseSerializeStateCodeTable::ReceivingNumericFracPartDigitOrNext,

    ReceivingNumericExpPartSignOrNext =
        BaseSerializeStateCodeTable::ReceivingNumericExpPartSignOrNext,

    ReceivingNumericExpPartDigit =
        BaseSerializeStateCodeTable::ReceivingNumericExpPartDigit,

    ReceivingNumericExpPartDigitOrNext =
        BaseSerializeStateCodeTable::ReceivingNumericExpPartDigitOrNext,

    ReceivingStringCharOrFinish =
        BaseSerializeStateCodeTable::ReceivingStringCharOrFinish,

    ReceivingArrayElemOrFinishLead =
        BaseSerializeStateCodeTable::ReceivingArrayElemOrFinishLead,

    ReceivingArrayElemOrFinishTail =
        BaseSerializeStateCodeTable::ReceivingArrayElemOrFinishTail,

    ReceivingObjectKeyOrFinishLead =
        BaseSerializeStateCodeTable::ReceivingObjectKeyOrFinishLead,

    ReceivingObjectKeyOrFinishTail =
        BaseSerializeStateCodeTable::ReceivingObjectKeyOrFinishTail,

    ReceivingObjectValue = BaseSerializeStateCodeTable::ReceivingObjectValue,

    Finished = BaseSerializeStateCodeTable::Finished,
};

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
struct Serializer {
    static constexpr size_t max_depth{ 64 };

    SerializerStateEnum states[max_depth];

    size_t depth;

    CodepointAcceptor& cpa;

    format_utils::FormatContext fmt_ctx;

    constexpr Serializer(CodepointAcceptor& cpa,
                         format_utils::FormatConfig const& fmt_config);

    constexpr void SendNull(this Serializer& self);

    constexpr void SendBoolean(this Serializer& self, bool value);

    constexpr void SendNumericStart(this Serializer& self);

    constexpr void SendNumericSign(this Serializer& self, bool is_neg);

    constexpr bool SendNumericIntPartDigit(this Serializer& self,
                                           unsigned digit);

    constexpr bool SendNumericFracPartDigit(this Serializer& self,
                                            unsigned digit);

    constexpr bool SendNumericExpPartE(this Serializer& self, unsigned char e);

    constexpr bool SendNumericExpPartSign(this Serializer& self,
                                          unsigned char sign);

    constexpr bool SendNumericExpPartDigit(this Serializer& self,
                                           unsigned digit);

    constexpr void SendNumericFinish(this Serializer& self);

    constexpr void SendStringStart(this Serializer& self);

    constexpr bool SendStringChar(this Serializer& self, unicode::unichar_t cp);

    constexpr void SendStringFinish(this Serializer& self);

    constexpr void SendArrayStart(this Serializer& self);

    constexpr void SendArrayFinish(this Serializer& self);

    constexpr void SendObjectStart(this Serializer& self);

    constexpr void SendObjectFinish(this Serializer& self);
};

enum struct DeserializerStateEnum : BaseDeserializeStateCodeTable::Type {
    SendingNull = BaseDeserializeStateCodeTable::SendingNull,

    SendingBoolean = BaseDeserializeStateCodeTable::SendingBoolean,

    SendingNumericStart = BaseDeserializeStateCodeTable::SendingNumericStart,

    SendingNumericSign = BaseDeserializeStateCodeTable::SendingNumericSign,

    SendingNumericIntPartDigitLead =
        BaseDeserializeStateCodeTable::SendingNumericIntPartDigitLead,

    SendingNumericIntPartDigitTail =
        BaseDeserializeStateCodeTable::SendingNumericIntPartDigitTail,

    SendingNumericFracPartDigit =
        BaseDeserializeStateCodeTable::SendingNumericFracPartDigit,

    SendingNumericExpPartE =
        BaseDeserializeStateCodeTable::SendingNumericExpPartE,

    SendingNumericExpPartSign =
        BaseDeserializeStateCodeTable::SendingNumericExpPartSign,

    SendingNumericExpPartDigit =
        BaseDeserializeStateCodeTable::SendingNumericExpPartDigit,

    SendingNumericFinish = BaseDeserializeStateCodeTable::SendingNumericFinish,

    SendingStringStart = BaseDeserializeStateCodeTable::SendingStringStart,

    SendingStringChar = BaseDeserializeStateCodeTable::SendingStringChar,

    SendingStringFinish = BaseDeserializeStateCodeTable::SendingStringFinish,

    SendingArrayStart = BaseDeserializeStateCodeTable::SendingArrayStart,

    SendingArrayElem = BaseDeserializeStateCodeTable::SendingArrayElem,

    SendingArrayFinish = BaseDeserializeStateCodeTable::SendingArrayFinish,

    SendingObjectStart = BaseDeserializeStateCodeTable::SendingObjectStart,

    SendingObjectKey = BaseDeserializeStateCodeTable::SendingObjectKey,

    SendingObjectValue = BaseDeserializeStateCodeTable::SendingObjectValue,

    SendingObjectFinish = BaseDeserializeStateCodeTable::SendingObjectFinish,

    Finished = BaseDeserializeStateCodeTable::Finished,

    Corrupted = BaseDeserializeStateCodeTable::Corrupted,
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
