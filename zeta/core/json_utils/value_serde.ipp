#pragma once

#include <zeta/core/json_utils/array_serde.ipp>
#include <zeta/core/json_utils/format_utils.ipp>
#include <zeta/core/json_utils/numeric_serde.ipp>
#include <zeta/core/json_utils/object_serde.ipp>
#include <zeta/core/json_utils/string_serde.ipp>
#include <zeta/core/json_utils/value_serde.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core::json_utils {

namespace value_serde::detail {

constexpr SerializerStateEnum ToMainSerializerStateEnum_(
    numeric_serde::serialize::StateEnum state) {
    switch (state) {
    case numeric_serde::serialize::StateEnum::ReceivingSign:
        return SerializerStateEnum::ReceivingNumericSign;
    case numeric_serde::serialize::StateEnum::ReceivingIntPartDigit:
        return SerializerStateEnum::ReceivingNumericIntPartDigit;
    case numeric_serde::serialize::StateEnum::ReceivingIntPartDigitOrNext:
        return SerializerStateEnum::ReceivingNumericIntPartDigitOrNext;
    case numeric_serde::serialize::StateEnum::ReceivingFracPartDigitOrNext:
        return SerializerStateEnum::ReceivingNumericFracPartDigitOrNext;
    case numeric_serde::serialize::StateEnum::ReceivingExpPartSignOrNext:
        return SerializerStateEnum::ReceivingNumericExpPartSignOrNext;
    case numeric_serde::serialize::StateEnum::ReceivingExpPartDigit:
        return SerializerStateEnum::ReceivingNumericExpPartDigit;
    case numeric_serde::serialize::StateEnum::ReceivingExpPartDigitOrNext:
        return SerializerStateEnum::ReceivingNumericExpPartDigitOrNext;
    case numeric_serde::serialize::StateEnum::Finished:
        return SerializerStateEnum::Finished;
    }
}

constexpr SerializerStateEnum ToMainSerializerStateEnum_(
    string_serde::serialize::StateEnum state) {
    switch (state) {
    case string_serde::serialize::StateEnum::ReceivingCharOrFinish:
        return SerializerStateEnum::ReceivingStringCharOrFinish;
    case string_serde::serialize::StateEnum::Finished:
        return SerializerStateEnum::Finished;
    }
}

constexpr SerializerStateEnum ToMainSerializerStateEnum_(
    array_serde::serialize::StateEnum state) {
    switch (state) {
    case array_serde::serialize::StateEnum::ReceivingElemOrFinishLead:
        return SerializerStateEnum::ReceivingArrayElemOrFinishLead;
    case array_serde::serialize::StateEnum::ReceivingElemOrFinishTail:
        return SerializerStateEnum::ReceivingArrayElemOrFinishTail;
    case array_serde::serialize::StateEnum::Finished:
        return SerializerStateEnum::Finished;
    }
}

constexpr SerializerStateEnum ToMainSerializerStateEnum_(
    object_serde::serialize::StateEnum state) {
    switch (state) {
    case object_serde::serialize::StateEnum::ReceivingKeyOrFinishLead:
        return SerializerStateEnum::ReceivingObjectKeyOrFinishLead;
    case object_serde::serialize::StateEnum::ReceivingKeyOrFinishTail:
        return SerializerStateEnum::ReceivingObjectKeyOrFinishTail;
    case object_serde::serialize::StateEnum::ReceivingValue:
        return SerializerStateEnum::ReceivingObjectValue;
    case object_serde::serialize::StateEnum::Finished:
        return SerializerStateEnum::Finished;
    }
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendLastElem_(Serializer<CodepointAcceptor>& ts,
                             bool allow_as_obj_key) {
    ZETA_Core_DebugAssert(0 < ts.depth);

    switch (ts.states[ts.depth - 1]) {
    case SerializerStateEnum::ReceivingValue: --ts.depth; break;

    case SerializerStateEnum::ReceivingArrayElemOrFinishLead: {
        array_serde::serialize::StateEnum state{
            array_serde::serialize::StateEnum::ReceivingElemOrFinishLead
        };

        array_serde::serialize::SendElem(ts.cpa, state, ts.fmt_ctx);

        ts.states[ts.depth - 1] = (ToMainSerializerStateEnum_)(state);

        break;
    }

    case SerializerStateEnum::ReceivingArrayElemOrFinishTail: {
        array_serde::serialize::StateEnum state{
            array_serde::serialize::StateEnum::ReceivingElemOrFinishTail
        };

        array_serde::serialize::SendElem(ts.cpa, state, ts.fmt_ctx);

        ts.states[ts.depth - 1] = (ToMainSerializerStateEnum_)(state);

        break;
    }

    case SerializerStateEnum::ReceivingObjectKeyOrFinishLead: {
        ZETA_Core_DebugAssert(allow_as_obj_key);

        object_serde::serialize::StateEnum state{
            object_serde::serialize::StateEnum::ReceivingKeyOrFinishLead
        };

        object_serde::serialize::SendKey(ts.cpa, state, ts.fmt_ctx);

        ts.states[ts.depth - 1] = (ToMainSerializerStateEnum_)(state);

        break;
    }

    case SerializerStateEnum::ReceivingObjectKeyOrFinishTail: {
        ZETA_Core_DebugAssert(allow_as_obj_key);

        object_serde::serialize::StateEnum state{
            object_serde::serialize::StateEnum::ReceivingKeyOrFinishTail
        };

        object_serde::serialize::SendKey(ts.cpa, state, ts.fmt_ctx);

        ts.states[ts.depth - 1] = (ToMainSerializerStateEnum_)(state);

        break;
    }

    case SerializerStateEnum::ReceivingObjectValue: {
        object_serde::serialize::StateEnum state{
            object_serde::serialize::StateEnum::ReceivingValue
        };

        object_serde::serialize::SendValue(ts.cpa, state, ts.fmt_ctx);

        ts.states[ts.depth - 1] = (ToMainSerializerStateEnum_)(state);

        break;
    }

    case SerializerStateEnum::ReceivingNumericSign:
    case SerializerStateEnum::ReceivingNumericIntPartDigit:
    case SerializerStateEnum::ReceivingNumericIntPartDigitOrNext:
    case SerializerStateEnum::ReceivingNumericFracPartDigitOrNext:
    case SerializerStateEnum::ReceivingNumericExpPartSignOrNext:
    case SerializerStateEnum::ReceivingNumericExpPartDigit:
    case SerializerStateEnum::ReceivingNumericExpPartDigitOrNext:
    case SerializerStateEnum::ReceivingStringCharOrFinish:
    case SerializerStateEnum::Finished: ZETA_Core_Unreachable();
    }
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void FinishLastElem_(Serializer<CodepointAcceptor>& ts) {
    ZETA_Core_DebugAssert(0 < ts.depth);

    ZETA_Core_DebugAssert(ts.states[ts.depth - 1] ==
                          SerializerStateEnum::Finished);

    if (1 < ts.depth) { --ts.depth; }
}

}  // namespace value_serde::detail

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr value_serde::Serializer<CodepointAcceptor>::Serializer(
    CodepointAcceptor& cpa, format_utils::FormatConfig const& fmt_config)
    : cpa{ cpa }, fmt_ctx{ fmt_config, 0 } {
    this->depth = 1;
    this->states[0] = SerializerStateEnum::ReceivingValue;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void value_serde::Serializer<CodepointAcceptor>::SendNull(
    this Serializer& self) {
    constexpr unicode::unichar_t null_str[]{ ascii::CharCodeTable::n,
                                             ascii::CharCodeTable::u,
                                             ascii::CharCodeTable::l,
                                             ascii::CharCodeTable::l };

    ZETA_Core_DebugAssert(0 < self.depth);

    detail::SendLastElem_(self, false);

    elem_stream::acceptor::Transfer(self.cpa, &null_str,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 4);
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void value_serde::Serializer<CodepointAcceptor>::SendBoolean(
    this Serializer& self, bool value) {
    ZETA_Core_DebugAssert(0 < self.depth);

    detail::SendLastElem_(self, false);

    constexpr unicode::unichar_t false_str[]{ ascii::CharCodeTable::f,
                                              ascii::CharCodeTable::a,
                                              ascii::CharCodeTable::l,
                                              ascii::CharCodeTable::s,
                                              ascii::CharCodeTable::e };

    constexpr unicode::unichar_t true_str[]{ ascii::CharCodeTable::t,
                                             ascii::CharCodeTable::r,
                                             ascii::CharCodeTable::u,
                                             ascii::CharCodeTable::e };

    elem_stream::acceptor::Transfer(self.cpa, value ? true_str : false_str,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), value ? 4 : 5);
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void value_serde::Serializer<CodepointAcceptor>::SendNumericStart(
    this Serializer& self) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.depth < self.max_depth);

    detail::SendLastElem_(self, false);

    self.states[self.depth++] = SerializerStateEnum::ReceivingNumericSign;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void value_serde::Serializer<CodepointAcceptor>::SendNumericSign(
    this Serializer& self, bool is_neg) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          SerializerStateEnum::ReceivingNumericSign);

    numeric_serde::serialize::StateEnum state{
        numeric_serde::serialize::StateEnum::ReceivingSign
    };

    numeric_serde::serialize::SendSign(self.cpa, state, is_neg);

    self.states[self.depth - 1] = detail::ToMainSerializerStateEnum_(state);
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool
value_serde::Serializer<CodepointAcceptor>::SendNumericIntPartDigit(
    this Serializer& self, unsigned digit) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingNumericIntPartDigit ||
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingNumericIntPartDigitOrNext);

