#pragma once

#include <zeta/core/comparison.hpp>
#include <zeta/core/comparison.ipp>
#include <zeta/core/comparison_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/reduce.ipp>

namespace zeta::core {

namespace comparison_utils::detail {

template <typename Comparator>
struct MinOperation_ {
    Comparator const& cmptr;

    template <typename A, typename B>
    constexpr decltype(auto) operator()(A&& a, B&& b) const {
        return comparison::Compare(
                   this->cmptr,
                   meta::AutoValueWrapper<comparison::OpEnum::LessEqual>{}, a,
                   b)
                   ? meta::Forward<A>(a)
                   : meta::Forward<B>(b);
    }
};

}  // namespace comparison_utils::detail

template <typename Comparator, typename Value0, typename... Values>
constexpr decltype(auto) comparison_utils::Min(Comparator const& cmptr,
                                               Value0&& value0,
                                               Values&&... values) {
    return reduce::TreeReduce(detail::MinOperation_{ cmptr },
                              meta::Forward<Value0>(value0),
                              meta::Forward<Values>(values)...);
}

template <typename Value0, typename... Values>
constexpr decltype(auto) comparison_utils::BasicMin(Value0&& value0,
                                                    Values&&... values) {
    return (Min)(comparison::UniversalBasicComparator{},
                 meta::Forward<Value0>(value0),
                 meta::Forward<Values>(values)...);
}

namespace comparison_utils::detail {

template <typename Comparator>
struct MaxOperation_ {
    Comparator const& cmptr;

    template <typename A, typename B>
    constexpr decltype(auto) operator()(A&& a, B&& b) const {
        return comparison::Compare(
                   this->cmptr,
                   meta::AutoValueWrapper<comparison::OpEnum::Less>{}, a, b)
                   ? meta::Forward<B>(b)
                   : meta::Forward<A>(a);
    }
};

}  // namespace comparison_utils::detail

template <typename Compareator, typename Value0, typename... Values>
constexpr decltype(auto) comparison_utils::Max(Compareator const& cmptr,
                                               Value0&& value0,
                                               Values&&... values) {
    return reduce::TreeReduce(detail::MaxOperation_{ cmptr },
                              meta::Forward<Value0>(value0),
                              meta::Forward<Values>(values)...);
}

template <typename Value0, typename... Values>
constexpr decltype(auto) comparison_utils::BasicMax(Value0&& value0,
                                                    Values&&... values) {
    auto&& ret{ (Max)(comparison::UniversalBasicComparator{},
                      meta::Forward<Value0>(value0),
                      meta::Forward<Values>(values)...) };

    if constexpr (meta::IsLValueRef<decltype(ret)>) {
        return ret;
    } else {
        return static_cast<meta::RemoveCVRef<decltype(ret)>>(meta::Move(ret));
    }
}

inline int comparison_utils::MemLexCompare(void const* a, void const* b,
                                           size_t a_size, size_t b_size) {
    if (a == b) { return 0; }

    if (0 < a_size) { ZETA_Core_DebugAssert(a != nullptr); }
    if (0 < b_size) { ZETA_Core_DebugAssert(b != nullptr); }

    if (a_size < b_size) {
        int cmp{ __builtin_memcmp(a, b, a_size) };
        return cmp == 0 ? -1 : cmp;
    }

    if (b_size < a_size) {
        int cmp{ __builtin_memcmp(a, b, b_size) };
        return cmp == 0 ? 1 : cmp;
    }

    return __builtin_memcmp(a, b, a_size);
}

inline int comparison_utils::MemSeqLexCompare(
    void const* a_, void const* b_, size_t a_elem_size, size_t b_elem_size,
    size_t a_elem_stride, size_t b_elem_stride, size_t a_elem_cnt,
    size_t b_elem_cnt) {
    unsigned char const* a{ static_cast<unsigned char const*>(a_) };
    unsigned char const* b{ static_cast<unsigned char const*>(b_) };

    if (0 < a_elem_cnt) {
        ZETA_Core_DebugAssert(a != nullptr);
        ZETA_Core_DebugAssert(a_elem_stride == 0 ||
                              a_elem_size <= a_elem_stride);
    }

    if (0 < b_elem_cnt) {
        ZETA_Core_DebugAssert(b != nullptr);
        ZETA_Core_DebugAssert(b_elem_stride == 0 ||
                              b_elem_size <= b_elem_stride);
    }

    if (a_elem_cnt == 0 && b_elem_cnt == 0) { return 0; }
    if (a_elem_cnt == 0) { return -1; }
    if (b_elem_cnt == 0) { return 1; }

    if (a_elem_cnt == 1) { a_elem_stride = 0; }
    if (b_elem_cnt == 1) { b_elem_stride = 0; }

    size_t cnt{ a_elem_stride == 0 && b_elem_stride == 0
                    ? 1
                    : (BasicMin)(a_elem_cnt, b_elem_cnt) };

    for (size_t i{ 0 }; i < cnt; ++i, a += a_elem_stride, b += b_elem_stride) {
        int cmp{ (MemLexCompare)(a, b, a_elem_size, b_elem_size) };
        if (cmp != 0) { return cmp; }
    }

    // NOLINTNEXTLINE(readcapability-implicit-bool-conversion)
    return (b_elem_cnt < a_elem_cnt) - (a_elem_cnt < b_elem_cnt);
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
        cmptr, meta::AutoValueWrapper<comparison::OpEnum::Order>{},
        meta::Forward<A>(a), meta::Forward<B>(b)) };
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

template <comparison::IsOpType OpType>
constexpr auto BasicPairWiseLexCompare_(OpType op) {
    if constexpr (meta::IsSame<OpType, meta::AutoValueWrapper<
                                           comparison::OpEnum::Order>>) {
        return comparison::Ordering::Equal;
    } else {
        return (meta::ToUnderlying(op) & comparison::equal_bit) != 0;
    }
}

template <comparison::IsOpType OpType, typename A, typename B, typename... Args>
constexpr auto BasicPairWiseLexCompare_(OpType op, A&& a, B&& b,
                                        Args&&... args) {
    if constexpr (sizeof...(args) == 0) {
        return comparison::BasicCompare(op, meta::Forward<A>(a),
                                        meta::Forward<B>(b));
    } else {
        comparison::Ordering cmp{ comparison::BasicCompare(
            meta::AutoValueWrapper<comparison::OpEnum::Order>{},
            meta::Forward<A>(a), meta::Forward<B>(b)) };

        if (cmp == comparison::Ordering::Equal) {
            return (BasicPairWiseLexCompare_)(op, meta::Forward<Args>(args)...);
        }

        if constexpr (meta::IsSame<OpType, meta::AutoValueWrapper<
                                               comparison::OpEnum::Order>>) {
            return cmp;
        } else {
            return (static_cast<unsigned>(cmp) &
                    meta::ToUnderlying(op.value)) != 0;
        }
    }
}

}  // namespace comparison_utils::detail

template <comparison::IsOpType OpType, typename... Args>
    requires requires { requires sizeof...(Args) % 2 == 0; }
constexpr auto comparison_utils::BasicPairWiseLexCompare(OpType op,
                                                         Args&&... args) {
    return detail::BasicPairWiseLexCompare_(op, meta::Forward<Args>(args)...);
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
