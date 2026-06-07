#pragma once

#include <zeta/core/meta.hpp>

namespace zeta::core::compare {

namespace compare_type {

struct ThreeWay {};
struct EqualTo {};
struct NotEqualTo {};
struct Less {};
struct LessEqual {};
struct Greater {};
struct GreaterEqual {};

template <typename CompareTypeTag>
constexpr bool IsCompareType{
    meta::IsAnyOf<CompareTypeTag, ThreeWay, EqualTo, NotEqualTo, Less,
                  LessEqual, Greater, GreaterEqual>
};

};  // namespace compare_type

template <typename Comparator, typename A, typename B>
constexpr int Compare(Comparator const& cmptr, compare_type::ThreeWay, A&& a,
                      B&& b);

template <typename Comparator, typename CompareTypeTag, typename A, typename B,
          typename = meta::EnableIf<
              compare_type::IsCompareType<CompareTypeTag> &&
                  !meta::IsAnyOf<CompareTypeTag, compare_type::ThreeWay>,
              void>>
constexpr bool Compare(Comparator const& cmptr, CompareTypeTag, A&& a, B&& b);

template <typename Comparator, typename A, typename B>
void CheckContract();

template <typename A, typename B, typename = void>
struct BasicComparator;  // IWYU pragma: export

template <typename CompareTypeTag, typename A, typename B>
constexpr int BasicCompare(CompareTypeTag, A const& a, B const& y);

struct UniversalBasicComparator {
    template <typename CompareTypeTag, typename A, typename B>
    static constexpr int Compare(CompareTypeTag, A const& a, B const& b);
};

template <typename CompareTypeTag, typename A, typename B>
constexpr int TypeErasedBasicCompare(CompareTypeTag, void const* a,
                                     void const* b);

template <typename A, typename B>
struct CppStdBasicEqualTo {
    constexpr bool operator()(A const& a, B const& b) const;
};

template <typename A, typename B>
struct CppStdBasicNotEqualTo {
    constexpr bool operator()(A const& a, B const& b) const;
};

template <typename A, typename B>
struct CppStdBasicLess {
    constexpr bool operator()(A const& a, B const& b) const;
};

template <typename A, typename B>
struct CppStdBasicLessEqual {
    constexpr bool operator()(A const& a, B const& b) const;
};

template <typename A, typename B>
struct CppStdBasicGreater {
    constexpr bool operator()(A const& a, B const& b) const;
};

template <typename A, typename B>
struct CppStdBasicGreaterEqual {
    constexpr bool operator()(A const& a, B const& b) const;
};

}  // namespace zeta::core::compare
