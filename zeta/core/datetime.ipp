#pragma once

#include <zeta/core/array.ipp>
#include <zeta/core/comparison.ipp>
#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/datetime.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/integral_utils.ipp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

namespace datetime::detail {

constexpr unsigned day_cnt_in_month_[2][13]{
    //    1   2   3   4   5   6   7   8   9  10  11  12
    { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
};

constexpr array::Array<array::Array<unsigned, 14>, 2> acc_day_cnt_in_month_{
    array::Array<unsigned, 14>{
        0,                                                          // 0
        0,                                                          // 1
        31,                                                         // 2
        31 + 28,                                                    // 3
        31 + 28 + 31,                                               // 4
        31 + 28 + 31 + 30,                                          // 5
        31 + 28 + 31 + 30 + 31,                                     // 6
        31 + 28 + 31 + 30 + 31 + 30,                                // 7
        31 + 28 + 31 + 30 + 31 + 30 + 31,                           // 8
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,                      // 9
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,                 // 10
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,            // 11
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,       // 12
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31,  // 13
    },
    array::Array<unsigned, 14>{
        0,                                                          // 0
        0,                                                          // 1
        31,                                                         // 2
        31 + 29,                                                    // 3
        31 + 29 + 31,                                               // 4
        31 + 29 + 31 + 30,                                          // 5
        31 + 29 + 31 + 30 + 31,                                     // 6
        31 + 29 + 31 + 30 + 31 + 30,                                // 7
        31 + 29 + 31 + 30 + 31 + 30 + 31,                           // 8
        31 + 29 + 31 + 30 + 31 + 30 + 31 + 31,                      // 9
        31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30,                 // 10
        31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,            // 11
        31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,       // 12
        31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31,  // 13
    },
};

constexpr BaseIntegral epoch_day_cnt_{ 365 * 400 + 400 / 4 - 400 / 100 +
                                       400 / 400 };

constexpr array::Array<YearMonthDay, 27> utc_leap_sec_ymd_table_{
    YearMonthDay{ 1972, 6, 30 },   //
    YearMonthDay{ 1972, 12, 31 },  //

    YearMonthDay{ 1973, 12, 31 },  //
    YearMonthDay{ 1974, 12, 31 },  //
    YearMonthDay{ 1975, 12, 31 },  //
    YearMonthDay{ 1976, 12, 31 },  //
    YearMonthDay{ 1977, 12, 31 },  //
    YearMonthDay{ 1978, 12, 31 },  //
    YearMonthDay{ 1979, 12, 31 },  //

    YearMonthDay{ 1981, 6, 30 },  //
    YearMonthDay{ 1982, 6, 30 },  //
    YearMonthDay{ 1983, 6, 30 },  //

    YearMonthDay{ 1985, 6, 30 },  //

    YearMonthDay{ 1987, 12, 31 },  //

    YearMonthDay{ 1989, 12, 31 },  //
    YearMonthDay{ 1990, 12, 31 },  //

    YearMonthDay{ 1992, 6, 30 },   //
    YearMonthDay{ 1993, 6, 30 },   //
    YearMonthDay{ 1994, 6, 30 },   //
    YearMonthDay{ 1995, 12, 31 },  //

    YearMonthDay{ 1997, 6, 30 },   //
    YearMonthDay{ 1998, 12, 31 },  //

    YearMonthDay{ 2005, 12, 31 },  //

    YearMonthDay{ 2008, 12, 31 },  //

    YearMonthDay{ 2012, 6, 30 },  //

    YearMonthDay{ 2015, 6, 30 },   //
    YearMonthDay{ 2016, 12, 31 },  //
};

}  // namespace datetime::detail

template <comparison::IsOpTag OpTag>
constexpr auto comparison::
    BasicComparator<datetime::HourMinSec, datetime::HourMinSec>::Compare(
        comparison::Tag, OpTag op_tag, datetime::HourMinSec const& a,
        datetime::HourMinSec const& b) {
    return comparison_utils::BasicPairWiseLexCompare(  //
        op_tag,                                        //
        a.hour, b.hour,                                //
        a.min, b.min,                                  //
        a.sec, b.sec                                   //
    );
}

template <comparison::IsOpTag OpTag>
constexpr auto
comparison::BasicComparator<datetime::MonthDay, datetime::MonthDay>::Compare(
    comparison::Tag, OpTag op, datetime::MonthDay const& a,
    datetime::MonthDay const& b) {
    return comparison_utils::BasicPairWiseLexCompare(  //
        op,                                            //
        a.month, b.month,                              //
        a.day, b.day                                   //
    );
}

constexpr bool datetime::YearMonthDay::IsValid(this YearMonthDay const& self) {
    return (1 <= self.month && self.month <= 12) &&
           (1 <= self.day &&
            self.day <=
                detail::day_cnt_in_month_[(IsLeapYear)(self.year)][self.month]);
}

template <comparison::IsOpTag OpTag>
constexpr auto comparison::
    BasicComparator<datetime::YearMonthDay, datetime::YearMonthDay>::Compare(
        comparison::Tag, OpTag op_tag, datetime::YearMonthDay const& a,
        datetime::YearMonthDay const& b) {
    return comparison_utils::BasicPairWiseLexCompare(  //
        op_tag,                                        //
        a.year, b.year,                                //
        a.month, b.month,                              //
        a.day, b.day                                   //
    );
}

template <comparison::IsOpTag OpTag>
constexpr auto comparison::BasicComparator<datetime::YearMonthDayHourMinSec,
                                           datetime::YearMonthDayHourMinSec>::
    Compare(comparison::Tag, OpTag op_tag,
            datetime::YearMonthDayHourMinSec const& a,
            datetime::YearMonthDayHourMinSec const& b) {
    return comparison_utils::BasicPairWiseLexCompare(  //
        op_tag,                                        //
        a.year, b.year,                                //
        a.month, b.month,                              //
        a.day, b.day,                                  //
        a.hour, b.hour,                                //
        a.min, b.min,                                  //
        a.sec, b.sec                                   //
    );
}

namespace datetime::detail {

constexpr YearMonthDay const* YearMonthDay_LowerBound_(
    YearMonthDay const* ymd_table, size_t table_size,
    YearMonthDay const& ymd_key) {
    while (0 < table_size) {
        size_t half_size{ table_size / 2 };

        YearMonthDay const& mid{ ymd_table[half_size] };

        if (ymd_key <= mid) {
            table_size = half_size;
        } else {
            ymd_table += half_size + 1;
            table_size -= half_size + 1;
        }
    }

    return ymd_table;
}

inline YearMonthDay const* YearMonthDay_UpperBound_(
    YearMonthDay const* ymd_table, size_t table_size,
    YearMonthDay const& ymd_key) {
    while (0 < table_size) {
        size_t half_size{ table_size / 2 };

        YearMonthDay const& mid{ ymd_table[half_size] };

        if (ymd_key < mid) {
            table_size = half_size;
        } else {
            ymd_table += half_size + 1;
            table_size -= half_size + 1;
        }
    }

    return ymd_table;
}

}  // namespace datetime::detail

template <datetime::IsTimeStandardLike TimeStandardLike>
constexpr bool datetime::YearMonthDayHourMinSec::IsValid(
    this YearMonthDayHourMinSec const& self,
    TimeStandardLike time_standard_like) {
    TimeStandard ts{ [=]() {
        if constexpr (meta::IsValueWrapperT<TimeStandardLike, TimeStandard>) {
            return time_standard_like.value;
        } else if constexpr (meta::IsSame<TimeStandardLike, TimeStandard>) {
            return time_standard_like;
        } else {
            ZETA_Core_StaticAssert(false);
        }
    } };

    if (!(1 <= self.month && self.month <= 12)) { return false; }

    if (!(1 <= self.day &&
          self.day <=
              detail::day_cnt_in_month_[(IsLeapYear)(self.year)][self.month])) {
        return false;
    }

    if (!(self.hour <= 23)) { return false; }

    if (!(self.min <= 59)) { return false; }

    if (ts == TimeStandard::GMT) {
        if (!(self.sec <= 59)) { return false; }
        return true;
    }

    if (ts == TimeStandard::UTC) {
        if (self.sec <= 59) { return true; }
        if (60 < self.sec) { return false; }

        if (self.hour != 23 || self.min != 59) { return false; }

        // 23:59:60 is valid only if it is leaping.

        YearMonthDay self_ymd{ self.year, self.month, self.day };

        auto* ymd{ detail::YearMonthDay_LowerBound_(
            detail::utc_leap_sec_ymd_table_.elems,
            detail::utc_leap_sec_ymd_table_.N, self_ymd) };

        return ymd != detail::utc_leap_sec_ymd_table_.elems +
                          detail::utc_leap_sec_ymd_table_.N &&
               *ymd == self_ymd;
    }

    ZETA_Core_Unreachable();
}

constexpr bool datetime::IsLeapYear(BaseIntegral year) {
    return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
}

constexpr unsigned datetime::GetYearDayOffsetFromMonthDay(bool is_leap_year,
                                                          MonthDay const& md) {
    return detail::acc_day_cnt_in_month_[is_leap_year][md.month] + md.day - 1U;
}

constexpr datetime::MonthDay datetime::GetMonthDayFromYearDayOffset(
    bool is_leap_year, unsigned day_offset_in_year) {
    ZETA_Core_DebugAssert(day_offset_in_year < 365U + is_leap_year);

    unsigned cur_month{ day_offset_in_year / 31 + 1 };

    day_offset_in_year -=
        detail::acc_day_cnt_in_month_[is_leap_year][cur_month];

    {
        unsigned cur_day_cnt_in_month{
            detail::day_cnt_in_month_[is_leap_year][cur_month]
        };

        if (cur_day_cnt_in_month <= day_offset_in_year) {
            ++cur_month;
            day_offset_in_year -= cur_day_cnt_in_month;
        }
    }

    return {
        .month = cur_month,
        .day = day_offset_in_year + 1U,
    };
}

constexpr datetime::BaseIntegral datetime::GetGlobalDayOffsetFromYearMonthDay(
    YearMonthDay const& ymd) {
    BaseIntegral global_year{ ymd.year - global_year_base };

    BaseIntegral global_epoch{ global_year / 400 };
    BaseIntegral global_epoch_year{ global_year % 400 };

    if (global_epoch_year < 0) {
        --global_epoch;
        global_epoch_year += 400;
    }

    return detail::epoch_day_cnt_ * global_epoch +
           (global_epoch_year == 0
                ? 0
                : 365 * global_epoch_year + (global_epoch_year - 1) / 4 -
                      (global_epoch_year - 1) / 100 + 1) +
           GetYearDayOffsetFromMonthDay((IsLeapYear)(global_epoch_year),
                                        { ymd.month, ymd.day });
}

constexpr datetime::YearMonthDay datetime::GetYearMonthDayFromGlobalDayOffset(
    BaseIntegral global_day_offset) {
    BaseIntegral epoch{ global_day_offset / detail::epoch_day_cnt_ };
    BaseIntegral epoch_day_offset{ global_day_offset % detail::epoch_day_cnt_ };

    if (epoch_day_offset < 0) {
        --epoch;
        epoch_day_offset += detail::epoch_day_cnt_;
    }

    BaseIntegral year;

    if (epoch_day_offset < 366) {
        year = 0;
    } else {
        epoch_day_offset -= 366;

        BaseIntegral a{ epoch_day_offset / 36524 };
        epoch_day_offset %= 36524;

        BaseIntegral b{ epoch_day_offset / 1461 };
        epoch_day_offset %= 1461;

        BaseIntegral c{ 0 };

        if (365 * 2 <= epoch_day_offset) {
            c += 2;
            epoch_day_offset -= 365 * 2;
        }

        if (365 <= epoch_day_offset) {
            ++c;
            epoch_day_offset -= 365;
        }

        year = 100 * a + 4 * b + c + 1;
    }

    MonthDay md{ (GetMonthDayFromYearDayOffset)((IsLeapYear)(year),
                                                static_cast<unsigned>(
                                                    epoch_day_offset)) };

    return {
        .year = global_year_base + 400 * epoch + year,
        .month = md.month,
        .day = md.day,
    };
}

template <datetime::IsTimeStandardLike TimeStandardLike>
constexpr datetime::BaseIntegral datetime::GetGlobalSecOffsetFromYearMonthDay(
    TimeStandardLike time_standard_like,
    datetime::YearMonthDayHourMinSec const& ymdhms) {
    TimeStandard ts{ [=]() {
        if constexpr (meta::IsValueWrapperT<TimeStandardLike, TimeStandard>) {
            return time_standard_like.value;
        } else if constexpr (meta::IsSame<TimeStandardLike, TimeStandard>) {
            return time_standard_like;
        } else {
            ZETA_Core_StaticAssert(false);
        }
    }() };

    BaseIntegral sec_offset{
        (60 * 60 * 24) *
            (GetGlobalDayOffsetFromYearMonthDay)({ ymdhms.year, ymdhms.month,
                                                   ymdhms.day }) +
        (60 * 60) * ymdhms.hour + 60 * ymdhms.min + ymdhms.sec
    };

    if (ts == TimeStandard::GMT) { return sec_offset; }

    if (ts == TimeStandard::UTC) {
        auto const* leap_ymd{ detail::YearMonthDay_LowerBound_(
            detail::utc_leap_sec_ymd_table_.elems,
            detail::utc_leap_sec_ymd_table_.N,
            { ymdhms.year, ymdhms.month, ymdhms.day }) };

        sec_offset += leap_ymd - detail::utc_leap_sec_ymd_table_.elems;

        return sec_offset;
    }

    ZETA_Core_Unreachable();
}

namespace datetime::detail {

constexpr array::Array<array::Array<BaseIntegral, 2>,
                       detail::utc_leap_sec_ymd_table_.N>
    utc_leap_sec_global_sec_offset_table_{ []() {
        array::Array<array::Array<BaseIntegral, 2>,
                     detail::utc_leap_sec_ymd_table_.N>
            arr;

        for (size_t i{ 0 }; i < detail::utc_leap_sec_ymd_table_.N; ++i) {
            arr[i] = {
                (GetGlobalSecOffsetFromYearMonthDay)(  //
                    TimeStandardTag::GMT{},
                    {
                        .year = detail::utc_leap_sec_ymd_table_[i].year,
                        .month = detail::utc_leap_sec_ymd_table_[i].month,
                        .day = detail::utc_leap_sec_ymd_table_[i].day,
                        .hour = 23,
                        .min = 59,
                        .sec = 59,
                    }),
                (GetGlobalSecOffsetFromYearMonthDay)(  //
                    TimeStandardTag::UTC{},
                    {
                        .year = detail::utc_leap_sec_ymd_table_[i].year,
                        .month = detail::utc_leap_sec_ymd_table_[i].month,
                        .day = detail::utc_leap_sec_ymd_table_[i].day,
                        .hour = 23,
                        .min = 59,
                        .sec = 60,
                    }),
            };
        }

        return arr;
    }() };

}  // namespace datetime::detail

template <datetime::IsTimeStandardLike TimeStandardLike>
constexpr datetime::YearMonthDayHourMinSec
datetime::GetYearMonthDayHourMinSecFromGlobalSecOffset(
    TimeStandardLike time_standard_like, BaseIntegral global_sec_offset) {
    TimeStandard ts{ [=]() {
        if constexpr (meta::IsValueWrapperT<TimeStandardLike, TimeStandard>) {
            return time_standard_like.value;
        } else if constexpr (meta::IsSame<TimeStandardLike, TimeStandard>) {
            return time_standard_like;
        } else {
            ZETA_Core_StaticAssert(false);
        }
    }() };

    if (ts == TimeStandard::GMT) {
        BaseIntegral global_day_offset{ global_sec_offset / (60 * 60 * 24) };
        BaseIntegral sec_offset_in_day{ global_sec_offset % (60 * 60 * 24) };

        if (sec_offset_in_day < 0) {
            --global_day_offset;
            sec_offset_in_day += (60 * 60 * 24);
        }

        YearMonthDay ymd{ GetYearMonthDayFromGlobalDayOffset(
            global_day_offset) };

        return {
            .year = ymd.year,
            .month = ymd.month,
            .day = ymd.day,
            .hour = static_cast<unsigned>(sec_offset_in_day / (60 * 60)),
            .min = static_cast<unsigned>((sec_offset_in_day / 60) % 60),
            .sec = static_cast<unsigned>(sec_offset_in_day % 60),
        };
    }

    if (ts == TimeStandard::UTC) {
        array::Array<BaseIntegral, 2> const* op_global_sec_offset{
            detail::utc_leap_sec_global_sec_offset_table_.elems
        };

        for (size_t op_global_sec_offset_size{
                 detail::utc_leap_sec_global_sec_offset_table_.N };
             0 < op_global_sec_offset_size;) {
            size_t half_size{ op_global_sec_offset_size / 2 };

            array::Array<BaseIntegral, 2> const* mid{ op_global_sec_offset +
                                                      half_size };

            if (global_sec_offset <= (*mid)[1]) {
                op_global_sec_offset_size = half_size;
            } else {
                op_global_sec_offset += half_size + 1;
                op_global_sec_offset_size -= half_size + 1;
            }
        }

        if (op_global_sec_offset !=
                detail::utc_leap_sec_global_sec_offset_table_.elems +
                    detail::utc_leap_sec_global_sec_offset_table_.N &&
            global_sec_offset == (*op_global_sec_offset)[1]) {
            YearMonthDay const& op_ymd{
                detail::utc_leap_sec_ymd_table_[static_cast<size_t>(
                    op_global_sec_offset -
                    detail::utc_leap_sec_global_sec_offset_table_.elems)]
            };

            return {
                .year = op_ymd.year,
                .month = op_ymd.month,
                .day = op_ymd.day,
                .hour = 23,
                .min = 59,
                .sec = 60,
            };
        }

        if (op_global_sec_offset ==
            detail::utc_leap_sec_global_sec_offset_table_.elems) {
            return (GetYearMonthDayHourMinSecFromGlobalSecOffset)(  //
                TimeStandardTag::GMT{}, global_sec_offset);
        }

        --op_global_sec_offset;

        return (GetYearMonthDayHourMinSecFromGlobalSecOffset)(  //
            TimeStandardTag::GMT{}, (*op_global_sec_offset)[0] +
                                        global_sec_offset -
                                        (*op_global_sec_offset)[1]);
    }

    ZETA_Core_Unreachable();
}

}  // namespace zeta::core
