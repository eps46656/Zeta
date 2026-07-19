#pragma once

#include <zeta/core/meta.hpp>

namespace zeta::core::comparison {

struct ComparisonTypeEnum {
    using Value = unsigned char;

    struct ThreeWay {
        static constexpr Value value{ 0b0001 };
    };

    struct EqualTo {
        static constexpr Value value{ 0b0010 };
    };

    struct NotEqualTo {
        static constexpr Value value{ 0b0011 };
    };

    struct Less {
        static constexpr Value value{ 0b0100 };
    };

    struct LessEqual {
        static constexpr Value value{ 0b0101 };
    };

    struct Greater {
        static constexpr Value value{ 0b1000 };
    };

    struct GreaterEqual {
        static constexpr Value value{ 0b1001 };
    };
};

template <typename CompareType>
constexpr bool IsComparisonType{ meta::IsAnySame<
    CompareType, ComparisonTypeEnum::ThreeWay, ComparisonTypeEnum::EqualTo,
    ComparisonTypeEnum::NotEqualTo, ComparisonTypeEnum::Less,
    ComparisonTypeEnum::LessEqual, ComparisonTypeEnum::Greater,
    ComparisonTypeEnum::GreaterEqual> };

template <typename Comparator, typename A, typename B>
constexpr int Compare(Comparator const& cmptr, ComparisonTypeEnum::ThreeWay,
                      A&& a, B&& b);

template <typename Comparator, typename CompareType, typename A, typename B,
          typename = meta::EnableIf<
              IsComparisonType<CompareType> &&
                  !meta::IsSame<CompareType, ComparisonTypeEnum::ThreeWay>,
              void>>
constexpr bool Compare(Comparator const& cmptr, CompareType, A&& a, B&& b);

template <typename Comparator, typename A, typename B>
void CheckContract();

template <typename A, typename B, typename = void>
struct BasicComparator;  // IWYU pragma: export

template <typename CompareType, typename A, typename B>
constexpr auto BasicCompare(CompareType, A const& a, B const& y);

struct UniversalBasicComparator {
    template <typename CompareType, typename A, typename B>
    static constexpr auto Compare(CompareType, A const& a, B const& b);
};

template <typename CompareType, typename A, typename B>
constexpr auto TypeErasedBasicCompare(CompareType, void const* a,
                                      void const* b);

template <typename ComparisonType, typename A, typename B>
struct CppStdBasicComparator {
    ZETA_Core_StaticAssert(IsComparisonType<ComparisonType>);

    constexpr auto operator()(A const& a, B const& b) const;
};

}  // namespace zeta::core::comparison
