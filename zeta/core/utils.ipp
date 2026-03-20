#pragma once

#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/hash.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool utils::operator==(Pair<XFirst, XSecond> const& x,
                       Pair<YFirst, YSecond> const& y) {
    return compare::BasicCompare(x, y) == 0;
}

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool utils::operator!=(Pair<XFirst, XSecond> const& x,
                       Pair<YFirst, YSecond> const& y) {
    return compare::BasicCompare(x, y) != 0;
}

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool utils::operator<(Pair<XFirst, XSecond> const& x,
                      Pair<YFirst, YSecond> const& y) {
    return compare::BasicCompare(x, y) < 0;
}

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool utils::operator<=(Pair<XFirst, XSecond> const& x,
                       Pair<YFirst, YSecond> const& y) {
    return compare::BasicCompare(x, y) <= 0;
}

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool utils::operator>(Pair<XFirst, XSecond> const& x,
                      Pair<YFirst, YSecond> const& y) {
    return compare::BasicCompare(x, y) > 0;
}

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool utils::operator>=(Pair<XFirst, XSecond> const& x,
                       Pair<YFirst, YSecond> const& y) {
    return compare::BasicCompare(x, y) >= 0;
}

template <typename First, typename Second>
unsigned long long hash::BasicHashImpl<utils::Pair<First, Second>>::operator()(
    utils::Pair<First, Second> const& x, unsigned long long salt) const {
    unsigned long long h1{ hash::BasicHash(x.first, salt) };
    unsigned long long h2{ hash::BasicHash(x.second, h1) };
    return hash::BasicHash(h2, salt);
}

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
int compare::BasicCompareImpl<utils::Pair<XFirst, XSecond>,
                              utils::Pair<YFirst, YSecond>>::
operator()(utils::Pair<XFirst, XSecond> const& x,
           utils::Pair<YFirst, YSecond> const& y) const {
    int cmp{ compare::BasicCompare(x.first, y.first) };
    if (cmp != 0) { return cmp; }
    return compare::BasicCompare(x.second, y.second);
}

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool utils::operator==(Triplet<XFirst, XSecond, XThird> const& x,
                       Triplet<YFirst, YSecond, YThird> const& y) {
    return compare::BasicCompare(x, y) == 0;
}

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool utils::operator!=(Triplet<XFirst, XSecond, XThird> const& x,
                       Triplet<YFirst, YSecond, YThird> const& y) {
    return compare::BasicCompare(x, y) != 0;
}

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool utils::operator<(Triplet<XFirst, XSecond, XThird> const& x,
                      Triplet<YFirst, YSecond, YThird> const& y) {
    return compare::BasicCompare(x, y) < 0;
}

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool utils::operator<=(Triplet<XFirst, XSecond, XThird> const& x,
                       Triplet<YFirst, YSecond, YThird> const& y) {
    return compare::BasicCompare(x, y) <= 0;
}

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool utils::operator>(Triplet<XFirst, XSecond, XThird> const& x,
                      Triplet<YFirst, YSecond, YThird> const& y) {
    return compare::BasicCompare(x, y) > 0;
}

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool utils::operator>=(Triplet<XFirst, XSecond, XThird> const& x,
                       Triplet<YFirst, YSecond, YThird> const& y) {
    return compare::BasicCompare(x, y) >= 0;
}

template <typename First, typename Second, typename Third>
unsigned long long
hash::BasicHashImpl<utils::Triplet<First, Second, Third>>::operator()(
    utils::Triplet<First, Second, Third> const& x,
    unsigned long long salt) const {
    unsigned long long h1{ hash::BasicHash(x.first, salt) };
    unsigned long long h2{ hash::BasicHash(x.second, h1) };
    unsigned long long h3{ hash::BasicHash(x.third, h2) };
    return hash::BasicHash(h3, salt);
}

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
int compare::BasicCompareImpl<utils::Triplet<XFirst, XSecond, XThird>,
                              utils::Triplet<YFirst, YSecond, YThird>>::
