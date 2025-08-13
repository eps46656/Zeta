#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_Date);
ZETA_Core_DeclareStruct(Zeta_Core_DateTime);

#define ZETA_Core_DateTime_min_year (-400 * 16)

#if ZETA_Core_ullong_width == 32
#define ZETA_Core_DateTime_max_year (ZETA_Core_DateTime_min_year + 400 * 16)
#elif ZETA_Core_ullong_width == 64
#define ZETA_Core_DateTime_max_year (ZETA_Core_DateTime_min_year + 400 * 1024)
#else
#error "Unsupported architecture."
#endif

ZETA_Core_StaticAssert(ZETA_Core_DateTime_min_year % 400 == 0);

struct Zeta_Core_Date {
    int year;
    short month;
    short day;
};

struct Zeta_Core_DateTime {
    int year;
    short month;
    short day;

    short hour;
    short min;
    short sec;
    int us;
};

unsigned long long Zeta_Core_DateToAbsDay(Zeta_Core_Date date);

Zeta_Core_Date Zeta_Core_AbsDayToDate(unsigned long long abs_day);

int Zeta_Core_GetDayInWeek(Zeta_Core_Date date);

unsigned long long Zeta_Core_UTCDateTimeToAbsUs(Zeta_Core_DateTime datetime);

Zeta_Core_DateTime Zeta_Core_AbsUsToUTCDateTime(unsigned long long abs_us);

unsigned long long Zeta_Core_GMTDateTimeToAbsUs(Zeta_Core_DateTime datetime);

Zeta_Core_DateTime Zeta_Core_AbsUsToGMTDateTime(unsigned long long abs_us);

ZETA_Core_ExternC_End;
