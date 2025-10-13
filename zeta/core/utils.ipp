#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

template <typename First, typename Second>
unsigned long long hash::HashCore<Pair<First, Second>>::operator()(
    Pair<First, Second> const& x, unsigned long long salt) const {
    unsigned long long h1{ hash::Hash(x.first, salt) };
    unsigned long long h2{ hash::Hash(x.second, h1) };
    return hash::Hash(h2, salt);
}

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
int compare::CompareCore<Pair<XFirst, XSecond>, Pair<YFirst, YSecond>>::
operator()(Pair<XFirst, XSecond> const& x,
           Pair<YFirst, YSecond> const& y) const {
    int cmp{ compare::Compare(x.first, y.first) };
    if (cmp != 0) { return cmp; }
    return Compare(x.second, y.second);
}

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool operator==(Pair<XFirst, XSecond> const& x,
                Pair<YFirst, YSecond> const& y) {
    return compare::Compare(x, y) == 0;
}

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool operator!=(Pair<XFirst, XSecond> const& x,
                Pair<YFirst, YSecond> const& y) {
    return compare::Compare(x, y) != 0;
}

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool operator<(Pair<XFirst, XSecond> const& x, Pair<YFirst, YSecond> const& y) {
    return compare::Compare(x, y) < 0;
}

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool operator<=(Pair<XFirst, XSecond> const& x,
                Pair<YFirst, YSecond> const& y) {
    return compare::Compare(x, y) <= 0;
}

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool operator>(Pair<XFirst, XSecond> const& x, Pair<YFirst, YSecond> const& y) {
    return compare::Compare(x, y) > 0;
}

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool operator>=(Pair<XFirst, XSecond> const& x,
                Pair<YFirst, YSecond> const& y) {
    return compare::Compare(x, y) >= 0;
}

// -----------------------------------------------------------------------------

template <typename First, typename Second, typename Third>
unsigned long long hash::HashCore<Triplet<First, Second, Third>>::operator()(
    Triplet<First, Second, Third> const& x, unsigned long long salt) const {
    unsigned long long h1{ hash::Hash(x.first, salt) };
    unsigned long long h2{ hash::Hash(x.second, h1) };
    unsigned long long h3{ hash::Hash(x.third, h2) };
    return hash::Hash(h3, salt);
}

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
int compare::CompareCore<Triplet<XFirst, XSecond, XThird>,
                         Triplet<YFirst, YSecond, YThird>>::
operator()(Triplet<XFirst, XSecond, XThird> const& x,
           Triplet<YFirst, YSecond, YThird> const& y) const {
    int cmp{ compare::Compare(x.first, y.first) };
    if (cmp != 0) { return cmp; }
    cmp = compare::Compare(x.second, y.second);
    if (cmp != 0) { return cmp; }
    return Compare(x.third, y.third);
}

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool operator==(Triplet<XFirst, XSecond, XThird> const& x,
                Triplet<YFirst, YSecond, YThird> const& y) {
    return compare::Compare(x, y) == 0;
}

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool operator!=(Triplet<XFirst, XSecond, XThird> const& x,
                Triplet<YFirst, YSecond, YThird> const& y) {
    return compare::Compare(x, y) != 0;
}

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool operator<(Triplet<XFirst, XSecond, XThird> const& x,
               Triplet<YFirst, YSecond, YThird> const& y) {
    return compare::Compare(x, y) < 0;
}

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool operator<=(Triplet<XFirst, XSecond, XThird> const& x,
                Triplet<YFirst, YSecond, YThird> const& y) {
    return compare::Compare(x, y) <= 0;
}

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool operator>(Triplet<XFirst, XSecond, XThird> const& x,
               Triplet<YFirst, YSecond, YThird> const& y) {
    return compare::Compare(x, y) > 0;
}

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool operator>=(Triplet<XFirst, XSecond, XThird> const& x,
                Triplet<YFirst, YSecond, YThird> const& y) {
    return compare::Compare(x, y) >= 0;
}

// -----------------------------------------------------------------------------

