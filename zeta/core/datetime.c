#include <zeta/core/datetime.h>
#include <zeta/core/debugger.h>

#define YearOffset_ (400 * 16)

static unsigned const acc_days_in_month_comm[] = {
    0,
    0,
    31,                                                         // 1
    31 + 28,                                                    // 2
    31 + 28 + 31,                                               // 3
    31 + 28 + 31 + 30,                                          // 4
    31 + 28 + 31 + 30 + 31,                                     // 5
    31 + 28 + 31 + 30 + 31 + 30,                                // 6
    31 + 28 + 31 + 30 + 31 + 30 + 31,                           // 7
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,                      // 8
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,                 // 9
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,            // 10
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,       // 11
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31,  // 12
};

static unsigned const acc_days_in_month_leap[] = {
    0,
    0,
    31,                                                         // 1
    31 + 29,                                                    // 2
    31 + 29 + 31,                                               // 3
    31 + 29 + 31 + 30,                                          // 4
    31 + 29 + 31 + 30 + 31,                                     // 5
    31 + 29 + 31 + 30 + 31 + 30,                                // 6
    31 + 29 + 31 + 30 + 31 + 30 + 31,                           // 7
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31,                      // 8
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30,                 // 9
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,            // 10
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,       // 11
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31,  // 12
};

static const Zeta_Core_Date leap_sec_hists[] = {
    (Zeta_Core_Date){ 1972, 6, 30 },  (Zeta_Core_Date){ 1972, 12, 31 },  //
    (Zeta_Core_Date){ 1973, 12, 31 },                                    //
    (Zeta_Core_Date){ 1974, 12, 31 },                                    //
    (Zeta_Core_Date){ 1975, 12, 31 },                                    //
    (Zeta_Core_Date){ 1976, 12, 31 },                                    //
    (Zeta_Core_Date){ 1977, 12, 31 },                                    //
    (Zeta_Core_Date){ 1978, 12, 31 },                                    //
    (Zeta_Core_Date){ 1979, 12, 31 },                                    //

    (Zeta_Core_Date){ 1981, 6, 30 },  //
    (Zeta_Core_Date){ 1982, 6, 30 },  //
    (Zeta_Core_Date){ 1983, 6, 30 },  //

    (Zeta_Core_Date){ 1985, 6, 30 },  //

    (Zeta_Core_Date){ 1987, 12, 31 },  //

    (Zeta_Core_Date){ 1989, 12, 31 },  //
    (Zeta_Core_Date){ 1990, 12, 31 },  //

    (Zeta_Core_Date){ 1992, 6, 30 },   //
    (Zeta_Core_Date){ 1993, 6, 30 },   //
    (Zeta_Core_Date){ 1994, 6, 30 },   //
    (Zeta_Core_Date){ 1995, 12, 31 },  //

    (Zeta_Core_Date){ 1997, 6, 30 },   //
    (Zeta_Core_Date){ 1998, 12, 31 },  //

    (Zeta_Core_Date){ 2005, 12, 31 },  //

    (Zeta_Core_Date){ 2008, 12, 31 },  //

    (Zeta_Core_Date){ 2012, 6, 30 },  //

    (Zeta_Core_Date){ 2015, 6, 30 },   //
    (Zeta_Core_Date){ 2016, 12, 31 },  //
};

#define IsLeapYear__(tmp_year, year)                                         \
    ({                                                                       \
        unsigned long long tmp_year = (year);                                \
        (tmp_year % 4 == 0) && (tmp_year % 100 != 0 || tmp_year % 400 == 0); \
    })

#define IsLeapYear_(year) IsLeapYear__(ZETA_Core_TmpName, (year))

#define AccDaysInYear__(tmp_y, abs_year)                     \
    ({                                                       \
        unsigned long long tmp_y = (abs_year) - 1;           \
        365 * tmp_y + tmp_y / 4 - tmp_y / 100 + tmp_y / 400; \
    })

#define AccDaysInYear_(abs_year) AccDaysInYear__(ZETA_Core_TmpName, (abs_year))

unsigned long long Zeta_Core_DateToAbsDay(Zeta_Core_Date date) {
    ZETA_Core_DebugAssert(ZETA_Core_DateTime_min_year <= date.year);
    ZETA_Core_DebugAssert(date.year <= ZETA_Core_DateTime_max_year);

    ZETA_Core_DebugAssert(1 <= date.month && date.month <= 12);

    bool_t is_leap_year = IsLeapYear_(date.year);

    unsigned const* acc_days_in_month =
        is_leap_year ? acc_days_in_month_leap : acc_days_in_month_comm;

    ZETA_Core_DebugAssert(1 <= date.day &&
                          date.day <= (int)(acc_days_in_month[date.month + 1] -
                                            acc_days_in_month[date.month]));

    return AccDaysInYear_(date.year - ZETA_Core_DateTime_min_year) +
           acc_days_in_month[date.month] + date.day - 1;
}

static unsigned long long FindY_(unsigned long long x) {
    /*
        365*y + floor(y/4) - floor(y/100) + floor(y/400) <= x

        y = 400*a + 100*b + 4*c + d
        0 <= a
        0 <= b <= 3
        0 <= c <= 24
        0 <= d <= 3

        146097*a + 36524*b + 1461*c + 365*d <= x
    */

    unsigned long long a = x / 146097;
    x %= 146097;

    unsigned long long b = ZETA_Core_GetMinOf(3, x / 36524);
    x -= 36524 * b;

    unsigned long long c = ZETA_Core_GetMinOf(24, x / 1461);
    x -= 1461 * c;

    unsigned long long d = ZETA_Core_GetMinOf(3, x / 365);

    return 400 * a + 100 * b + 4 * c + d;
}

