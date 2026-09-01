#include <fstream>
#include <vector>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/json_utils.ipp>
#include <zeta/core/lin_seq_elem_stream.ipp>
#include <zeta/core/utf8.ipp>
#include <zeta/core/utils.hpp>
#include <zeta/core_test/random.hpp>

#define ZetaDir "D:/ZetaDevelops/ZetaDevelop/Zeta"

struct NumericInfo {
    bool is_ok;
    bool sign;
    bool has_frac;
    zeta::core::unicode::unichar_t e;
    zeta::core::unicode::unichar_t exp_part_sign;
    std::string int_part_digits;
    std::string frac_part_digits;
    std::string exp_part_digits;
};

constexpr bool operator==(NumericInfo const& lhs, NumericInfo const& rhs) {
    if (lhs.is_ok != rhs.is_ok) {
        ZETA_Core_Debug_PrintCurPos;
        return false;
    }

    if (lhs.sign != rhs.sign) {
        ZETA_Core_Debug_PrintCurPos;
        return false;
    }

    if (lhs.int_part_digits != rhs.int_part_digits) {
        ZETA_Core_Debug_PrintCurPos;
        return false;
    }

    if (lhs.has_frac != rhs.has_frac) {
        ZETA_Core_Debug_PrintCurPos;
        return false;
    }

    if (lhs.has_frac) {
        if (lhs.frac_part_digits != rhs.frac_part_digits) {
            ZETA_Core_Debug_PrintCurPos;
            return false;
        }
    }

    if (lhs.e != rhs.e) { return false; }

    if (lhs.e != zeta::core::unicode::null_codepoint) {
        if (lhs.exp_part_sign != rhs.exp_part_sign) {
            ZETA_Core_Debug_PrintVar(lhs.exp_part_sign);
            ZETA_Core_Debug_PrintVar(rhs.exp_part_sign);

            ZETA_Core_Debug_PrintCurPos;
            return false;
        }

        if (lhs.exp_part_digits != rhs.exp_part_digits) {
            ZETA_Core_Debug_PrintCurPos;
            return false;
        }
    }

    return true;
}

constexpr bool operator!=(NumericInfo const& lhs, NumericInfo const& rhs) {
    return !(lhs == rhs);
}

struct JsonNode {
    enum struct TypeEnum : unsigned char {
        Null = 0,
        Boolean = 1,
        Numeric = 2,
        String = 3,
        Array = 4,
        Object = 5,
    };

    TypeEnum type;

    union {
        bool boolean_value;
        NumericInfo numeric_info;
        std::deque<zeta::core::unicode::unichar_t> string_value;
        std::deque<JsonNode*> array_value;
        std::deque<std::pair<JsonNode*, JsonNode*>> object_value;
    };

    constexpr JsonNode(TypeEnum type) : type{ type } {
        switch (type) {
        case TypeEnum::Null: break;

        case TypeEnum::Boolean: break;

        case TypeEnum::Numeric:
            new (&this->numeric_info) decltype(this->numeric_info);
            break;

        case TypeEnum::String:
            new (&string_value) decltype(this->string_value);
            break;

        case TypeEnum::Array:
            new (&this->array_value) decltype(this->array_value);
            break;

        case TypeEnum::Object:
            new (&this->object_value) decltype(this->object_value);
            break;
        }
    }

    constexpr ~JsonNode() {
        switch (this->type) {
        case TypeEnum::Null: break;

        case TypeEnum::Boolean: break;

        case TypeEnum::Numeric: {
            this->numeric_info.~NumericInfo();
            break;
        }

        case TypeEnum::String:
            this->string_value.~decltype(this->string_value)();
            break;

        case TypeEnum::Array: {
            for (auto* elem : this->array_value) { elem->~JsonNode(); }

            this->array_value.~decltype(this->array_value)();

            break;
        }

        case TypeEnum::Object: {
            for (auto& [key, value] : this->object_value) {
                key->~JsonNode();
                value->~JsonNode();
            }

            this->object_value.~decltype(this->object_value)();

            break;
        }
        }
    }
};

template <>
struct zeta::core::debug_utils::VarPrinter<NumericInfo> {
    static std::ostream& Print(std::ostream& os,
                               NumericInfo const& numeric_info) {
        if (!numeric_info.is_ok) { return os << "Corrupted"; }

        if (numeric_info.sign) { os << "-"; }

        os << numeric_info.int_part_digits;

        if (numeric_info.has_frac) {
            os << "." << numeric_info.frac_part_digits;
        }

        if (numeric_info.e == 'e' || numeric_info.e == 'E') {
            os << static_cast<char>(numeric_info.e)
               << static_cast<char>(numeric_info.exp_part_sign)
               << numeric_info.exp_part_digits;
        }

        return os;
    }
};

constexpr bool operator==(JsonNode const& lhs, JsonNode const& rhs) {
    if (&lhs == &rhs) { return true; }

    if (lhs.type != rhs.type) {
        ZETA_Core_Debug_PrintCurPos;
        return false;
    }

    switch (lhs.type) {
    case JsonNode::TypeEnum::Null: return true;
    case JsonNode::TypeEnum::Boolean:
        if (lhs.boolean_value == rhs.boolean_value) {
            return true;
        } else {
            ZETA_Core_Debug_PrintCurPos;
            return false;
        }
    case JsonNode::TypeEnum::Numeric:
        if (lhs.numeric_info == rhs.numeric_info) {
            return true;
        } else {
            ZETA_Core_Debug_PrintCurPos;
            return false;
        }
    case JsonNode::TypeEnum::String:
        if (lhs.string_value == rhs.string_value) {
            return true;
        } else {
            ZETA_Core_Debug_PrintCurPos;
            return false;
        }
    case JsonNode::TypeEnum::Array: {
        if (lhs.array_value.size() != rhs.array_value.size()) {
            ZETA_Core_Debug_PrintCurPos;
            return false;
        }

        for (size_t i{ 0 }; i < lhs.array_value.size(); ++i) {
            if (*lhs.array_value[i] != *rhs.array_value[i]) {
                ZETA_Core_Debug_PrintCurPos;
                return false;
            }
        }

        return true;
    }

    case JsonNode::TypeEnum::Object: {
        if (lhs.object_value.size() != rhs.object_value.size()) {
            ZETA_Core_Debug_PrintCurPos;
            return false;
        }

        for (size_t i{ 0 }; i < lhs.object_value.size(); ++i) {
            if (*lhs.object_value[i].first != *rhs.object_value[i].first) {
                ZETA_Core_Debug_PrintCurPos;
                return false;
            }

            if (*lhs.object_value[i].second != *rhs.object_value[i].second) {
                ZETA_Core_Debug_PrintCurPos;
                return false;
            }
        }

        return true;
    }
    }
}

constexpr bool operator!=(JsonNode const& lhs, JsonNode const& rhs) {
    return !(lhs == rhs);
}

constexpr std::string GenRanomDigits(bool allow_leading_zero) {
    size_t len{ zeta::core_test::GetRandomInt<size_t, size_t>(1, 48) };

    std::string ret;
    ret.resize(len);

    ret[0] = static_cast<char>('0' + zeta::core_test::GetRandomInt<int, int>(
                                         allow_leading_zero ? 0 : 1, 9));

    for (size_t i{ 1 }; i < len; ++i) {
        ret[i] = static_cast<char>(
            '0' + zeta::core_test::GetRandomInt<int, int>(0, 9));
    }

    return ret;
}

