#pragma once

#include <zeta/core/ascii.hpp>
#include <zeta/core/elem_stream.hpp>
#include <zeta/core/json_utils/utils.hpp>
#include <zeta/core/pair.hpp>
#include <zeta/core/unicode.hpp>

namespace zeta::core::json_utils::string_serde {

struct Serializer {
    enum struct StateEnum : unsigned char {
        ReceivingCharOrFinish = 0,
        Finished = 127,
    };

    StateEnum state;

    template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
    constexpr void Init(this Serializer& self, CodepointAcceptor&& cpa);

    template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
    constexpr bool SendChar(this Serializer& self, CodepointAcceptor&& cpa,
                            unicode::unichar_t cp, bool prefer_unicode_escape,
                            bool prefer_capital_hex);

    template <elem_stream::acceptor::IsAcceptor CodepointAcceptor,
              elem_stream::provider::IsProvider CodepointProvider>
    constexpr pair::Pair<size_t, unicode::unichar_t> SendChar(
        this Serializer& self, CodepointAcceptor&& cpa, CodepointProvider&& cpp,
        unicode::unichar_t max_cnt, bool prefer_unicode_escape,
        bool prefer_capital_hex);

    template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
    constexpr void SendFinish(this Serializer& self, CodepointAcceptor&& cpa);
};

struct Deserializer {
    enum StateEnum : BaseDeserializerStateCodeTable::Type {
        SendingChar = BaseDeserializerStateCodeTable::SendingStringChar,

        Finished = BaseDeserializerStateCodeTable::SendingStringFinish,

        Corrupted = BaseDeserializerStateCodeTable::Corrupted,
    };

    StateEnum state;

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr void Init(this Deserializer& self,
                        BufferedCodepointProvider<CodepointProvider>& bcpp);

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr unicode::unichar_t ReceiveChar(
        this Deserializer& self,
        BufferedCodepointProvider<CodepointProvider>& bcpp);

    template <elem_stream::provider::IsProvider CodepointProvider,
              elem_stream::provider::IsProvider CodepointAcceptor>
    constexpr size_t ReceiveChar(
        this Deserializer& self,
        BufferedCodepointProvider<CodepointProvider>& bcpp,
        CodepointAcceptor&& acceptor, size_t max_cnt);
};

}  // namespace zeta::core::json_utils::string_serde