Zeta_Core_Date Zeta_Core_AbsDayToDate(unsigned long long abs_day) {
    ZETA_Core_DebugAssert(0 <= abs_day);

    unsigned long long abs_year = FindY_(abs_day) + 1;

    abs_day -= AccDaysInYear_(abs_year);

    bool_t is_leap_year = IsLeapYear_(abs_year);

    int month_lb = 1;
    int month_rb = 12;

    unsigned const* acc_days_in_month =
        is_leap_year ? acc_days_in_month_leap : acc_days_in_month_comm;

    while (month_lb < month_rb) {
        int mid = (month_lb + month_rb + 1) / 2;

        if (acc_days_in_month[mid] <= abs_day) {
            month_lb = mid;
        } else {
            month_rb = mid - 1;
        }
    }

    abs_day -= acc_days_in_month[month_lb];

    return (Zeta_Core_Date){ ZETA_Core_DateTime_min_year + abs_year, month_lb,
                             abs_day + 1 };
}

int Zeta_Core_GetDayInWeek(Zeta_Core_Date date) {
    Zeta_Core_Date base = { .year = 1970, .month = 1, .day = 1 };

    unsigned long long base_abs_day = Zeta_Core_DateToAbsDay(base);
    unsigned long long cur_abs_day = Zeta_Core_DateToAbsDay(date);

    unsigned long long delta = base_abs_day <= cur_abs_day
                                   ? cur_abs_day - base_abs_day
                                   : base_abs_day - cur_abs_day;

    return (delta + 4) % 7;
}

unsigned long long Zeta_Core_UTCDateTimeToAbsUs(Zeta_Core_DateTime datetime) {
    Zeta_Core_Date date =
        (Zeta_Core_Date){ datetime.year, datetime.month, datetime.day };

    unsigned long long abs_day = Zeta_Core_DateToAbsDay(date);

    unsigned hist_lb = 0;
    unsigned hist_rb = sizeof(leap_sec_hists) / sizeof(leap_sec_hists[0]);

    while (hist_lb < hist_rb) {
        unsigned mid = (hist_lb + hist_rb) / 2;

        unsigned long long cur_abs_day =
            Zeta_Core_DateToAbsDay(leap_sec_hists[mid]);

        if (cur_abs_day < abs_day) {
            hist_lb = mid + 1;
        } else {
            hist_rb = mid;
        }
    }

    unsigned long long abs_sec =
        ((abs_day * 24 + datetime.hour) * 60 + datetime.min) * 60 +
        datetime.sec + hist_lb;

    return abs_sec * 1000000 + datetime.us;
}

Zeta_Core_DateTime Zeta_Core_AbsUsToUTCDateTime(unsigned long long abs_us) {
    ZETA_Core_DebugAssert(0 <= abs_us);

    unsigned long long abs_sec = abs_us / 1000000;
    unsigned long long us = abs_us % 1000000;

    unsigned leap_sec_hists_length =
        sizeof(leap_sec_hists) / sizeof(leap_sec_hists[0]);

    unsigned hist_lb = 0;
    unsigned hist_rb = leap_sec_hists_length;

    while (hist_lb < hist_rb) {
        unsigned mid = (hist_lb + hist_rb) / 2;

        unsigned long long cur_abs_sec =
            Zeta_Core_DateToAbsDay(leap_sec_hists[mid]) * 86400 + mid + 86401;

        if (cur_abs_sec <= abs_sec) {
            hist_lb = mid + 1;
        } else {
            hist_rb = mid;
        }
    }

    if (hist_lb < leap_sec_hists_length) {
        unsigned long long hist_leap_abs_sec =
            Zeta_Core_DateToAbsDay(leap_sec_hists[hist_lb]) * 86400 + hist_lb +
            86400;

        if (abs_sec == hist_leap_abs_sec) {
            Zeta_Core_DateTime ret;
            ret.year = leap_sec_hists[hist_lb].year;
            ret.month = leap_sec_hists[hist_lb].month;
            ret.day = leap_sec_hists[hist_lb].day;
            ret.hour = 23;
            ret.min = 59;
            ret.sec = 60;
            ret.us = us;

            return ret;
        }
    }

    unsigned long long k = abs_sec - hist_lb;

    Zeta_Core_DateTime ret;

    ret.us = us;

    ret.sec = k % 60;
    k /= 60;

    ret.min = k % 60;
    k /= 60;

    ret.hour = k % 24;
    k /= 24;

    Zeta_Core_Date date = Zeta_Core_AbsDayToDate(k);

    ret.year = date.year;
    ret.month = date.month;
    ret.day = date.day;

    return ret;
}

unsigned long long Zeta_Core_GMTDateTimeToAbsUs(Zeta_Core_DateTime datetime) {
    Zeta_Core_Date date =
        (Zeta_Core_Date){ datetime.year, datetime.month, datetime.day };

    unsigned long long ret = Zeta_Core_DateToAbsDay(date);
    ret = ret * 24 + datetime.hour;
    ret = ret * 60 + datetime.min;
    ret = ret * 60 + datetime.sec;
    ret = ret * 1000000 + datetime.us;

    return ret;
}

Zeta_Core_DateTime Zeta_Core_AbsUsToGMTDateTime(unsigned long long abs_us) {
    ZETA_Core_DebugAssert(0 <= abs_us);

    unsigned long long k = abs_us;

    Zeta_Core_DateTime ret;

    ret.us = k % 1000000;
    k /= 1000000;

    ret.sec = k % 60;
    k /= 60;

    ret.min = k % 60;
    k /= 60;

    ret.hour = k % 24;
    k /= 24;

    Zeta_Core_Date date = Zeta_Core_AbsDayToDate(k);

    ret.year = date.year;
    ret.month = date.month;
    ret.day = date.day;

    return ret;
}
