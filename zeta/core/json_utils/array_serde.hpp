#pragma once

#include <zeta/core/json_utils/utils.hpp>

namespace zeta::core::json_utils::array_serde {

struct Serializer {
    enum struct StateEnum : unsigned char {
        ReceivingElemOrFinish = 0,
        Finished = 127,
    };

    StateEnum state;
};

struct Deserializer {
    enum StateEnum : BaseDeserializerStateCodeTable::Type {
        SendingElem = BaseDeserializerStateCodeTable::SendingArrayElem,
        Finished = BaseDeserializerStateCodeTable::Finished,

        Corrupted = BaseDeserializerStateCodeTable::Corrupted,
    };

    StateEnum state;

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr void Init(this Deserializer& self,
                        BufferedCodepointProvider<CodepointProvider>& bcpp);

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr void ReceivedElem(
        this Deserializer& self,
        BufferedCodepointProvider<CodepointProvider>& bcpp);
};

}  // namespace zeta::core::json_utils::array_serde
