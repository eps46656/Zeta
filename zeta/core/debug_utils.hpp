#pragma once

#if __has_feature(address_sanitizer)
#include <sanitizer/common_interface_defs.h>  // IWYU pragma: keep
#endif

#include <sstream>
#include <string>
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
        << "\n\n";                                                   \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_Debug_PrintCurPos                                    \
    zeta::core::debug_utils::PrintPos(                                 \
        zeta::core::debug_utils::debug_str_stream, __FILE__, __LINE__, \
        __PRETTY_FUNCTION__)                                           \
        << "\n\n";                                                     \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_Pause    \
    ZETA_Core_PrintCurPos; \
    std::cin.get();        \
    ZETA_Core_StaticAssert(true)

#if ZETA_Core_EnableDebug

#define ZETA_Core_DebugPause ZETA_Core_Pause

#else

#define ZETA_Core_DebugPause

#endif

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
constexpr std::string GetTypeStr();

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
