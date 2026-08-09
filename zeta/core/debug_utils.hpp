#pragma once

#if __has_feature(address_sanitizer)
#include <sanitizer/common_interface_defs.h>  // IWYU pragma: keep
#endif

#include <deque>
#include <sstream>
#include <string>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>

#define ZETA_Core_DebugStructPadding char ZETA_Core_TmpName[sizeof(void*)]

#if __has_feature(address_sanitizer)
#define ZETA_Core_PrintStackTrace __sanitizer_print_stack_trace()
#else
#define ZETA_Core_PrintStackTrace ZETA_Core_StaticAssert(true)
#endif

#if ZETA_Core_EnableDebug

#define ZETA_Core_WhenEnableDebug(code) code

#else

#define ZETA_Core_WhenEnableDebug(code)              \
    if constexpr (false) { ZETA_Core_Unused(code); } \
    ZETA_Core_StaticAssert(true)

#endif

#define ZETA_Core_EnPrint 1

#if ZETA_Core_EnPrint

#define ZETA_Core_WhenEnPrint(...) __VA_ARGS__

#else

#define ZETA_Core_WhenEnPrint(...) ZETA_Core_Unused(__VA_ARGS__)

#endif

#define ZETA_Core_Pause         \
    ZETA_Core_ForcePrintCurPos; \
    std::cin.get();             \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_DebugPause ZETA_Core_WhenEnableDebug(ZETA_Core_Pause)

namespace zeta::core::debug_utils {

enum struct PhaseEnum : unsigned char {
    Internal = 0,
    External = 1,
};

enum PurposeEnum : unsigned char {
    Trace = 0,
    Info = 2,
    Warning = 1,
    Assert = 3,
};

enum struct ChannelEnum : unsigned char {
    Main = 0,
    Debug = 1,
    Persistance = 2,
};

struct LogEntry {
    char const* file_name;
    unsigned long long line_num;
    char const* func_name;
    unsigned long long flow_id;
    unsigned long long time_stamp;

    PhaseEnum phase;
    PurposeEnum purpose;

    void* elem;

    std::string (*serialize_to_txt_func)(void* elem);
};

struct LogQueue {
    std::deque<LogEntry> entries;
};

struct FieldFormat {
    unsigned indent;
    unsigned width;
};

namespace default_field_format {

constexpr FieldFormat name{
    .indent = 8,
    .width = 12,
};

constexpr FieldFormat type{
    .indent = 8,
    .width = 24,
};

constexpr FieldFormat bin{
    .indent = 8,
    .width = 72,
};

constexpr FieldFormat dec{
    .indent = 8,
    .width = 48,
};

constexpr FieldFormat hex{
    .indent = 8,
    .width = 8,
};

}  // namespace default_field_format

constexpr int file_width{ 36 };
constexpr int line_width{ 6 };
constexpr int func_width{ 24 };

constexpr int var_name_width{ 12 };
constexpr int type_str_width{ 24 };

constexpr int dec_width{ 20 };
constexpr int hex_width{ 16 };

constexpr int space_width{ 8 };

inline auto space_str{ std::string(space_width, ' ') };

inline std::ostringstream debug_str_stream;

template <typename T>
constexpr std::string GetTypeStr();

template <meta::IsEnum Enum>
constexpr std::string GetEnumNameStr(Enum e);

template <typename T, typename = void>
struct VarPrinter;  // IWYU pragma: export

std::ostream& PrintPos(std::ostream& os, char const* file, int line,
                       char const* func);

template <typename T>
std::ostream& PrintVar(std::ostream& os, char const* file, int line,
                       char const* func, char const* var_name, T const& var);

void PrintDebugStrStream();

void ClearDebugStrStream();

}  // namespace zeta::core::debug_utils
