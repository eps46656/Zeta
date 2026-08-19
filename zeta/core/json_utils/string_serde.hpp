#pragma once

#include <zeta/core/ascii.hpp>
#include <zeta/core/elem_stream.hpp>
#include <zeta/core/json_utils/utils.hpp>
#include <zeta/core/pair.hpp>
#include <zeta/core/unicode.hpp>

namespace zeta::core::json_utils::string_serde {

namespace serialize {

enum struct StateEnum : unsigned char {
    ReceivingCharOrFinish =
        BaseSerializeStateCodeTable::ReceivingStringCharOrFinish,
    Finished = BaseSerializeStateCodeTable::Finished,
};

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendStart(CodepointAcceptor&& cpa, StateEnum& state);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool SendChar(CodepointAcceptor&& cpa, StateEnum& state,
                        unicode::unichar_t cp, bool prefer_unicode_escape,
                        bool prefer_uppercase_hex);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendFinish(CodepointAcceptor&& cpa, StateEnum& state);

}  // namespace serialize

namespace deserialize {

enum StateEnum : BaseDeserializeStateCodeTable::Type {
    SendingChar = BaseDeserializeStateCodeTable::SendingStringChar,

    Finished = BaseDeserializeStateCodeTable::SendingStringFinish,

    Corrupted = BaseDeserializeStateCodeTable::Corrupted,
};

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void ReceiveStart(BufferedCodepointProvider<CodepointProvider>& bcpp,
                            StateEnum& state);

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unicode::unichar_t ReceiveChar(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state);

}  // namespace deserialize

}  // namespace zeta::core::json_utils::string_serde