constexpr NumericInfo GenRandomNumericInfo() {
    NumericInfo numeric_info;

    numeric_info.is_ok = true;

    numeric_info.sign = zeta::core_test::GetRandomInt<int, int>(0, 1) == 1;

    numeric_info.has_frac = zeta::core_test::GetRandomInt<int, int>(0, 1) == 1;

    numeric_info.int_part_digits = GenRanomDigits(false);

    if (numeric_info.has_frac) {
        numeric_info.frac_part_digits = GenRanomDigits(true);
    }

    switch (zeta::core_test::GetRandomInt<int, int>(0, 2)) {
    case 0: numeric_info.e = zeta::core::unicode::null_codepoint; break;
    case 1: numeric_info.e = zeta::core::ascii::CharCodeTable::e; break;
    case 2: numeric_info.e = zeta::core::ascii::CharCodeTable::E; break;
    }

    if (numeric_info.e == zeta::core::unicode::null_codepoint) {
        numeric_info.exp_part_sign = zeta::core::unicode::null_codepoint;
    } else {
        switch (zeta::core_test::GetRandomInt<int, int>(0, 2)) {
        case 0:
            numeric_info.exp_part_sign = zeta::core::unicode::null_codepoint;
            break;
        case 1:
            numeric_info.exp_part_sign = zeta::core::ascii::CharCodeTable::plus;
            break;
        case 2:
            numeric_info.exp_part_sign =
                zeta::core::ascii::CharCodeTable::minus;
            break;
        }

        numeric_info.exp_part_digits = GenRanomDigits(true);
    }

    return numeric_info;
}

constexpr void MakeCorrupted_EmptyIntPartDigit(NumericInfo& numeric_info) {
    numeric_info.int_part_digits.clear();
}

constexpr void MakeCorrupted_EmptyFracPartDigit(NumericInfo& numeric_info) {
    numeric_info.frac_part_digits.clear();
}

constexpr std::deque<size_t> RandomPartition(size_t total, size_t part_cnt) {
    ZETA_Core_DebugAssert(0 < part_cnt);
    ZETA_Core_DebugAssert(part_cnt <= total);

    if (part_cnt == 1) { return std::deque<size_t>{ total }; }

    std::deque<size_t> cnts;
    cnts.resize(part_cnt - 1);

    for (size_t i{ 0 }; i < part_cnt - 1; ++i) {
        cnts[i] =
            zeta::core_test::GetRandomInt<size_t, size_t>(0, total - part_cnt);
    }

    std::sort(cnts.begin(), cnts.end());

    std::deque<size_t> partition;
    partition.resize(part_cnt);

    partition[0] = cnts[0] + 1;

    for (size_t i{ 1 }; i < part_cnt - 1; ++i) {
        partition[i] = cnts[i] - cnts[i - 1] + 1;
    }

    partition[part_cnt - 1] = total - part_cnt - cnts[part_cnt - 2] + 1;

    return partition;
}

constexpr JsonNode* GenRandomJsonNode(size_t energy);

constexpr JsonNode* GenRandomNullJsonNode() {
    ZETA_Core_Debug_PrintCurPos;

    return new JsonNode{ JsonNode::TypeEnum::Null };
}

constexpr JsonNode* GenRandomBooleanJsonNode() {
    ZETA_Core_Debug_PrintCurPos;

    JsonNode* json_node{ new JsonNode{ JsonNode::TypeEnum::Boolean } };

    json_node->boolean_value =
        zeta::core_test::GetRandomInt<int, int>(0, 1) == 1;

    return json_node;
}

constexpr JsonNode* GenRandomNumericJsonNode() {
    ZETA_Core_Debug_PrintCurPos;

    NumericInfo numeric_info{ GenRandomNumericInfo() };

    JsonNode* json_node{ new JsonNode{ JsonNode::TypeEnum::Numeric } };

    json_node->numeric_info = numeric_info;

    return json_node;
}

constexpr zeta::core::unicode::unichar_t GenRandomUnicodeChar() {
    while (true) {
        int level{ zeta::core_test::GetRandomInt<int, int>(1, 4) };

        zeta::core::unicode::unichar_t cp{ zeta::core_test::GetRandomInt<
            zeta::core::unicode::unichar_t, zeta::core::unicode::unichar_t>(
            zeta::core::utf8::range_mins[level],
            zeta::core::utf8::range_maxs[level]) };

        if (cp < zeta::core::unicode::surrogate_range_min ||
            zeta::core::unicode::surrogate_range_max < cp) {
            return cp;
        }
    }
}

constexpr JsonNode* GenRandomStringJsonNode() {
    JsonNode* json_node{ new JsonNode{ JsonNode::TypeEnum::String } };

    size_t len{ zeta::core_test::GetRandomInt<size_t, size_t>(1, 32) };

    json_node->string_value.resize(len);

    for (size_t i{ 0 }; i < len; ++i) {
        json_node->string_value[i] = (GenRandomUnicodeChar)();
    }

    return json_node;
}

constexpr JsonNode* GenRandomArrayJsonNode(size_t energy) {
    ZETA_Core_DebugAssert(0 < energy);

    JsonNode* json_node{ new JsonNode{ JsonNode::TypeEnum::Array } };

    size_t len{ zeta::core_test::GetRandomInt<size_t, size_t>(
        1, std::min<size_t>(8, energy)) };

    std::deque<size_t> partition{ (RandomPartition)(energy, len) };

    json_node->array_value.resize(len);

    for (size_t i{ 0 }; i < len; ++i) {
        json_node->array_value[i] = (GenRandomJsonNode)(partition[i]);
    }

    return json_node;
}

constexpr JsonNode* GenRandomObjectJsonNode(size_t energy) {
    ZETA_Core_DebugAssert(0 < energy);

    JsonNode* json_node{ new JsonNode{ JsonNode::TypeEnum::Object } };

    size_t len{ zeta::core_test::GetRandomInt<size_t, size_t>(
        1, std::min<size_t>(8, energy)) };

    std::deque<size_t> partition{ (RandomPartition)(energy, len) };

    json_node->object_value.resize(len);

    for (size_t i{ 0 }; i < len; ++i) {
        json_node->object_value[i].first = (GenRandomStringJsonNode)();
        json_node->object_value[i].second = (GenRandomJsonNode)(partition[i]);
    }

    return json_node;
}

constexpr JsonNode* GenRandomJsonNode(size_t energy) {
    ZETA_Core_DebugAssert(0 < energy);

    switch (zeta::core_test::GetRandomInt<int, int>(energy == 1 ? 0 : 4, 5)) {
    case 0: return GenRandomNullJsonNode();
    case 1: return GenRandomBooleanJsonNode();
    case 2: return GenRandomNumericJsonNode();
    case 3: return GenRandomStringJsonNode();
    case 4: return GenRandomArrayJsonNode(energy);
    case 5: return GenRandomObjectJsonNode(energy);
    default: ZETA_Core_Unreachable();
    }
}

constexpr std::string NumericInfoToString(NumericInfo const& numeric_info) {
    std::string ret;

    if (numeric_info.sign) { ret += '-'; }

    ret.append(numeric_info.int_part_digits);

    if (numeric_info.has_frac) {
        ret.push_back('.');
        ret.append(numeric_info.frac_part_digits);
    }

    if (numeric_info.e != zeta::core::unicode::null_codepoint) {
        ret.push_back(static_cast<char>(numeric_info.e));

        if (numeric_info.exp_part_sign != zeta::core::unicode::null_codepoint) {
            ret.push_back(static_cast<char>(numeric_info.exp_part_sign));
        }

        ret.append(numeric_info.exp_part_digits);
    }

    return ret;
}