operator()(utils::Triplet<XFirst, XSecond, XThird> const& x,
           utils::Triplet<YFirst, YSecond, YThird> const& y) const {
    int cmp{ compare::BasicCompare(x.first, y.first) };
    if (cmp != 0) { return cmp; }
    cmp = compare::BasicCompare(x.second, y.second);
    if (cmp != 0) { return cmp; }
    return compare::BasicCompare(x.third, y.third);
}

template <typename X, typename Y>
void utils::Swap(X&& x  // NOLINT(cppcoreguidelines-missing-std-forward)
                 ,
                 Y&& y  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    if (&x == &y) { return; }
    auto tmp{ meta::Move(x) };
    x = meta::Move(y);
    y = meta::Move(tmp);
}

namespace utils::detail {

template <typename Operation, typename T0>
constexpr decltype(auto) LeftReduce_(Operation const&, T0&& x0) {
    return meta::Forward<T0>(x0);
}

template <typename Operation, typename T0, typename T1, typename... Ts>
constexpr decltype(auto) LeftReduce_(Operation const& opr, T0&& x0, T1&& x1,
                                     Ts&&... xs) {
    return LeftReduce_(opr(meta::Forward<T0>(x0), meta::Forward<T1>(x1)),
                       meta::Forward<Ts>(xs)...);
}

}  // namespace utils::detail

template <typename Operation, typename T0, typename... Ts>
constexpr decltype(auto) utils::LeftReduce(Operation const& opr, T0&& x0,
                                           Ts&&... xs) {
    return detail::LeftReduce_(opr, meta::Forward<T0>(x0),
                               meta::Forward<Ts>(xs)...);
}

namespace utils::detail {

template <typename Operation, typename T0>
static constexpr decltype(auto) RightReduce_(Operation const&, T0&& x0) {
    return meta::Forward<T0>(x0);
}

template <typename Operation, typename T0, typename T1, typename... Ts>
static constexpr decltype(auto) RightReduce_(Operation const& opr, T0&& x0,
                                             T1&& x1, Ts&&... xs) {
    return opr(meta::Forward<T0>(x0), RightReduce_(opr, meta::Forward<T1>(x1),
                                                   meta::Forward<Ts>(xs)...));
}

}  // namespace utils::detail

template <typename Operation, typename T0, typename... Ts>
constexpr decltype(auto) utils::RightReduce(Operation const& opr, T0&& x0,
                                            Ts&&... xs) {
    return detail::RightReduce_(opr, meta::Forward<T0>(x0),
                                meta::Forward<Ts>(xs)...);
}

namespace utils::detail {

template <size_t Beg, size_t End>
struct TreeReduce_ {
    template <typename Operation, typename... Ts>
    static constexpr decltype(auto) F(Operation const& opr, Ts&&... xs) {
        constexpr size_t Mid{ (Beg + End) / 2 };

        return opr(TreeReduce_<Beg, Mid>::F(opr, meta::Forward<Ts>(xs)...),
                   TreeReduce_<Mid, End>::F(opr, meta::Forward<Ts>(xs)...));
    }
};

template <size_t Beg>
struct TreeReduce_<Beg, Beg + 1> {
    template <typename Operation, typename... Ts>
    static constexpr decltype(auto) F(Operation const&, Ts&&... xs) {
        return meta::GetNthArg<Beg>(meta::Forward<Ts>(xs)...);
    }
};

}  // namespace utils::detail

template <typename Operation, typename T0, typename... Ts>
constexpr decltype(auto) utils::TreeReduce(Operation const& opr, T0&& x0,
                                           Ts&&... xs) {
    return detail::TreeReduce_<0, 1 + sizeof...(xs)>::F(
        opr, meta::Forward<T0>(x0), meta::Forward<Ts>(xs)...);
}

