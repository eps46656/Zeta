#pragma once

#include <zeta/core/elem_stream.hpp>
#include <zeta/core/json_utils/format_utils.hpp>
#include <zeta/core/json_utils/utils.hpp>

namespace zeta::core::json_utils::object_serde {

namespace serialize {
enum struct StateEnum : unsigned char {
    ReceivingKeyOrFinishLead =
        BaseSerializeStateCodeTable::ReceivingObjectKeyOrFinishLead,
    ReceivingKeyOrFinishTail =
        BaseSerializeStateCodeTable::ReceivingObjectKeyOrFinishTail,
    ReceivingValue = BaseSerializeStateCodeTable::ReceivingObjectValue,
    Finished = BaseSerializeStateCodeTable::Finished,
};

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendStart(CodepointAcceptor&& cpa, StateEnum& state,
                         format_utils::FormatContext& fmt_ctx);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendKey(CodepointAcceptor&& cpa, StateEnum& state,
                       format_utils::FormatContext& fmt_ctx);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendValue(CodepointAcceptor&& cpa, StateEnum& state,
                         format_utils::FormatContext& fmt_ctx);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendFinish(CodepointAcceptor&& cpa, StateEnum& state,
                          format_utils::FormatContext& fmt_ctx);

}  // namespace serialize

namespace deserialize {
enum struct StateEnum : BaseDeserializeStateCodeTable::Type {
    SendingKey = BaseDeserializeStateCodeTable::SendingObjectKey,
    SendingValue = BaseDeserializeStateCodeTable::SendingObjectValue,

    Finished = BaseDeserializeStateCodeTable::Finished,

    Corrupted = BaseDeserializeStateCodeTable::Corrupted,
};

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void ReceiveStart(BufferedCodepointProvider<CodepointProvider>& bcpp,
                            StateEnum& state);

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void ReceivedKey(BufferedCodepointProvider<CodepointProvider>& bcpp,
                           StateEnum& state);

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void ReceivedValue(BufferedCodepointProvider<CodepointProvider>& bcpp,
                             StateEnum& state);

}  // namespace deserialize

}  // namespace zeta::core::json_utils::object_serde