/*
constexpr NumericInfo ParseNumeric(std::string const& numeric_str) {
    size_t len{ numeric_str.size() };

    std::vector<zeta::core::unicode::unichar_t> test_string_cp;
    test_string_cp.resize(len);

    for (size_t i{ 0 }; i < len; ++i) {
        test_string_cp[i] = static_cast<zeta::core::unicode::unichar_t>(
            static_cast<unsigned char>(numeric_str[i]));
    }

    zeta::core::lin_seq_elem_stream::Provider test_string_cp_provider{
        .data = test_string_cp.data(),
        .elem_size = sizeof(zeta::core::unicode::unichar_t),
        .elem_stride = sizeof(zeta::core::unicode::unichar_t),
        .elem_cnt = len,
    };

    zeta::core::json_utils::BufferedCodepointProvider<
        zeta::core::lin_seq_elem_stream::Provider>
        bcpp{ test_string_cp_provider };

    NumericInfo numeric_info;

    numeric_info.has_frac = false;
    numeric_info.e = zeta::core::unicode::null_codepoint;
    numeric_info.exp_part_sign = zeta::core::unicode::null_codepoint;

    struct {
        size_t sending_sign{ 0 };
        size_t sending_int_part_digit{ 0 };
        size_t sending_frac_part_digit{ 0 };
        size_t sending_exp_part_e{ 0 };
        size_t sending_exp_part_sign{ 0 };
        size_t sending_exp_part_digit{ 0 };
    } state_cnt;

    zeta::core::json_utils::numeric_endec::dec::State deserialize_state{
        zeta::core::json_utils::numeric_endec::dec::State::SendingSign
    };

    while (true) {
        switch (deserialize_state) {
        case zeta::core::json_utils::numeric_endec::dec::State::SendingSign: {
            ZETA_Core_DebugAssert(state_cnt.sending_sign == 0);
            ZETA_Core_DebugAssert(state_cnt.sending_int_part_digit == 0);
            ZETA_Core_DebugAssert(state_cnt.sending_frac_part_digit == 0);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_e == 0);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_sign == 0);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_digit == 0);

            ++state_cnt.sending_sign;

            numeric_info.sign =
                zeta::core::json_utils::numeric_endec::dec::ReceiveSign(
                    bcpp, deserialize_state)
                    .GetValue();

            break;
        }

        case zeta::core::json_utils::numeric_endec::dec::State::
            SendingIntPartDigitLead:
        case zeta::core::json_utils::numeric_endec::dec::State::
            SendingIntPartDigitTail: {
            ZETA_Core_DebugAssert(state_cnt.sending_sign == 1);
            ZETA_Core_DebugAssert(state_cnt.sending_frac_part_digit == 0);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_e == 0);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_sign == 0);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_digit == 0);

            ++state_cnt.sending_int_part_digit;

            numeric_info.int_part_digits.push_back(static_cast<char>(
                '0' +
                zeta::core::json_utils::numeric_endec::dec::ReceiveIntPartDigit(
                    bcpp, deserialize_state)
                    .GetValue()));

            break;
        }

        case zeta::core::json_utils::numeric_endec::dec::State::
            SendingFracPartDigit: {
            ZETA_Core_DebugAssert(state_cnt.sending_sign == 1);
            ZETA_Core_DebugAssert(0 < state_cnt.sending_int_part_digit);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_e == 0);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_sign == 0);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_digit == 0);

            ++state_cnt.sending_frac_part_digit;

            numeric_info.has_frac = true;

            numeric_info.frac_part_digits.push_back(static_cast<char>(
                '0' + zeta::core::json_utils::numeric_endec::dec::
                          ReceiveFracPartDigit(bcpp, deserialize_state)
                              .GetValue()));

            break;
        }

        case zeta::core::json_utils::numeric_endec::dec::State::
            SendingExpPartE: {
            ZETA_Core_DebugAssert(state_cnt.sending_sign == 1);
            ZETA_Core_DebugAssert(0 < state_cnt.sending_int_part_digit);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_e == 0);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_sign == 0);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_digit == 0);

            ++state_cnt.sending_exp_part_e;

            numeric_info.e =
                zeta::core::json_utils::numeric_endec::dec::ReceiveExpPartE(
                    bcpp, deserialize_state)
                    .GetValue();

            break;
        }

        case zeta::core::json_utils::numeric_endec::dec::State::
            SendingExpPartSign: {
            ZETA_Core_DebugAssert(state_cnt.sending_sign == 1);
            ZETA_Core_DebugAssert(0 < state_cnt.sending_int_part_digit);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_e == 1);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_sign == 0);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_digit == 0);

            ++state_cnt.sending_exp_part_sign;

            numeric_info.exp_part_sign =
                zeta::core::json_utils::numeric_endec::dec::ReceiveExpPartSign(
                    bcpp, deserialize_state)
                    .GetValue();

            break;
        }

        case zeta::core::json_utils::numeric_endec::dec::State::
            SendingExpPartDigit: {
            ZETA_Core_DebugAssert(state_cnt.sending_sign == 1);
            ZETA_Core_DebugAssert(0 < state_cnt.sending_int_part_digit);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_e == 1);
            ZETA_Core_DebugAssert(state_cnt.sending_exp_part_sign == 1);

            ++state_cnt.sending_exp_part_digit;

            numeric_info.exp_part_digits.push_back(static_cast<char>(
                '0' +
                zeta::core::json_utils::numeric_endec::dec::ReceiveExpPartDigit(
                    bcpp, deserialize_state)
                    .GetValue()));

            break;
        }

        case zeta::core::json_utils::numeric_endec::dec::State::Finished: {
            numeric_info.is_ok = true;

            ZETA_Core_DebugAssert(state_cnt.sending_sign == 1);
            ZETA_Core_DebugAssert(0 < state_cnt.sending_int_part_digit);

            ZETA_Core_DebugAssert(bcpp.IsEnd());

            return numeric_info;
        }

        case zeta::core::json_utils::numeric_endec::dec::State::Corrupted:

            numeric_info.is_ok = false;
            return numeric_info;
        }
    }
}
*/