    numeric_serde::serialize::StateEnum state;

    switch (self.states[self.depth - 1]) {
    case SerializerStateEnum::ReceivingNumericIntPartDigit:
        state = numeric_serde::serialize::StateEnum::ReceivingIntPartDigit;
        break;
    case SerializerStateEnum::ReceivingNumericIntPartDigitOrNext:
        state =
            numeric_serde::serialize::StateEnum::ReceivingIntPartDigitOrNext;
        break;
    default: ZETA_Core_Unreachable();
    }

    bool ret{ numeric_serde::serialize::SendIntPartDigit(self.cpa, state,
                                                         digit) };

    self.states[self.depth - 1] = detail::ToMainSerializerStateEnum_(state);

    return ret;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool
value_serde::Serializer<CodepointAcceptor>::SendNumericFracPartDigit(
    this Serializer& self, unsigned digit) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingNumericIntPartDigitOrNext ||
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingNumericFracPartDigitOrNext);

    numeric_serde::serialize::StateEnum state;

    switch (self.states[self.depth - 1]) {
    case SerializerStateEnum::ReceivingNumericIntPartDigitOrNext:
        state =
            numeric_serde::serialize::StateEnum::ReceivingIntPartDigitOrNext;
        break;
    case SerializerStateEnum::ReceivingNumericFracPartDigitOrNext:
        state =
            numeric_serde::serialize::StateEnum::ReceivingFracPartDigitOrNext;
        break;
    default: ZETA_Core_Unreachable();
    }

    bool ret{ numeric_serde::serialize::SendFracPartDigit(self.cpa, state,
                                                          digit) };

    self.states[self.depth - 1] = detail::ToMainSerializerStateEnum_(state);

    return ret;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool value_serde::Serializer<CodepointAcceptor>::SendNumericExpPartE(
    this Serializer& self, unsigned char e) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingNumericIntPartDigitOrNext ||
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingNumericFracPartDigitOrNext);

    numeric_serde::serialize::StateEnum state;

    switch (self.states[self.depth - 1]) {
    case SerializerStateEnum::ReceivingNumericIntPartDigitOrNext:
        state =
            numeric_serde::serialize::StateEnum::ReceivingIntPartDigitOrNext;
        break;
    case SerializerStateEnum::ReceivingNumericFracPartDigitOrNext:
        state =
            numeric_serde::serialize::StateEnum::ReceivingFracPartDigitOrNext;
        break;
    default: ZETA_Core_Unreachable();
    }

    bool ret{ numeric_serde::serialize::SendExpPartE(self.cpa, state, e) };

    self.states[self.depth - 1] = detail::ToMainSerializerStateEnum_(state);

    return ret;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool
value_serde::Serializer<CodepointAcceptor>::SendNumericExpPartSign(
    this Serializer& self, unsigned char sign) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(
        self.states[self.depth - 1] ==
        SerializerStateEnum::ReceivingNumericExpPartSignOrNext);

    numeric_serde::serialize::StateEnum state{
        numeric_serde::serialize::StateEnum::ReceivingExpPartSignOrNext
    };

    bool ret{ numeric_serde::serialize::SendExpPartSign(self.cpa, state,
                                                        sign) };

    self.states[self.depth - 1] = detail::ToMainSerializerStateEnum_(state);

    return ret;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool
value_serde::Serializer<CodepointAcceptor>::SendNumericExpPartDigit(
    this Serializer& self, unsigned digit) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingNumericExpPartSignOrNext ||
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingNumericExpPartDigit ||
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingNumericExpPartDigitOrNext);

    numeric_serde::serialize::StateEnum state;

    switch (self.states[self.depth - 1]) {
    case SerializerStateEnum::ReceivingNumericExpPartSignOrNext:
        state = numeric_serde::serialize::StateEnum::ReceivingExpPartSignOrNext;
        break;
    case SerializerStateEnum::ReceivingNumericExpPartDigit:
        state = numeric_serde::serialize::StateEnum::ReceivingExpPartDigit;
        break;
    case SerializerStateEnum::ReceivingNumericExpPartDigitOrNext:
        state =
            numeric_serde::serialize::StateEnum::ReceivingExpPartDigitOrNext;
        break;
    default: ZETA_Core_Unreachable();
    }

    bool ret{ numeric_serde::serialize::SendExpPartDigit(self.cpa, state,
                                                         digit) };

    self.states[self.depth - 1] = detail::ToMainSerializerStateEnum_(state);

    return ret;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void value_serde::Serializer<CodepointAcceptor>::SendNumericFinish(
    this Serializer& self) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingNumericIntPartDigitOrNext ||
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingNumericFracPartDigitOrNext ||
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingNumericExpPartDigitOrNext);

    numeric_serde::serialize::StateEnum state;

    switch (self.states[self.depth - 1]) {
    case SerializerStateEnum::ReceivingNumericIntPartDigitOrNext:
        state =
            numeric_serde::serialize::StateEnum::ReceivingIntPartDigitOrNext;
        break;
    case SerializerStateEnum::ReceivingNumericFracPartDigitOrNext:
        state =
            numeric_serde::serialize::StateEnum::ReceivingFracPartDigitOrNext;
        break;
    case SerializerStateEnum::ReceivingNumericExpPartDigitOrNext:
        state =
            numeric_serde::serialize::StateEnum::ReceivingExpPartDigitOrNext;
        break;
    default: ZETA_Core_Unreachable();
    }

    numeric_serde::serialize::SendFinish(self.cpa, state);

    self.states[self.depth - 1] = detail::ToMainSerializerStateEnum_(state);

    detail::FinishLastElem_(self);
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void value_serde::Serializer<CodepointAcceptor>::SendStringStart(
    this Serializer& self) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.depth < self.max_depth);

    detail::SendLastElem_(self, true);

    string_serde::serialize::StateEnum state;

    string_serde::serialize::SendStart(self.cpa, state);

    self.states[self.depth++] = detail::ToMainSerializerStateEnum_(state);
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool value_serde::Serializer<CodepointAcceptor>::SendStringChar(
    this Serializer& self, unicode::unichar_t cp) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          SerializerStateEnum::ReceivingStringCharOrFinish);

    string_serde::serialize::StateEnum state{
        string_serde::serialize::StateEnum::ReceivingCharOrFinish
    };

    bool ret{ string_serde::serialize::SendChar(
        self.cpa, state, cp, self.fmt_ctx.config.string.prefer_unicode_escape,
        self.fmt_ctx.config.string.prefer_uppercase_hex) };

    self.states[self.depth - 1] = detail::ToMainSerializerStateEnum_(state);

    return ret;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void value_serde::Serializer<CodepointAcceptor>::SendStringFinish(
    this Serializer& self) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          SerializerStateEnum::ReceivingStringCharOrFinish);

    string_serde::serialize::StateEnum state{
        string_serde::serialize::StateEnum::ReceivingCharOrFinish
    };

    string_serde::serialize::SendFinish(self.cpa, state);

    self.states[self.depth - 1] = detail::ToMainSerializerStateEnum_(state);

    detail::FinishLastElem_(self);
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void value_serde::Serializer<CodepointAcceptor>::SendArrayStart(
    this Serializer& self) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.depth < self.max_depth);

    detail::SendLastElem_(self, false);

    array_serde::serialize::StateEnum state;

    array_serde::serialize::SendStart(self.cpa, state, self.fmt_ctx);

    self.states[self.depth++] = detail::ToMainSerializerStateEnum_(state);
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void value_serde::Serializer<CodepointAcceptor>::SendArrayFinish(
    this Serializer& self) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingArrayElemOrFinishLead ||
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingArrayElemOrFinishTail);

    array_serde::serialize::StateEnum state;

    switch (self.states[self.depth - 1]) {
    case SerializerStateEnum::ReceivingArrayElemOrFinishLead:
        state = array_serde::serialize::StateEnum::ReceivingElemOrFinishLead;
        break;
    case SerializerStateEnum::ReceivingArrayElemOrFinishTail:
        state = array_serde::serialize::StateEnum::ReceivingElemOrFinishTail;
        break;
    default: ZETA_Core_Unreachable();
    }

    array_serde::serialize::SendFinish(self.cpa, state, self.fmt_ctx);

    self.states[self.depth - 1] = detail::ToMainSerializerStateEnum_(state);

    detail::FinishLastElem_(self);
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void value_serde::Serializer<CodepointAcceptor>::SendObjectStart(
    this Serializer& self) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.depth < self.max_depth);

    detail::SendLastElem_(self, false);

    object_serde::serialize::StateEnum state;

    object_serde::serialize::SendStart(self.cpa, state, self.fmt_ctx);

    self.states[self.depth++] = detail::ToMainSerializerStateEnum_(state);
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void value_serde::Serializer<CodepointAcceptor>::SendObjectFinish(
    this Serializer& self) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingObjectKeyOrFinishLead ||
        self.states[self.depth - 1] ==
            SerializerStateEnum::ReceivingObjectKeyOrFinishTail);

    object_serde::serialize::StateEnum state;

    switch (self.states[self.depth - 1]) {
    case SerializerStateEnum::ReceivingObjectKeyOrFinishLead:
        state = object_serde::serialize::StateEnum::ReceivingKeyOrFinishLead;
        break;
    case SerializerStateEnum::ReceivingObjectKeyOrFinishTail:
        state = object_serde::serialize::StateEnum::ReceivingKeyOrFinishTail;
        break;
    default: ZETA_Core_Unreachable();
    }

    object_serde::serialize::SendFinish(self.cpa, state, self.fmt_ctx);

    self.states[self.depth - 1] = detail::ToMainSerializerStateEnum_(state);

    detail::FinishLastElem_(self);
}