namespace detail {

template <size_t N>
struct GetNth_ {
    template <typename Arg0, typename Arg1, typename Arg2, typename Arg3,
              typename... Args>
    static decltype(auto) Get(Arg0&&, Arg1&&, Arg2&&, Arg3&&, Args&&... args) {
        return GetNth_<N - 4>::Get(Forward<Args>(args)...);
    }
};

template <>
struct GetNth_<0> {
    template <typename Arg0, typename... Args>
    static decltype(auto) Get(Arg0&& x, Args&&...) {
        return Forward<Arg0>(x);
    }
};

template <>
struct GetNth_<1> {
    template <typename Arg0, typename Arg1, typename... Args>
    static decltype(auto) Get(Arg0&&, Arg1&& x, Args&&...) {
        return Forward<Arg1>(x);
    }
};

template <>
struct GetNth_<2> {
    template <typename Arg0, typename Arg1, typename Arg2, typename... Args>
    static decltype(auto) Get(Arg0&&, Arg1&&, Arg2&& x, Args&&...) {
        return Forward<Arg2>(x);
    }
};

template <>
struct GetNth_<3> {
    template <typename Arg0, typename Arg1, typename Arg2, typename Arg3,
              typename... Args>
    static decltype(auto) Get(Arg0&&, Arg1&&, Arg2&&, Arg3&& x, Args&&...) {
        return Forward<Arg3>(x);
    }
};

}  // namespace detail

template <size_t N, typename... Args>
decltype(auto) GetNth(Args&&... args) {
    static_assert(N < 1 + sizeof...(Args));
    return detail::GetNth_<N>::Get(Forward<Args>(args)...);
}

// -----------------------------------------------------------------------------

template <typename T>
RemoveRef<T>&& Move(T&& t) {
    return static_cast<RemoveRef<T>&&>(t);
}

template <typename T>
constexpr T&& Forward(RemoveRef<T>& t) {
    return static_cast<T&&>(t);
}

template <typename X, typename Y>
void Swap(X&& x, Y&& y) {
    if (&x == &y) { return; }
    auto tmp{ Move(x) };
    x = Move(y);
    y = Move(tmp);
}

// -----------------------------------------------------------------------------

namespace detail {

template <typename Operation, typename T0>
decltype(auto) MakeLeftAssocOperation_(Operation const&, T0&& x0) {
    return Forward<T0>(x0);
}

template <typename Operation, typename T0, typename T1, typename... Ts>
decltype(auto) MakeLeftAssocOperation_(Operation const& opr, T0&& x0, T1&& x1,
                                       Ts&&... xs) {
    return MakeLeftAssocOperation_(opr(Forward<T0>(x0), Forward<T1>(x1)),
                                   Forward<Ts>(xs)...);
}

}  // namespace detail

template <typename Operation, typename T0, typename... Ts>
decltype(auto) MakeLeftAssocOperation(Operation const& opr, T0&& x0,
                                      Ts&&... xs) {
    return detail::MakeLeftAssocOperation_(opr, Forward<T0>(x0),
                                           Forward<Ts>(xs)...);
}

// -----------------------------------------------------------------------------

namespace detail {

template <typename Operation, typename T0>
decltype(auto) MakeRightAssocOperation_(Operation const&, T0&& x0) {
    return Forward<T0>(x0);
}

template <typename Operation, typename T0, typename T1, typename... Ts>
decltype(auto) MakeRightAssocOperation_(Operation const& opr, T0&& x0, T1&& x1,
                                        Ts&&... xs) {
    return opr(Forward<T0>(x0), MakeRightAssocOperation_(opr, Forward<T1>(x1),
                                                         Forward<Ts>(xs)...));
}

}  // namespace detail

template <typename Operation, typename T0, typename... Ts>
decltype(auto) MakeRightAssocOperation(Operation const& opr, T0&& x0,
                                       Ts&&... xs) {
    return detail::MakeRightAssocOperation_(opr, Forward<T0>(x0),
                                            Forward<Ts>(xs)...);
}

// -----------------------------------------------------------------------------

namespace detail {

template <size_t Beg, size_t End>
struct MakeTreeAssocOperator_ {
    template <typename Operation, typename... Ts>
    static decltype(auto) F(Operation const& opr, Ts&&... xs) {
        constexpr size_t Mid{ (Beg + End) / 2 };

        return opr(
            MakeTreeAssocOperator_<Beg, Mid>::F(opr, Forward<Ts>(xs)...),
            MakeTreeAssocOperator_<Mid, End>::F(opr, Forward<Ts>(xs)...));
    }
};

template <size_t Beg>
struct MakeTreeAssocOperator_<Beg, Beg + 1> {
    template <typename Operation, typename... Ts>
    static decltype(auto) F(Operation const&, Ts&&... xs) {
        return GetNth<Beg>(Forward<Ts>(xs)...);
    }
};

}  // namespace detail

