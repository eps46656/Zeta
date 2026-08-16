#pragma once

#include <zeta/core/json_utils/utils.hpp>
#include <zeta/core/pair.hpp>

namespace zeta::core::json_utils::numeric_serde {

struct Deserializer {
    enum StateEnum : BaseDeserializerStateCodeTable::Type {
        SendingSign = BaseDeserializerStateCodeTable::SendingNumericSign,

        SendingIntPartDigitLead =
            BaseDeserializerStateCodeTable::SendingNumericIntPartDigitLead,

        SendingIntPartDigitTail =
            BaseDeserializerStateCodeTable::SendingNumericIntPartDigitTail,

        SendingFracPartDigit =
            BaseDeserializerStateCodeTable::SendingNumericFracPartDigit,

        SendingExpPartE =
            BaseDeserializerStateCodeTable::SendingNumericExpPartE,

        SendingExpPartSign =
            BaseDeserializerStateCodeTable::SendingNumericExpPartSign,

        SendingExpPartDigit =
            BaseDeserializerStateCodeTable::SendingNumericExpPartDigit,

        Finished = BaseDeserializerStateCodeTable::SendingNumericFinish,

        Corrupted = BaseDeserializerStateCodeTable::Corrupted,
    };

    StateEnum state;

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr void Init(this Deserializer& self,
                        BufferedCodepointProvider<CodepointProvider>& bcpp);

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr bool ReceiveSign(
        this Deserializer& self,
        BufferedCodepointProvider<CodepointProvider>& bcpp);

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr unsigned ReceiveIntPartDigit(
        this Deserializer& self,
        BufferedCodepointProvider<CodepointProvider>& bcpp);

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr unsigned ReceiveFracPartDigit(
        this Deserializer& self,
        BufferedCodepointProvider<CodepointProvider>& bcpp);

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr unsigned char ReceiveExpPartE(
        this Deserializer& self,
        BufferedCodepointProvider<CodepointProvider>& bcpp);

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr pair::Pair<unsigned char, bool> ReceiveExpPartSign(
        this Deserializer& self,
        BufferedCodepointProvider<CodepointProvider>& bcpp);

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr unsigned ReceiveExpPartDigit(
        this Deserializer& self,
        BufferedCodepointProvider<CodepointProvider>& bcpp);
};

}  // namespace zeta::core::json_utils::numeric_serde
