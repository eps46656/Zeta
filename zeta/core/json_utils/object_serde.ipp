#pragma once

#include <zeta/core/json_utils/object_serde.hpp>
#include <zeta/core/json_utils/utils.ipp>

namespace zeta::core::json_utils {

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void object_serde::Deserializer::Init(
    this Deserializer& self,
    BufferedCodepointProvider<CodepointProvider>& bcpp) {
    unicode::unichar_t cp;

    do {
        if (bcpp.IsEnd()) {
            self.state = StateEnum::Corrupted;
            return;
        }

        cp = bcpp.Fetch();
    } while ((IsInvisible)(cp));

    if (cp != ascii::CharCodeTable::brace_l) {
        self.state = StateEnum::Corrupted;
        bcpp.Revert();
        return;
    }

    do {
        if (bcpp.IsEnd()) {
            self.state = StateEnum::Corrupted;
            return;
        }

        cp = bcpp.Fetch();
    } while ((IsInvisible)(cp));

    if (cp == ascii::CharCodeTable::brace_r) {
        self.state = StateEnum::Finished;
    } else {
        self.state = StateEnum::SendingKey;
        bcpp.Revert();
    }
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void object_serde::Deserializer::ReceivedKey(
    this Deserializer& self,
    BufferedCodepointProvider<CodepointProvider>& bcpp) {
    ZETA_Core_DebugAssert(self.state == StateEnum::SendingKey);

    unicode::unichar_t cp;

    do {
        if (bcpp.IsEnd()) {
            self.state = StateEnum::Corrupted;
            return;
        }

        cp = bcpp.Fetch();
    } while ((IsInvisible)(cp));

    if (cp == ascii::CharCodeTable::colon) {
        self.state = StateEnum::SendingValue;
    } else {
        self.state = StateEnum::Corrupted;
        bcpp.Revert();
    }
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void object_serde::Deserializer::ReceivedValue(
    this Deserializer& self,
    BufferedCodepointProvider<CodepointProvider>& bcpp) {
    ZETA_Core_DebugAssert(self.state == StateEnum::SendingValue);

    unicode::unichar_t cp;

    do {
        if (bcpp.IsEnd()) {
            self.state = StateEnum::Corrupted;
            return;
        }

        cp = bcpp.Fetch();
    } while ((IsInvisible)(cp));

    if (cp == ascii::CharCodeTable::comma) {
        self.state = StateEnum::SendingKey;
    } else if (cp == ascii::CharCodeTable::brace_r) {
        self.state = StateEnum::Finished;
    } else {
        self.state = StateEnum::Corrupted;
        bcpp.Revert();
    }
}

}  // namespace zeta::core::json_utils
