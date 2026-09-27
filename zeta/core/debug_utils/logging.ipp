

#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <zeta/core/debug_utils/logging.hpp>
#include <zeta/core/meta.hpp>

#if __has_feature(address_sanitizer)
#include <sanitizer/common_interface_defs.h>
#endif

ZETA_Core_ClangdPreambleBarrier;

#define ZETA_Core_DebugUtils_Logging_MakeCurTimePosLog()             \
    (zeta::core::debug_utils::logging::TimePosLog{                   \
        .time = zeta::core::debug_utils::logging::detail::GetNow_(), \
        .file = __FILE__,                                            \
        .line = __LINE__,                                            \
    })

#define ZETA_Core_DebugUtils_Logging_MakeMsgLog(msg) \
    (zeta::core::debug_utils::logging::LogContent{   \
        .type{ "Message" },                          \
        .name{},                                     \
        .vals{ (msg) },                              \
    })

#define ZETA_Core_DebugUtils_Logging_MakeVarLog(var, ...)                    \
    (zeta::core::debug_utils::logging::LogContent{                           \
        .type{                                                               \
            zeta::core::debug_utils::logging::GetTypeStr<decltype(var)>() }, \
        .name{ #var },                                                       \
        .vals{ zeta::core::debug_utils::logging::detail::ToStr_(             \
            var __VA_OPT__(, ) __VA_ARGS__) },                               \
    })

#if __has_feature(address_sanitizer)
#define ZETA_Core_DebugUtils_PrintStackTrace() __sanitizer_print_stack_trace()
#else
#define ZETA_Core_DebugUtils_PrintStackTrace() static_assert(true)
#endif

#define ZETA_Core_DebugUtils_Logging_PrintLog_(tmp_stream, stream, log, \
                                               imm_flush)               \
    {                                                                   \
        auto& tmp_stream{ stream };                                     \
        (log).Print(tmp_stream);                                        \
        if (imm_flush) { tmp_stream.flush(); }                          \
    }                                                                   \
    static_assert(true)

#define ZETA_Core_DebugUtils_Logging_PrintLog(stream, log, imm_flush)          \
    ZETA_Core_DebugUtils_Logging_PrintLog_(ZETA_Core_TmpName, (stream), (log), \
                                           (imm_flush))

#define ZETA_Core_DebugUtils_Logging_ImmLogCurPos()                            \
    ZETA_Core_DebugUtils_Logging_PrintLog(                                     \
        zeta::core::debug_utils::logging::imm_os,                              \
        (zeta::core::debug_utils::logging::Log{                                \
            .time_pos_log{ ZETA_Core_DebugUtils_Logging_MakeCurTimePosLog() }, \
            .log_contents{},                                                   \
        }),                                                                    \
        true)

#define ZETA_Core_DebugUtils_Logging_ImmLogMsg(msg)                            \
    ZETA_Core_DebugUtils_Logging_PrintLog(                                     \
        zeta::core::debug_utils::logging::imm_os,                              \
        (zeta::core::debug_utils::logging::Log{                                \
            .time_pos_log{ ZETA_Core_DebugUtils_Logging_MakeCurTimePosLog() }, \
            .log_contents{ ZETA_Core_DebugUtils_Logging_MakeMsgLog(msg) },     \
        }),                                                                    \
        true)

#define ZETA_Core_DebugUtils_Logging_ImmLogVar(var, ...)                       \
    ZETA_Core_DebugUtils_Logging_PrintLog(                                     \
        zeta::core::debug_utils::logging::imm_os,                              \
        (zeta::core::debug_utils::logging::Log{                                \
            .time_pos_log{ ZETA_Core_DebugUtils_Logging_MakeCurTimePosLog() }, \
            .log_contents{                                                     \
                ZETA_Core_DebugUtils_Logging_MakeVarLog(var, __VA_ARGS__) },   \
        }),                                                                    \
        true)

#pragma push_macro("Space")

#define Space(width) std::setw(width) << ""