namespace decode_json_text_to_json_node {

std::stringstream json_log_ss;
std::string json_log_indent_str{ "    " };
std::string json_log_acc_indent_str;

template <zeta::core::elem_stream::provider::IsProvider CodepointProvider>
constexpr JsonNode* ReceiveValue(
    zeta::core::json_utils::Decoder<CodepointProvider>& decoder);

template <zeta::core::elem_stream::provider::IsProvider CodepointProvider>
constexpr JsonNode* ReceiveNull(
    zeta::core::json_utils::Decoder<CodepointProvider>& decoder) {
    ZETA_Core_DebugAssert(0 < decoder.depth);

    ZETA_Core_DebugAssert(decoder.GetState() ==
                          zeta::core::json_utils::DecState::SendingNull);

    JsonNode* json_node{ new JsonNode{ JsonNode::TypeEnum::Null } };

    decoder.ReceiveNull().CheckHasValue();

    json_log_ss << "null";

    return json_node;
}

template <zeta::core::elem_stream::provider::IsProvider CodepointProvider>
constexpr JsonNode* ReceiveBoolean(
    zeta::core::json_utils::Decoder<CodepointProvider>& decoder) {
    ZETA_Core_DebugAssert(0 < decoder.depth);

    ZETA_Core_DebugAssert(decoder.GetState() ==
                          zeta::core::json_utils::DecState::SendingBoolean);

    auto try_result{ decoder.ReceiveBoolean() };

    if (!try_result.HasValue()) { return nullptr; }

    JsonNode* json_node{ new JsonNode{ JsonNode::TypeEnum::Boolean } };

    json_node->boolean_value = try_result.GetValue();

    json_log_ss << (json_node->boolean_value ? "true" : "false");

    return json_node;
}

template <zeta::core::elem_stream::provider::IsProvider CodepointProvider>
constexpr JsonNode* ReceiveNumeric(
    zeta::core::json_utils::Decoder<CodepointProvider>& decoder) {
    ZETA_Core_DebugAssert(0 < decoder.depth);

    ZETA_Core_DebugAssert(decoder.GetState() ==
                          zeta::core::json_utils::DecState::SendingNumericSign);

    JsonNode* json_node{ new JsonNode{ JsonNode::TypeEnum::Numeric } };
    json_node->numeric_info.has_frac = false;
    json_node->numeric_info.e = zeta::core::unicode::null_codepoint;
    json_node->numeric_info.exp_part_sign = zeta::core::unicode::null_codepoint;

    while (true) {
        ZETA_Core_Debug_PrintVar(
            zeta::core::meta::ToUnderlying(decoder.GetState()));

        switch (decoder.GetState()) {
        case zeta::core::json_utils::DecState::SendingNumericSign: {
            auto ret{ decoder.ReceiveNumericSign() };

            if (ret.HasValue()) {
                json_node->numeric_info.sign = ret.GetValue();
            }

            break;
        }

        case zeta::core::json_utils::DecState::SendingNumericIntPartDigit: {
            auto ret{ decoder.ReceiveNumericIntPartDigit() };

            if (!ret.HasValue()) { break; }

            json_node->numeric_info.int_part_digits.push_back(
                static_cast<char>('0' + ret.GetValue()));

            break;
        }

        case zeta::core::json_utils::DecState::SendingNumericFracPartDigit: {
            auto ret{ decoder.ReceiveNumericFracPartDigit() };

            if (!ret.HasValue()) { break; }

            json_node->numeric_info.has_frac = true;

            json_node->numeric_info.frac_part_digits.push_back(
                static_cast<char>('0' + ret.GetValue()));

            break;
        }

        case zeta::core::json_utils::DecState::SendingNumericExpPartE: {
            auto ret{ decoder.ReceiveNumericExpPartE() };

            if (!ret.HasValue()) { break; }

            json_node->numeric_info.e = ret.GetValue();

            ZETA_Core_Debug_PrintVar(&json_node->numeric_info.e);
            ZETA_Core_Debug_PrintVar(
                static_cast<char>(json_node->numeric_info.e));

            break;
        }

        case zeta::core::json_utils::DecState::SendingNumericExpPartSign: {
            auto ret{ decoder.ReceiveNumericExpPartSign() };

            if (!ret.HasValue()) { break; }

            json_node->numeric_info.exp_part_sign = ret.GetValue();

            break;
        }

        case zeta::core::json_utils::DecState::SendingNumericExpPartDigit: {
            auto ret{ decoder.ReceiveNumericExpPartDigit() };

            if (!ret.HasValue()) { break; }

            json_node->numeric_info.exp_part_digits.push_back(
                static_cast<char>('0' + ret.GetValue()));

            break;
        }

        case zeta::core::json_utils::DecState::SendingNumericFinish: {
            json_node->numeric_info.is_ok = true;
            decoder.ReceiveFinish().CheckHasValue();

            zeta::core::debug_utils::VarPrinter<NumericInfo>::Print(
                json_log_ss, json_node->numeric_info);

            return json_node;
        }

        default: ZETA_Core_Unreachable();
        }
    }
}

template <zeta::core::elem_stream::provider::IsProvider CodepointProvider>
constexpr JsonNode* ReceiveString(
    zeta::core::json_utils::Decoder<CodepointProvider>& decoder) {
    ZETA_Core_DebugAssert(0 < decoder.depth);

    ZETA_Core_DebugAssert(decoder.GetState() ==
                          zeta::core::json_utils::DecState::SendingStringStart);

    if (!decoder.ReceiveStringStart().HasValue()) { return nullptr; }

    JsonNode* json_node{ new JsonNode{ JsonNode::TypeEnum::String } };

    while (true) {
        ZETA_Core_Debug_PrintVar(
            zeta::core::meta::ToUnderlying(decoder.GetState()));

        switch (decoder.GetState()) {
        case zeta::core::json_utils::DecState::SendingStringChar: {
            auto ret{ decoder.ReceiveStringChar() };

            if (!ret.HasValue()) { break; }

            json_node->string_value.push_back(ret.GetValue());

            break;
        }

        case zeta::core::json_utils::DecState::SendingStringFinish:
            decoder.ReceiveFinish().CheckHasValue();

            json_log_ss << '"';

            for (auto const& ch : json_node->string_value) {
                json_log_ss << static_cast<char>(ch);
            }

            json_log_ss << '"';

            return json_node;

        default: ZETA_Core_Unreachable();
        }
    }
}

template <zeta::core::elem_stream::provider::IsProvider CodepointProvider>
constexpr JsonNode* ReceiveArray(
    zeta::core::json_utils::Decoder<CodepointProvider>& decoder) {
    ZETA_Core_DebugAssert(0 < decoder.depth);

    ZETA_Core_DebugAssert(decoder.GetState() ==
                          zeta::core::json_utils::DecState::SendingArrayStart);

    if (!decoder.ReceiveArrayStart().HasValue()) { return nullptr; }

    JsonNode* json_node{ new JsonNode{ JsonNode::TypeEnum::Array } };

    json_log_ss << "[";
    json_log_acc_indent_str.append(json_log_indent_str);

    while (true) {
        ZETA_Core_Debug_PrintVar(
            zeta::core::meta::ToUnderlying(decoder.GetState()));

        switch (decoder.GetState()) {
        case zeta::core::json_utils::DecState::SendingArrayElem: {
            if (!decoder.ReceiveArrayElem().HasValue()) { break; }

            if (!json_node->array_value.empty()) { json_log_ss << ","; }

            json_log_ss << "\n" << json_log_acc_indent_str;

            json_node->array_value.push_back(ReceiveValue(decoder));

            break;
        }

        case zeta::core::json_utils::DecState::SendingArrayFinish: {
            decoder.ReceiveFinish().CheckHasValue();

            json_log_acc_indent_str.resize(json_log_acc_indent_str.size() -
                                           json_log_indent_str.size());

            if (!json_node->array_value.empty()) {
                json_log_ss << "\n" << json_log_acc_indent_str;
            }

            json_log_ss << "]";

            return json_node;
        }

        default: ZETA_Core_Unreachable();
        }
    }
}

template <zeta::core::elem_stream::provider::IsProvider CodepointProvider>
constexpr JsonNode* ReceiveObject(
    zeta::core::json_utils::Decoder<CodepointProvider>& decoder) {
    ZETA_Core_DebugAssert(0 < decoder.depth);

    ZETA_Core_DebugAssert(decoder.GetState() ==
                          zeta::core::json_utils::DecState::SendingObjectStart);

    if (!decoder.ReceiveObjectStart().HasValue()) { return nullptr; }

    JsonNode* json_node{ new JsonNode{ JsonNode::TypeEnum::Object } };

    json_log_ss << "{";
    json_log_acc_indent_str.append(json_log_indent_str);

    while (true) {
        ZETA_Core_Debug_PrintVar(
            zeta::core::meta::ToUnderlying(decoder.GetState()));

        switch (decoder.GetState()) {
        case zeta::core::json_utils::DecState::SendingObjectKey: {
            {
                auto result{ decoder.ReceiveObjectKey() };

                if (!result.HasValue()) {
                    ZETA_Core_Debug_PrintVar(
                        static_cast<unsigned char>(result.GetReason()));
                    break;
                }
            }

            if (!json_node->object_value.empty()) { json_log_ss << ","; }

            json_log_ss << "\n" << json_log_acc_indent_str;

            ZETA_Core_Debug_PrintVar(
                zeta::core::meta::ToUnderlying(decoder.GetState()));

            json_node->object_value.push_back(std::pair<JsonNode*, JsonNode*>{
                ReceiveValue(decoder), nullptr });

            json_log_ss << ": ";

            break;
        }

        case zeta::core::json_utils::DecState::SendingObjectValue: {
            if (!decoder.ReceiveObjectValue().HasValue()) { break; }

            ZETA_Core_DebugAssert(!json_node->object_value.empty());
            ZETA_Core_DebugAssert(json_node->object_value.back().second ==
                                  nullptr);

            json_node->object_value.back().second = ReceiveValue(decoder);

            break;
        }

        case zeta::core::json_utils::DecState::SendingObjectFinish: {
            decoder.ReceiveFinish().CheckHasValue();

            json_log_acc_indent_str.resize(json_log_acc_indent_str.size() -
                                           json_log_indent_str.size());

            if (!json_node->object_value.empty()) {
                json_log_ss << "\n" << json_log_acc_indent_str;
            }

            json_log_ss << "}";

            return json_node;
        }

        default: ZETA_Core_Unreachable();
        }
    }
}

template <zeta::core::elem_stream::provider::IsProvider CodepointProvider>
constexpr JsonNode* ReceiveValue(
    zeta::core::json_utils::Decoder<CodepointProvider>& decoder) {
    ZETA_Core_DebugAssert(0 < decoder.depth);

    ZETA_Core_Debug_PrintVar(
        zeta::core::meta::ToUnderlying(decoder.GetState()));

    JsonNode* json_node;

    switch (decoder.GetState()) {
    case zeta::core::json_utils::DecState::SendingNull:
        json_node = ReceiveNull(decoder);
        break;

    case zeta::core::json_utils::DecState::SendingBoolean:
        json_node = ReceiveBoolean(decoder);
        break;

    case zeta::core::json_utils::DecState::SendingNumericSign:
        json_node = ReceiveNumeric(decoder);
        break;

    case zeta::core::json_utils::DecState::SendingStringStart:
        json_node = ReceiveString(decoder);
        break;

    case zeta::core::json_utils::DecState::SendingArrayStart:
        json_node = ReceiveArray(decoder);
        break;

    case zeta::core::json_utils::DecState::SendingObjectStart:
        json_node = ReceiveObject(decoder);
        break;

    case zeta::core::json_utils::DecState::Finished: return nullptr;

    default: ZETA_Core_Unreachable();
    }

    ZETA_Core_Debug_PrintVar(json_node);

    if (json_node != nullptr) {
        ZETA_Core_Debug_PrintVar(static_cast<unsigned>(json_node->type));
    }

    return json_node;
}

}  // namespace decode_json_text_to_json_node

