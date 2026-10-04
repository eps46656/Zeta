#pragma once

#include <zeta/core/comparison.hpp>
#include <zeta/core/comparison.ipp>
#include <zeta/core/comparison_utils.hpp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/lin_seq_utils.ipp>
#include <zeta/core/meta.hpp>
#include <zeta/core/reduce.ipp>

namespace zeta::core {

namespace comparison_utils::detail {

template <typename Comparator>
struct MinOperation_ {
    Comparator const& cmptr;

    template <typename A, typename B>
    constexpr decltype(auto) operator()(this MinOperation_ const& self,
                                        A const& a, B const& b) {
        return comparison::Compare(self.cmptr, comparison::OpTags::LessEqual{},
                                   a, b)
                   ? a
                   : b;
    }
};

}  // namespace comparison_utils::detail

template <typename Comparator, typename Value0, typename... Values>
constexpr decltype(auto) comparison_utils::Min(Comparator const& cmptr,
                                               Value0 const& value0,
                                               Values const&... values) {
    return reduce::TreeReduce(detail::MinOperation_{ cmptr }, value0,
                              values...);
}

template <typename Value0, typename... Values>
constexpr decltype(auto) comparison_utils::BasicMin(Value0 const& value0,
                                                    Values const&... values) {
    return (Min)(comparison::UniversalBasicComparator{}, value0, values...);
}

namespace comparison_utils::detail {

template <typename Comparator>
struct MaxOperation_ {
    Comparator const& cmptr;

    template <typename A, typename B>
    constexpr decltype(auto) operator()(this MaxOperation_ const& self,
                                        A const& a, B const& b) {
        return comparison::Compare(self.cmptr, comparison::OpTags::Less{}, a, b)
                   ? b
                   : a;
    }
};

}  // namespace comparison_utils::detail

template <typename Compareator, typename Value0, typename... Values>
constexpr decltype(auto) comparison_utils::Max(Compareator const& cmptr,
                                               Value0 const& value0,
                                               Values const&... values) {
    return reduce::TreeReduce(detail::MaxOperation_{ cmptr }, value0,
                              values...);
}

template <typename Value0, typename... Values>
constexpr decltype(auto) comparison_utils::BasicMax(Value0 const& value0,
                                                    Values const&... values) {
    return (Max)(comparison::UniversalBasicComparator{}, value0, values...);
}

template <comparison::IsOpTag OpTag>
constexpr auto comparison_utils::MemLexCompare(OpTag, void const* a,
                                               void const* b, size_t a_size,
                                               size_t b_size) {
    if (a == b) {
        return comparison::DecayOrdering(OpTag{}, comparison::Ordering::Equal);
    }

    if (0 < a_size) { ZETA_Core_DebugUtils_Diag_PromiseAssert(a != nullptr); }
    if (0 < b_size) { ZETA_Core_DebugUtils_Diag_PromiseAssert(b != nullptr); }

    if (a_size < b_size) {
        int cmp{ __builtin_memcmp(a, b, a_size) };
        return comparison::BasicCompare(OpTag{}, cmp == 0 ? -1 : cmp, 0);
    }

    if (b_size < a_size) {
        int cmp{ __builtin_memcmp(a, b, b_size) };
        return comparison::BasicCompare(OpTag{}, cmp == 0 ? 1 : cmp, 0);
    }

    return comparison::BasicCompare(OpTag{}, __builtin_memcmp(a, b, a_size), 0);
}

template <comparison::IsOpTag OpTag>
constexpr decltype(auto) comparison_utils::LinMemSeqLexCompare(
    OpTag, void const* a_, void const* b_, size_t a_elem_size,
    size_t b_elem_size, ptrdiff_t a_elem_stride, ptrdiff_t b_elem_stride,
    size_t a_elem_cnt, size_t b_elem_cnt) {
    unsigned char const* a{ static_cast<unsigned char const*>(a_) };
    unsigned char const* b{ static_cast<unsigned char const*>(b_) };

    if (0 < a_elem_cnt) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(a != nullptr);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            a_elem_stride == 0 ||
            a_elem_size <=
                static_cast<size_t>(integral_math::Abs(a_elem_stride)));
    }

    if (0 < b_elem_cnt) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(b != nullptr);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            b_elem_stride == 0 ||
            b_elem_size <=
                static_cast<size_t>(integral_math::Abs(b_elem_stride)));
    }

    if (a_elem_cnt == 1) { a_elem_stride = 0; }
    if (b_elem_cnt == 1) { b_elem_stride = 0; }

    size_t cnt{ (BasicMin)(a_elem_cnt, b_elem_cnt) };

    if (a_elem_stride == 0 && b_elem_stride == 0 && 0 < cnt) { cnt = 1; }

    for (size_t i{ 0 }; i < cnt; ++i, a += a_elem_stride, b += b_elem_stride) {
        comparison::Ordering cmp{ (MemLexCompare)(comparison::OpTags::Order{},
                                                  a, b, a_elem_size,
                                                  b_elem_size) };

        if (cmp != comparison::Ordering::Equal) {
            return comparison::DecayOrdering(OpTag{}, cmp);
        }
    }

    return comparison::BasicCompare(OpTag{}, a_elem_cnt, b_elem_cnt);
}

template <typename Comparator, comparison::IsOpTag OpTag, typename ElemA,
          typename ElemB>
