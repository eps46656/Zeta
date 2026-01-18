#pragma once

namespace zeta::core::compare {

template <typename A, typename B>
int Compare(A const& a, B const& b);

template <typename A, typename B>
int TypeErasedCompare(void const* a, void const* b);

// -----------------------------------------------------------------------------

template <typename A, typename B>
struct CppStdEqualTo {
    bool operator()(A const& a, B const& b) const;
};

template <typename A, typename B>
struct CppStdNotEqualTo {
    bool operator()(A const& a, B const& b) const;
};

template <typename A, typename B>
struct CppStdLess {
    bool operator()(A const& a, B const& b) const;
};

template <typename A, typename B>
struct CppStdLessEqual {
    bool operator()(A const& a, B const& b) const;
};

template <typename A, typename B>
struct CppStdGreater {
    bool operator()(A const& a, B const& b) const;
};

template <typename A, typename B>
struct CppStdGreaterEqual {
    bool operator()(A const& a, B const& b) const;
};

// -----------------------------------------------------------------------------

template <typename A, typename B, typename = void>
struct CompareCore;

}  // namespace zeta::core::compare