namespace utils::detail {

struct MinOperation_ {
    template <typename X, typename Y>
    constexpr decltype(auto) operator()(X&& x, Y&& y) const {
        return y < x ? meta::Forward<Y>(y) : meta::Forward<X>(x);
    }
};

}  // namespace utils::detail

template <typename T0, typename... Ts>
constexpr decltype(auto) utils::Min(T0&& x0, Ts&&... xs) {
    return TreeReduce(detail::MinOperation_{}, meta::Forward<T0>(x0),
                      meta::Forward<Ts>(xs)...);
}

namespace utils::detail {

struct MaxOperation_ {
    template <typename X, typename Y>
    constexpr decltype(auto) operator()(X&& x, Y&& y) const {
        return x < y ? meta::Forward<Y>(y) : meta::Forward<X>(x);
    }
};

}  // namespace utils::detail

template <typename T0, typename... Ts>
constexpr decltype(auto) utils::Max(T0&& x0, Ts&&... xs) {
    return TreeReduce(detail::MaxOperation_{}, meta::Forward<T0>(x0),
                      meta::Forward<Ts>(xs)...);
}

namespace utils::detail {

struct SumOperation_ {
    template <typename X, typename Y>
    constexpr decltype(auto) operator()(X&& x, Y&& y) const {
        return meta::Forward<X>(x) + meta::Forward<Y>(y);
    }
};

}  // namespace utils::detail

template <typename T0, typename... Ts>
constexpr decltype(auto) utils::Sum(T0&& x0, Ts&&... xs) {
    return TreeReduce(detail::SumOperation_{}, meta::Forward<T0>(x0),
                      meta::Forward<Ts>(xs)...);
}

template <typename Node, typename GetLinkFunc>
utils::Pair<Node*, size_t> utils::GetMostLink(Node* n,
                                              GetLinkFunc const& get_link) {
    if (n == nullptr) { return { nullptr, 0 }; }

    for (size_t i{ 1 };; ++i) {
        Node* n_nxt{ get_link(n) };
        if (n_nxt == nullptr) { return { n, i }; }
        n = n_nxt;
    }
}

inline int utils::MemCompare(void const* a, void const* b, size_t size) {
    if (a == b || size == 0) { return 0; }

    ZETA_Core_DebugAssert(a != nullptr);
    ZETA_Core_DebugAssert(b != nullptr);

    return __builtin_memcmp(a, b, size);
}

inline void utils::MemSwap(void* x_, void* y_, size_t size) {
    auto* x{ static_cast<char*>(x_) };
    auto* y{ static_cast<char*>(y_) };

    if (x == y || size == 0) { return; }

    ZETA_Core_DebugAssert(x != nullptr);
    ZETA_Core_DebugAssert(y != nullptr);

    for (size_t i{ 0 }; i < size; ++i) { Swap(x[i], y[i]); }
}

inline void utils::MemCopy(void* dst, void const* src, size_t size) {
    if (dst == src || size == 0) { return; }

    ZETA_Core_DebugAssert(dst != nullptr);
    ZETA_Core_DebugAssert(src != nullptr);

    __builtin_memcpy(dst, src, size);
}

inline void utils::MemMove(void* dst, void const* src, size_t size) {
    if (dst == src || size == 0) { return; }

    ZETA_Core_DebugAssert(dst != nullptr);
    ZETA_Core_DebugAssert(src != nullptr);

    __builtin_memmove(dst, src, size);
}

inline void* utils::MemRotate(void* data_, size_t l_size, size_t r_size) {
    auto* data{ static_cast<char*>(data_) };

    if (l_size == 0 && r_size == 0) { return data; }

    ZETA_Core_DebugAssert(data != nullptr);

    char* ret{ data + r_size };

    char* end{ data + l_size + r_size };

    for (char* iter{ data }; 0 < l_size && 0 < r_size;) {
        char* jter{ iter + l_size };

        for (; jter != end; ++iter, ++jter) { Swap(*iter, *jter); }

        r_size %= l_size;
        l_size -= r_size;
    }

    return ret;
}