namespace zeta::core::debug_utils {

namespace logging::detail {

constexpr std::chrono::system_clock::time_point GetNow_() {
    return std::chrono::system_clock::now();
}

constexpr void OStreamFormatReset_(std::ostream& os) {
    os << std::setfill(' ') << ZETA_Core_DebugUtils_Logging_ResetColorCode;
}

}  // namespace logging::detail

template <typename T>
constexpr std::string logging::GetTypeStr() {
    std::string func_name{ __PRETTY_FUNCTION__ };

    size_t l_size{ func_name.find("T = ") + 4 };
    size_t r_size{ 1 };

    return func_name.substr(l_size, func_name.size() - r_size - l_size);
}

constexpr void logging::TimePosLog::Clear(this TimePosLog& time_pos_log) {
    time_pos_log.time = {};
    time_pos_log.file.clear();
    time_pos_log.line = 0;
}

constexpr void logging::TimePosLog::Print(this TimePosLog const& time_pos_log,
                                          std::ostream& os) {
    detail::OStreamFormatReset_(os);

    os                                                 //
        << ZETA_Core_DebugUtils_Logging_TimeColorCode  //
        << std::format(
               "{:%Y/%m/%d %H:%M:%S %z}",
               std::chrono::zoned_time{
                   time_zone, std::chrono::floor<std::chrono::milliseconds>(
                                  time_pos_log.time) })   //
        << ZETA_Core_DebugUtils_Logging_ResetColorCode    //
        << ZETA_Core_DebugUtils_Logging_Indent            //
        << ZETA_Core_DebugUtils_Logging_PosColorCode      //
        << time_pos_log.file << ':' << time_pos_log.line  //
        << ZETA_Core_DebugUtils_Logging_ResetColorCode    //
        << ZETA_Core_DebugUtils_Logging_Newline           //
        ;
}

constexpr void logging::LogContent::Clear(this LogContent& log_content) {
    log_content.type.clear();
    log_content.name.clear();
    log_content.vals.clear();
}

constexpr void logging::LogContent::Print(this LogContent const& log_content,
                                          std::ostream& os) {
    detail::OStreamFormatReset_(os);

    bool type_overflow{ var_log_type_pretty_len < log_content.type.size() };
    bool name_overflow{ var_log_name_pretty_len < log_content.name.size() };

    os << ZETA_Core_DebugUtils_Logging_TypeColorCode << std::right
       << std::setw(var_log_type_pretty_len) << log_content.type
       << ZETA_Core_DebugUtils_Logging_ResetColorCode;

    if (type_overflow) {
        os << ZETA_Core_DebugUtils_Logging_Newline
           << Space(var_log_type_pretty_len);
    }

    os << ZETA_Core_DebugUtils_Logging_Indent
       << ZETA_Core_DebugUtils_Logging_NameColorCode << std::left
       << std::setw(var_log_name_pretty_len) << log_content.name
       << ZETA_Core_DebugUtils_Logging_ResetColorCode;

    if (name_overflow) {
        os << ZETA_Core_DebugUtils_Logging_Newline
           << Space(var_log_type_pretty_len)
           << ZETA_Core_DebugUtils_Logging_Indent
           << Space(var_log_name_pretty_len);
    }

    os << " = ";

    if (!log_content.vals.empty()) {
        os << ZETA_Core_DebugUtils_Logging_ValColorCode << log_content.vals[0]
           << ZETA_Core_DebugUtils_Logging_ResetColorCode
           << ZETA_Core_DebugUtils_Logging_Newline;
    }

    for (size_t i{ 1 }; i < log_content.vals.size(); ++i) {
        os << Space(var_log_type_pretty_len)
           << ZETA_Core_DebugUtils_Logging_Indent
           << Space(var_log_name_pretty_len)
           << "   " ZETA_Core_DebugUtils_Logging_ValColorCode
           << log_content.vals[i] << ZETA_Core_DebugUtils_Logging_ResetColorCode
           << ZETA_Core_DebugUtils_Logging_Newline;
    }

    if (!name_overflow &&
        4 < var_log_name_pretty_len - log_content.name.size() &&
        log_content.vals.size() < 2) {
        os << ZETA_Core_DebugUtils_Logging_Newline;
    }
}

constexpr void logging::Log::Clear(this Log& log) {
    log.time_pos_log.Clear();
    log.log_contents.clear();
}

constexpr void logging::Log::Print(this Log const& log, std::ostream& os) {
    log.time_pos_log.Print(os);

    for (auto const& log_content : log.log_contents) { log_content.Print(os); }

    if (log.log_contents.empty()) {
        os << ZETA_Core_DebugUtils_Logging_Newline;
    }
}

namespace logging::detail {

template <typename T>
constexpr std::deque<std::string> ToStr_(T const& var) {
    return VarPrinter<T, NullFormatTag>::ToStr(var, NullFormatTag{});
}

template <typename T, typename FormatTag>
constexpr std::deque<std::string> ToStr_(T const& var,
                                         FormatTag const& format_tag) {
    return VarPrinter<T, FormatTag>::ToStr(var, format_tag);
}

}  // namespace logging::detail

template <>
struct logging::VarPrinter<bool, logging::NullFormatTag> {
    static constexpr std::deque<std::string> ToStr(bool const& val,
                                                   NullFormatTag) {
        return { val ? " True" : "False" };
    }
};

namespace logging::detail {

template <integral::IsIntegral Integral, size_t Numeral>
constexpr size_t GetWidth_() {
    using UnsignedIntegral = integral::MakeUnsignedOf<Integral>;

    UnsignedIntegral val{ integral::RangeMaxOf<UnsignedIntegral> };

    size_t ret{ 0 };

    for (; 0 < val; val /= Numeral) { ++ret; }

    return ret;
}

constexpr std::string GetIntegralRuler_(size_t digit_cnt,
                                        size_t group_digit_cnt,
                                        size_t sep_width,
                                        size_t group_idx_stride) {
    size_t group_cnt{ (digit_cnt + group_digit_cnt - 1) / group_digit_cnt };

    std::stringstream ss;

    ss << "   ";

    for (size_t i{ group_cnt - 1 };; --i) {
        ss << std::setw(static_cast<unsigned>(group_digit_cnt))
           << i * group_idx_stride;

        if (i == 0) { break; }

        ss << Space(static_cast<unsigned>(sep_width));
    }

    return ss.str();
}

}  // namespace logging::detail

namespace logging::detail {

template <integral::IsUnsignedIntegral UnsignedIntegral>
constexpr std::string UnsignedIntegralToBinStr_(UnsignedIntegral value,
                                                size_t group_digit_cnt,
                                                size_t sep_width) {
    constexpr size_t width{ detail::GetWidth_<UnsignedIntegral, 2>() };

    size_t group_cnt{ (width + group_digit_cnt - 1) / group_digit_cnt };

    std::string ret;

    ret.resize(3 + group_digit_cnt * group_cnt + sep_width * (group_cnt - 1),
               ' ');

    char* cur{ ret.data() + ret.size() };

    if (value == 0) { *(--cur) = '0'; }

    for (size_t i{ 1 }; 0 < value; ++i) {
        *(--cur) = '0' + value % 2;

        value /= 2;

        if (i % group_digit_cnt == 0) { cur -= sep_width; }
    }

    ret[1] = '0';
    ret[2] = 'b';

    return ret;
}

template <integral::IsUnsignedIntegral UnsignedIntegral>
constexpr std::string UnsignedIntegralToDecStr_(UnsignedIntegral value,
                                                size_t group_digit_cnt,
                                                size_t sep_width) {
    constexpr size_t width{ detail::GetWidth_<UnsignedIntegral, 10>() };

    size_t group_cnt{ (width + group_digit_cnt - 1) / group_digit_cnt };

    std::string ret;

    ret.resize(3 + group_digit_cnt * group_cnt + sep_width * (group_cnt - 1),
               ' ');

    char* cur{ ret.data() + ret.size() };

    if (value == 0) { *(--cur) = '0'; }

    for (size_t i{ 1 }; 0 < value; ++i) {
        *(--cur) = '0' + value % 10;

        value /= 10;

        if (i % group_digit_cnt == 0) { cur -= sep_width; }
    }

    ret[1] = '0';
    ret[2] = 'd';

    return ret;
}

template <integral::IsUnsignedIntegral UnsignedIntegral>
constexpr std::string UnsignedIntegralToHexStr_(UnsignedIntegral value,
                                                size_t group_digit_cnt,
                                                size_t sep_width) {
    constexpr size_t width{ detail::GetWidth_<UnsignedIntegral, 16>() };

    size_t group_cnt{ (width + group_digit_cnt - 1) / group_digit_cnt };

    std::string ret;

    ret.resize(3 + group_digit_cnt * group_cnt + sep_width * (group_cnt - 1),
               ' ');

    char* cur{ ret.data() + ret.size() };

    if (value == 0) { *(--cur) = '0'; }

    for (size_t i{ 1 }; 0 < value; ++i) {
        int digit{ static_cast<int>(value % 16) };

        *(--cur) =
            static_cast<char>(digit < 10 ? '0' + digit : 'A' + digit - 10);

        value /= 16;

        if (i % group_digit_cnt == 0) { cur -= sep_width; }
    }

    ret[1] = '0';
    ret[2] = 'x';

    return ret;
}

}  // namespace logging::detail

template <integral::IsIntegral Integral>
struct logging::VarPrinter<Integral, logging::BinFormatTag> {
    static constexpr std::deque<std::string> ToStr(Integral const& val,
                                                   BinFormatTag) {
        constexpr size_t width{ detail::GetWidth_<Integral, 2>() };

        std::deque<std::string> ret;

        ret.push_back(ZETA_Core_DebugUtils_Logging_ValSecondaryColorCode +
                      detail::GetIntegralRuler_(width, 8, 1, 1) +
                      "    byte" ZETA_Core_DebugUtils_Logging_ResetColorCode);

        ret.push_back(ZETA_Core_DebugUtils_Logging_ValSecondaryColorCode +
                      detail::GetIntegralRuler_(width, 8, 1, 8) +
                      "    digit" ZETA_Core_DebugUtils_Logging_ResetColorCode);

        using UnsignedIntegral = integral::MakeUnsignedOf<Integral>;

        UnsignedIntegral unsigned_val{ static_cast<UnsignedIntegral>(val) };

        ret.push_back(
            detail::UnsignedIntegralToBinStr_(
                static_cast<UnsignedIntegral>(val < 0 ? -val : val), 8, 1) +
            "    sign & mag");

        ret.back()[0] = val < 0 ? '-' : '+';

        if (val < 0) {
            ret.push_back(
                detail::UnsignedIntegralToBinStr_(
                    static_cast<UnsignedIntegral>(unsigned_val), 8, 1) +
                "    2's comp");
        }

        return ret;
    }
};

template <integral::IsIntegral Integral>
struct logging::VarPrinter<Integral, logging::DecFormatTag> {
    static constexpr std::deque<std::string> ToStr(Integral const& val,
                                                   DecFormatTag) {
        constexpr size_t width{ detail::GetWidth_<Integral, 10>() };

        std::deque<std::string> ret;

        ret.push_back(ZETA_Core_DebugUtils_Logging_ValSecondaryColorCode +
                      detail::GetIntegralRuler_(width, 3, 1, 3) +
                      "    digit" ZETA_Core_DebugUtils_Logging_ResetColorCode);

        using UnsignedIntegral = integral::MakeUnsignedOf<Integral>;

        ret.push_back(
            detail::UnsignedIntegralToDecStr_(
                static_cast<UnsignedIntegral>(val < 0 ? -val : val), 3, 1) +
            "    sign & mag");

        ret.back()[0] = val < 0 ? '-' : '+';

        return ret;
    }
};

template <integral::IsIntegral Integral>
struct logging::VarPrinter<Integral, logging::HexFormatTag> {
    static constexpr std::deque<std::string> ToStr(Integral const& val,
                                                   HexFormatTag) {
        constexpr size_t width{ detail::GetWidth_<Integral, 16>() };

        std::deque<std::string> ret;

        ret.push_back(ZETA_Core_DebugUtils_Logging_ValSecondaryColorCode +
                      detail::GetIntegralRuler_(width, 8, 1, 4) +
                      "    byte" ZETA_Core_DebugUtils_Logging_ResetColorCode);

        ret.push_back(ZETA_Core_DebugUtils_Logging_ValSecondaryColorCode +
                      detail::GetIntegralRuler_(width, 8, 1, 8) +
                      "    digit" ZETA_Core_DebugUtils_Logging_ResetColorCode);

        using UnsignedIntegral = integral::MakeUnsignedOf<Integral>;

        UnsignedIntegral unsigned_val{ static_cast<UnsignedIntegral>(val) };

        ret.push_back(
            detail::UnsignedIntegralToHexStr_(
                static_cast<UnsignedIntegral>(val < 0 ? -val : val), 8, 1) +
            "    sign & mag");

        ret.back()[0] = val < 0 ? '-' : '+';

        if (val < 0) {
            ret.push_back(
                detail::UnsignedIntegralToHexStr_(
                    static_cast<UnsignedIntegral>(unsigned_val), 8, 1) +
                "    2's comp");
        }

        return ret;
    }
};

template <integral::IsIntegral Integral>
struct logging::VarPrinter<Integral, logging::BinHexFormatTag> {
    static constexpr std::deque<std::string> ToStr(Integral const& val,
                                                   BinHexFormatTag) {
        constexpr size_t width{ detail::GetWidth_<Integral, 2>() };

        std::deque<std::string> ret;

        ret.push_back(ZETA_Core_DebugUtils_Logging_ValSecondaryColorCode +
                      detail::GetIntegralRuler_(width, 8, 1, 1) +
                      "    byte" ZETA_Core_DebugUtils_Logging_ResetColorCode);

        using UnsignedIntegral = integral::MakeUnsignedOf<Integral>;

        UnsignedIntegral unsigned_val{ static_cast<UnsignedIntegral>(val) };

        ret.push_back(
            detail::UnsignedIntegralToHexStr_(
                static_cast<UnsignedIntegral>(val < 0 ? -val : val), 2, 7) +
            "    sign & mag");

        {
            std::string& last_str{ ret.back() };

            last_str.insert(last_str.begin() + 3, 6, ' ');

            last_str[0] = val < 0 ? '-' : '+';
        }

        if (val < 0) {
            ret.push_back(
                detail::UnsignedIntegralToHexStr_(
                    static_cast<UnsignedIntegral>(unsigned_val), 2, 7) +
                "    2's comp");
        }

        ret.push_back(
            detail::UnsignedIntegralToBinStr_(
                static_cast<UnsignedIntegral>(val < 0 ? -val : val), 8, 1) +
            "    sign & mag");

        ret.back()[0] = val < 0 ? '-' : '+';

        if (val < 0) {
            ret.push_back(
                detail::UnsignedIntegralToBinStr_(
                    static_cast<UnsignedIntegral>(unsigned_val), 8, 1) +
                "    2's comp");
        }

        return ret;
    }
};

template <integral::IsIntegral Integral>
struct logging::VarPrinter<Integral, logging::NullFormatTag> {
    static constexpr std::deque<std::string> ToStr(Integral const& val,
                                                   NullFormatTag) {
        return VarPrinter<Integral, DecFormatTag>::ToStr(val, DecFormatTag{});
    }
};

template <typename T>
struct logging::VarPrinter<T*, logging::BinFormatTag> {
    static constexpr std::deque<std::string> ToStr(T* ptr, BinFormatTag) {
        return VarPrinter<uintptr_t, BinFormatTag>::ToStr(
            reinterpret_cast<uintptr_t>(ptr), BinFormatTag{});
    }
};

template <typename T>
struct logging::VarPrinter<T*, logging::HexFormatTag> {
    static constexpr std::deque<std::string> ToStr(T* ptr, HexFormatTag) {
        return VarPrinter<uintptr_t, HexFormatTag>::ToStr(
            reinterpret_cast<uintptr_t>(ptr), HexFormatTag{});
    }
};

template <typename T>
struct logging::VarPrinter<T*, logging::BinHexFormatTag> {
    static constexpr std::deque<std::string> ToStr(T* ptr, BinHexFormatTag) {
        return VarPrinter<uintptr_t, BinHexFormatTag>::ToStr(
            reinterpret_cast<uintptr_t>(ptr), BinHexFormatTag{});
    }
};

template <typename T>
struct logging::VarPrinter<T*, logging::NullFormatTag> {
    static constexpr std::deque<std::string> ToStr(T* ptr, NullFormatTag) {
        return VarPrinter<uintptr_t, BinHexFormatTag>::ToStr(
            reinterpret_cast<uintptr_t>(ptr), BinHexFormatTag{});
    }
};

template <>
struct logging::VarPrinter<char[], logging::NullFormatTag> {
    static constexpr std::deque<std::string> ToStr(char const* val,
                                                   NullFormatTag) {
        return { std::string{ val } };
    }
};

template <size_t N>
struct logging::VarPrinter<char[N], logging::NullFormatTag> {
    static constexpr std::deque<std::string> ToStr(char const* val,
                                                   NullFormatTag) {
        return { std::string{ val } };
    }
};

template <>
struct logging::VarPrinter<char*, logging::NullFormatTag> {
    static constexpr std::deque<std::string> ToStr(char const* val,
                                                   NullFormatTag) {
        return { std::string{ val } };
    }
};

template <>
struct logging::VarPrinter<char const*, logging::NullFormatTag> {
    static constexpr std::deque<std::string> ToStr(char const* val,
                                                   NullFormatTag) {
        return { std::string{ val } };
    }
};

template <>
struct logging::VarPrinter<std::string, logging::NullFormatTag> {
    static constexpr std::deque<std::string> ToStr(std::string const& str,
                                                   NullFormatTag) {
        return { str };
    }
};

constexpr logging::CircularLogger::CircularLogger(size_t max_log_cnt)
    : max_log_cnt{ max_log_cnt } {}

constexpr void logging::CircularLogger::Push(this CircularLogger& self,
                                             Log const& log) {
    while (self.max_log_cnt <= self.logs.size()) { self.logs.pop_front(); }

    self.logs.push_back(log);
}

constexpr void logging::CircularLogger::Clear(this CircularLogger& self) {
    self.logs.clear();
}

constexpr void logging::CircularLogger::Flush(this CircularLogger& self,
                                              std::ostream& os) {
    for (auto const& log : self.logs) {
        ZETA_Core_DebugUtils_Logging_PrintLog(os, log, false);
    }

    os.flush();

    self.logs.clear();
}

}  // namespace zeta::core::debug_utils

#pragma pop_macro("Space")
