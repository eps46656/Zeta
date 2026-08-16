#pragma once

#include <zeta/core/json_utils/numeric_serde.hpp>
#include <zeta/core/json_utils/utils.ipp>

namespace zeta::core::json_utils {

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void numeric_serde::Deserializer::Init(
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

    if (cp != ascii::CharCodeTable::minus &&
        !(ascii::CharCodeTable::num_0 <= cp &&
          cp <= ascii::CharCodeTable::num_9)) {
        self.state = StateEnum::Corrupted;
    } else {
        self.state = StateEnum::SendingSign;
    }

    bcpp.Revert();
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr bool numeric_serde::Deserializer::ReceiveSign(
    this Deserializer& self,
    BufferedCodepointProvider<CodepointProvider>& bcpp) {
    ZETA_Core_DebugAssert(self.state == StateEnum::SendingSign);

    unicode::unichar_t cp;

    if (bcpp.IsEnd()) {
        self.state = StateEnum::Corrupted;
        return false;
    }

    cp = bcpp.Fetch();

    bool ret;

    if (cp == ascii::CharCodeTable::minus) {
        ret = true;

        if (bcpp.IsEnd()) {
            self.state = StateEnum::Corrupted;
            return false;
        }

        cp = bcpp.Fetch();
    } else {
        ret = false;
    }

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        self.state = StateEnum::SendingIntPartDigitLead;
    } else {
        self.state = StateEnum::Corrupted;
    }

    bcpp.Revert();

    return ret;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned numeric_serde::Deserializer::ReceiveIntPartDigit(
    this Deserializer& self,
    BufferedCodepointProvider<CodepointProvider>& bcpp) {
    ZETA_Core_DebugAssert(self.state == StateEnum::SendingIntPartDigitLead ||
                          self.state == StateEnum::SendingIntPartDigitTail);

    unicode::unichar_t cp;

    ZETA_Core_DebugAssert(!bcpp.IsEnd());

    cp = bcpp.Fetch();

    ZETA_Core_DebugAssert(ascii::CharCodeTable::num_0 <= cp &&
                          cp <= ascii::CharCodeTable::num_9);

    unsigned ret{ cp - ascii::CharCodeTable::num_0 };

    if (bcpp.IsEnd()) {
        self.state = StateEnum::Finished;
        return ret;
    }

    bool first_digit_zero;

    if (self.state == StateEnum::SendingIntPartDigitLead) {
        first_digit_zero = ret == 0;
        self.state = StateEnum::SendingIntPartDigitTail;
    } else {
        first_digit_zero = false;
    }

    cp = bcpp.Fetch();

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        if (first_digit_zero) { self.state = StateEnum::Corrupted; }

        bcpp.Revert();

        return ret;
    }

    if (cp == ascii::CharCodeTable::point) {
        if (bcpp.IsEnd()) {
            self.state = StateEnum::Corrupted;
            return ret;
        }

        cp = bcpp.Fetch();

        if (ascii::CharCodeTable::num_0 <= cp &&
            cp <= ascii::CharCodeTable::num_9) {
            self.state = StateEnum::SendingFracPartDigit;
        } else {
            self.state = StateEnum::Corrupted;
        }

        bcpp.Revert();

        return ret;
    }

    if (cp == ascii::CharCodeTable::e || cp == ascii::CharCodeTable::E) {
        self.state = StateEnum::SendingExpPartE;
        bcpp.Revert();
        return ret;
    }

    if ((IsTokenEnd)(cp)) {
        self.state = StateEnum::Finished;
    } else {
        self.state = StateEnum::Corrupted;
    }

    bcpp.Revert();

    return ret;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned numeric_serde::Deserializer::ReceiveFracPartDigit(
    this Deserializer& self,
    BufferedCodepointProvider<CodepointProvider>& bcpp) {
    ZETA_Core_DebugAssert(self.state == StateEnum::SendingFracPartDigit);

    unicode::unichar_t cp;

    if (bcpp.IsEnd()) {
        self.state = StateEnum::Corrupted;
        return 0;
    }

    cp = bcpp.Fetch();

    ZETA_Core_DebugAssert(ascii::CharCodeTable::num_0 <= cp &&
                          cp <= ascii::CharCodeTable::num_9);

    unsigned ret{ cp - ascii::CharCodeTable::num_0 };

    if (bcpp.IsEnd()) {
        self.state = StateEnum::Finished;
        return ret;
    }

    cp = bcpp.Fetch();

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        bcpp.Revert();
        return ret;
    }

    if (cp == ascii::CharCodeTable::e || cp == ascii::CharCodeTable::E) {
        self.state = StateEnum::SendingExpPartE;
        bcpp.Revert();
        return ret;
    }

    if ((IsTokenEnd)(cp)) {
        self.state = StateEnum::Finished;
    } else {
        self.state = StateEnum::Corrupted;
    }

    bcpp.Revert();

    return ret;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned char numeric_serde::Deserializer::ReceiveExpPartE(
    this Deserializer& self,
    BufferedCodepointProvider<CodepointProvider>& bcpp) {
    ZETA_Core_DebugAssert(self.state == StateEnum::SendingExpPartE);

    unicode::unichar_t cp;

    ZETA_Core_DebugAssert(!bcpp.IsEnd());

    cp = bcpp.Fetch();

    ZETA_Core_DebugAssert(cp == ascii::CharCodeTable::e ||
                          cp == ascii::CharCodeTable::E);

    self.state = StateEnum::SendingExpPartSign;

    return static_cast<unsigned char>(cp);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr pair::Pair<unsigned char, bool>
numeric_serde::Deserializer::ReceiveExpPartSign(
    this Deserializer& self,
    BufferedCodepointProvider<CodepointProvider>& bcpp) {
    ZETA_Core_DebugAssert(self.state == StateEnum::SendingExpPartSign);

    unicode::unichar_t cp;

    if (bcpp.IsEnd()) {
        self.state = StateEnum::Corrupted;
        return { ascii::CharCodeTable::empty, false };
    }

    cp = bcpp.Fetch();

    pair::Pair<unsigned char, bool> ret;

    if (cp == ascii::CharCodeTable::minus) {
        ret.first = ascii::CharCodeTable::minus;
        ret.second = true;

        if (bcpp.IsEnd()) {
            self.state = StateEnum::Corrupted;
            return ret;
        }

        cp = bcpp.Fetch();
    } else if (cp == ascii::CharCodeTable::plus) {
        ret.first = ascii::CharCodeTable::plus;
        ret.second = false;

        if (bcpp.IsEnd()) {
            self.state = StateEnum::Corrupted;
            return ret;
        }

        cp = bcpp.Fetch();
    } else {
        ret.first = ascii::CharCodeTable::empty;
        ret.second = false;
    }

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        self.state = StateEnum::SendingExpPartDigit;
    } else {
        self.state = StateEnum::Corrupted;
    }

    bcpp.Revert();

    return ret;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned numeric_serde::Deserializer::ReceiveExpPartDigit(
    this Deserializer& self,
    BufferedCodepointProvider<CodepointProvider>& bcpp) {
    ZETA_Core_DebugAssert(self.state == StateEnum::SendingExpPartDigit);

    unicode::unichar_t cp;

    ZETA_Core_DebugAssert(!bcpp.IsEnd());

    cp = bcpp.Fetch();

    ZETA_Core_DebugAssert(ascii::CharCodeTable::num_0 <= cp &&
                          cp <= ascii::CharCodeTable::num_9);

    unsigned ret{ cp - ascii::CharCodeTable::num_0 };

    if (bcpp.IsEnd()) {
        self.state = StateEnum::Finished;
        return ret;
    }

    cp = bcpp.Fetch();

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        bcpp.Revert();
        return ret;
    }

    if ((IsTokenEnd)(cp)) {
        self.state = StateEnum::Finished;
    } else {
        self.state = StateEnum::Corrupted;
    }

    bcpp.Revert();

    return ret;
}

}  // namespace zeta::core::json_utils
