#pragma once

namespace zeta::core::compare {

template <typename X, typename Y>
int Compare(X const& x, Y const& y);

template <typename X, typename Y>
int TypeErasedCompare(void const* x, void const* y);

// -----------------------------------------------------------------------------

template <typename X, typename Y>
struct CppStdEqualTo {
    bool operator()(X const& x, Y const& y) const;
};

template <typename X, typename Y>
struct CppStdNotEqualTo {
    bool operator()(X const& x, Y const& y) const;
};

template <typename X, typename Y>
struct CppStdLess {
    bool operator()(X const& x, Y const& y) const;
};

template <typename X, typename Y>
struct CppStdLessEqual {
    bool operator()(X const& x, Y const& y) const;
};

template <typename X, typename Y>
struct CppStdGreater {
    bool operator()(X const& x, Y const& y) const;
};

template <typename X, typename Y>
struct CppStdGreaterEqual {
    bool operator()(X const& x, Y const& y) const;
};

// -----------------------------------------------------------------------------

template <typename X, typename Y, typename _ = void>
struct CompareCore;

}  // namespace zeta::core::compare