namespace encode_json_node_to_json_text {

template <zeta::core::elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendValue(
    zeta::core::json_utils::Encoder<CodepointAcceptor>& serializer,
    JsonNode* json_node);

template <zeta::core::elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendNull(
    zeta::core::json_utils::Encoder<CodepointAcceptor>& serializer,
    JsonNode* json_node) {
    ZETA_Core_DebugAssert(json_node->type == JsonNode::TypeEnum::Null);

    ZETA_Core_Debug_PrintCurPos;

    serializer.SendNull().CheckHasValue();
}

template <zeta::core::elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendBoolean(
    zeta::core::json_utils::Encoder<CodepointAcceptor>& serializer,
    JsonNode* json_node) {
    ZETA_Core_DebugAssert(json_node->type == JsonNode::TypeEnum::Boolean);

    ZETA_Core_Debug_PrintCurPos;

    serializer.SendBoolean(json_node->boolean_value).CheckHasValue();
}

template <zeta::core::elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendNeumeric(
    zeta::core::json_utils::Encoder<CodepointAcceptor>& serializer,
    JsonNode* json_node) {
    ZETA_Core_DebugAssert(json_node->type == JsonNode::TypeEnum::Numeric);

    ZETA_Core_Debug_PrintCurPos;

    ZETA_Core_Debug_PrintCurPos;

    serializer.SendNumericSign(json_node->numeric_info.sign).CheckHasValue();

    for (char d : json_node->numeric_info.int_part_digits) {
        ZETA_Core_Debug_PrintCurPos;

        serializer.SendNumericIntPartDigit(static_cast<unsigned char>(d - '0'))
            .CheckHasValue();
    }

    ZETA_Core_Debug_PrintVar(json_node->numeric_info.has_frac);

    if (json_node->numeric_info.has_frac) {
        for (char d : json_node->numeric_info.frac_part_digits) {
            ZETA_Core_Debug_PrintCurPos;

            ZETA_Core_Debug_PrintVar(static_cast<unsigned char>(d - '0'));

            serializer
                .SendNumericFracPartDigit(static_cast<unsigned char>(d - '0'))
                .CheckHasValue();
        }
    }

    ZETA_Core_Debug_PrintVar(json_node->numeric_info.e);

    if (json_node->numeric_info.e != zeta::core::unicode::null_codepoint) {
        ZETA_Core_Debug_PrintCurPos;

        ZETA_Core_Debug_PrintVar(json_node);
        ZETA_Core_Debug_PrintVar(&json_node->numeric_info.e);
        ZETA_Core_Debug_PrintVar(json_node->numeric_info.e);

        serializer.SendNumericExpPartE(json_node->numeric_info.e)
            .CheckHasValue();

        if (json_node->numeric_info.exp_part_sign !=
            zeta::core::unicode::null_codepoint) {
            ZETA_Core_Debug_PrintCurPos;

            serializer
                .SendNumericExpPartSign(json_node->numeric_info.exp_part_sign)
                .CheckHasValue();
        }

        for (char d : json_node->numeric_info.exp_part_digits) {
            ZETA_Core_Debug_PrintCurPos;

            serializer
                .SendNumericExpPartDigit(static_cast<unsigned char>(d - '0'))
                .CheckHasValue();
        }
    }
    ZETA_Core_Debug_PrintCurPos;

    serializer.SendFinish().CheckHasValue();
}

template <zeta::core::elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendString(
    zeta::core::json_utils::Encoder<CodepointAcceptor>& serializer,
    JsonNode* json_node) {
    ZETA_Core_DebugAssert(json_node->type == JsonNode::TypeEnum::String);

    ZETA_Core_Debug_PrintCurPos;

    serializer.SendStringStart().CheckHasValue();

    for (zeta::core::unicode::unichar_t cp : json_node->string_value) {
        ZETA_Core_Debug_PrintCurPos;

        serializer.SendStringChar(cp).CheckHasValue();
    }

    ZETA_Core_Debug_PrintCurPos;

    serializer.SendFinish().CheckHasValue();
}

template <zeta::core::elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendArray(
    zeta::core::json_utils::Encoder<CodepointAcceptor>& serializer,
    JsonNode* json_node) {
    ZETA_Core_DebugAssert(json_node->type == JsonNode::TypeEnum::Array);

    ZETA_Core_Debug_PrintCurPos;

    serializer.SendArrayStart().CheckHasValue();

    for (JsonNode* elem : json_node->array_value) {
        ZETA_Core_Debug_PrintCurPos;

        (SendValue)(serializer, elem);
    }

    ZETA_Core_Debug_PrintCurPos;

    serializer.SendFinish().CheckHasValue();
}

template <zeta::core::elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendObject(
    zeta::core::json_utils::Encoder<CodepointAcceptor>& serializer,
    JsonNode* json_node) {
    ZETA_Core_DebugAssert(json_node->type == JsonNode::TypeEnum::Object);

    ZETA_Core_Debug_PrintCurPos;

    serializer.SendObjectStart().CheckHasValue();

    for (auto const& [key, value] : json_node->object_value) {
        ZETA_Core_Debug_PrintCurPos;

        (SendValue)(serializer, key);
        ZETA_Core_Debug_PrintCurPos;

        (SendValue)(serializer, value);
    }

    ZETA_Core_Debug_PrintCurPos;

    serializer.SendFinish().CheckHasValue();

    ZETA_Core_Debug_PrintCurPos;
}

template <zeta::core::elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void SendValue(
    zeta::core::json_utils::Encoder<CodepointAcceptor>& serializer,
    JsonNode* json_node) {
    ZETA_Core_Debug_PrintVar(json_node);
    ZETA_Core_Debug_PrintVar(static_cast<unsigned>(json_node->type));

    switch (json_node->type) {
    case JsonNode::TypeEnum::Null: (SendNull)(serializer, json_node); break;
    case JsonNode::TypeEnum::Boolean:
        (SendBoolean)(serializer, json_node);
        break;
    case JsonNode::TypeEnum::Numeric:
        (SendNeumeric)(serializer, json_node);
        break;
    case JsonNode::TypeEnum::String: (SendString)(serializer, json_node); break;
    case JsonNode::TypeEnum::Array: (SendArray)(serializer, json_node); break;
    case JsonNode::TypeEnum::Object: (SendObject)(serializer, json_node); break;
    }
}

}  // namespace encode_json_node_to_json_text