inline int utils::ElemCompare(void const* a_, void const* b_, size_t width,
                              size_t a_stride, size_t b_stride, size_t cnt) {
    auto const* a{ static_cast<char const*>(a_) };
    auto const* b{ static_cast<char const*>(b_) };

    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= a_stride);
    ZETA_Core_DebugAssert(width <= b_stride);

    if (cnt == 0) { return 0; }

    if (width == a_stride && width == b_stride) {
        ZETA_Core_PrintCurPos;
        return MemCompare(a, b, width * cnt);
    }

    for (; 0 < cnt; a += a_stride, b += b_stride, --cnt) {
        int cmp{ MemCompare(a, b, width) };

        if (cmp != 0) {
            ZETA_Core_PrintVar(cmp);
            return cmp;
        }
    }

    return 0;
}

inline void utils::ElemCopy(void* dst_, void const* src_, size_t width,
                            size_t dst_stride, size_t src_stride, size_t cnt) {
    auto* dst{ static_cast<char*>(dst_) };
    auto const* src{ static_cast<char const*>(src_) };

    if (dst == src || cnt == 0) { return; }

    ZETA_Core_DebugAssert(dst != nullptr);
    ZETA_Core_DebugAssert(src != nullptr);
    ZETA_Core_DebugAssert(width <= dst_stride);
    ZETA_Core_DebugAssert(width <= src_stride);

    if (width == 0) { return; }

    if (width == dst_stride && width == src_stride) {
        MemCopy(dst, src, width * cnt);
        return;
    }

    for (size_t i{ 0 }; i < cnt; ++i, dst += dst_stride, src += src_stride) {
        MemCopy(dst, src, width);
    }
}

inline void utils::ElemMove(void* dst_, void const* src_, size_t width,
                            size_t dst_stride, size_t src_stride, size_t cnt) {
    auto* dst{ static_cast<char*>(dst_) };
    auto const* src{ static_cast<char const*>(src_) };

    ZETA_Core_DebugAssert(dst_stride == 0 || width <= dst_stride);
    ZETA_Core_DebugAssert(src_stride == 0 || width <= src_stride);
    ZETA_Core_DebugAssert((dst_stride == 0) == (src_stride == 0));

    if (width == 0 || cnt == 0) { return; }

    ZETA_Core_DebugAssert(dst != nullptr);
    ZETA_Core_DebugAssert(src != nullptr);

    if ((width == dst_stride && width == src_stride) ||
        (dst_stride == 0 && src_stride == 0) || cnt == 1) {
        MemMove(dst, src, width * cnt);
        return;
    }

    char* dst_end{ dst + dst_stride * (cnt - 1) + width };
    char const* src_end{ src + src_stride * (cnt - 1) + width };

    if (dst_end <= src || src_end <= dst) {
        ElemCopy(dst, src, width, dst_stride, src_stride, cnt);
        return;
    }

    if (dst <= src && dst_end <= src_end) {
        for (; 0 < cnt--; dst += dst_stride, src += src_stride) {
            MemCopy(dst, src, width);
        }

        return;
    }

    if (src <= dst && src_end <= dst_end) {
        dst += dst_stride * cnt;
        src += src_stride * cnt;

        while (0 < cnt--) {
            MemCopy(dst -= dst_stride, src -= src_stride, width);
        }

        return;
    }

    ZETA_Core_DebugAssert(false);

    constexpr size_t buffer_capacity{ integral::WidthOf<size_t> + 4 };

    size_t begs[buffer_capacity];
    size_t cnts[buffer_capacity];

    size_t buffer_i{ 0 };

    begs[buffer_i] = 0;
    cnts[buffer_i] = cnt;
    ++buffer_i;

    while (0 < buffer_i--) {
        size_t cur_beg{ begs[buffer_i] };
        size_t cur_cnt{ cnts[buffer_i] };

        if (cur_cnt == 1) {
            MemMove(dst + dst_stride * cur_beg, src + src_stride * cur_beg,
                    width);

            continue;
        }

        size_t cur_l_cnt{ cur_cnt / 2 };
        size_t cur_r_cnt{ cur_cnt - cur_l_cnt };

        char* dst_mid{ dst + dst_stride * cur_l_cnt };
        char const* src_mid{ src + src_stride * cur_l_cnt };

        if (dst_mid <= src_mid) {
            ZETA_Core_DebugAssert(buffer_i < buffer_capacity);

            begs[buffer_i] = cur_beg;
            cnts[buffer_i] = cur_l_cnt;
            ++buffer_i;

            ZETA_Core_DebugAssert(buffer_i < buffer_capacity);

            begs[buffer_i] = cur_beg + cur_l_cnt;
            cnts[buffer_i] = cur_r_cnt;
            ++buffer_i;
        } else {
            ZETA_Core_DebugAssert(buffer_i < buffer_capacity);

            begs[buffer_i] = cur_beg + cur_l_cnt;
            cnts[buffer_i] = cur_r_cnt;
            ++buffer_i;

            ZETA_Core_DebugAssert(buffer_i < buffer_capacity);

            begs[buffer_i] = cur_beg;
            cnts[buffer_i] = cur_l_cnt;
            ++buffer_i;
        }
    }
}

