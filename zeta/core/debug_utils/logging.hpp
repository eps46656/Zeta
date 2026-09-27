#pragma once

#include <chrono>
#include <deque>
#include <iostream>
#include <string>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>

#if ZETA_Core_DebugEnable

#define ZETA_Core_DebugStructPadding char ZETA_Core_TmpName[sizeof(void*)]

#else

#define ZETA_Core_DebugStructPadding static_assert(true)

#endif

namespace zeta::core::debug_utils::logging {

inline std::ostream& imm_os{ std::cout };

inline std::chrono::time_zone const* time_zone{ std::chrono::current_zone() };

#define ZETA_Core_DebugUtils_Logging_ResetColorCode "\033[0m"

#define ZETA_Core_DebugUtils_Logging_TimeColorCode "\033[38;5;220m"
#define ZETA_Core_DebugUtils_Logging_PosColorCode "\033[38;5;248m"

#define ZETA_Core_DebugUtils_Logging_TypeColorCode "\033[38;5;99m"
#define ZETA_Core_DebugUtils_Logging_NameColorCode "\033[38;5;46m"
#define ZETA_Core_DebugUtils_Logging_ValColorCode "\033[38;5;87m"

#define ZETA_Core_DebugUtils_Logging_ValSecondaryColorCode "\033[38;5;61m"

#define ZETA_Core_DebugUtils_Logging_WarningColorCode "\033[38;5;208m"

#define ZETA_Core_DebugUtils_Logging_Indent "    "

#define ZETA_Core_DebugUtils_Logging_Newline "\n"

constexpr size_t var_log_type_pretty_len{ 29 };
constexpr size_t var_log_name_pretty_len{ 48 };

template <typename T>
constexpr std::string GetTypeStr();

enum struct LogType : unsigned char {
    TimePos = 0,
    Var = 1,
};

struct TimePosLog {
    std::chrono::system_clock::time_point time;

    std::string file;
    size_t line;

    constexpr void Clear(this TimePosLog& time_pos_log);

    constexpr void Print(this TimePosLog const& time_pos_log, std::ostream& os);
};

struct LogContent {
    std::string type;
    std::string name;
    std::deque<std::string> vals;

    constexpr void Clear(this LogContent& log_content);

    constexpr void Print(this LogContent const& var_log, std::ostream& os);
};

struct Log {
    TimePosLog time_pos_log;
    std::deque<LogContent> log_contents;

    constexpr void Clear(this Log& log);

    constexpr void Print(this Log const& log, std::ostream& os);
};

template <typename T, typename FormatTag>
struct VarPrinter;

struct NullFormatTag {};

struct BinFormatTag {};
struct DecFormatTag {};
struct HexFormatTag {};
struct BinHexFormatTag {};

struct CircularLogger {
    std::deque<Log> logs;
    size_t max_log_cnt;

    constexpr CircularLogger(size_t max_log_cnt);

    constexpr void Push(this CircularLogger& self, Log const& log);

    constexpr void Clear(this CircularLogger& self);

    constexpr void Flush(this CircularLogger& self, std::ostream& os);
};

}  // namespace zeta::core::debug_utils::logging