constexpr std::vector<zeta::core::unicode::unichar_t> GenRandomLogicalString() {
    size_t len{ zeta::core_test::GetRandomInt<size_t, size_t>(0, 128) };

    ZETA_Core_Debug_PrintVar(len);

    std::vector<zeta::core::unicode::unichar_t> ret;
    ret.resize(len);

    /*

    0 ~ 0x1F
    0x20 ~ 0x7F
    0x80 ~ 0xD7FF
    0xE000 ~ 0x10FFFF

    */

    for (size_t i{ 0 }; i < len; ++i) {
        switch (zeta::core_test::GetRandomInt<int, int>(0, 3)) {
        case 0:
            ret[i] =
                zeta::core_test::GetRandomInt<zeta::core::unicode::unichar_t,
                                              zeta::core::unicode::unichar_t>(
                    0, 0x1F);
            break;
        case 1:
            ret[i] =
                zeta::core_test::GetRandomInt<zeta::core::unicode::unichar_t,
                                              zeta::core::unicode::unichar_t>(
                    0x20, 0x7F);
            break;
        case 2:
            ret[i] =
                zeta::core_test::GetRandomInt<zeta::core::unicode::unichar_t,
                                              zeta::core::unicode::unichar_t>(
                    0x80, 0xD7FF);
            break;
        case 3:
            ret[i] =
                zeta::core_test::GetRandomInt<zeta::core::unicode::unichar_t,
                                              zeta::core::unicode::unichar_t>(
                    0xE000, 0x10FFFF);
            break;
        }
    }

    return ret;
}

/*
constexpr std::vector<zeta::core::unicode::unichar_t> LogicalStringToJsontring(
    std::vector<zeta::core::unicode::unichar_t> const& raw_string) {
    std::vector<zeta::core::unicode::unichar_t> ret;
    ret.resize(2 + 256);

    zeta::core::lin_seq_elem_stream::Acceptor raw_string_cp_acceptor{
        .data = ret.data(),
        .elem_size = sizeof(zeta::core::unicode::unichar_t),
        .elem_stride = sizeof(zeta::core::unicode::unichar_t),
        .elem_cnt = ret.size(),
    };

    auto try_expand{ [&]() {
        if (raw_string_cp_acceptor.elem_cnt <= 32) {
            size_t cur_idx{ static_cast<size_t>(
                static_cast<zeta::core::unicode::unichar_t*>(
                    raw_string_cp_acceptor.data) -
                ret.data()) };

            ret.resize(ret.size() + 256);

            raw_string_cp_acceptor.data = ret.data() + cur_idx;
            raw_string_cp_acceptor.elem_cnt = ret.size() - cur_idx;
        }
    } };

    zeta::core::json_utils::string_endec::enc::State string_serialize_state;

    zeta::core::json_utils::string_endec::enc::SendStart(raw_string_cp_acceptor,
                                                         string_serialize_state)
        .CheckHasValue();

    for (zeta::core::unicode::unichar_t cp : raw_string) {
        try_expand();

        zeta::core::json_utils::string_endec::enc::SendChar(
            raw_string_cp_acceptor, string_serialize_state, cp,
            zeta::core_test::GetRandomInt<int, int>(0, 1) == 1,
            zeta::core_test::GetRandomInt<int, int>(0, 1) == 1)
            .CheckHasValue();
    }

    try_expand();

    zeta::core::json_utils::string_endec::enc::SendFinish(
        raw_string_cp_acceptor, string_serialize_state)
        .CheckHasValue();

    ret.resize(static_cast<size_t>(static_cast<zeta::core::unicode::unichar_t*>(
                                       raw_string_cp_acceptor.data) -
                                   ret.data()));

    return ret;
}
*/

/*
constexpr std::vector<zeta::core::unicode::unichar_t> JsonStringToLogicalString(
    std::vector<zeta::core::unicode::unichar_t> const& json_string) {
    std::vector<zeta::core::unicode::unichar_t> ret;

    zeta::core::lin_seq_elem_stream::Provider json_string_cp_provider{
        .data = json_string.data(),
        .elem_size = sizeof(zeta::core::unicode::unichar_t),
        .elem_stride = sizeof(zeta::core::unicode::unichar_t),
        .elem_cnt = json_string.size(),
    };

    zeta::core::json_utils::BufferedCodepointProvider<
        zeta::core::lin_seq_elem_stream::Provider>
        bcpp{ json_string_cp_provider };

    zeta::core::json_utils::string_endec::dec::State string_deserialize_state;

    zeta::core::json_utils::string_endec::dec::ReceiveStart(
        bcpp, string_deserialize_state)
        .CheckHasValue();

    while (string_deserialize_state !=
           zeta::core::json_utils::string_endec::dec::State::Finished) {
        ZETA_Core_DebugAssert(
            string_deserialize_state !=
            zeta::core::json_utils::string_endec::dec::State::Corrupted);

        ret.push_back(zeta::core::json_utils::string_endec::dec::ReceiveChar(
                          bcpp, string_deserialize_state)
                          .GetValue());
    }

    return ret;
}
*/

template <typename T>
struct DequeElemStream {
    std::deque<T> deque;

    static constexpr size_t GetElemSize(
        zeta::core::elem_stream::provider::Tag) {
        return sizeof(T);
    }

    static constexpr size_t GetElemSize(
        zeta::core::elem_stream::acceptor::Tag) {
        return sizeof(T);
    }

    constexpr bool IsEnd(this DequeElemStream<T> const& self,
                         zeta::core::elem_stream::provider::Tag) {
        return self.deque.empty();
    }

    static constexpr bool IsEnd(zeta::core::elem_stream::acceptor::Tag) {
        return false;
    }

    constexpr size_t Transfer(this DequeElemStream<T>& self,
                              zeta::core::elem_stream::provider::Tag, void* dst,
                              size_t elem_size, size_t elem_stride,
                              size_t elem_cnt) {
        size_t transfer_elem_size{ std::min(elem_size, sizeof(T)) };
        size_t transfer_elem_cnt{ std::min(self.deque.size(), elem_cnt) };

        for (size_t i{ 0 }; i < transfer_elem_cnt; ++i) {
            std::memcpy(dst, static_cast<void*>(&self.deque.front()),
                        transfer_elem_size);

            self.deque.pop_front();
            dst = static_cast<char*>(dst) + elem_stride;
        }

        return transfer_elem_cnt;
    }

    constexpr size_t Transfer(this DequeElemStream<T>& self,
                              zeta::core::elem_stream::acceptor::Tag,
                              void const* src, size_t elem_size,
                              size_t elem_stride, size_t elem_cnt) {
        size_t transfer_elem_size{ std::min(elem_size, sizeof(T)) };
        size_t transfer_elem_cnt{ elem_cnt };

        for (size_t i{ 0 }; i < transfer_elem_cnt; ++i) {
            self.deque.emplace_back();

            std::memcpy(&self.deque.back(), src, transfer_elem_size);

            src = static_cast<char const*>(src) + elem_stride;
        }

        return transfer_elem_cnt;
    }
};