template <typename Operation, typename T0, typename... Ts>
decltype(auto) MakeTreeAssocOperation(Operation const& opr, T0&& x0,
                                      Ts&&... xs) {
    return detail::MakeTreeAssocOperator_<0, 1 + sizeof...(xs)>::F(
        opr, Forward<T0>(x0), Forward<Ts>(xs)...);
}

// -----------------------------------------------------------------------------

namespace detail {

struct MinOperation_ {
    template <typename X, typename Y>
    decltype(auto) operator()(X&& x, Y&& y) const {
        return y < x ? Forward<Y>(y) : Forward<X>(x);
    }
};

}  // namespace detail

template <typename T0, typename... Ts>
decltype(auto) Min(T0&& x0, Ts&&... xs) {
    return MakeTreeAssocOperation(detail::MinOperation_{}, Forward<T0>(x0),
                                  Forward<Ts>(xs)...);
}

// -----------------------------------------------------------------------------

namespace detail {

struct MaxOperation_ {
    template <typename X, typename Y>
    decltype(auto) operator()(X&& x, Y&& y) const {
        return x < y ? Forward<Y>(y) : Forward<X>(x);
    }
};

}  // namespace detail

template <typename T0, typename... Ts>
decltype(auto) Max(T0&& x0, Ts&&... xs) {
    return MakeTreeAssocOperation(detail::MaxOperation_{}, Forward<T0>(x0),
                                  Forward<Ts>(xs)...);
}

// -----------------------------------------------------------------------------

namespace detail {

struct SumOperation_ {
    template <typename X, typename Y>
    decltype(auto) operator()(X&& x, Y&& y) const {
        return Forward<X>(x) + Forward<Y>(y);
    }
};

}  // namespace detail

template <typename T0, typename... Ts>
decltype(auto) Sum(T0&& x0, Ts&&... xs) {
    return MakeTreeAssocOperation(detail::SumOperation_{}, Forward<T0>(x0),
                                  Forward<Ts>(xs)...);
}

// -----------------------------------------------------------------------------

template <typename Node, typename GetLinkFunc>
Pair<Node*, size_t> GetMostLink(Node* n, GetLinkFunc const& get_link) {
    if (n == nullptr) { return { nullptr, 0 }; }

    for (size_t i{ 1 };; ++i) {
        Node* n_nxt{ get_link(n) };
        if (n_nxt == nullptr) { return { n, i }; }
        n = n_nxt;
    }
}

// -----------------------------------------------------------------------------

inline int MemCompare(void const* a, void const* b, size_t size) {
    if (a == b || size == 0) { return 0; }

    ZETA_Core_DebugAssert(a != nullptr);
    ZETA_Core_DebugAssert(b != nullptr);

    return __builtin_memcmp(a, b, size);
}

inline void MemSwap(void* x_, void* y_, size_t size) {
    auto x{ static_cast<char*>(x_) };
    auto y{ static_cast<char*>(y_) };

    if (x == y || size == 0) { return; }

    ZETA_Core_DebugAssert(x != nullptr);
    ZETA_Core_DebugAssert(y != nullptr);

    for (size_t i{ 0 }; i < size; ++i) { Swap(x[i], y[i]); }
}

inline void MemCopy(void* dst, void const* src, size_t size) {
    if (dst == src || size == 0) { return; }

    ZETA_Core_DebugAssert(dst != nullptr);
    ZETA_Core_DebugAssert(src != nullptr);

    __builtin_memcpy(dst, src, size);
}

inline void MemMove(void* dst, void const* src, size_t size) {
    if (dst == src || size == 0) { return; }

    ZETA_Core_DebugAssert(dst != nullptr);
    ZETA_Core_DebugAssert(src != nullptr);

    __builtin_memmove(dst, src, size);
}

