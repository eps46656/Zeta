#pragma once

#include <zeta/core/json_utils/format_utils.ipp>
#include <zeta/core/json_utils/utils.hpp>

namespace zeta::core::json_utils::array_serde {

namespace serialize {

enum StateEnum : BaseDeserializeStateCodeTable::Type {
    ReceivingElemOrFinishLead =
        BaseSerializeStateCodeTable::ReceivingArrayElemOrFinishLead,
    ReceivingElemOrFinishTail =
        BaseSerializeStateCodeTable::ReceivingArrayElemOrFinishTail,
    Finished = BaseDeserializeStateCodeTable::Finished,
};

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendStart(CodepointAcceptor&& cpa, StateEnum& state,
                         format_utils::FormatContext& fmt_ctx);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendElem(CodepointAcceptor&& cpa, StateEnum& state,
                        format_utils::FormatContext& fmt_ctx);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendFinish(CodepointAcceptor&& cpa, StateEnum& state,
                          format_utils::FormatContext& fmt_ctx);

}  // namespace serialize

namespace deserialize {

enum StateEnum : BaseDeserializeStateCodeTable::Type {
    SendingElem = BaseDeserializeStateCodeTable::SendingArrayElem,
    Finished = BaseDeserializeStateCodeTable::Finished,

    Corrupted = BaseDeserializeStateCodeTable::Corrupted,
};

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void ReceiveStart(BufferedCodepointProvider<CodepointProvider>& bcpp,
                            StateEnum& state);

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void ReceivedElem(BufferedCodepointProvider<CodepointProvider>& bcpp,
                            StateEnum& state);

}  // namespace deserialize

}  // namespace zeta::core::json_utils::array_serde
