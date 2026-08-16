#pragma once

#include <zeta/core/json_utils/array_serde.ipp>
#include <zeta/core/json_utils/numeric_serde.ipp>
#include <zeta/core/json_utils/object_serde.ipp>
#include <zeta/core/json_utils/string_serde.ipp>
#include <zeta/core/json_utils/value_serde.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core::json_utils {

namespace value_serde::detail {

constexpr DeserializerStateEnum ToMainDeserializerStateEnum(
    numeric_serde::Deserializer::StateEnum numeric_deserializer_state) {
    switch (numeric_deserializer_state) {
    case numeric_serde::Deserializer::StateEnum::SendingSign:
        return DeserializerStateEnum::SendingNumericSign;
    case numeric_serde::Deserializer::StateEnum::SendingIntPartDigitLead:
        return DeserializerStateEnum::SendingNumericIntPartDigitLead;
    case numeric_serde::Deserializer::StateEnum::SendingIntPartDigitTail:
        return DeserializerStateEnum::SendingNumericIntPartDigitTail;
    case numeric_serde::Deserializer::StateEnum::SendingFracPartDigit:
        return DeserializerStateEnum::SendingNumericFracPartDigit;
    case numeric_serde::Deserializer::StateEnum::SendingExpPartE:
        return DeserializerStateEnum::SendingNumericExpPartE;
    case numeric_serde::Deserializer::StateEnum::SendingExpPartSign:
        return DeserializerStateEnum::SendingNumericExpPartSign;
    case numeric_serde::Deserializer::StateEnum::SendingExpPartDigit:
        return DeserializerStateEnum::SendingNumericExpPartDigit;
    case numeric_serde::Deserializer::StateEnum::Finished:
        return DeserializerStateEnum::SendingNumericFinish;
    case numeric_serde::Deserializer::StateEnum::Corrupted:
        return DeserializerStateEnum::Corrupted;
    }
}

constexpr DeserializerStateEnum ToMainDeserializerStateEnum_(
    array_serde::Deserializer::StateEnum array_deserializer_state) {
    switch (array_deserializer_state) {
    case array_serde::Deserializer::StateEnum::SendingElem:
        return DeserializerStateEnum::SendingArrayElem;
    case array_serde::Deserializer::StateEnum::Finished:
        return DeserializerStateEnum::SendingArrayFinish;
    case array_serde::Deserializer::StateEnum::Corrupted:
        return DeserializerStateEnum::Corrupted;
    }
}

constexpr DeserializerStateEnum ToMainDeserializerStateEnum_(
    object_serde::Deserializer::StateEnum object_deserializer_state) {
    switch (object_deserializer_state) {
    case object_serde::Deserializer::StateEnum::SendingKey:
        return DeserializerStateEnum::SendingObjectKey;
    case object_serde::Deserializer::StateEnum::SendingValue:
        return DeserializerStateEnum::SendingObjectValue;
    case object_serde::Deserializer::StateEnum::Finished:
        return DeserializerStateEnum::SendingObjectFinish;
    case object_serde::Deserializer::StateEnum::Corrupted:
        return DeserializerStateEnum::Corrupted;
    }
}

constexpr DeserializerStateEnum ToMainDeserializerStateEnum(
    string_serde::Deserializer::StateEnum string_deserializer_state) {
    switch (string_deserializer_state) {
    case string_serde::Deserializer::StateEnum::SendingChar:
        return DeserializerStateEnum::SendingStringChar;
    case string_serde::Deserializer::StateEnum::Finished:
        return DeserializerStateEnum::SendingStringFinish;
    case string_serde::Deserializer::StateEnum::Corrupted:
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
            td.states[td.depth++] = DeserializerStateEnum::Corrupted;
            return;
        }

        cp = td.bcpp.Fetch();
    } while ((IsInvisible)(cp));

    td.bcpp.Revert();

    ZETA_Core_DebugAssert(cp);

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
constexpr void FinishLastElem_(
    value_serde::Deserializer<CodepointProvider>& td) {
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
        ZETA_Core_Debug_PrintCurPos;

        array_serde::Deserializer array_deserializer{
            array_serde::Deserializer::StateEnum::SendingElem
        };

        array_deserializer.ReceivedElem(td.bcpp);

        td.states[td.depth - 1] =
            (ToMainDeserializerStateEnum_)(array_deserializer.state);

        break;
    }

    case DeserializerStateEnum::SendingObjectKey: {
        ZETA_Core_Debug_PrintCurPos;

        object_serde::Deserializer object_deserializer{
            object_serde::Deserializer::StateEnum::SendingKey
        };

        object_deserializer.ReceivedKey(td.bcpp);

        td.states[td.depth - 1] =
            (ToMainDeserializerStateEnum_)(object_deserializer.state);

        break;
    }

