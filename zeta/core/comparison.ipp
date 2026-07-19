#pragma once

#include <zeta/core/comparison.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

template <typename Comparator, typename A, typename B>
constexpr int comparison::Compare(Comparator const& cmptr,
                                  ComparisonTypeEnum::ThreeWay, A&& a, B&& b) {
    return cmptr.Compare(ComparisonTypeEnum::ThreeWay{}, meta::Forward<A>(a),
                         meta::Forward<B>(b));
}

template <typename Comparator, typename CompareType, typename A, typename B,
          typename>
constexpr bool comparison::Compare(Comparator const& cmptr, CompareType, A&& a,
                                   B&& b) {
    return cmptr.Compare(CompareType{}, meta::Forward<A>(a),
                         meta::Forward<B>(b));
}

template <typename Comparatr, typename A, typename B>
void comparison::CheckContract() {
#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, ...)                         \
    ZETA_Core_Unused([&](Comparatr& cmptr, A& a, B& b) { \
        ZETA_Core_Unused(cmptr);                         \
        ZETA_Core_Unused(a);                             \
        ZETA_Core_Unused(b);                             \
        (method)(__VA_ARGS__);                           \
    })

    CheckMethod(                         //
        Compare,                         // method
                                         //
        cmptr,                           // cmptr
        ComparisonTypeEnum::ThreeWay{},  // CompareType
        a,                               // a
        b                                // b
    );

    CheckMethod(                        //
        Compare,                        // method
                                        //
        cmptr,                          // cmptr
        ComparisonTypeEnum::EqualTo{},  // CompareType
        a,                              // a
        b                               // b
    );

    CheckMethod(                           //
        Compare,                           // method
                                           //
        cmptr,                             // cmptr
        ComparisonTypeEnum::NotEqualTo{},  // CompareType
        a,                                 // a
        b                                  // b
    );

    CheckMethod(                     //
        Compare,                     // method
                                     //
        cmptr,                       // cmptr
        ComparisonTypeEnum::Less{},  // CompareType
        a,                           // a
        b                            // b
    );

    CheckMethod(                          //
        Compare,                          // method
                                          //
        cmptr,                            // cmptr
        ComparisonTypeEnum::LessEqual{},  // CompareType
        a,                                // a
        b                                 // b
    );

    CheckMethod(                        //
        Compare,                        // method
                                        //
        cmptr,                          // cmptr
        ComparisonTypeEnum::Greater{},  // CompareType
        a,                              // a
        b                               // b
    );

    CheckMethod(                             //
        Compare,                             // method
                                             //
        cmptr,                               // cmptr
        ComparisonTypeEnum::GreaterEqual{},  // CompareType
        a,                                   // a
        b                                    // b
    );

#pragma pop_macro("CheckMethod")
}

template <typename A, typename B>
struct comparison::BasicComparator<
    A, B,
    meta::EnableIf<
        (integral::IsIntegral<A> || meta::IsPointer<A> || meta::IsArray<A>) &&
            (integral::IsIntegral<B> || meta::IsPointer<B> || meta::IsArray<B>),
        void>> {
    constexpr int Compare(ComparisonTypeEnum::ThreeWay, A const& a,
                          B const& b) const {
        return (b < a) - (a < b);
    }

    constexpr bool Compare(ComparisonTypeEnum::EqualTo, A const& a,
                           B const& b) const {
        return a == b;
    }

    constexpr bool Compare(ComparisonTypeEnum::NotEqualTo, A const& a,
                           B const& b) const {
        return a != b;
    }

    constexpr bool Compare(ComparisonTypeEnum::Less, A const& a,
                           B const& b) const {
        return a < b;
    }

    constexpr bool Compare(ComparisonTypeEnum::LessEqual, A const& a,
                           B const& b) const {
        return a <= b;
    }

    constexpr bool Compare(ComparisonTypeEnum::Greater, A const& a,
                           B const& b) const {
        return a > b;
    }

    constexpr bool Compare(ComparisonTypeEnum::GreaterEqual, A const& a,
                           B const& b) const {
        return a >= b;
    }
};

template <typename CompareType, typename A, typename B>
constexpr auto comparison::BasicCompare(CompareType, A const& a, B const& b) {
    return (Compare)(BasicComparator<A, B>{}, CompareType{}, a, b);
}

template <typename CompareType, typename A, typename B>
constexpr auto comparison::UniversalBasicComparator::Compare(CompareType,
                                                             A const& a,
                                                             B const& b) {
    return (BasicCompare)(CompareType{}, a, b);
}

template <typename CompareType, typename A, typename B>
constexpr auto comparison::TypeErasedBasicCompare(CompareType, void const* a,
                                                  void const* b) {
    return (BasicCompare)(CompareType{}, *static_cast<A const*>(a),
                          *static_cast<B const*>(b));
}

template <typename ComparisonType, typename A, typename B>
constexpr auto
comparison::CppStdBasicComparator<ComparisonType, A, B>::operator()(
    A const& a, B const& b) const {
    return (BasicCompare)(ComparisonType{}, a, b);
}

}  // namespace zeta::core
