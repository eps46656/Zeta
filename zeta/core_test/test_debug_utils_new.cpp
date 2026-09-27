#include <iostream>
#include <zeta/core/debug_utils/diag.ipp>

constexpr void main1() {
    for (int i{ 0 }; i < 10; ++i) {
        zeta::core::debug_utils::PrintLog(
            std::cout, zeta::core::debug_utils::Log{
                           .time_pos_log{
                               .time = std::chrono::system_clock::now(),
                               .file = __FILE__,
                               .line = __LINE__,
                           },
                           .var_logs{
                               zeta::core::debug_utils::VarLog{
                                   .var_type = "int",
                                   .var_name = "test_var",
                                   .var_vals{ "42" },
                               },
                           },
                       });
    }

    zeta::core::debug_utils::PrintLog(
        std::cout,
        zeta::core::debug_utils::Log{
            .time_pos_log{
                .time = std::chrono::system_clock::now(),
                .file = __FILE__,
                .line = __LINE__,
            },
            .var_logs{
                zeta::core::debug_utils::VarLog{
                    .var_type = "int",
                    .var_name =
                        "long long long long long long long long long long var",
                    .var_vals{ "42" },
                },
            },
        });

    zeta::core::debug_utils::PrintLog(
        std::cout,
        zeta::core::debug_utils::Log{
            .time_pos_log{
                .time = std::chrono::system_clock::now(),
                .file = __FILE__,
                .line = __LINE__,
            },
            .var_logs{
                zeta::core::debug_utils::VarLog{
                    .var_type =
                        "long long long long long long long long long long int",
                    .var_name = "var",
                    .var_vals{ "42" },
                },
            },
        });

    zeta::core::debug_utils::PrintLog(
        std::cout,
        zeta::core::debug_utils::Log{
            .time_pos_log{
                .time = std::chrono::system_clock::now(),
                .file = __FILE__,
                .line = __LINE__,
            },
            .var_logs{
                zeta::core::debug_utils::VarLog{
                    .var_type =
                        "long long long long long long long long long long int",
                    .var_name =
                        "long long long long long long long long long long var",
                    .var_vals{ "42" },
                },
                zeta::core::debug_utils::VarLog{
                    .var_type =
                        "long long long long long long long long long long int",
                    .var_name =
                        "long long long long long long long long long long var",
                    .var_vals{ "42" },
                },
                zeta::core::debug_utils::VarLog{
                    .var_type =
                        "long long long long long long long long long long int",
                    .var_name =
                        "long long long long long long long long long long var",
                    .var_vals{ "42" },
                },
                zeta::core::debug_utils::VarLog{
                    .var_type = "int",
                    .var_name = "test_var",
                    .var_vals{ "42" },
                },
            },
        });

    zeta::core::debug_utils::PrintLog(
        std::cout,
        zeta::core::debug_utils::Log{
            .time_pos_log{
                .time = std::chrono::system_clock::now(),
                .file = __FILE__,
                .line = __LINE__,
            },
            .var_logs{
                zeta::core::debug_utils::VarLog{
                    .var_type =
                        "long long long long long long long long long long int",
                    .var_name = "var",
                    .var_vals{ "42" },
                },
                zeta::core::debug_utils::VarLog{
                    .var_type =
                        "long long long long long long long long long long int",
                    .var_name = "var",
                    .var_vals{ "42" },
                },
                zeta::core::debug_utils::VarLog{
                    .var_type =
                        "long long long long long long long long long long int",
                    .var_name = "var",
                    .var_vals{ "42" },
                },
            },
        });

    for (int i{ 0 }; i < 10; ++i) {
        zeta::core::debug_utils::PrintLog(
            std::cout, zeta::core::debug_utils::Log{
                           .time_pos_log{
                               .time = std::chrono::system_clock::now(),
                               .file = __FILE__,
                               .line = __LINE__,
                           },
                           .var_logs{
                               zeta::core::debug_utils::VarLog{
                                   .var_type = "int",
                                   .var_name = "test_var",
                                   .var_vals{ "42" },
                               },
                               zeta::core::debug_utils::VarLog{
                                   .var_type = "int",
                                   .var_name = "test_var",
                                   .var_vals{ "42" },
                               },
                               zeta::core::debug_utils::VarLog{
                                   .var_type = "int",
                                   .var_name = "test_var",
                                   .var_vals{ "42" },
                               },
                               zeta::core::debug_utils::VarLog{
                                   .var_type = "int",
                                   .var_name = "test_var",
                                   .var_vals{ "42" },
                               },
                               zeta::core::debug_utils::VarLog{
                                   .var_type = "int",
                                   .var_name = "test_var",
                                   .var_vals{ "42" },
                               },
                           },
                       });
    }
}

constexpr void main2() {
    int a{ 42 };
    int b{ -42 };
    unsigned int c{ 0xFFFE4665 };
    int* d{ &a };

    ZETA_Core_DebugUtils_Diag_LogVar(a,
                                     zeta::core::debug_utils::BinFormatTag{});

    ZETA_Core_DebugUtils_Diag_LogVar(a,
                                     zeta::core::debug_utils::DecFormatTag{});

    ZETA_Core_DebugUtils_Diag_LogVar(a,
                                     zeta::core::debug_utils::HexFormatTag{});

    ZETA_Core_DebugUtils_Diag_LogVar(a);

    ZETA_Core_DebugUtils_Diag_LogVar(b,
                                     zeta::core::debug_utils::BinFormatTag{});

    ZETA_Core_DebugUtils_Diag_LogVar(b,
                                     zeta::core::debug_utils::DecFormatTag{});

    ZETA_Core_DebugUtils_Diag_LogVar(b,
                                     zeta::core::debug_utils::HexFormatTag{});

    ZETA_Core_DebugUtils_Diag_LogVar(b);

    ZETA_Core_DebugUtils_Diag_LogVar(c,
                                     zeta::core::debug_utils::BinFormatTag{});

    ZETA_Core_DebugUtils_Diag_LogVar(c,
                                     zeta::core::debug_utils::DecFormatTag{});

    ZETA_Core_DebugUtils_Diag_LogVar(c,
                                     zeta::core::debug_utils::HexFormatTag{});

    ZETA_Core_DebugUtils_Diag_LogVar(
        c, zeta::core::debug_utils::BinHexFormatTag{});

    ZETA_Core_DebugUtils_Diag_LogVar(c);

    ZETA_Core_DebugUtils_Diag_LogVar(d,
                                     zeta::core::debug_utils::BinFormatTag{});

    ZETA_Core_DebugUtils_Diag_LogVar(d,
                                     zeta::core::debug_utils::HexFormatTag{});

    ZETA_Core_DebugUtils_Diag_LogVar(d);

    ZETA_Core_DebugUtils_Diag_LogCurPos();
    ZETA_Core_DebugUtils_Diag_LogCurPos();
    ZETA_Core_DebugUtils_Diag_LogCurPos();
    ZETA_Core_DebugUtils_Diag_LogCurPos();

    ZETA_Core_DebugUtils_Diag_PromiseAssert(false);
}

int main() {
    main2();
    return 0;
}
