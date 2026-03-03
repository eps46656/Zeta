#pragma once

namespace zeta::core::compare {

template <typename A, typename B, typename = void>
struct BasicCompareImpl;

namespace ops {

int LexMemCompare(void const* a, void const* b, size_t a_size, size_t b_size);

int LexElemCompare(void const* a, void const* b, size_t a_width, size_t b_width,
                   size_t a_stride, size_t b_stride, size_t a_cnt,
                   size_t b_cnt);

template <typename A, typename B>
int BasicCompare(A const& a, B const& b);

template <typename A, typename B>
int TypeErasedBasicCompare(void const* a, void const* b);

}  // namespace ops

template <typename A, typename B>
struct CppStdBasicEqualTo {
    bool operator()(A const& a, B const& b) const;
};

template <typename A, typename B>
struct CppStdBasicNotEqualTo {
    bool operator()(A const& a, B const& b) const;
};

template <typename A, typename B>
struct CppStdBasicLess {
    bool operator()(A const& a, B const& b) const;
};

template <typename A, typename B>
struct CppStdBasicLessEqual {
    bool operator()(A const& a, B const& b) const;
};

template <typename A, typename B>
struct CppStdBasicGreater {
    bool operator()(A const& a, B const& b) const;
};

template <typename A, typename B>
struct CppStdBasicGreaterEqual {
    bool operator()(A const& a, B const& b) const;
};

}  // namespace zeta::core::compare