inline void* MemRotate(void* data_, size_t l_size, size_t r_size) {
    auto data{ static_cast<char*>(data_) };

    if (l_size == 0 && r_size == 0) { return data; }

    ZETA_Core_DebugAssert(data != NULL);

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

inline unsigned long long MemHash(void const* data_, size_t size,
                                  unsigned long long salt) {
    constexpr unsigned long long fnv_offset_basis{ 14695981039346656037ULL };
    constexpr unsigned long long fnv_prime{ 1099511628211ULL };

    auto data{ static_cast<unsigned char const*>(data_) };
    ZETA_Core_DebugAssert(data != NULL);

    unsigned long long ret{ fnv_offset_basis };

    for (size_t i{ 0 }; i < size; ++i) {
        ret = (ret * fnv_prime) ^ data[i] + salt;
    }

    return ret;
}

// -----------------------------------------------------------------------------

inline int ElemCompare(void const* a_, void const* b_, size_t width,
                       size_t a_stride, size_t b_stride, size_t cnt) {
    auto a{ static_cast<char const*>(a_) };
    auto b{ static_cast<char const*>(b_) };

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

inline void ElemCopy(void* dst_, void const* src_, size_t width,
                     size_t dst_stride, size_t src_stride, size_t cnt) {
    auto dst{ static_cast<char*>(dst_) };
    auto src{ static_cast<char const*>(src_) };

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

inline unsigned long long ElemHash(void const* data_, size_t width,
                                   size_t stride, size_t cnt,
                                   unsigned long long salt) {
    auto data{ static_cast<unsigned char const*>(data_) };

    ZETA_Core_DebugAssert(data != nullptr);
    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(0 < stride);
    ZETA_Core_DebugAssert(width <= stride);

    unsigned long long ret{ 14695981039346656037ULL };

    for (size_t i{ 0 }; i < cnt; ++i, data += stride) {
        for (size_t j{ 0 }; j < width; ++j) {
            ret = (ret * 1099511628211ULL) ^ data[j] + salt;
        }
    }

    return ret;
}

inline void ElemMove(void* dst_, void const* src_, size_t width,
                     size_t dst_stride, size_t src_stride, size_t cnt) {
    auto dst{ static_cast<char*>(dst_) };
    auto src{ static_cast<char const*>(src_) };

    ZETA_Core_DebugAssert(width <= dst_stride);
    ZETA_Core_DebugAssert(width <= src_stride);
    ZETA_Core_DebugAssert(0 < dst_stride);
    ZETA_Core_DebugAssert(0 < src_stride);

    if (cnt == 0) { return; }

    ZETA_Core_DebugAssert(dst != nullptr);
    ZETA_Core_DebugAssert(src != nullptr);

    if (width == dst_stride && width == src_stride) {
        MemMove(dst, src, width * cnt);
        return;
    }

    if (dst == src && dst_stride == src_stride) { return; }

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

    size_t buffer_capacity{ static_cast<size_t>(FloorLog2(cnt)) + 4 };

    auto begs{ static_cast<size_t*>(__builtin_alloca_with_align(
        sizeof(size_t) * buffer_capacity, __CHAR_BIT__ * alignof(size_t))) };

    auto cnts{ static_cast<size_t*>(__builtin_alloca_with_align(
        sizeof(size_t) * buffer_capacity, __CHAR_BIT__ * alignof(size_t))) };

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

inline void* ElemRotate(void* data_, size_t width, size_t stride, size_t l_size,
                        size_t r_size) {
    auto data{ static_cast<char*>(data_) };

    if (width == stride) {
        return MemRotate(data, stride * l_size, stride * r_size);
    }

    if (l_size == 0 && r_size == 0) { return data; }

    ZETA_Core_DebugAssert(data != NULL);

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

// -----------------------------------------------------------------------------

template <typename XIterator, typename YIterator>
int SeqCompare(XIterator x, YIterator y, size_t size) {
    if (x == y) { return 0; }

    for (size_t i{ 0 }; i < size; ++i, ++x, ++y) {
        int cmp{ ThreeWayCompare(*x, *y) };
        if (cmp != 0) { return cmp; }
    }

    return 0;
}

template <typename DstIterator, typename SrcIterator>
void SeqCopy(DstIterator dst, SrcIterator src, size_t size) {
    if (dst == src) { return; }

    for (size_t i{ 0 }; i < size; ++i, ++dst, ++src) { *dst = *src; }
}

template <typename DstIterator, typename SrcIterator>
void SeqMove(DstIterator dst, SrcIterator src, size_t size) {
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
Iterator SeqRotate(Iterator beg, Iterator mid, Iterator end) {
    // TODO

    ZETA_Core_Unused(beg);
    ZETA_Core_Unused(mid);
    ZETA_Core_Unused(end);
}

// -----------------------------------------------------------------------------

template <typename UnsignedIntegral>
constexpr UnsignedIntegral UIntCeilDiv(UnsignedIntegral x, UnsignedIntegral y) {
    ZETA_Core_StaticAssert(IsUnsignedIntegral<UnsignedIntegral>);

    ZETA_Core_DebugAssert(0 < y);

    return x == 0 ? 0 : (x - 1) / y + 1;
}

// -----------------------------------------------------------------------------

template <typename UnsignedIntegral>
constexpr UnsignedIntegral UIntAlignDown(UnsignedIntegral val,
                                         UnsignedIntegral align) {
    ZETA_Core_StaticAssert(IsUnsignedIntegral<UnsignedIntegral>);

    ZETA_Core_DebugAssert(0 < align);

    return val - val % align;
}

template <typename UnsignedIntegral>
constexpr UnsignedIntegral UIntAlignUp(UnsignedIntegral val,
                                       UnsignedIntegral align) {
    ZETA_Core_StaticAssert(IsUnsignedIntegral<UnsignedIntegral>);

    ZETA_Core_DebugAssert(0 < align);

    return val == 0 ? 0 : val - 1 - (val - 1) % align + align;
}

// -----------------------------------------------------------------------------

inline constexpr unsigned long long Power(unsigned long long base,
                                          unsigned exp) {
    if (base == 0) { return 0; }

    if (__builtin_popcountll(base) == 1) {
        return 1ULL << (static_cast<unsigned>(__builtin_ctzll(base)) * exp);
    }

    unsigned long long ret{ 1 };

    for (; 0 < exp; exp /= 2) {
        if (exp % 2 != 0) { ret *= base; }
        base *= base;
    }

    return ret;
}

// -----------------------------------------------------------------------------

inline constexpr unsigned FloorLog2(unsigned long long x) {
    ZETA_Core_DebugAssert(0 < x);
    return ZETA_Core_ullong_width - 1 -
           static_cast<unsigned>(__builtin_clzll(x));
}

inline constexpr unsigned CeilLog2(unsigned long long x) {
    return x <= 1 ? 0
                  : ZETA_Core_ullong_width -
                        static_cast<unsigned>(__builtin_clzll(x - 1));
}

inline constexpr unsigned FloorLog(unsigned long long x, unsigned base) {
    ZETA_Core_DebugAssert(1 < base);

    if (base == 2) { return FloorLog2(x); }

    unsigned ceil_log2_base{ CeilLog2(base) };

    unsigned ret{ 0 };

    for (;;) {
        unsigned cur_ret{ FloorLog2(x) / ceil_log2_base };

        ret += cur_ret;

        if (cur_ret <= 1) {
            if (cur_ret == 1) { x /= base; }
            break;
        }

        x /= Power(base, cur_ret);
    }

    for (; base <= x; x /= base) { ++ret; }

    return ret;
}

inline constexpr unsigned CeilLog(unsigned long long x, unsigned base) {
    ZETA_Core_DebugAssert(1 < base);

    if (base == 2) { return CeilLog2(x); }

    return x <= 1 ? 0 : FloorLog(x - 1, base) + 1;
}

// -----------------------------------------------------------------------------

inline constexpr int FindPrevOne(unsigned long long val, int pos) {
    ZETA_Core_DebugAssert(-1 <= pos);
    ZETA_Core_DebugAssert(pos <= ZETA_Core_ullong_width);

    if (pos <= 0) { return -1; }

    val &= ~0ULL >> (ZETA_Core_ullong_width - pos);

    return val == 0 ? -1 : ZETA_Core_ullong_width - 1 - __builtin_clzll(val);
}

inline constexpr int FindNextOne(unsigned long long val, int pos) {
    ZETA_Core_DebugAssert(-1 <= pos);
    ZETA_Core_DebugAssert(pos <= ZETA_Core_ullong_width);

    if (ZETA_Core_ullong_width - 1 <= pos) { return -1; }

    val &= ~0ULL << (pos + 1);

    return val == 0 ? -1 : __builtin_ctzll(val);
}

// -----------------------------------------------------------------------------

inline unsigned long long GCD(unsigned long long x, unsigned long long y) {
    if (x == 0) { return Max(1ULL, y); }

    for (;;) {
        if ((y %= x) == 0) { return x; }
        if ((x %= y) == 0) { return y; }
    }
}

inline unsigned long long LCM(unsigned long long x, unsigned long long y) {
    unsigned long long gcd{ GCD(x, y) };
    return x / gcd * y;
}

}  // namespace zeta::core
