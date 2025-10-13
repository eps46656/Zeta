#pragma once

#include <sanitizer/common_interface_defs.h>

#include <iostream>
#include <sstream>
#include <zeta/core/define.hpp>

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

#define ZETA_Core_PrintCurPos                                        \
    zeta::core::debug_utils::PrintPos(std::cout, __FILE__, __LINE__, \
                                      __PRETTY_FUNCTION__)           \
        << '\n';                                                     \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_Debug_PrintCurPos                                    \
    zeta::core::debug_utils::PrintPos(                                 \
        zeta::core::debug_utils::debug_str_stream, __FILE__, __LINE__, \
        __PRETTY_FUNCTION__)                                           \
        << '\n';                                                       \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_PrintVar(var)                                      \
    zeta::core::debug_utils::PrintVar(std::cout, __FILE__, __LINE__, \
                                      __PRETTY_FUNCTION__,           \
                                      ZETA_Core_ToStr(var), (var))   \
        << '\n';                                                     \
    if (ZETA_Core_ImmPrint) { std::cout.flush(); }                   \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_Debug_PrintVar(var)                                  \
    zeta::core::debug_utils::PrintVar(                                 \
        zeta::core::debug_utils::debug_str_stream, __FILE__, __LINE__, \
        __PRETTY_FUNCTION__, ZETA_Core_ToStr(var), (var))              \
        << '\n';                                                       \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_DebugAssert_(tmp_cond, cond)                                 \
    {                                                                          \
        auto tmp_cond{ cond };                                                 \
                                                                               \
        if (tmp_cond) {                                                        \
        } else {                                                               \
            ZETA_Core_PrintVar("Debug Assert !!!");                            \
                                                                               \
            zeta::core::debug_utils::PrintVar(std::cout, __FILE__, __LINE__,   \
                                              __PRETTY_FUNCTION__,             \
                                              ZETA_Core_ToStr(cond), tmp_cond) \
                << '\n';                                                       \
                                                                               \
            zeta::core::debug_utils::PrintDebugStrStream();                    \
                                                                               \
            ZETA_Core_PrintStackTrace;                                         \
                                                                               \
            std::cout.flush();                                                 \
                                                                               \
            exit(1);                                                           \
        }                                                                      \
    }

#define ZETA_Core_DebugAssert(...) \
    ZETA_Core_WhenEnableDebug(     \
        ZETA_Core_DebugAssert_(ZETA_Core_TmpName, (__VA_ARGS__)))

// -----------------------------------------------------------------------------

namespace zeta::core::debug_utils {

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
std::string GetTypeStr();

template <typename T, typename = void>
struct PrintVarCore;

template <typename T>
PrintVarCore<T> const& GetPrintVarCore();

std::ostream& PrintPos(std::ostream& os, char const* file, int line,
                       char const* func);

template <typename T>
std::ostream& PrintVar(std::ostream& os, char const* file, int line,
                       char const* func, char const* var_name, T const& var);

void PrintDebugStrStream();

void ClearDebugStrStream();

}  // namespace zeta::core::debug_utils