constexpr decltype(auto) comparison_utils::LinObjSeqLexCompare(
    Comparator const& cmptr, OpTag, ElemA const* a, ElemB const* b,
    ptrdiff_t a_elem_stride, ptrdiff_t b_elem_stride, size_t a_elem_cnt,
    size_t b_elem_cnt) {
    lin_seq_utils::Check(a, a_elem_stride, a_elem_cnt);
    lin_seq_utils::Check(b, b_elem_stride, b_elem_cnt);

    if (a_elem_cnt == 1) { a_elem_stride = 0; }
    if (b_elem_cnt == 1) { b_elem_stride = 0; }

    size_t cnt{ (BasicMin)(a_elem_cnt, b_elem_cnt) };

    if (a_elem_stride == 0 && b_elem_stride == 0 && 0 < cnt) { cnt = 1; }

    for (size_t i{ cnt }; 0 < i--; a = utils::PtrInc(a, a_elem_stride),
                                   b = utils::PtrInc(b, b_elem_stride)) {
        comparison::Ordering cmp{ comparison::Compare(
            cmptr, comparison::OpTags::Order{}, *a, *b) };

        if (cmp != comparison::Ordering::Equal) {
            return comparison::DecayOrdering(OpTag{}, cmp);
        }
    }

    return comparison::BasicCompare(OpTag{}, a_elem_cnt, b_elem_cnt);
}

template <comparison::IsOpTag OpTag, typename ElemA, typename ElemB>
constexpr decltype(auto) comparison_utils::BasicLinObjSeqLexCompare(
    OpTag, ElemA const* a, ElemB const* b, ptrdiff_t a_elem_stride,
    ptrdiff_t b_elem_stride, size_t a_elem_cnt, size_t b_elem_cnt) {
    return (LinObjSeqLexCompare)(comparison::UniversalBasicComparator{},
                                 OpTag{}, a, b, a_elem_stride, b_elem_stride,
                                 a_elem_cnt, b_elem_cnt);
}

namespace comparison_utils::detail {

constexpr comparison::Ordering PairWiseLexCompare_() {
    return comparison::Ordering::Equal;
}

template <typename A, typename B, typename Comparator, typename... Args>
constexpr comparison::Ordering PairWiseLexCompare_(A&& a, B&& b,
                                                   Comparator const& cmptr,
                                                   Args&&... args) {
    comparison::Ordering cmp{ comparison::Compare(
        cmptr, comparison::OpTags::Order{}, meta::Forward<A>(a),
        meta::Forward<B>(b)) };
    return cmp == comparison::Ordering::Equal ? (PairWiseLexCompare_)(args...)
                                              : cmp;
}

}  // namespace comparison_utils::detail

template <typename... Args>
    requires requires { requires sizeof...(Args) % 3 == 0; }
constexpr comparison::Ordering comparison_utils::PairWiseLexCompare(
    Args&&... args) {
    return detail::PairWiseLexCompare_(meta::Forward<Args>(args)...);
}

namespace comparison_utils::detail {

template <comparison::IsOpTag OpTag>
constexpr auto BasicPairWiseLexCompare_(OpTag op) {
    return comparison::DecayOrdering(op, comparison::Ordering::Equal);
}

template <comparison::IsOpTag OpTag, typename A, typename B, typename... Args>
constexpr auto BasicPairWiseLexCompare_(OpTag, A const& a, B const& b,
                                        Args const&... args) {
    if constexpr (sizeof...(args) == 0) {
        return comparison::BasicCompare(OpTag{}, a, b);
    } else if (meta::IsSame<OpTag, comparison::OpTags::Equal>) {
        return comparison::BasicCompare(OpTag{}, a, b) &&
               (BasicPairWiseLexCompare_)(OpTag{}, args...);
    } else if (meta::IsSame<OpTag, comparison::OpTags::NotEqual>) {
        return comparison::BasicCompare(OpTag{}, a, b) ||
               (BasicPairWiseLexCompare_)(OpTag{}, args...);
    } else {
        comparison::Ordering cmp{ comparison::BasicCompare(
            comparison::OpTags::Order{}, a, b) };

        return cmp == comparison::Ordering::Equal
                   ? (BasicPairWiseLexCompare_)(OpTag{}, args...)
                   : comparison::DecayOrdering(OpTag{}, cmp);
    }
}

}  // namespace comparison_utils::detail

template <comparison::IsOpTag OpTag, typename... Args>
    requires requires { requires sizeof...(Args) % 2 == 0; }
constexpr auto comparison_utils::BasicPairWiseLexCompare(OpTag op,
                                                         Args const&... args) {
    return detail::BasicPairWiseLexCompare_(op, args...);
}

template <typename Comparator, typename SeqA, typename SeqB>
int SeqWiseLexCompare(Comparator const& cmptr, SeqA&& a, SeqB&& b) {
    bool a_has_elem{ a.HasElem() };
    bool b_has_elem{ b.HasElem() };

    for (;;) {
        switch (a_has_elem * 0b10 + b_has_elem * 0b01) {
        case 0b00: return 0;
        case 0b01: return -1;
        case 0b10: return 1;
        }

        auto&& x_elem{ a.Get() };
        auto&& y_elem{ b.Get() };

        int cmp{ comparison::Compare(cmptr, x_elem, y_elem) };

        if (cmp != 0) { return cmp; }

        a_has_elem = a.Next();
        b_has_elem = b.Next();
    }
}

}  // namespace zeta::core