inline void* utils::ElemRotate(void* data_, size_t width, size_t stride,
                               size_t l_size, size_t r_size) {
    auto* data{ static_cast<char*>(data_) };

    if (width == 0 || stride == 0) { return data; }

    ZETA_Core_DebugAssert(width <= stride);

    if (width == stride) {
        return MemRotate(data, stride * l_size, stride * r_size);
    }

    if (l_size == 0 && r_size == 0) { return data; }

    ZETA_Core_DebugAssert(data != nullptr);

    char* ret{ data + stride * r_size };

    char* end{ data + stride * (l_size + r_size) };

    for (char* iter{ data }; 0 < l_size && 0 < r_size;) {
        char* jter{ iter + l_size };

        for (; jter != end; iter += stride, jter += stride) {
            MemSwap(iter, jter, width);
        }

        r_size %= l_size;
        l_size -= r_size;
    }

    return ret;
}

template <typename XIterator, typename YIterator>
int utils::SeqCompare(XIterator x, YIterator y, size_t size) {
    if (x == y) { return 0; }

    for (size_t i{ 0 }; i < size; ++i, ++x, ++y) {
        int cmp{ ThreeWayCompare(*x, *y) };
        if (cmp != 0) { return cmp; }
    }

    return 0;
}

template <typename DstIterator, typename SrcIterator>
void utils::SeqCopy(DstIterator dst, SrcIterator src, size_t size) {
    if (dst == src) { return; }

    for (size_t i{ 0 }; i < size; ++i, ++dst, ++src) { *dst = *src; }
}

template <typename DstIterator, typename SrcIterator>
void utils::SeqMove(DstIterator dst, SrcIterator src, size_t size) {
    if (dst == src) { return; }

    if (dst < src) {
        SeqCopy(dst, src, size);
        return;
    }

    SrcIterator src_end{ src + size };

    if (src_end <= dst) {
        SeqCopy(dst, src, size);
        return;
    }

    DstIterator dst_end{ dst + size };

    for (size_t i{ 0 }; i < size; ++i) { *(--dst_end) = *(--src_end); }
}

template <typename Iterator>
Iterator utils::SeqRotate(Iterator beg, Iterator mid, Iterator end) {
    // TODO

    ZETA_Core_Unused(beg);
    ZETA_Core_Unused(mid);
    ZETA_Core_Unused(end);
}

template <typename T>
T& utils::GetInstRef(T* inst) {
    return *inst;
}

template <typename T>
T& utils::GetInstRef(T& inst) {
    return inst;
}

template <typename T>
T* utils::GetInstPtr(T* inst) {
    return inst;
}

template <typename T>
T* utils::GetInstPtr(T& inst) {
    return &inst;
}

}  // namespace zeta::core