template <zeta::core::elem_stream::provider::IsProvider CharProvider>
struct CharToUnicharProvider {
    CharProvider provider;

    mutable zeta::core::utf8::Decoder utf8_decoder;

    static constexpr size_t GetElemSize(
        zeta::core::elem_stream::provider::Tag) {
        return sizeof(zeta::core::unicode::unichar_t);
    }

    constexpr bool IsEnd(zeta::core::elem_stream::provider::Tag) const {
        if (false) {
            return zeta::core::elem_stream::provider::IsEnd(this->provider);
        } else {
            this->utf8_decoder.Decode(
                const_cast<zeta::core::meta::RemoveConst<CharProvider>&>(
                    this->provider));

            return !this->utf8_decoder.has_decoded_codepoint;
        }
    }

    constexpr size_t Transfer(zeta::core::elem_stream::provider::Tag, void* dst,
                              size_t elem_size, size_t elem_stride,
                              size_t elem_cnt) {
        if (false) {
            size_t transfer_elem_cnt{ 0 };

            for (size_t i{ 0 };
                 i < elem_cnt &&
                 !this->IsEnd(zeta::core::elem_stream::provider::Tag{});
                 ++i, ++transfer_elem_cnt) {
                zeta::core::unicode::unichar_t cp;

                zeta::core::elem_stream::provider::Transfer(
                    this->provider, &cp, sizeof(zeta::core::unicode::unichar_t),
                    sizeof(zeta::core::unicode::unichar_t), 1);

                std::memcpy(dst, &cp,
                            std::min(elem_size,
                                     sizeof(zeta::core::unicode::unichar_t)));

                dst = static_cast<char*>(dst) + elem_stride;
            }

            return transfer_elem_cnt;
        } else {
            zeta::core::lin_seq_elem_stream::Acceptor a{
                .data = dst,
                .elem_size = elem_size,
                .elem_stride = elem_stride,
                .elem_cnt = elem_cnt,
            };

            this->utf8_decoder.DecodeAndPush(this->provider, a);

            return elem_cnt - a.elem_cnt;
        }
    }
};

template <zeta::core::elem_stream::acceptor::IsAcceptor CharAcceptor>
struct UnicharToCharAcceptor {
    CharAcceptor acceptor;

    zeta::core::utf8::Encoder utf8_encoder;

    static constexpr size_t GetElemSize(
        zeta::core::elem_stream::acceptor::Tag) {
        return sizeof(zeta::core::unicode::unichar_t);
    }

    constexpr bool IsEnd(zeta::core::elem_stream::acceptor::Tag) const {
        return zeta::core::elem_stream::acceptor::IsEnd(this->acceptor);
    }

    constexpr size_t Transfer(zeta::core::elem_stream::acceptor::Tag,
                              void const* src, size_t elem_size,
                              size_t elem_stride, size_t elem_cnt) {
        zeta::core::lin_seq_elem_stream::Provider p{
            .data = src,
            .elem_size = elem_size,
            .elem_stride = elem_stride,
            .elem_cnt = elem_cnt,
        };

        this->utf8_encoder.EncodeAndPush(p, this->acceptor);

        return elem_cnt - p.elem_cnt;

        /*
        size_t transfer_elem_cnt{ 0 };

        for (size_t i{ 0 }; i < elem_cnt && !this->IsEnd();
             ++i, ++transfer_elem_cnt) {
            unsigned char cp{ static_cast<unsigned char>(
                *static_cast<zeta::core::unicode::unichar_t const*>(src)) };

            ZETA_Core_Debug_PrintVar(cp);

            zeta::core::elem_stream::acceptor::Transfer(
                this->acceptor, &cp, sizeof(unsigned char),
                sizeof(unsigned char), 1);

            src = static_cast<char const*>(src) + elem_stride;
        }

        return transfer_elem_cnt;
        */
    }
};

constexpr std::deque<unsigned char> JsonNodeToJsonText(
    JsonNode* json_node,
    zeta::core::json_utils::FormatConfig const& fmt_config) {
    DequeElemStream<unsigned char> deque_elem_stream{};

    UnicharToCharAcceptor<DequeElemStream<unsigned char>&>
        unichar_to_char_acceptor{
            .acceptor{ deque_elem_stream },
            .utf8_encoder{},
        };

    ZETA_Core_Debug_PrintCurPos;

    zeta::core::json_utils::Encoder<
        UnicharToCharAcceptor<DequeElemStream<unsigned char>&>>
        encoder{ unichar_to_char_acceptor, fmt_config };

    ZETA_Core_Debug_PrintCurPos;

    encode_json_node_to_json_text::SendValue(encoder, json_node);

    ZETA_Core_Debug_PrintCurPos;

    return deque_elem_stream.deque;
}

/*
constexpr JsonNode* JsonTextToJsonNode(
    std::deque<unsigned char> const& json_text) {
    DequeElemStream<char> deque_elem_stream{
        .deque{ json_text.begin(), json_text.end() },
    };

    CharToUnicharProvider<DequeElemStream<char>&> char_to_unichar_provider{
        .provider{ deque_elem_stream },
        .utf8_decoder{},
    };

    ZETA_Core_Debug_PrintCurPos;

    zeta::core::json_utils::BufferedCodepointProvider<
        CharToUnicharProvider<DequeElemStream<char>&>>
        bcpp{ char_to_unichar_provider };

    ZETA_Core_Debug_PrintCurPos;

    zeta::core::json_utils::Decoder<
        CharToUnicharProvider<DequeElemStream<char>&>>
        decoder{ bcpp };

    ZETA_Core_Debug_PrintCurPos;

    return decode_json_text_to_json_node::ReceiveValue(decoder);
}
*/

/*
inline void main1() {
    char test_string[]{ "-456E+6" };

    auto numeric_info{ ParseNumeric(test_string) };

    ZETA_Core_PrintVar(static_cast<bool>(numeric_info.is_ok));
    ZETA_Core_PrintVar(static_cast<bool>(numeric_info.sign));
    ZETA_Core_PrintVar(static_cast<bool>(numeric_info.has_frac));
    ZETA_Core_PrintVar(static_cast<char>(numeric_info.e));
    ZETA_Core_PrintVar(static_cast<char>(numeric_info.exp_part_sign));
    ZETA_Core_PrintVar(numeric_info.int_part_digits);
    ZETA_Core_PrintVar(numeric_info.frac_part_digits);
    ZETA_Core_PrintVar(numeric_info.exp_part_digits);
}
*/

