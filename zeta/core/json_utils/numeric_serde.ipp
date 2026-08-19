#pragma once

#include <zeta/core/json_utils/numeric_serde.hpp>
#include <zeta/core/json_utils/utils.ipp>

namespace zeta::core::json_utils {

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void numeric_serde::serialize::SendStart(CodepointAcceptor&&,
                                                   StateEnum& state) {
    state = StateEnum::ReceivingSign;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void numeric_serde::serialize::SendSign(CodepointAcceptor&& cpa,
                                                  StateEnum& state,
                                                  bool is_neg) {
    ZETA_Core_DebugAssert(state == StateEnum::ReceivingSign);

    constexpr unicode::unichar_t minus{ ascii::CharCodeTable::minus };

    if (is_neg) {
        elem_stream::acceptor::Transfer(cpa, &minus, sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);
    }

    state = StateEnum::ReceivingIntPartDigit;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool numeric_serde::serialize::SendIntPartDigit(
    CodepointAcceptor&& cpa, StateEnum& state, unsigned digit) {
    ZETA_Core_DebugAssert(state == StateEnum::ReceivingIntPartDigit ||
                          state == StateEnum::ReceivingIntPartDigitOrNext);

    if (!(0 <= digit && digit <= 9)) { return false; }

    unicode::unichar_t cp{ static_cast<unicode::unichar_t>(
        static_cast<unsigned char>(ascii::CharCodeTable::num_0 + digit)) };

    elem_stream::acceptor::Transfer(cpa, &cp, sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    if (state == StateEnum::ReceivingIntPartDigit) {
        state = digit == 0 ? StateEnum::ReceivingFracPartDigitOrNext
                           : StateEnum::ReceivingIntPartDigitOrNext;
    }

    return true;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool numeric_serde::serialize::SendFracPartDigit(
    CodepointAcceptor&& cpa, StateEnum& state, unsigned digit) {
    ZETA_Core_DebugAssert(state == StateEnum::ReceivingIntPartDigitOrNext ||
                          state == StateEnum::ReceivingFracPartDigitOrNext);

    if (!(0 <= digit && digit <= 9)) { return false; }

    unicode::unichar_t digit_cp{ static_cast<unicode::unichar_t>(
        static_cast<unsigned char>(ascii::CharCodeTable::num_0 + digit)) };

    if (state == StateEnum::ReceivingIntPartDigitOrNext) {
        unicode::unichar_t cp[]{ ascii::CharCodeTable::point, digit_cp };

        elem_stream::acceptor::Transfer(cpa, cp, sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 2);

        state = StateEnum::ReceivingFracPartDigitOrNext;
    } else {
        elem_stream::acceptor::Transfer(cpa, &digit_cp,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);
    }

    return true;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool numeric_serde::serialize::SendExpPartE(CodepointAcceptor&& cpa,
                                                      StateEnum& state,
                                                      unsigned char e) {
    ZETA_Core_DebugAssert(state == StateEnum::ReceivingIntPartDigitOrNext ||
                          state == StateEnum::ReceivingFracPartDigitOrNext);

    if (!(e == ascii::CharCodeTable::e || e == ascii::CharCodeTable::E)) {
        return false;
    }

    unicode::unichar_t cp{ static_cast<unicode::unichar_t>(e) };

    elem_stream::acceptor::Transfer(cpa, &cp, sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    state = StateEnum::ReceivingExpPartSignOrNext;

    return true;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool numeric_serde::serialize::SendExpPartSign(
    CodepointAcceptor&& cpa, StateEnum& state, unsigned char sign) {
    ZETA_Core_DebugAssert(state == StateEnum::ReceivingExpPartSignOrNext);

    if (!(sign == ascii::CharCodeTable::empty ||
          sign == ascii::CharCodeTable::minus ||
          sign == ascii::CharCodeTable::plus)) {
        return false;
    }

    if (sign != ascii::CharCodeTable::empty) {
        unicode::unichar_t cp{ static_cast<unicode::unichar_t>(sign) };

        elem_stream::acceptor::Transfer(cpa, &cp, sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);
    }

    state = StateEnum::ReceivingExpPartDigit;

    return true;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool numeric_serde::serialize::SendExpPartDigit(
    CodepointAcceptor&& cpa, StateEnum& state, unsigned digit) {
    ZETA_Core_DebugAssert(state == StateEnum::ReceivingExpPartSignOrNext ||
                          state == StateEnum::ReceivingExpPartDigit ||
                          state == StateEnum::ReceivingExpPartDigitOrNext);

    if (!(0 <= digit && digit <= 9)) { return false; }

    unicode::unichar_t cp{ static_cast<unicode::unichar_t>(
        static_cast<unsigned char>(ascii::CharCodeTable::num_0 + digit)) };

    elem_stream::acceptor::Transfer(cpa, &cp, sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    state = StateEnum::ReceivingExpPartDigitOrNext;

    return true;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void numeric_serde::serialize::SendFinish(CodepointAcceptor&&,
                                                    StateEnum& state) {
    ZETA_Core_DebugAssert(state == StateEnum::ReceivingIntPartDigitOrNext ||
                          state == StateEnum::ReceivingFracPartDigitOrNext ||
                          state == StateEnum::ReceivingExpPartDigitOrNext);

    state = StateEnum::Finished;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void numeric_serde::deserialize::ReceiveStart(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state) {
    unicode::unichar_t cp;

    do {
        if (bcpp.IsEnd()) {
            state = StateEnum::Corrupted;
            return;
        }

        cp = bcpp.Fetch();
    } while ((IsInvisible)(cp));

    if (cp != ascii::CharCodeTable::minus &&
        !(ascii::CharCodeTable::num_0 <= cp &&
          cp <= ascii::CharCodeTable::num_9)) {
        state = StateEnum::Corrupted;
    } else {
        state = StateEnum::SendingSign;
    }

    bcpp.Revert();
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr bool numeric_serde::deserialize::ReceiveSign(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state) {
    ZETA_Core_DebugAssert(state == StateEnum::SendingSign);

    unicode::unichar_t cp;

    if (bcpp.IsEnd()) {
        state = StateEnum::Corrupted;
        return false;
    }

    cp = bcpp.Fetch();

    bool ret;

    if (cp == ascii::CharCodeTable::minus) {
        ret = true;

        if (bcpp.IsEnd()) {
            state = StateEnum::Corrupted;
            return false;
        }

        cp = bcpp.Fetch();
    } else {
        ret = false;
    }

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        state = StateEnum::SendingIntPartDigitLead;
    } else {
        state = StateEnum::Corrupted;
    }

    bcpp.Revert();

    return ret;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned numeric_serde::deserialize::ReceiveIntPartDigit(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state) {
    ZETA_Core_DebugAssert(state == StateEnum::SendingIntPartDigitLead ||
                          state == StateEnum::SendingIntPartDigitTail);

    unicode::unichar_t cp;

    ZETA_Core_DebugAssert(!bcpp.IsEnd());

    cp = bcpp.Fetch();

    ZETA_Core_DebugAssert(ascii::CharCodeTable::num_0 <= cp &&
                          cp <= ascii::CharCodeTable::num_9);

    unsigned ret{ cp - ascii::CharCodeTable::num_0 };

    if (bcpp.IsEnd()) {
        state = StateEnum::Finished;
        return ret;
    }

    bool first_digit_zero;

    if (state == StateEnum::SendingIntPartDigitLead) {
        first_digit_zero = ret == 0;
        state = StateEnum::SendingIntPartDigitTail;
    } else {
        first_digit_zero = false;
    }

    cp = bcpp.Fetch();

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        if (first_digit_zero) { state = StateEnum::Corrupted; }

        bcpp.Revert();

        return ret;
    }

    if (cp == ascii::CharCodeTable::point) {
        if (bcpp.IsEnd()) {
            state = StateEnum::Corrupted;
            return ret;
        }

        cp = bcpp.Fetch();

        if (ascii::CharCodeTable::num_0 <= cp &&
            cp <= ascii::CharCodeTable::num_9) {
            state = StateEnum::SendingFracPartDigit;
        } else {
            state = StateEnum::Corrupted;
        }

        bcpp.Revert();

        return ret;
    }

    if (cp == ascii::CharCodeTable::e || cp == ascii::CharCodeTable::E) {
        state = StateEnum::SendingExpPartE;
        bcpp.Revert();
        return ret;
    }

    if ((IsTokenEnd)(cp)) {
        state = StateEnum::Finished;
    } else {
        state = StateEnum::Corrupted;
    }

    bcpp.Revert();

    return ret;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned numeric_serde::deserialize::ReceiveFracPartDigit(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state) {
    ZETA_Core_DebugAssert(state == StateEnum::SendingFracPartDigit);

    unicode::unichar_t cp;

    if (bcpp.IsEnd()) {
        state = StateEnum::Corrupted;
        return 0;
    }

    cp = bcpp.Fetch();

    ZETA_Core_DebugAssert(ascii::CharCodeTable::num_0 <= cp &&
                          cp <= ascii::CharCodeTable::num_9);

    unsigned ret{ cp - ascii::CharCodeTable::num_0 };

    if (bcpp.IsEnd()) {
        state = StateEnum::Finished;
        return ret;
    }

    cp = bcpp.Fetch();

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        bcpp.Revert();
        return ret;
    }

    if (cp == ascii::CharCodeTable::e || cp == ascii::CharCodeTable::E) {
        state = StateEnum::SendingExpPartE;
        bcpp.Revert();
        return ret;
    }

    if ((IsTokenEnd)(cp)) {
        state = StateEnum::Finished;
    } else {
        state = StateEnum::Corrupted;
    }

    bcpp.Revert();

    return ret;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned char numeric_serde::deserialize::ReceiveExpPartE(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state) {
    ZETA_Core_DebugAssert(state == StateEnum::SendingExpPartE);

    unicode::unichar_t cp;

    ZETA_Core_DebugAssert(!bcpp.IsEnd());

    cp = bcpp.Fetch();

    ZETA_Core_DebugAssert(cp == ascii::CharCodeTable::e ||
                          cp == ascii::CharCodeTable::E);

    state = StateEnum::SendingExpPartSign;

    return static_cast<unsigned char>(cp);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr pair::Pair<unsigned char, bool>
numeric_serde::deserialize::ReceiveExpPartSign(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state) {
    ZETA_Core_DebugAssert(state == StateEnum::SendingExpPartSign);

    unicode::unichar_t cp;

    if (bcpp.IsEnd()) {
        state = StateEnum::Corrupted;
        return { ascii::CharCodeTable::empty, false };
    }

    cp = bcpp.Fetch();

    pair::Pair<unsigned char, bool> ret;

    if (cp == ascii::CharCodeTable::minus) {
        ret.first = ascii::CharCodeTable::minus;
        ret.second = true;

        if (bcpp.IsEnd()) {
            state = StateEnum::Corrupted;
            return ret;
        }

        cp = bcpp.Fetch();
    } else if (cp == ascii::CharCodeTable::plus) {
        ret.first = ascii::CharCodeTable::plus;
        ret.second = false;

        if (bcpp.IsEnd()) {
            state = StateEnum::Corrupted;
            return ret;
        }

        cp = bcpp.Fetch();
    } else {
        ret.first = ascii::CharCodeTable::empty;
        ret.second = false;
    }

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        state = StateEnum::SendingExpPartDigit;
    } else {
        state = StateEnum::Corrupted;
    }

    bcpp.Revert();

    return ret;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unsigned numeric_serde::deserialize::ReceiveExpPartDigit(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state) {
    ZETA_Core_DebugAssert(state == StateEnum::SendingExpPartDigit);

    unicode::unichar_t cp;

    ZETA_Core_DebugAssert(!bcpp.IsEnd());

    cp = bcpp.Fetch();

    ZETA_Core_DebugAssert(ascii::CharCodeTable::num_0 <= cp &&
                          cp <= ascii::CharCodeTable::num_9);

    unsigned ret{ cp - ascii::CharCodeTable::num_0 };

    if (bcpp.IsEnd()) {
        state = StateEnum::Finished;
        return ret;
    }

    cp = bcpp.Fetch();

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        bcpp.Revert();
        return ret;
    }

    if ((IsTokenEnd)(cp)) {
        state = StateEnum::Finished;
    } else {
        state = StateEnum::Corrupted;
    }

    bcpp.Revert();

    return ret;
}

}  // namespace zeta::core::json_utils
