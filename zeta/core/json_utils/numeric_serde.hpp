#pragma once

#include <zeta/core/json_utils/utils.hpp>
#include <zeta/core/pair.hpp>

namespace zeta::core::json_utils::numeric_serde {

namespace serialize {

enum struct StateEnum : unsigned char {
    ReceivingSign = BaseSerializeStateCodeTable::ReceivingNumericSign,
    // SendSign -> ReceivingNumericIntPartDigit

    ReceivingIntPartDigit =
        BaseSerializeStateCodeTable::ReceivingNumericIntPartDigit,
    // SendIntPartDigit
    //   if 0: -> ReceivingNumericFracPartDigitOrNext
    //   else: -> ReceivingNumericIntPartDigitOrNext

    ReceivingIntPartDigitOrNext =
        BaseSerializeStateCodeTable::ReceivingNumericIntPartDigitOrNext,
    // SendIntPartDigit -> ReceivingNumericIntPartDigitOrNext
    // SendFracPartDigit -> ReceivingNumericFracPartDigitOrNext
    // SendExpPartE -> ReceivingNumericExpPartSignOrNext
    // SendFinish -> Finished

    ReceivingFracPartDigitOrNext =
        BaseSerializeStateCodeTable::ReceivingNumericFracPartDigitOrNext,
    // SendFracPartDigit -> ReceivingNumericFracPartDigitOrNext
    // SendExpPartE -> ReceivingNumericExpPartSignOrNext
    // SendFinish -> Finished

    ReceivingExpPartSignOrNext =
        BaseSerializeStateCodeTable::ReceivingNumericExpPartSignOrNext,
    // SendExpPartSign -> ReceivingNumericExpPartDigit
    // SendExpPartDigit -> ReceivingNumericExpPartDigitOrNext

    ReceivingExpPartDigit =
        BaseSerializeStateCodeTable::ReceivingNumericExpPartDigit,
    // SendExpPartDigit -> ReceivingNumericExpPartDigitOrNext

    ReceivingExpPartDigitOrNext =
        BaseSerializeStateCodeTable::ReceivingNumericExpPartDigitOrNext,
    // SendExpPartDigit -> ReceivingNumericExpPartDigitOrNext
    // SendFinish -> Finished

    Finished = BaseSerializeStateCodeTable::Finished,
};

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendStart(CodepointAcceptor&& cpa, StateEnum& state);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendSign(CodepointAcceptor&& cpa, StateEnum& state, bool is_neg);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool SendIntPartDigit(CodepointAcceptor&& cpa, StateEnum& state,
                                unsigned digit);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool SendFracPartDigit(CodepointAcceptor&& cpa, StateEnum& state,
                                 unsigned digit);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool SendExpPartE(CodepointAcceptor&& cpa, StateEnum& state,
                            unsigned char e);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool SendExpPartSign(CodepointAcceptor&& cpa, StateEnum& state,
                               unsigned char sign);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool SendExpPartDigit(CodepointAcceptor&& cpa, StateEnum& state,
                                unsigned digit);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendFinish(CodepointAcceptor&& cpa, StateEnum& state);

}  // namespace serialize

namespace deserialize {

enum StateEnum : BaseDeserializeStateCodeTable::Type {
    SendingSign = BaseDeserializeStateCodeTable::SendingNumericSign,

    SendingIntPartDigitLead =
        BaseDeserializeStateCodeTable::SendingNumericIntPartDigitLead,

    SendingIntPartDigitTail =
        BaseDeserializeStateCodeTable::SendingNumericIntPartDigitTail,

    SendingFracPartDigit =
        BaseDeserializeStateCodeTable::SendingNumericFracPartDigit,

    SendingExpPartE = BaseDeserializeStateCodeTable::SendingNumericExpPartE,

    SendingExpPartSign =
        BaseDeserializeStateCodeTable::SendingNumericExpPartSign,

    SendingExpPartDigit =
        BaseDeserializeStateCodeTable::SendingNumericExpPartDigit,

    Finished = BaseDeserializeStateCodeTable::SendingNumericFinish,

    Corrupted = BaseDeserializeStateCodeTable::Corrupted,
};

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void ReceiveStart(BufferedCodepointProvider<CodepointProvider>& bcpp,
                            StateEnum& state);

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr bool ReceiveSign(BufferedCodepointProvider<CodepointProvider>& bcpp,
                           StateEnum& state);

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned ReceiveIntPartDigit(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state);

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned ReceiveFracPartDigit(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state);

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned char ReceiveExpPartE(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state);

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr pair::Pair<unsigned char, bool> ReceiveExpPartSign(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state);

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned ReceiveExpPartDigit(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state);

}  // namespace deserialize

}  // namespace zeta::core::json_utils::numeric_serde
