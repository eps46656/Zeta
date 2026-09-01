#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/pair.hpp>

namespace zeta::core::datetime {

using BaseIntegral = long long;

constexpr BaseIntegral year_range_half{ integral::RangeMinOf<BaseIntegral> /
                                        86400 / 8 };

constexpr BaseIntegral global_year_base{ 2000 };

constexpr BaseIntegral year_range_min{ global_year_base - year_range_half };

constexpr BaseIntegral year_range_max{ global_year_base + year_range_half };

enum struct TimeStandard : unsigned char {
    GMT = 0,
    UTC = 1,
};

struct TimeStandardTag {
    using GMT = meta::AutoValueWrapper<TimeStandard::GMT>;
    using UTC = meta::AutoValueWrapper<TimeStandard::UTC>;
};

template <typename T>
concept IsTimeStandardLike =
    meta::IsAnySame<T, TimeStandard, TimeStandardTag::GMT,
                    TimeStandardTag::UTC>;

struct HourMinSec {
    unsigned hour : 5;  // 0 ~ 23
    unsigned min : 6;   // 0 ~ 59
    unsigned sec : 6;   // 0 ~ 60

    constexpr bool operator==(HourMinSec const& other) const = default;
    constexpr bool operator!=(HourMinSec const& other) const = default;
};

}  // namespace zeta::core::datetime

namespace zeta::core {

template <>
struct comparison::BasicComparator<datetime::HourMinSec, datetime::HourMinSec> {
    template <IsOpTag OpTag>
    static constexpr auto Compare(comparison::Tag, OpTag,
                                  datetime::HourMinSec const& a,
                                  datetime::HourMinSec const& b);
};

template <>
struct comparison::EnableNativeOperatorByBasicComparison<datetime::HourMinSec,
                                                         datetime::HourMinSec> {
    static constexpr bool enable_equal{ false };
    static constexpr bool enable_not_equal{ false };
    static constexpr bool enable_less{ true };
    static constexpr bool enable_less_equal{ true };
    static constexpr bool enable_greater{ true };
    static constexpr bool enable_greater_equal{ true };
};

}  // namespace zeta::core

namespace zeta::core::datetime {

struct MonthDay {
    unsigned month : 4;  // 1 ~ 12
    unsigned day : 5;    // 1 ~ 31

    constexpr bool operator==(MonthDay const& other) const = default;
    constexpr bool operator!=(MonthDay const& other) const = default;
};

}  // namespace zeta::core::datetime

namespace zeta::core {

template <>
struct comparison::BasicComparator<datetime::MonthDay, datetime::MonthDay> {
    template <IsOpTag OpTag>
    static constexpr auto Compare(comparison::Tag, OpTag,
                                  datetime::MonthDay const& a,
                                  datetime::MonthDay const& b);
};

template <>
struct comparison::EnableNativeOperatorByBasicComparison<datetime::MonthDay,
                                                         datetime::MonthDay> {
    static constexpr bool enable_equal{ false };
    static constexpr bool enable_not_equal{ false };
    static constexpr bool enable_less{ true };
    static constexpr bool enable_less_equal{ true };
    static constexpr bool enable_greater{ true };
    static constexpr bool enable_greater_equal{ true };
};

}  // namespace zeta::core

namespace zeta::core::datetime {

struct YearMonthDay {
    BaseIntegral year;
    unsigned month : 4;  // 1 ~ 12
    unsigned day : 5;    // 1 ~ 31

    constexpr bool IsValid(this YearMonthDay const& self);

    constexpr bool operator==(YearMonthDay const& other) const = default;
    constexpr bool operator!=(YearMonthDay const& other) const = default;
};

}  // namespace zeta::core::datetime

namespace zeta::core {

template <>
struct comparison::BasicComparator<datetime::YearMonthDay,
                                   datetime::YearMonthDay> {
    template <IsOpTag OpTag>
    static constexpr auto Compare(comparison::Tag, OpTag,
                                  datetime::YearMonthDay const& a,
                                  datetime::YearMonthDay const& b);
};

template <>
struct comparison::EnableNativeOperatorByBasicComparison<
    datetime::YearMonthDay, datetime::YearMonthDay> {
    static constexpr bool enable_equal{ false };
    static constexpr bool enable_not_equal{ false };
    static constexpr bool enable_less{ true };
    static constexpr bool enable_less_equal{ true };
    static constexpr bool enable_greater{ true };
    static constexpr bool enable_greater_equal{ true };
};

}  // namespace zeta::core

namespace zeta::core::datetime {

struct YearMonthDayHourMinSec {
    BaseIntegral year;
    unsigned month : 4;  // 1 ~ 12
    unsigned day : 5;    // 1 ~ 31

    unsigned hour : 5;  // 0 ~ 23
    unsigned min : 6;   // 0 ~ 59
    unsigned sec : 6;   // 0 ~ 60

    template <IsTimeStandardLike TimeStandardLike>
    constexpr bool IsValid(this YearMonthDayHourMinSec const& self,
                           TimeStandardLike time_standard_like);

    constexpr bool operator==(YearMonthDayHourMinSec const& other) const =
        default;

    constexpr bool operator!=(YearMonthDayHourMinSec const& other) const =
        default;
};

}  // namespace zeta::core::datetime

namespace zeta::core {

template <>
struct comparison::BasicComparator<datetime::YearMonthDayHourMinSec,
                                   datetime::YearMonthDayHourMinSec> {
    template <IsOpTag OpTag>
    static constexpr auto Compare(comparison::Tag, OpTag,
                                  datetime::YearMonthDayHourMinSec const& a,
                                  datetime::YearMonthDayHourMinSec const& b);
};

template <>
struct comparison::EnableNativeOperatorByBasicComparison<
    datetime::YearMonthDayHourMinSec, datetime::YearMonthDayHourMinSec> {
    static constexpr bool enable_equal{ false };
    static constexpr bool enable_not_equal{ false };
    static constexpr bool enable_less{ true };
    static constexpr bool enable_less_equal{ true };
    static constexpr bool enable_greater{ true };
    static constexpr bool enable_greater_equal{ true };
};

}  // namespace zeta::core

namespace zeta::core::datetime {

constexpr bool IsLeapYear(BaseIntegral year);

constexpr unsigned GetYearDayOffsetFromMonthDay(bool is_leap_year,
                                                MonthDay const& md);

constexpr MonthDay GetMonthDayFromYearDayOffset(bool is_leap_year,
                                                unsigned day_offset_in_year);

constexpr BaseIntegral GetGlobalDayOffsetFromYearMonthDay(
    YearMonthDay const& ymd);

constexpr YearMonthDay GetYearMonthDayFromGlobalDayOffset(
    BaseIntegral global_day_offset);

template <IsTimeStandardLike TimeStandardLike>
constexpr BaseIntegral GetGlobalSecOffsetFromYearMonthDay(
    TimeStandardLike time_standard_like, YearMonthDayHourMinSec const& ymdhms);

template <IsTimeStandardLike TimeStandardLike>
constexpr YearMonthDayHourMinSec GetYearMonthDayHourMinSecFromGlobalSecOffset(
    TimeStandardLike time_standard_like, BaseIntegral global_sec_offset);

}  // namespace zeta::core::datetime