    case DeserializerStateEnum::SendingObjectValue: {
        ZETA_Core_Debug_PrintCurPos;

        object_serde::Deserializer object_deserializer{
            object_serde::Deserializer::StateEnum::SendingValue
        };

        object_deserializer.ReceivedValue(td.bcpp);

        td.states[td.depth - 1] =
            (ToMainDeserializerStateEnum_)(object_deserializer.state);

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
        expected_cp_len =
            sizeof(expected_false_cp) / sizeof(unicode::unichar_t) - 1;
        expected_bool_value = false;
        break;
    }

    case ascii::CharCodeTable::t: {
        expected_cp = expected_true_cp + 1;
        expected_cp_len =
            sizeof(expected_true_cp) / sizeof(unicode::unichar_t) - 1;
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

    numeric_serde::Deserializer numeric_deserializer;

    numeric_deserializer.Init(self.bcpp);

    self.states[self.depth - 1] =
        detail::ToMainDeserializerStateEnum(numeric_deserializer.state);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr bool value_serde::Deserializer<CodepointProvider>::ReceiveNumericSign(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingNumericSign);

    numeric_serde::Deserializer numeric_deserializer{
        numeric_serde::Deserializer::StateEnum::SendingSign
    };

    bool ret{ numeric_deserializer.ReceiveSign(self.bcpp) };

    self.states[self.depth - 1] =
        detail::ToMainDeserializerStateEnum(numeric_deserializer.state);

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

    numeric_serde::Deserializer numeric_deserializer;

    switch (self.states[self.depth - 1]) {
    case DeserializerStateEnum::SendingNumericIntPartDigitLead:
        numeric_deserializer.state =
            numeric_serde::Deserializer::StateEnum::SendingIntPartDigitLead;
        break;

    case DeserializerStateEnum::SendingNumericIntPartDigitTail:
        numeric_deserializer.state =
            numeric_serde::Deserializer::StateEnum::SendingIntPartDigitTail;
        break;

    default: ZETA_Core_DebugAssert(false);
    }

    unsigned ret{ numeric_deserializer.ReceiveIntPartDigit(self.bcpp) };

    self.states[self.depth - 1] =
        detail::ToMainDeserializerStateEnum(numeric_deserializer.state);

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

    numeric_serde::Deserializer numeric_deserializer{
        numeric_serde::Deserializer::StateEnum::SendingFracPartDigit
    };

    unsigned ret{ numeric_deserializer.ReceiveFracPartDigit(self.bcpp) };

    self.states[self.depth - 1] =
        detail::ToMainDeserializerStateEnum(numeric_deserializer.state);

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

    numeric_serde::Deserializer numeric_deserializer{
        numeric_serde::Deserializer::StateEnum::SendingExpPartE
    };

    unsigned char ret{ numeric_deserializer.ReceiveExpPartE(self.bcpp) };

    self.states[self.depth - 1] =
        detail::ToMainDeserializerStateEnum(numeric_deserializer.state);

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

    numeric_serde::Deserializer numeric_deserializer{
        numeric_serde::Deserializer::StateEnum::SendingExpPartSign
    };

    pair::Pair<unsigned char, bool> ret{
        numeric_deserializer.ReceiveExpPartSign(self.bcpp)
    };

    self.states[self.depth - 1] =
        detail::ToMainDeserializerStateEnum(numeric_deserializer.state);

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

    numeric_serde::Deserializer numeric_deserializer{
        numeric_serde::Deserializer::StateEnum::SendingExpPartDigit
    };

    unsigned ret{ numeric_deserializer.ReceiveExpPartDigit(self.bcpp) };

    self.states[self.depth - 1] =
        detail::ToMainDeserializerStateEnum(numeric_deserializer.state);

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

    string_serde::Deserializer string_deserializer;

    string_deserializer.Init(self.bcpp);

    self.states[self.depth - 1] =
        detail::ToMainDeserializerStateEnum(string_deserializer.state);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unicode::unichar_t
value_serde::Deserializer<CodepointProvider>::ReceiveStringChar(
    this Deserializer& self) {
    detail::CheckMainDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.states[self.depth - 1] ==
                          DeserializerStateEnum::SendingStringChar);

    string_serde::Deserializer string_deserializer{
        string_serde::Deserializer::StateEnum::SendingChar
    };

    unicode::unichar_t ret{ string_deserializer.ReceiveChar(self.bcpp) };

    self.states[self.depth - 1] =
        detail::ToMainDeserializerStateEnum(string_deserializer.state);

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

    array_serde::Deserializer array_deserializer;

    array_deserializer.Init(self.bcpp);

    self.states[self.depth - 1] =
        detail::ToMainDeserializerStateEnum_(array_deserializer.state);
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

    object_serde::Deserializer object_deserializer;

    object_deserializer.Init(self.bcpp);

    self.states[self.depth - 1] =
        detail::ToMainDeserializerStateEnum_(object_deserializer.state);
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
