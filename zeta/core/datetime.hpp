#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>

#if ZETA_Core_ullong_width == 32

#define ZETA_Core_DateTime_min_year (-400 * 16)
#define ZETA_Core_DateTime_max_year (+400 * (32 - 16))

#elif ZETA_Core_ullong_width == 64

#define ZETA_Core_DateTime_min_year (-400 * 16)
#define ZETA_Core_DateTime_max_year (+400 * (1024 - 16))

#else

#error "Unsupported architecture."

#endif

ZETA_Core_StaticAssert(ZETA_Core_DateTime_min_year % 400 == 0);

namespace zeta::core::datetime {

struct Date;
struct DateTime;

// -----------------------------------------------------------------------------

struct Date {
    int year;  // ZETA_Core_DateTime_min_year ~ ZETA_Core_DateTime_max_year
    unsigned char month;  // 1 ~ 12
    unsigned char day;    // 1 ~ 31
};

struct DateTime {
    int year;  // ZETA_Core_DateTime_min_year ~ ZETA_Core_DateTime_max_year
    unsigned char month;  // 1 ~ 12
    unsigned char day;    // 1 ~ 31

    unsigned char hour;  // 0 ~ 23
    unsigned char min;   // 0 ~ 59
    unsigned char sec;   // 0 ~ 60

    u32_t us;  // 0 ~ 999999
};

unsigned long long DateToAbsDay(Date date);

Date AbsDayToDate(unsigned long long abs_day);

int GetDayInWeek(Date date);

unsigned long long UTCDateTimeToAbsUs(DateTime datetime);

DateTime AbsUsToUTCDateTime(unsigned long long abs_us);

unsigned long long GMTDateTimeToAbsUs(DateTime datetime);

DateTime AbsUsToGMTDateTime(unsigned long long abs_us);

}  // namespace zeta::core::datetime
