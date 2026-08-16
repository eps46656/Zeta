#pragma once

#include <zeta/core/elem_stream.hpp>
#include <zeta/core/json_utils/format_utils.hpp>
#include <zeta/core/json_utils/utils.hpp>

namespace zeta::core::json_utils::object_serde {

struct Serializer {
    enum struct StateEnum : unsigned char {
        ReceivingKeyOrFinish =
            BaseSerializerStateCodeTable::ReceivingObjectKeyOrFinish,
        ReceivingValue = BaseSerializerStateCodeTable::ReceivingObjectValue,
    };

    StateEnum state;

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr void Init(this Serializer& self, CodepointProvider&& cpp,
                        format_utils::FormatContext const& fmt_ctx);

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr void SendKey(this Serializer& self, CodepointProvider&& cpp);

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr void SendValue(this Serializer& self, CodepointProvider&& cpp);

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr void SendFinish(this Serializer& self, CodepointProvider&& cpp);
};

struct Deserializer {
    enum struct StateEnum : BaseDeserializerStateCodeTable::Type {
        SendingKey = BaseDeserializerStateCodeTable::SendingObjectKey,
        SendingValue = BaseDeserializerStateCodeTable::SendingObjectValue,

        Finished = BaseDeserializerStateCodeTable::Finished,

        Corrupted = BaseDeserializerStateCodeTable::Corrupted,
    };

    StateEnum state;

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr void Init(this Deserializer& self,
                        BufferedCodepointProvider<CodepointProvider>& bcpp);

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr void ReceivedKey(
        this Deserializer& self,
        BufferedCodepointProvider<CodepointProvider>& bcpp);

    template <elem_stream::provider::IsProvider CodepointProvider>
    constexpr void ReceivedValue(
        this Deserializer& self,
        BufferedCodepointProvider<CodepointProvider>& bcpp);
};

}  // namespace zeta::core::json_utils::object_serde