namespace value_serde::detail {

constexpr DeserializerStateEnum ToMainDeserializerStateEnum(
    numeric_serde::deserialize::StateEnum state) {
    switch (state) {
    case numeric_serde::deserialize::StateEnum::SendingSign:
        return DeserializerStateEnum::SendingNumericSign;
    case numeric_serde::deserialize::StateEnum::SendingIntPartDigitLead:
        return DeserializerStateEnum::SendingNumericIntPartDigitLead;
    case numeric_serde::deserialize::StateEnum::SendingIntPartDigitTail:
        return DeserializerStateEnum::SendingNumericIntPartDigitTail;
    case numeric_serde::deserialize::StateEnum::SendingFracPartDigit:
        return DeserializerStateEnum::SendingNumericFracPartDigit;
    case numeric_serde::deserialize::StateEnum::SendingExpPartE:
        return DeserializerStateEnum::SendingNumericExpPartE;
    case numeric_serde::deserialize::StateEnum::SendingExpPartSign:
        return DeserializerStateEnum::SendingNumericExpPartSign;
    case numeric_serde::deserialize::StateEnum::SendingExpPartDigit:
        return DeserializerStateEnum::SendingNumericExpPartDigit;
    case numeric_serde::deserialize::StateEnum::Finished:
        return DeserializerStateEnum::SendingNumericFinish;
    case numeric_serde::deserialize::StateEnum::Corrupted:
        return DeserializerStateEnum::Corrupted;
    }
}

constexpr DeserializerStateEnum ToMainDeserializerStateEnum_(
    array_serde::deserialize::StateEnum state) {
    switch (state) {
    case array_serde::deserialize::StateEnum::SendingElem:
        return DeserializerStateEnum::SendingArrayElem;
    case array_serde::deserialize::StateEnum::Finished:
        return DeserializerStateEnum::SendingArrayFinish;
    case array_serde::deserialize::StateEnum::Corrupted:
        return DeserializerStateEnum::Corrupted;
    }
}

constexpr DeserializerStateEnum ToMainDeserializerStateEnum_(
    object_serde::deserialize::StateEnum state) {
    switch (state) {
    case object_serde::deserialize::StateEnum::SendingKey:
        return DeserializerStateEnum::SendingObjectKey;
    case object_serde::deserialize::StateEnum::SendingValue:
        return DeserializerStateEnum::SendingObjectValue;
    case object_serde::deserialize::StateEnum::Finished:
        return DeserializerStateEnum::SendingObjectFinish;
    case object_serde::deserialize::StateEnum::Corrupted:
        return DeserializerStateEnum::Corrupted;
    }
}

constexpr DeserializerStateEnum ToMainDeserializerStateEnum_(
    string_serde::deserialize::StateEnum state) {
    switch (state) {
    case string_serde::deserialize::StateEnum::SendingChar:
        return DeserializerStateEnum::SendingStringChar;
    case string_serde::deserialize::StateEnum::Finished:
        return DeserializerStateEnum::SendingStringFinish;
    case string_serde::deserialize::StateEnum::Corrupted:
        return DeserializerStateEnum::Corrupted;
    }
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void CheckMainDeserializer_(
    value_serde::Deserializer<CodepointProvider> const& td) {
    ZETA_Core_DebugAssert(td.depth <= td.max_depth);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void DiscoverNextElem_(
    value_serde::Deserializer<CodepointProvider>& td) {
    if (td.depth == td.max_depth - 1) {
        td.states[td.depth++] = DeserializerStateEnum::Corrupted;
        return;
    }

    unicode::unichar_t cp;

    do {
        if (td.bcpp.IsEnd()) {
            if (td.depth == 0) {
                td.states[td.depth++] = DeserializerStateEnum::Finished;
            } else {
                td.states[td.depth++] = DeserializerStateEnum::Corrupted;
            }

            return;
        }

        cp = td.bcpp.Fetch();
    } while ((IsInvisible)(cp));

    td.bcpp.Revert();

    switch (cp) {
    case ascii::CharCodeTable::n: {
        td.states[td.depth++] = DeserializerStateEnum::SendingNull;
        return;
    }

    case ascii::CharCodeTable::t:
    case ascii::CharCodeTable::f: {
        td.states[td.depth++] = DeserializerStateEnum::SendingBoolean;
        return;
    }

    case ascii::CharCodeTable::minus: {
        td.states[td.depth++] = DeserializerStateEnum::SendingNumericStart;
        return;
    }

    case ascii::CharCodeTable::double_quote: {
        td.states[td.depth++] = DeserializerStateEnum::SendingStringStart;
        return;
    }

    case ascii::CharCodeTable::bracket_l: {
        td.states[td.depth++] = DeserializerStateEnum::SendingArrayStart;
        return;
    }

    case ascii::CharCodeTable::brace_l: {
        td.states[td.depth++] = DeserializerStateEnum::SendingObjectStart;
        return;
    }
    }

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        td.states[td.depth++] = DeserializerStateEnum::SendingNumericStart;
        return;
    }

    td.states[td.depth++] = DeserializerStateEnum::Corrupted;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void FinishLastElem_(Deserializer<CodepointProvider>& td) {
    ZETA_Core_DebugAssert(0 < td.depth);

    switch (td.states[td.depth - 1]) {
    case DeserializerStateEnum::SendingNull:
    case DeserializerStateEnum::SendingBoolean:
    case DeserializerStateEnum::SendingNumericFinish:
    case DeserializerStateEnum::SendingStringFinish:
    case DeserializerStateEnum::SendingArrayFinish:
    case DeserializerStateEnum::SendingObjectFinish: break;
    default: ZETA_Core_Unreachable();
    }

    --td.depth;

    if (td.depth == 0) {
        td.states[td.depth++] = DeserializerStateEnum::Finished;
        return;
    }

    switch (td.states[td.depth - 1]) {
    case DeserializerStateEnum::SendingArrayElem: {
        array_serde::deserialize::StateEnum state{
            array_serde::deserialize::StateEnum::SendingElem
        };

        array_serde::deserialize::ReceivedElem(td.bcpp, state);

        td.states[td.depth - 1] = (ToMainDeserializerStateEnum_)(state);

        break;
    }

    case DeserializerStateEnum::SendingObjectKey: {
        object_serde::deserialize::StateEnum state{
            object_serde::deserialize::StateEnum::SendingKey
        };

        object_serde::deserialize::ReceivedKey(td.bcpp, state);

        td.states[td.depth - 1] = (ToMainDeserializerStateEnum_)(state);

        break;
    }

    case DeserializerStateEnum::SendingObjectValue: {
        object_serde::deserialize::StateEnum state{
            object_serde::deserialize::StateEnum::SendingValue
        };

        object_serde::deserialize::ReceivedValue(td.bcpp, state);

        td.states[td.depth - 1] = (ToMainDeserializerStateEnum_)(state);

        break;
    }

    default: ZETA_Core_Unreachable();
    }
}

}  // namespace value_serde::detail

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr value_serde::Deserializer<CodepointProvider>::Deserializer(
    BufferedCodepointProvider<CodepointProvider>& bcpp)
    : bcpp{ bcpp } {
    this->depth = 0;
    detail::DiscoverNextElem_(*this);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void value_serde::Deserializer<CodepointProvider>::ReceiveNull(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingNull);

    ZETA_Core_DebugAssert(!self.bcpp.IsEnd());

    constexpr unicode::unichar_t expected_cp[]{
        ascii::CharCodeTable::n,  //
        ascii::CharCodeTable::u,  //
        ascii::CharCodeTable::l,  //
        ascii::CharCodeTable::l,  //
    };

    for (int i{ 0 }; i < 4; ++i) {
        if (self.bcpp.IsEnd() || self.bcpp.Fetch() != expected_cp[i]) {
            ZETA_Core_DebugAssert(i != 0);
            self.states[self.depth - 1] = DeserializerStateEnum::Corrupted;
            return;
        }
    }

    detail::FinishLastElem_(self);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr bool value_serde::Deserializer<CodepointProvider>::ReceiveBoolean(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingBoolean);

    ZETA_Core_DebugAssert(!self.bcpp.IsEnd());

    constexpr unicode::unichar_t expected_false_cp[]{
        ascii::CharCodeTable::f,  //
        ascii::CharCodeTable::a,  //
        ascii::CharCodeTable::l,  //
        ascii::CharCodeTable::s,  //
        ascii::CharCodeTable::e,  //
    };

    constexpr unicode::unichar_t expected_true_cp[]{
        ascii::CharCodeTable::t,  //
        ascii::CharCodeTable::r,  //
        ascii::CharCodeTable::u,  //
        ascii::CharCodeTable::e,  //
    };

    ZETA_Core_DebugAssert(!self.bcpp.IsEnd());

    unicode::unichar_t const* expected_cp;
    int expected_cp_len;
    bool expected_bool_value;

    switch (self.bcpp.Fetch()) {
    case ascii::CharCodeTable::f: {
        expected_cp = expected_false_cp + 1;
        expected_cp_len = 5 - 1;
        expected_bool_value = false;
        break;
    }

    case ascii::CharCodeTable::t: {
        expected_cp = expected_true_cp + 1;
        expected_cp_len = 4 - 1;
        expected_bool_value = true;
        break;
    }

    default: {
        self.states[self.depth - 1] = DeserializerStateEnum::Corrupted;
        self.bcpp.Revert();
        return false;
    }
    }

    for (int i{ 0 }; i < expected_cp_len; ++i) {
        if (self.bcpp.IsEnd() || self.bcpp.Fetch() != expected_cp[i]) {
            self.states[self.depth - 1] = DeserializerStateEnum::Corrupted;
            return false;
        }
    }

    detail::FinishLastElem_(self);

    return expected_bool_value;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void
value_serde::Deserializer<CodepointProvider>::ReceiveNumericStart(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingNumericStart);

    numeric_serde::deserialize::StateEnum state;

    numeric_serde::deserialize::ReceiveStart(self.bcpp, state);

    self.states[self.depth - 1] = detail::ToMainDeserializerStateEnum(state);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr bool value_serde::Deserializer<CodepointProvider>::ReceiveNumericSign(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingNumericSign);

    numeric_serde::deserialize::StateEnum state{
        numeric_serde::deserialize::StateEnum::SendingSign
    };

    bool ret{ numeric_serde::deserialize::ReceiveSign(self.bcpp, state) };

    self.states[self.depth - 1] = detail::ToMainDeserializerStateEnum(state);

    return ret;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned
value_serde::Deserializer<CodepointProvider>::ReceiveNumericIntPartDigit(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(
        self.states[self.depth - 1] ==
            DeserializerStateEnum::SendingNumericIntPartDigitLead ||
        self.states[self.depth - 1] ==
            DeserializerStateEnum::SendingNumericIntPartDigitTail);

    numeric_serde::deserialize::StateEnum state;

    switch (self.states[self.depth - 1]) {
    case DeserializerStateEnum::SendingNumericIntPartDigitLead:
        state = numeric_serde::deserialize::StateEnum::SendingIntPartDigitLead;
        break;

    case DeserializerStateEnum::SendingNumericIntPartDigitTail:
        state = numeric_serde::deserialize::StateEnum::SendingIntPartDigitTail;
        break;

    default: ZETA_Core_Unreachable();
    }

    unsigned ret{ numeric_serde::deserialize::ReceiveIntPartDigit(self.bcpp,
                                                                  state) };

    self.states[self.depth - 1] = detail::ToMainDeserializerStateEnum(state);

    return ret;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned
value_serde::Deserializer<CodepointProvider>::ReceiveNumericFracPartDigit(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingNumericFracPartDigit);

    numeric_serde::deserialize::StateEnum state{
        numeric_serde::deserialize::StateEnum::SendingFracPartDigit
    };

    unsigned ret{ numeric_serde::deserialize::ReceiveFracPartDigit(self.bcpp,
                                                                   state) };

    self.states[self.depth - 1] = detail::ToMainDeserializerStateEnum(state);

    return ret;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned char
value_serde::Deserializer<CodepointProvider>::ReceiveNumericExpPartE(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingNumericExpPartE);

    numeric_serde::deserialize::StateEnum state{
        numeric_serde::deserialize::StateEnum::SendingExpPartE
    };

    unsigned char ret{ numeric_serde::deserialize::ReceiveExpPartE(self.bcpp,
                                                                   state) };

    self.states[self.depth - 1] = detail::ToMainDeserializerStateEnum(state);

    return ret;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr pair::Pair<unsigned char, bool>
value_serde::Deserializer<CodepointProvider>::ReceiveNumericExpPartSign(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingNumericExpPartSign);

    numeric_serde::deserialize::StateEnum state{
        numeric_serde::deserialize::StateEnum::SendingExpPartSign
    };

    pair::Pair<unsigned char, bool> ret{
        numeric_serde::deserialize::ReceiveExpPartSign(self.bcpp, state)
    };

    self.states[self.depth - 1] = detail::ToMainDeserializerStateEnum(state);

    return ret;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned
value_serde::Deserializer<CodepointProvider>::ReceiveNumericExpPartDigit(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingNumericExpPartDigit);

    numeric_serde::deserialize::StateEnum state{
        numeric_serde::deserialize::StateEnum::SendingExpPartDigit
    };

    unsigned ret{ numeric_serde::deserialize::ReceiveExpPartDigit(self.bcpp,
                                                                  state) };

    self.states[self.depth - 1] = detail::ToMainDeserializerStateEnum(state);

    return ret;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void
value_serde::Deserializer<CodepointProvider>::ReceiveNumericFinish(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingNumericFinish);

    detail::FinishLastElem_(self);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void value_serde::Deserializer<CodepointProvider>::ReceiveStringStart(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingStringStart);

    string_serde::deserialize::StateEnum state;

    string_serde::deserialize::ReceiveStart(self.bcpp, state);

    self.states[self.depth - 1] = detail::ToMainDeserializerStateEnum_(state);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unicode::unichar_t
value_serde::Deserializer<CodepointProvider>::ReceiveStringChar(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingStringChar);

    string_serde::deserialize::StateEnum state{
        string_serde::deserialize::StateEnum::SendingChar
    };

    unicode::unichar_t ret{ string_serde::deserialize::ReceiveChar(self.bcpp,
                                                                   state) };

    self.states[self.depth - 1] = detail::ToMainDeserializerStateEnum_(state);

    return ret;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void
value_serde::Deserializer<CodepointProvider>::ReceiveStringFinish(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingStringFinish);

    detail::FinishLastElem_(self);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void value_serde::Deserializer<CodepointProvider>::ReceiveArrayStart(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingArrayStart);

    array_serde::deserialize::StateEnum state;

    array_serde::deserialize::ReceiveStart(self.bcpp, state);

    self.states[self.depth - 1] = detail::ToMainDeserializerStateEnum_(state);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void value_serde::Deserializer<CodepointProvider>::ReceiveArrayElem(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingArrayElem);

    detail::DiscoverNextElem_(self);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void value_serde::Deserializer<CodepointProvider>::ReceiveArrayFinish(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingArrayFinish);

    detail::FinishLastElem_(self);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void value_serde::Deserializer<CodepointProvider>::ReceiveObjectStart(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingObjectStart);

    object_serde::deserialize::StateEnum state;

    object_serde::deserialize::ReceiveStart(self.bcpp, state);

    self.states[self.depth - 1] = detail::ToMainDeserializerStateEnum_(state);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void value_serde::Deserializer<CodepointProvider>::ReceiveObjectKey(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingObjectKey);

    detail::DiscoverNextElem_(self);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void value_serde::Deserializer<CodepointProvider>::ReceiveObjectValue(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingObjectValue);

    detail::DiscoverNextElem_(self);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void
value_serde::Deserializer<CodepointProvider>::ReceiveObjectFinish(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingObjectFinish);

    detail::FinishLastElem_(self);
}

}  // namespace zeta::core::json_utils
