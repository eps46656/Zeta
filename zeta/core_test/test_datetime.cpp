#include <zeta/core/datetime.ipp>
#include <zeta/core_test/random.hpp>

inline void main1() {
    for (unsigned is_leap_year{ 0 }; is_leap_year < 2U; ++is_leap_year) {
        for (unsigned day_offset{ 0 }; day_offset < 365U + is_leap_year;
             ++day_offset) {
            unsigned month;
            unsigned day;

            {
                auto p{ zeta::core::datetime::GetMonthDayFromYearDayOffset(
                    is_leap_year, day_offset) };

                month = p.month;
                day   = p.day;
            }

            unsigned re_day_offset{
                zeta::core::datetime::GetYearDayOffsetFromMonthDay(
                    is_leap_year, { month, day })
            };

            ZETA_Core_DebugUtils_Diag_PromiseAssert(re_day_offset ==
                                                    day_offset);

            ZETA_Core_DebugUtils_Logging_ImmLogVar(day_offset);
        }
    }
}

inline void main2() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 1735663456U };

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

    ZETA_Core_DebugUtils_Logging_ImmLogVar(random_seed);
    ZETA_Core_DebugUtils_Logging_ImmLogVar(fixed_seed);
    ZETA_Core_DebugUtils_Logging_ImmLogVar(seed);

    zeta::core_test::random::SetRandomSeed(seed);

    long long global_day_offset_range_half{ 365 * 100000 };

    long long global_day_offset_range_min{ -global_day_offset_range_half };
    long long global_day_offset_range_max{ global_day_offset_range_half };

    for (unsigned test_i{ 0 }; test_i < 1024 * 1024; ++test_i) {
        long long global_day_offset{
            zeta::core_test::random::GenUniformRandomInt<long long>(
                global_day_offset_range_min, global_day_offset_range_max)
        };

        zeta::core::datetime::YearMonthDay ymd{
            zeta::core::datetime::GetYearMonthDayFromGlobalDayOffset(
                global_day_offset)
        };

        long long re_global_day_offset{
            zeta::core::datetime::GetGlobalDayOffsetFromYearMonthDay(ymd)
        };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(re_global_day_offset ==
                                                global_day_offset);
    }
}

inline void main3() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 1735663456U };

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

    ZETA_Core_DebugUtils_Logging_ImmLogVar(random_seed);
    ZETA_Core_DebugUtils_Logging_ImmLogVar(fixed_seed);
    ZETA_Core_DebugUtils_Logging_ImmLogVar(seed);

    zeta::core_test::random::SetRandomSeed(seed);

    long long global_sec_offset_range_half{ 86400LL * 365 * 100 };

    long long global_sec_offset_range_min{ -global_sec_offset_range_half };
    long long global_sec_offset_range_max{ global_sec_offset_range_half };

    for (unsigned test_i{ 0 }; test_i < 1024 * 16; ++test_i) {
        long long global_sec_offset{
            zeta::core_test::random::GenUniformRandomInt<long long>(
                global_sec_offset_range_min, global_sec_offset_range_max)
        };

        ZETA_Core_DebugUtils_Diag_LogVar(global_sec_offset);

        zeta::core::datetime::YearMonthDayHourMinSec ymdhms{
            zeta::core::datetime::GetYearMonthDayHourMinSecFromGlobalSecOffset(
                zeta::core::datetime::TimeStandard::UTC, global_sec_offset)
        };

        ZETA_Core_DebugUtils_Diag_LogVar(ymdhms.year);
        ZETA_Core_DebugUtils_Diag_LogVar(static_cast<unsigned>(ymdhms.month));
        ZETA_Core_DebugUtils_Diag_LogVar(static_cast<unsigned>(ymdhms.day));
        ZETA_Core_DebugUtils_Diag_LogVar(static_cast<unsigned>(ymdhms.hour));
        ZETA_Core_DebugUtils_Diag_LogVar(static_cast<unsigned>(ymdhms.min));
        ZETA_Core_DebugUtils_Diag_LogVar(static_cast<unsigned>(ymdhms.sec));

        long long re_global_sec_offset{
            zeta::core::datetime::GetGlobalSecOffsetFromYearMonthDay(
                zeta::core::datetime::TimeStandard::UTC, ymdhms)
        };

        ZETA_Core_DebugUtils_Diag_LogVar(re_global_sec_offset);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(re_global_sec_offset ==
                                                global_sec_offset);
    }
}

int main() {
    main3();

    ZETA_Core_DebugUtils_Logging_ImmLogVar("ok");

    return 0;
}
