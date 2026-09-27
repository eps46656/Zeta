#pragma once

#include <zeta/core/debug_utils/diag.hpp>
#include <zeta/core/debug_utils/logging.ipp>

#if ZETA_Core_DebugEnable

#define ZETA_Core_DebugUtils_Diag_LogCurPos()                                  \
    {                                                                          \
        zeta::core::debug_utils::diag::diag_logger.Push({                      \
            .time_pos_log{ ZETA_Core_DebugUtils_Logging_MakeCurTimePosLog() }, \
            .log_contents{},                                                   \
        });                                                                    \
    }                                                                          \
    static_assert(true)

#else

#define ZETA_Core_DebugUtils_Diag_LogCurPos() static_assert(true)

#endif

#if ZETA_Core_DebugEnable

#define ZETA_Core_DebugUtils_Diag_LogMsg(var, ...)                             \
    {                                                                          \
        zeta::core::debug_utils::diag::diag_logger.Push({                      \
            .time_pos_log{ ZETA_Core_DebugUtils_Logging_MakeCurTimePosLog() }, \
            .log_contents{                                                     \
                ZETA_Core_DebugUtils_Logging_MakeMsgLog(var, __VA_ARGS__),     \
            },                                                                 \
        });                                                                    \
    }                                                                          \
    static_assert(true)

#else

#define ZETA_Core_DebugUtils_Diag_LogMsg(var, ...) static_assert(true)

#endif

#if ZETA_Core_DebugEnable

#define ZETA_Core_DebugUtils_Diag_LogVar(var, ...)                             \
    {                                                                          \
        zeta::core::debug_utils::diag::diag_logger.Push({                      \
            .time_pos_log{ ZETA_Core_DebugUtils_Logging_MakeCurTimePosLog() }, \
            .log_contents{                                                     \
                ZETA_Core_DebugUtils_Logging_MakeVarLog(var, __VA_ARGS__),     \
            },                                                                 \
        });                                                                    \
    }                                                                          \
    static_assert(true)

#else

#define ZETA_Core_DebugUtils_Diag_LogVar(var, ...) static_assert(true)

#endif

#define ZETA_Core_DebugUtils_Diag_Assert_(tmp_cond, tmp_cond_log, cond)       \
    {                                                                         \
        decltype(auto) tmp_cond{ (cond) };                                    \
                                                                              \
        if (tmp_cond) {                                                       \
        } else {                                                              \
            zeta::core::debug_utils::logging::Log tmp_cond_log{               \
                .time_pos_log{                                                \
                    ZETA_Core_DebugUtils_Logging_MakeCurTimePosLog() },       \
                .log_contents{ zeta::core::debug_utils::logging::LogContent{  \
                    .type{ zeta::core::debug_utils::logging::GetTypeStr<      \
                        decltype(cond)>() },                                  \
                    .name{ #cond },                                           \
                    .vals{ zeta::core::debug_utils::logging::detail::ToStr_(  \
                        tmp_cond) },                                          \
                } },                                                          \
            };                                                                \
                                                                              \
            zeta::core::debug_utils::diag::detail::PrintBanner_(              \
                zeta::core::debug_utils::diag::diag_os,                       \
                zeta::core::debug_utils::diag::assert_block_banner_width,     \
                " Assert Begin ");                                            \
                                                                              \
            ZETA_Core_DebugUtils_Logging_PrintLog(                            \
                zeta::core::debug_utils::diag::diag_os, tmp_cond_log, false); \
                                                                              \
            zeta::core::debug_utils::diag::diag_logger.Flush(                 \
                zeta::core::debug_utils::diag::diag_os);                      \
                                                                              \
            ZETA_Core_DebugUtils_PrintStackTrace();                           \
                                                                              \
            ZETA_Core_DebugUtils_Logging_PrintLog(                            \
                zeta::core::debug_utils::diag::diag_os, tmp_cond_log, false); \
                                                                              \
            zeta::core::debug_utils::diag::detail::PrintBanner_(              \
                zeta::core::debug_utils::diag::diag_os,                       \
                zeta::core::debug_utils::diag::assert_block_banner_width,     \
                " Assert End ");                                              \
                                                                              \
            zeta::core::debug_utils::diag::diag_os.flush();                   \
                                                                              \
            std::abort();                                                     \
        }                                                                     \
    }

#define ZETA_Core_DebugUtils_Diag_Assert(cond)                              \
    ZETA_Core_DebugUtils_Diag_Assert_(ZETA_Core_TmpName, ZETA_Core_TmpName, \
                                      cond)

#define ZETA_Core_DebugUtils_Diag_PromiseAssert(cond) \
    ZETA_Core_DebugUtils_Diag_Assert(cond)

#define ZETA_Core_DebugUtils_Diag_ProbeAssert(cond) \
    ZETA_Core_DebugUtils_Diag_Assert(cond)

#define ZETA_Core_DebugUtils_Diag_Unreachable()         \
    {                                                   \
        ZETA_Core_DebugUtils_Diag_PromiseAssert(false); \
        __builtin_unreachable();                        \
    }                                                   \
    static_assert(true)

namespace zeta::core::debug_utils {

inline logging::CircularLogger diag::diag_logger{ 4096 };

namespace diag::detail {

constexpr void PrintBanner_(std::ostream& os, size_t width,
                            std::string const& title) {
    logging::detail::OStreamFormatReset_(diag_os);

    auto print_line{ [&os](size_t width, bool make_newline) {
        os << std::setfill('=');

        os << ZETA_Core_DebugUtils_Diag_BannerColorCode
           << std::setw(static_cast<unsigned>(width)) << ""
           << ZETA_Core_DebugUtils_Logging_ResetColorCode;

        if (make_newline) { os << ZETA_Core_DebugUtils_Logging_Newline; }

        os << std::setfill(' ');
    } };

    print_line(width, true);

    size_t width_l{ (width - title.size()) / 2 };
    size_t width_r{ width - title.size() - width_l };

    print_line(width_l, false);

    os << ZETA_Core_DebugUtils_Diag_BannerColorCode << title
       << ZETA_Core_DebugUtils_Logging_ResetColorCode;

    print_line(width_r, true);

    print_line(width, true);
}

}  // namespace diag::detail

}  // namespace zeta::core::debug_utils
