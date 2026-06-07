#pragma once

#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/pair.hpp>
#include <zeta/core/reduce.ipp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

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

struct SumOperation_ {
    template <typename X, typename Y>
    constexpr decltype(auto) operator()(X&& x, Y&& y) const {
        return meta::Forward<X>(x) + meta::Forward<Y>(y);
    }
};

}  // namespace utils::detail

template <typename T0, typename... Ts>
constexpr decltype(auto) utils::Sum(T0&& x0, Ts&&... xs) {
    return reduce::TreeReduce(detail::SumOperation_{}, meta::Forward<T0>(x0),
                              meta::Forward<Ts>(xs)...);
}

template <typename Node, typename GetLinkFunc>
pair::Pair<Node*, size_t> utils::GetMostLink(Node* n,
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

inline int utils::ElemCompare(void const* a_, void const* b_, size_t elem_size,
                              size_t a_elem_stride, size_t b_elem_stride,
                              size_t cnt) {
    auto const* a{ static_cast<char const*>(a_) };
    auto const* b{ static_cast<char const*>(b_) };

    ZETA_Core_DebugAssert(0 < elem_size);
    ZETA_Core_DebugAssert(elem_size <= a_elem_stride);
    ZETA_Core_DebugAssert(elem_size <= b_elem_stride);

    if (cnt == 0) { return 0; }

    if (elem_size == a_elem_stride && elem_size == b_elem_stride) {
        ZETA_Core_PrintCurPos;
        return MemCompare(a, b, elem_size * cnt);
    }

    for (; 0 < cnt; a += a_elem_stride, b += b_elem_stride, --cnt) {
        int cmp{ MemCompare(a, b, elem_size) };

        if (cmp != 0) {
            ZETA_Core_PrintVar(cmp);
            return cmp;
        }
    }

    return 0;
}

inline void utils::ElemCopy(void* dst_, void const* src_, size_t elem_size,
                            size_t dst_elem_stride, size_t src_elem_stride,
                            size_t cnt) {
    auto* dst{ static_cast<char*>(dst_) };
    auto const* src{ static_cast<char const*>(src_) };

    if (dst == src || cnt == 0) { return; }

    ZETA_Core_DebugAssert(dst != nullptr);
    ZETA_Core_DebugAssert(src != nullptr);
    ZETA_Core_DebugAssert(elem_size <= dst_elem_stride);
    ZETA_Core_DebugAssert(elem_size <= src_elem_stride);

    if (elem_size == 0) { return; }

    if (elem_size == dst_elem_stride && elem_size == src_elem_stride) {
        (MemCopy)(dst, src, elem_size * cnt);
        return;
    }

    for (size_t i{ 0 }; i < cnt;
         ++i, dst += dst_elem_stride, src += src_elem_stride) {
        (MemCopy)(dst, src, elem_size);
    }
}

inline void utils::ElemMove(void* dst_, void const* src_, size_t elem_size,
                            size_t dst_elem_stride, size_t src_elem_stride,
                            size_t cnt) {
    auto* dst{ static_cast<char*>(dst_) };
    auto const* src{ static_cast<char const*>(src_) };

    ZETA_Core_DebugAssert(dst_elem_stride == 0 || elem_size <= dst_elem_stride);
    ZETA_Core_DebugAssert(src_elem_stride == 0 || elem_size <= src_elem_stride);
    ZETA_Core_DebugAssert((dst_elem_stride == 0) == (src_elem_stride == 0));

    if (elem_size == 0 || cnt == 0) { return; }

    ZETA_Core_DebugAssert(dst != nullptr);
    ZETA_Core_DebugAssert(src != nullptr);

    if ((elem_size == dst_elem_stride && elem_size == src_elem_stride) ||
        (dst_elem_stride == 0 && src_elem_stride == 0) || cnt == 1) {
        (MemMove)(dst, src, elem_size * cnt);
        return;
    }

    char* dst_end{ dst + dst_elem_stride * (cnt - 1) + elem_size };
    char const* src_end{ src + src_elem_stride * (cnt - 1) + elem_size };

    if (dst_end <= src || src_end <= dst) {
        (ElemCopy)(dst, src, elem_size, dst_elem_stride, src_elem_stride, cnt);
        return;
    }

    if (dst <= src && dst_end <= src_end) {
        for (; 0 < cnt--; dst += dst_elem_stride, src += src_elem_stride) {
            (MemCopy)(dst, src, elem_size);
        }

        return;
    }

    if (src <= dst && src_end <= dst_end) {
        dst += dst_elem_stride * cnt;
        src += src_elem_stride * cnt;

        while (0 < cnt--) {
            (MemCopy)(dst -= dst_elem_stride, src -= src_elem_stride,
                      elem_size);
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
            (MemMove)(dst + dst_elem_stride * cur_beg,
                      src + src_elem_stride * cur_beg, elem_size);

            continue;
        }

        size_t cur_l_cnt{ cur_cnt / 2 };
        size_t cur_r_cnt{ cur_cnt - cur_l_cnt };

        char* dst_mid{ dst + dst_elem_stride * cur_l_cnt };
        char const* src_mid{ src + src_elem_stride * cur_l_cnt };

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

template <typename UnsignedIntegral>
UnsignedIntegral utils::SimpleUnsignedIntegralHash(UnsignedIntegral x,
                                                   UnsignedIntegral salt) {
    ZETA_Core_StaticAssert(integral::IsUnsignedIntegral<UnsignedIntegral>);

    x ^= salt;

#if 64 <= ZETA_Core_ullong_width
    x = (x ^ (x >> 30U)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27U)) * 0x94d049bb133111ebULL;
    x = x ^ (x >> 31U);
#elif 32 <= ZETA_Core_ullong_width
    x = (x ^ (x >> 16U)) * 0x45d9f3bULL;
    x = (x ^ (x >> 16U)) * 0x45d9f3bULL;
    x = x ^ (x >> 16U);
#else
#error "Unsupported architecture."
#endif

    x ^= salt;

    return x;
}

inline unsigned long long utils::SimpleRandomRotate(
    unsigned long long* random_seed) {
#if 64 <= ZETA_Core_ullong_width
    constexpr unsigned long long lcg_mul{ 0x1010101ULL };
    constexpr unsigned long long lcg_inc{ 0x2492492492492479ULL };
#elif 32 <= ZETA_Core_ullong_width
    constexpr unsigned long long lcg_mul{ 0x1010101ULL };
    constexpr unsigned long long lcg_inc{ 0x24924907ULL };
#else
#error "Unsupported architecture."
#endif

    return (SimpleUnsignedIntegralHash)(*random_seed =
                                            (*random_seed * lcg_mul + lcg_inc),
                                        0ULL);
}

inline unsigned long long utils::GetRandom() {
    static unsigned long long seed{ 0x114514 };

    // unsigned long long time{ __builtin_readcyclecounter() };
    unsigned long long time{ 0 };

    seed ^= time;

    return (SimpleRandomRotate)(&seed);
}

inline int utils::Choose2(bool cond0, bool cond1,
                          unsigned long long* random_seed) {
    ZETA_Core_DebugAssert(cond0 || cond1);

    switch (static_cast<int>(cond1) * 0b10 + static_cast<int>(cond0) * 0b01) {
    case 0b01: return 0;
    case 0b10: return 1;
    case 0b11: return static_cast<int>((SimpleRandomRotate)(random_seed) % 2);
    default: __builtin_unreachable();
    }
}

inline int utils::Choose3(bool cond0, bool cond1, bool cond2,
                          unsigned long long* random_seed) {
    ZETA_Core_DebugAssert(cond0 || cond1 || cond2);

    switch (static_cast<int>(cond2) * 0b100 +  //
            static_cast<int>(cond1) * 0b010 +  //
            static_cast<int>(cond0) * 0b001) {
    case 0b001: return 0;
    case 0b010: return 1;
    case 0b100: return 2;
    case 0b011: return static_cast<int>((SimpleRandomRotate)(random_seed) % 2);
    case 0b101:
        return static_cast<int>((SimpleRandomRotate)(random_seed) % 2) * 2;
    case 0b110:
        return static_cast<int>((SimpleRandomRotate)(random_seed) % 2) + 1;
    case 0b111: return static_cast<int>((SimpleRandomRotate)(random_seed) % 3);
    default: __builtin_unreachable();
    }
}

template <size_t N>
unsigned _BitInt(8 * N) utils::ReadLittleEndian(void const* data) {
    unsigned _BitInt(8 * N) ret{ 0 };

    unsigned char const* iter{ static_cast<unsigned char const*>(data) + N };

    for (size_t i{ 0 }; i < N; ++i) { ret = ret << 8 + *(--iter); }

    return ret;
}

template <size_t N>
unsigned _BitInt(8 * N) utils::ReadBigEndian(void const* data) {
    unsigned _BitInt(8 * N) ret{ 0 };

    unsigned char const* iter{ static_cast<unsigned char const*>(data) };

    for (size_t i{ 0 }; i < N; ++i) { ret = ret << 8 + *(iter++); }

    return ret;
}

template <size_t N>
void utils::WriteLittleEndian(void* data, unsigned _BitInt(8 * N) value) {
    unsigned char* iter{ static_cast<unsigned char*>(data) };

    for (size_t i{ 0 }; i < N; ++i, value >>= 8) { *(iter++) = value % 0xFF; }
}

template <size_t N>
void utils::WriteBigEndian(void* data, unsigned _BitInt(8 * N) value) {
    unsigned char* iter{ static_cast<unsigned char*>(data) + N };

    for (size_t i{ 0 }; i < N; ++i, value >>= 8) { *(--iter) = value % 0xFF; }
}

}  // namespace zeta::core