/*
constexpr void main2() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 1'784'375'523 };

    unsigned seed{ random_seed };
    // unsigned seed{ fixed_seed };

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    size_t test_cnt{ 1024 * 1024 };

    for (size_t test_i{ 0 }; test_i < test_cnt; ++test_i) {
        if (test_i % 1024 == 0) { ZETA_Core_PrintVar(test_i / 1024); }

        auto numeric_info{ GenRandomNumericInfo() };

        ZETA_Core_Debug_PrintVar(numeric_info.is_ok);
        ZETA_Core_Debug_PrintVar(numeric_info.sign);
        ZETA_Core_Debug_PrintVar(numeric_info.has_frac);
        ZETA_Core_Debug_PrintVar(static_cast<char>(numeric_info.e));
        ZETA_Core_Debug_PrintVar(static_cast<char>(numeric_info.exp_part_sign));
        ZETA_Core_Debug_PrintVar(numeric_info.int_part_digits);
        ZETA_Core_Debug_PrintVar(numeric_info.frac_part_digits);
        ZETA_Core_Debug_PrintVar(numeric_info.exp_part_digits);

        auto numeric_str{ NumericInfoToString(numeric_info) };

        ZETA_Core_Debug_PrintVar(numeric_str);

        auto parsed_numeric_info{ ParseNumeric(numeric_str) };

        ZETA_Core_Debug_PrintVar(parsed_numeric_info.is_ok);
        ZETA_Core_Debug_PrintVar(parsed_numeric_info.sign);
        ZETA_Core_Debug_PrintVar(parsed_numeric_info.has_frac);
        ZETA_Core_Debug_PrintVar(static_cast<char>(parsed_numeric_info.e));
        ZETA_Core_Debug_PrintVar(
            static_cast<char>(parsed_numeric_info.exp_part_sign));
        ZETA_Core_Debug_PrintVar(parsed_numeric_info.int_part_digits);
        ZETA_Core_Debug_PrintVar(parsed_numeric_info.frac_part_digits);
        ZETA_Core_Debug_PrintVar(parsed_numeric_info.exp_part_digits);

        ZETA_Core_DebugAssert(numeric_info.is_ok == parsed_numeric_info.is_ok);
        ZETA_Core_DebugAssert(numeric_info.sign == parsed_numeric_info.sign);
        ZETA_Core_DebugAssert(numeric_info.has_frac ==
                              parsed_numeric_info.has_frac);
        ZETA_Core_DebugAssert(numeric_info.e == parsed_numeric_info.e);
        ZETA_Core_DebugAssert(numeric_info.exp_part_sign ==
                              parsed_numeric_info.exp_part_sign);
        ZETA_Core_DebugAssert(numeric_info.int_part_digits ==
                              parsed_numeric_info.int_part_digits);
        ZETA_Core_DebugAssert(numeric_info.frac_part_digits ==
                              parsed_numeric_info.frac_part_digits);
        ZETA_Core_DebugAssert(numeric_info.exp_part_digits ==
                              parsed_numeric_info.exp_part_digits);

        zeta::core::debug_utils::ClearDebugStrStream();
    }
}
*/

/*
constexpr void main3() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 1'784'375'523 };

    // unsigned seed{ random_seed };
    unsigned seed{ fixed_seed };

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    size_t test_cnt{ 1024 * 1024 * 8 };

    for (size_t test_i{ 0 }; test_i < test_cnt; ++test_i) {
        if (test_i % 1024 == 0) { ZETA_Core_PrintVar(test_i / 1024); }

        std::vector<zeta::core::unicode::unichar_t> logical_string{
            GenRandomLogicalString()
        };

        for (size_t i{ 0 }; i < logical_string.size(); ++i) {
            ZETA_Core_Debug_PrintVar(logical_string[i]);
        }

        std::vector<zeta::core::unicode::unichar_t> json_string{
            LogicalStringToJsontring(logical_string)
        };

        for (size_t i{ 0 }; i < json_string.size(); ++i) {
            ZETA_Core_Debug_PrintVar(json_string[i]);
        }

        std::vector<zeta::core::unicode::unichar_t> parsed_logical_string{
            JsonStringToLogicalString(json_string)
        };

        for (size_t i{ 0 }; i < parsed_logical_string.size(); ++i) {
            ZETA_Core_Debug_PrintVar(parsed_logical_string[i]);
        }

        ZETA_Core_DebugAssert(logical_string == parsed_logical_string);

        zeta::core::debug_utils::ClearDebugStrStream();
    }
}
*/

constexpr void main4() {
    char sample_json_str[]{ R"({
        "null": null,
        "boolean.true": true,
        "numeric.1": 123,
        "numeric.2": -456E+6,
        "string.1": "Hello, world!",
    })" };

    CharToUnicharProvider<DequeElemStream<char>> char_to_unichar_provider{
        .provider{ .deque{ sample_json_str,
                           sample_json_str + sizeof(sample_json_str) - 1 } },
        .utf8_decoder{},
    };

    zeta::core::json_utils::Decoder<
        CharToUnicharProvider<DequeElemStream<char>>&>
        decoder{ char_to_unichar_provider };

    JsonNode* root_node{ decode_json_text_to_json_node::ReceiveValue(decoder) };

    zeta::core::debug_utils::ClearDebugStrStream();

    UnicharToCharAcceptor<DequeElemStream<char>> unichar_to_char_acceptor{
        .acceptor{},
        .utf8_encoder{},
    };

    zeta::core::json_utils::Encoder<decltype(unichar_to_char_acceptor)>
        serializer{
            unichar_to_char_acceptor,
            zeta::core::json_utils::FormatConfig{
                .newline{
                    .type =
                        zeta::core::json_utils::FormatConfig::Newline::Type::LF,
                },

                .space{
                    .before_colon = false,
                    .after_colon = true,

                    .before_comma = false,
                    .after_comma = true,
                },

                .indent{
                    .type = zeta::core::json_utils::FormatConfig::Indent::Type::
                        Space,
                    .cnt = 4,
                },

                .string{
                    .prefer_unicode_escape = true,
                    .prefer_uppercase_hex = false,
                },
            }
        };

    encode_json_node_to_json_text::SendValue(serializer, root_node);

    for (char ch : unichar_to_char_acceptor.acceptor.deque) { std::cout << ch; }

    std::cout << "\n";

    delete root_node;

    std::cout << decode_json_text_to_json_node::json_log_ss.str() << std::endl;
}

constexpr void main5() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 1'787'333'379 };

    // unsigned seed{ random_seed };
    unsigned seed{ fixed_seed };

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    constexpr zeta::core::json_utils::FormatConfig fmt_config{
        .newline{
            .type = zeta::core::json_utils::FormatConfig::Newline::Type::LF,
        },

        .space{
            .before_colon = false,
            .after_colon = true,

            .before_comma = false,
            .after_comma = true,
        },

        .indent{
            .type = zeta::core::json_utils::FormatConfig::Indent::Type::Space,
            .cnt = 4,
        },

        .string{
            .prefer_unicode_escape = false,
            .prefer_uppercase_hex = true,
        },
    };

    ZETA_Core_PrintCurPos;

    JsonNode* root_node_1{ (GenRandomJsonNode(3000)) };

    ZETA_Core_PrintCurPos;

    DequeElemStream<char> deque_elem_stream;

    UnicharToCharAcceptor<DequeElemStream<char>&> unichar_to_char_acceptor_1{
        .acceptor{ deque_elem_stream },
        .utf8_encoder{},
    };

    zeta::core::json_utils::Encoder<
        UnicharToCharAcceptor<DequeElemStream<char>&>>
        encoder_1{ unichar_to_char_acceptor_1, fmt_config };

    encode_json_node_to_json_text::SendValue(encoder_1, root_node_1);

    ZETA_Core_PrintVar(deque_elem_stream.deque.size());

    {
        std::ofstream fs{ std::string{ ZetaDir "/json/test_json_1.json" },
                          std::ios::binary };

        for (char ch : deque_elem_stream.deque) { fs << ch; }

        fs.close();
    }

    CharToUnicharProvider<DequeElemStream<char>&> char_to_unichar_provider{
        .provider{ deque_elem_stream },
        .utf8_decoder{},
    };

    ZETA_Core_Debug_PrintCurPos;

    ZETA_Core_Debug_PrintCurPos;

    zeta::core::json_utils::Decoder<
        CharToUnicharProvider<DequeElemStream<char>&>>
        decode_1{ char_to_unichar_provider };

    ZETA_Core_Debug_PrintCurPos;

    JsonNode* root_node_2{ decode_json_text_to_json_node::ReceiveValue(
        decode_1) };

    ZETA_Core_DebugAssert(*root_node_1 == *root_node_2);

    ZETA_Core_PrintCurPos;
}

int main() {
    for (size_t i{ 0 }; i < 128; ++i) {
        ZETA_Core_PrintVar(i);
        main5();
        zeta::core::debug_utils::ClearDebugStrStream();
    }

    ZETA_Core_PrintVar("ok");
    return 0;
}
