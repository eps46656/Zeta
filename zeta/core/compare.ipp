#pragma once

#include <zeta/core/compare.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

template <typename Comparator, typename A, typename B>
constexpr int compare::Compare(Comparator const& cmptr, compare_type::ThreeWay,
                               A&& a, B&& b) {
    return cmptr.Compare(compare_type::ThreeWay{}, meta::Forward<A>(a),
                         meta::Forward<B>(b));
}

template <typename Comparator, typename CompareTypeTag, typename A, typename B,
          typename>
constexpr bool compare::Compare(Comparator const& cmptr, CompareTypeTag, A&& a,
                                B&& b) {
    return cmptr.Compare(CompareTypeTag{}, meta::Forward<A>(a),
                         meta::Forward<B>(b));
}

template <typename Comparatr, typename A, typename B>
void compare::CheckContract() {
#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method, ...)                         \
    ZETA_Core_Unused([&](Comparatr& cmptr, A& a, B& b) { \
        ZETA_Core_Unused(cmptr);                         \
        ZETA_Core_Unused(a);                             \
        ZETA_Core_Unused(b);                             \
        (method)(__VA_ARGS__);                           \
    })

    CheckMethod(                   //
        Compare,                   // method
                                   //
        cmptr,                     // cmptr
        compare_type::ThreeWay{},  // CompareTypeTag
        a,                         // a
        b                          // b
    );

    CheckMethod(                  //
        Compare,                  // method
                                  //
        cmptr,                    // cmptr
        compare_type::EqualTo{},  // CompareTypeTag
        a,                        // a
        b                         // b
    );

    CheckMethod(                     //
        Compare,                     // method
                                     //
        cmptr,                       // cmptr
        compare_type::NotEqualTo{},  // CompareTypeTag
        a,                           // a
        b                            // b
    );

    CheckMethod(               //
        Compare,               // method
                               //
        cmptr,                 // cmptr
        compare_type::Less{},  // CompareTypeTag
        a,                     // a
        b                      // b
    );

    CheckMethod(                    //
        Compare,                    // method
                                    //
        cmptr,                      // cmptr
        compare_type::LessEqual{},  // CompareTypeTag
        a,                          // a
        b                           // b
    );

    CheckMethod(                  //
        Compare,                  // method
                                  //
        cmptr,                    // cmptr
        compare_type::Greater{},  // CompareTypeTag
        a,                        // a
        b                         // b
    );

    CheckMethod(                       //
        Compare,                       // method
                                       //
        cmptr,                         // cmptr
        compare_type::GreaterEqual{},  // CompareTypeTag
        a,                             // a
        b                              // b
    );

#pragma pop_macro("CheckMethod")
}

template <typename A, typename B>
struct compare::BasicComparator<
    A, B,
    meta::EnableIf<
        (integral::IsIntegral<A> || meta::IsPointer<A> || meta::IsArray<A>) &&
            (integral::IsIntegral<B> || meta::IsPointer<B> || meta::IsArray<B>),
        void>> {
    constexpr int Compare(compare_type::ThreeWay, A const& a,
                          B const& b) const {
        return (b < a) - (a < b);
    }

    constexpr bool Compare(compare_type::EqualTo, A const& a,
                           B const& b) const {
        return a == b;
    }

    constexpr bool Compare(compare_type::NotEqualTo, A const& a,
                           B const& b) const {
        return a != b;
    }

    constexpr bool Compare(compare_type::Less, A const& a, B const& b) const {
        return a < b;
    }

    constexpr bool Compare(compare_type::LessEqual, A const& a,
                           B const& b) const {
        return a <= b;
    }

    constexpr bool Compare(compare_type::Greater, A const& a,
                           B const& b) const {
        return a > b;
    }

    constexpr bool Compare(compare_type::GreaterEqual, A const& a,
                           B const& b) const {
        return a >= b;
    }
};

template <typename CompareTypeTag, typename A, typename B>
constexpr int compare::BasicCompare(CompareTypeTag, A const& a, B const& b) {
    return (Compare)(BasicComparator<A, B>{}, CompareTypeTag{}, a, b);
}

template <typename CompareTypeTag, typename A, typename B>
constexpr int compare::UniversalBasicComparator::Compare(CompareTypeTag,
                                                         A const& a,
                                                         B const& b) {
    return (BasicCompare)(CompareTypeTag{}, a, b);
}

template <typename CompareTypeTag, typename A, typename B>
constexpr int compare::TypeErasedBasicCompare(CompareTypeTag, void const* a,
                                              void const* b) {
    return (BasicCompare)(CompareTypeTag{}, *static_cast<A const*>(a),
                          *static_cast<B const*>(b));
}

template <typename A, typename B>
constexpr bool compare::CppStdBasicEqualTo<A, B>::operator()(A const& a,
                                                             B const& b) const {
    return (BasicCompare)(compare_type::EqualTo{}, a, b);
}

template <typename A, typename B>
constexpr bool compare::CppStdBasicNotEqualTo<A, B>::operator()(
    A const& a, B const& b) const {
    return (BasicCompare)(compare_type::NotEqualTo{}, a, b);
}

template <typename A, typename B>
constexpr bool compare::CppStdBasicLess<A, B>::operator()(A const& a,
                                                          B const& b) const {
    return (BasicCompare)(compare_type::Less{}, a, b);
}

template <typename A, typename B>
constexpr bool compare::CppStdBasicLessEqual<A, B>::operator()(
    A const& a, B const& b) const {
    return (BasicCompare)(compare_type::LessEqual{}, a, b);
}

template <typename A, typename B>
constexpr bool compare::CppStdBasicGreater<A, B>::operator()(A const& a,
                                                             B const& b) const {
    return (BasicCompare)(compare_type::Greater{}, a, b);
}

template <typename A, typename B>
constexpr bool compare::CppStdBasicGreaterEqual<A, B>::operator()(
    A const& a, B const& b) const {
    return (BasicCompare)(compare_type::GreaterEqual{}, a, b);
}

}  // namespace zeta::core
