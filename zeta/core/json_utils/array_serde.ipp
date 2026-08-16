#pragma once

#include <zeta/core/json_utils/array_serde.hpp>
#include <zeta/core/json_utils/utils.ipp>

namespace zeta::core::json_utils {

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void array_serde::Deserializer::Init(
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

    if (cp != ascii::CharCodeTable::bracket_l) {
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

    if (cp == ascii::CharCodeTable::bracket_r) {
        self.state = StateEnum::Finished;
    } else {
        self.state = StateEnum::SendingElem;
        bcpp.Revert();
    }
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void array_serde::Deserializer::ReceivedElem(
    this Deserializer& self,
    BufferedCodepointProvider<CodepointProvider>& bcpp) {
    ZETA_Core_DebugAssert(self.state == StateEnum::SendingElem);

    unicode::unichar_t cp;

    do {
        if (bcpp.IsEnd()) {
            self.state = StateEnum::Corrupted;
            return;
        }

        cp = bcpp.Fetch();
    } while ((IsInvisible)(cp));

    if (cp == ascii::CharCodeTable::comma) {
        self.state = StateEnum::SendingElem;
    } else if (cp == ascii::CharCodeTable::bracket_r) {
        self.state = StateEnum::Finished;
    } else {
        self.state = StateEnum::Corrupted;
        bcpp.Revert();
    }
}

}  // namespace zeta::core::json_utils
