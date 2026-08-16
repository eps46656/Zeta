#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/json_utils/string_serde.hpp>

namespace zeta::core::json_utils {

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void string_serde::Serializer::Init(this Serializer& self,
                                              CodepointAcceptor&& cpa) {
    unicode::unichar_t cp{ ascii::CharCodeTable::double_quote };

    elem_stream::acceptor::Transfer(cpa, &cp, sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    self.state = StateEnum::ReceivingCharOrFinish;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr bool string_serde::Serializer::SendChar(this Serializer& self,
                                                  CodepointAcceptor&& cpa,
                                                  unicode::unichar_t cp,
                                                  bool prefer_unicode_escape,
                                                  bool prefer_capital_hex) {
    ZETA_Core_DebugAssert(self.state == StateEnum::ReceivingCharOrFinish);

    if (0x10FFFF < cp || (0xD800 <= cp && cp <= 0xDFFF)) { return false; }

    unicode::unichar_t send_cp_buffer[12];

    bool hit_escape{ true };

    switch (cp) {
    case ascii::CharCodeTable::backspace:
        send_cp_buffer[1] = ascii::CharCodeTable::b;
        break;

    case ascii::CharCodeTable::horizontal_tab:
        send_cp_buffer[1] = ascii::CharCodeTable::t;
        break;

    case ascii::CharCodeTable::line_feed:
        send_cp_buffer[1] = ascii::CharCodeTable::n;
        break;

    case ascii::CharCodeTable::form_feed:
        send_cp_buffer[1] = ascii::CharCodeTable::f;
        break;

    case ascii::CharCodeTable::carriage_return:
        send_cp_buffer[1] = ascii::CharCodeTable::r;
        break;

    case ascii::CharCodeTable::double_quote:
        send_cp_buffer[1] = ascii::CharCodeTable::double_quote;
        break;

    case ascii::CharCodeTable::backslash:
        send_cp_buffer[1] = ascii::CharCodeTable::backslash;
        break;

    default: hit_escape = false; break;
    }

    if (hit_escape) {
        send_cp_buffer[0] = ascii::CharCodeTable::backslash;

        elem_stream::acceptor::Transfer(cpa, send_cp_buffer,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 2);

        return true;
    }

    auto to_hex_4{ [prefer_capital_hex](unicode::unichar_t* dst,
                                        unicode::unichar_t cp) {
        dst += 4;

        for (int i{ 0 }; i < 4; ++i, cp /= 16) {
            --dst;

            unicode::unichar_t digit{ cp % 16 };

            if (digit < 10) {
                *dst = ascii::CharCodeTable::num_0 + (cp % 16);
            } else if (prefer_capital_hex) {
                *dst = ascii::CharCodeTable::A + (digit - 10);
            } else {
                *dst = ascii::CharCodeTable::a + (digit - 10);
            }
        }
    } };

    if (cp <= 0x1F) { goto SINGLE_U_ESCAPE; }
    if (cp <= 0x7F) { goto DIRECT; }
    if (!prefer_unicode_escape) { goto DIRECT; }
    if (cp <= 0xFFFF) { goto SINGLE_U_ESCAPE; }
    goto DOUBLE_U_ESCAPE;

DIRECT: {
    elem_stream::acceptor::Transfer(cpa, &cp, sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);
    return true;
}

SINGLE_U_ESCAPE: {
    send_cp_buffer[0] = ascii::CharCodeTable::backslash;
    send_cp_buffer[1] = ascii::CharCodeTable::u;

    to_hex_4(send_cp_buffer + 2, cp);

    elem_stream::acceptor::Transfer(cpa, send_cp_buffer,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 6);

    return true;
}

DOUBLE_U_ESCAPE: {
    cp -= 0x10000;

    send_cp_buffer[0] = ascii::CharCodeTable::backslash;
    send_cp_buffer[1] = ascii::CharCodeTable::u;

    to_hex_4(send_cp_buffer + 2, cp / 0x400 + unicode::surrogate_h_range_min);

    send_cp_buffer[6] = ascii::CharCodeTable::backslash;
    send_cp_buffer[7] = ascii::CharCodeTable::u;

    to_hex_4(send_cp_buffer + 8, cp % 0x400 + unicode::surrogate_l_range_min);

    elem_stream::acceptor::Transfer(cpa, send_cp_buffer,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 12);

    return true;
}
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor,
          elem_stream::provider::IsProvider CodepointProvider>
constexpr pair::Pair<size_t, unicode::unichar_t>
string_serde::Serializer::SendChar(this Serializer& self,
                                   CodepointAcceptor&& cpa,
                                   CodepointProvider&& cpp,
                                   unicode::unichar_t max_cnt,
                                   bool prefer_unicode_escape,
                                   bool prefer_capital_hex) {
    unicode::unichar_t cp;

    size_t cnt{ 0 };

    for (; cnt < max_cnt;) {
        elem_stream::provider::Transfer(cpp, &cp, sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);

        if (!self.SendChar(cpa, cp, prefer_unicode_escape,
                           prefer_capital_hex)) {
            return { cnt, cp };
        }

        ++cnt;
    }

    return { cnt, null_cp };
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void string_serde::Serializer::SendFinish(this Serializer& self,
                                                    CodepointAcceptor&& cpa) {
    ZETA_Core_DebugAssert(self.state == StateEnum::ReceivingCharOrFinish);

    unicode::unichar_t cp{ ascii::CharCodeTable::double_quote };

    elem_stream::acceptor::Transfer(cpa, &cp, sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    self.state = StateEnum::Finished;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void string_serde::Deserializer::Init(
    this Deserializer& self,
    BufferedCodepointProvider<CodepointProvider>& bcpp) {
    if (bcpp.IsEnd()) {
        self.state = StateEnum::Corrupted;
    } else if (bcpp.Fetch() != ascii::CharCodeTable::double_quote) {
        self.state = StateEnum::Corrupted;
        bcpp.Revert();
    } else if (bcpp.IsEnd()) {
        self.state = StateEnum::Corrupted;
    } else if (bcpp.Fetch() == ascii::CharCodeTable::double_quote) {
        self.state = StateEnum::Finished;
    } else {
        self.state = StateEnum::SendingChar;
        bcpp.Revert();
    }
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unicode::unichar_t string_serde::Deserializer::ReceiveChar(
    this Deserializer& self,
    BufferedCodepointProvider<CodepointProvider>& bcpp) {
    ZETA_Core_DebugAssert(self.state == StateEnum::SendingChar);

    ZETA_Core_DebugAssert(!bcpp.IsEnd());

    unicode::unichar_t cp;

    cp = bcpp.Fetch();

    ZETA_Core_DebugAssert(cp != ascii::CharCodeTable::double_quote);

    auto look_ahead{ [&]() {
        if (bcpp.IsEnd()) {
            ZETA_Core_Debug_PrintCurPos;
            self.state = StateEnum::Corrupted;
        } else if (bcpp.Fetch() == ascii::CharCodeTable::double_quote) {
            self.state = StateEnum::Finished;
        } else {
            bcpp.Revert();
        }
    } };

    if (cp < 0x0020 || 0x10FFFF < cp) {
        self.state = StateEnum::Corrupted;
        bcpp.Revert();
        return null_cp;
    }

    if (cp != ascii::CharCodeTable::backslash) {
        look_ahead();
        return cp;
    }

    if (bcpp.IsEnd()) {
        ZETA_Core_Debug_PrintCurPos;
        self.state = StateEnum::Corrupted;
        return null_cp;
    }

    cp = bcpp.Fetch();

    if (cp < 0x0020 || 0x10FFFF < cp) {
        ZETA_Core_Debug_PrintCurPos;
        self.state = StateEnum::Corrupted;
        bcpp.Revert();
        return null_cp;
    }

    if (cp != ascii::CharCodeTable::u) {
        unicode::unichar_t ret;

        switch (cp) {
        case ascii::CharCodeTable::b:
            ret = ascii::CharCodeTable::backspace;
            break;

        case ascii::CharCodeTable::t:
            ret = ascii::CharCodeTable::horizontal_tab;
            break;

        case ascii::CharCodeTable::n:
            ret = ascii::CharCodeTable::line_feed;
            break;

        case ascii::CharCodeTable::f:
            ret = ascii::CharCodeTable::form_feed;
            break;

        case ascii::CharCodeTable::r:
            ret = ascii::CharCodeTable::carriage_return;
            break;

        case ascii::CharCodeTable::double_quote:
            ret = ascii::CharCodeTable::double_quote;
            break;

        case ascii::CharCodeTable::slash:
            ret = ascii::CharCodeTable::slash;
            break;

        case ascii::CharCodeTable::backslash:
            ret = ascii::CharCodeTable::backslash;
            break;

        default:
            ZETA_Core_Debug_PrintCurPos;
            self.state = StateEnum::Corrupted;
            bcpp.Revert();
            return null_cp;
        }

        look_ahead();

        return ret;
    }

    auto from_hex_4{ [&self, &bcpp]() {
        unicode::unichar_t ret{ 0 };

        for (int i{ 0 }; i < 4; ++i) {
            if (bcpp.IsEnd()) {
                ZETA_Core_Debug_PrintCurPos;
                self.state = StateEnum::Corrupted;
                return null_cp;
            }

            unicode::unichar_t cp{ bcpp.Fetch() };

            unicode::unichar_t k;

            if (ascii::CharCodeTable::num_0 <= cp &&
                cp <= ascii::CharCodeTable::num_9) {
                k = cp - ascii::CharCodeTable::num_0;
            } else if (ascii::CharCodeTable::a <= cp &&
                       cp <= ascii::CharCodeTable::f) {
                k = cp - ascii::CharCodeTable::a + 10;
            } else if (ascii::CharCodeTable::A <= cp &&
                       cp <= ascii::CharCodeTable::F) {
                k = cp - ascii::CharCodeTable::A + 10;
            } else {
                ZETA_Core_Debug_PrintCurPos;
                self.state = StateEnum::Corrupted;
                bcpp.Revert();
                return null_cp;
            }

            ret = ret * 16 + k;
        }

        return ret;
    } };

    unicode::unichar_t unichar_buffer_h{ from_hex_4() };

    if (self.state == StateEnum::Corrupted) { return null_cp; }

    if (unichar_buffer_h < unicode::surrogate_range_min ||
        unicode::surrogate_range_max < unichar_buffer_h) {
        look_ahead();
        return unichar_buffer_h;
    }

    if (unichar_buffer_h < unicode::surrogate_h_range_min ||
        unicode::surrogate_h_range_max < unichar_buffer_h || bcpp.IsEnd()) {
        ZETA_Core_Debug_PrintCurPos;
        self.state = StateEnum::Corrupted;
        return null_cp;
    }

    if (bcpp.Fetch() != ascii::CharCodeTable::backslash ||
        bcpp.Fetch() != ascii::CharCodeTable::u) {
        ZETA_Core_Debug_PrintCurPos;
        self.state = StateEnum::Corrupted;
        bcpp.Revert();
        return null_cp;
    }

    unicode::unichar_t unichar_buffer_l{ from_hex_4() };

    if (self.state == StateEnum::Corrupted) { return null_cp; }

    if (unichar_buffer_l < unicode::surrogate_l_range_min ||
        unicode::surrogate_l_range_max < unichar_buffer_l) {
        ZETA_Core_Debug_PrintCurPos;
        self.state = StateEnum::Corrupted;
        bcpp.Revert();
        return null_cp;
    }

    look_ahead();

    return 0x10000 +
           ((unichar_buffer_h - unicode::surrogate_h_range_min) << 10) +
           (unichar_buffer_l - unicode::surrogate_l_range_min);
}

template <elem_stream::provider::IsProvider CodepointProvider,
          elem_stream::provider::IsProvider CodepointAcceptor>
constexpr size_t string_serde::Deserializer::ReceiveChar(
    this Deserializer& self, BufferedCodepointProvider<CodepointProvider>& bcpp,
    CodepointAcceptor&& acceptor, size_t max_cnt) {
    size_t cnt{ 0 };

    for (; cnt < max_cnt; ++cnt) {
        unicode::unichar_t cp{ self.ReceiveChar(bcpp) };

        if (cp == null_cp) { return cnt; }

        elem_stream::provider::Transfer(acceptor, &cp,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);
    }

    return cnt;
}

}  // namespace zeta::core::json_utils
