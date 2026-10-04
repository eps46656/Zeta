#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/lifecycle.hpp>
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

constexpr int utils::MemCompare(void const* a, void const* b, size_t size) {
    if (a == b || size == 0) { return 0; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(a != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(b != nullptr);

    return __builtin_memcmp(a, b, size);
}

constexpr void utils::MemSwap(void* x_, void* y_, size_t size) {
    auto* x{ static_cast<char*>(x_) };
    auto* y{ static_cast<char*>(y_) };

    if (x == y || size == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(x != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(y != nullptr);

    for (size_t i{ 0 }; i < size; ++i) { (Swap)(x[i], y[i]); }
}

constexpr void utils::MemCopy(void* dst, void const* src, size_t size) {
    if (dst == src || size == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(src != nullptr);

    __builtin_memcpy(dst, src, size);
}

constexpr void utils::MemMove(void* dst, void const* src, size_t size) {
    if (dst == src || size == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(src != nullptr);

    __builtin_memmove(dst, src, size);
}

constexpr void* utils::MemRotate(void* data_, size_t l_size, size_t r_size) {
    auto* data{ static_cast<char*>(data_) };

    if (l_size == 0 && r_size == 0) { return data; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(data != nullptr);

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

template <typename T>
constexpr T* utils::PtrInc(T* ptr, size_t shift) {
    return reinterpret_cast<T*>(
        reinterpret_cast<meta::MakeConstIf<char, meta::IsConst<T>>*>(ptr) +
        shift);
}

template <typename T>
constexpr T* utils::PtrInc(T* ptr, ptrdiff_t shift) {
    return reinterpret_cast<T*>(
        reinterpret_cast<meta::MakeConstIf<char, meta::IsConst<T>>*>(ptr) +
        shift);
}

template <typename T>
constexpr T* utils::PtrDec(T* ptr, size_t shift) {
    return reinterpret_cast<T*>(
        reinterpret_cast<meta::MakeConstIf<char, meta::IsConst<T>>*>(ptr) -
        shift);
}

template <typename T>
constexpr T* utils::PtrDec(T* ptr, ptrdiff_t shift) {
    return reinterpret_cast<T*>(
        reinterpret_cast<meta::MakeConstIf<char, meta::IsConst<T>>*>(ptr) -
        shift);
}

namespace utils::detail {

template <integral::IsIntegral StrideIntegral>
constexpr void EquistrideLinSeqCopy_(void* dst_, void const* src_,
                                     size_t elem_size,
                                     StrideIntegral elem_stride, size_t cnt) {
    char* dst{ static_cast<char*>(dst_) };
    char const* src{ static_cast<char const*>(src_) };

    if (dst == src || elem_size == 0 || cnt == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(src != nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        elem_stride == 0 ||
        elem_size <= static_cast<size_t>(integral_math::Abs(elem_stride)));

    if (elem_stride == 0 || cnt == 1) {
        (MemCopy)(dst, src, elem_size);
        return;
    }

    StrideIntegral shift{ elem_stride * static_cast<StrideIntegral>(cnt - 1) };

    if constexpr (integral::IsSignedIntegral<StrideIntegral>) {
        if (elem_stride < 0) {
            dst += shift;
            src += shift;

            elem_stride = -elem_stride;
            shift = -shift;
        }
    }

    if (elem_size == static_cast<size_t>(elem_stride)) {
        (MemCopy)(dst, src, static_cast<size_t>(shift + elem_stride));
        return;
    }

    for (size_t i{ 0 }; i < cnt; ++i, dst += elem_stride, src += elem_stride) {
        (MemCopy)(dst, src, elem_size);
    }
}

}  // namespace utils::detail

constexpr void utils::EquistrideLinSeqCopy(void* dst, void const* src,
                                           size_t elem_size, size_t elem_stride,
                                           size_t cnt) {
    detail::EquistrideLinSeqCopy_<size_t>(dst, src, elem_size, elem_stride,
                                          cnt);
}

constexpr void utils::EquistrideLinSeqCopy(void* dst_, void const* src_,
                                           size_t elem_size,
                                           ptrdiff_t elem_stride, size_t cnt) {
    detail::EquistrideLinSeqCopy_<ptrdiff_t>(dst_, src_, elem_size, elem_stride,
                                             cnt);
}

namespace utils::detail {

template <integral::IsIntegral StrideIntegral, typename DstElem,
          typename SrcElem>
constexpr void EquistrideLinSeqCopy_(void* dst_, void const* src_,
                                     meta::TypeWrapper<DstElem>,
                                     meta::TypeWrapper<SrcElem>,
                                     StrideIntegral elem_stride, size_t cnt) {
    char* dst{ static_cast<char*>(dst_) };
    char const* src{ static_cast<char const*>(src_) };

    if (dst == src || cnt == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(src != nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        elem_stride == 0 ||
        sizeof(DstElem) <=
            static_cast<size_t>(integral_math::Abs(elem_stride)) ||
        sizeof(SrcElem) <=
            static_cast<size_t>(integral_math::Abs(elem_stride)));

    if (elem_stride == 0 || cnt == 1) {
        *static_cast<DstElem*>(dst) = *static_cast<SrcElem const*>(src);
        return;
    }

    StrideIntegral shift{ elem_stride * static_cast<StrideIntegral>(cnt - 1) };

    if constexpr (integral::IsSignedIntegral<StrideIntegral>) {
        if (elem_stride < 0) {
            dst += shift;
            src += shift;

            elem_stride = -elem_stride;
            shift = -shift;
        }
    }

    for (size_t i{ 0 }; i < cnt; ++i, dst += elem_stride, src += elem_stride) {
        *static_cast<DstElem*>(dst) = *static_cast<SrcElem const*>(src);
    }
}

}  // namespace utils::detail

template <typename DstElem, typename SrcElem>
constexpr void utils::EquistrideLinSeqCopy(void* dst, void const* src,
                                           meta::TypeWrapper<DstElem>,
                                           meta::TypeWrapper<SrcElem>,
                                           size_t elem_stride, size_t cnt) {
    detail::EquistrideLinSeqCopy_<size_t>(
        dst, src, meta::TypeWrapper<DstElem>{}, meta::TypeWrapper<SrcElem>{},
        elem_stride, cnt);
}

template <typename DstElem, typename SrcElem>
constexpr void utils::EquistrideLinSeqCopy(void* dst, void const* src,
                                           meta::TypeWrapper<DstElem>,
                                           meta::TypeWrapper<SrcElem>,
                                           ptrdiff_t elem_stride, size_t cnt) {
    detail::EquistrideLinSeqCopy_<ptrdiff_t>(
        dst, src, meta::TypeWrapper<DstElem>{}, meta::TypeWrapper<SrcElem>{},
        elem_stride, cnt);
}

namespace utils::detail {

template <integral::IsIntegral StrideIntegral>
constexpr void LinSeqCopy_(void* dst_, void const* src_, size_t elem_size,
                           StrideIntegral dst_elem_stride,
                           StrideIntegral src_elem_stride, size_t cnt) {
    auto* dst{ static_cast<char*>(dst_) };
    auto const* src{ static_cast<char const*>(src_) };

    if (dst == src || cnt == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        dst_elem_stride == 0 ||
        elem_size <= static_cast<size_t>(integral_math::Abs(dst_elem_stride)));

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        src_elem_stride == 0 ||
        elem_size <= static_cast<size_t>(integral_math::Abs(src_elem_stride)));

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <= 1 || dst_elem_stride != 0 ||
                                            src_elem_stride == 0);

    if (elem_size == 0 || cnt == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(src != nullptr);

    if (dst_elem_stride == 0 || cnt == 1) {  // src_elem_stride == 0
        (MemCopy)(dst, src, elem_size);
        return;
    }

    if (elem_size == static_cast<size_t>(dst_elem_stride) &&
        elem_size == static_cast<size_t>(src_elem_stride)) {
        (MemCopy)(dst, src, elem_size * cnt);
        return;
    }

    if constexpr (integral::IsSignedIntegral<StrideIntegral>) {
        StrideIntegral neg_elem_size{ static_cast<StrideIntegral>(elem_size) };

        if (neg_elem_size == dst_elem_stride &&
            neg_elem_size == src_elem_stride) {
            size_t shift{ elem_size * (cnt - 1) };
            (MemCopy)(dst - shift, src - shift, shift + elem_size);
            return;
        }
    }

    for (size_t i{ 0 }; i < cnt;
         ++i, dst += dst_elem_stride, src += src_elem_stride) {
        (MemCopy)(dst, src, elem_size);
    }
}

}  // namespace utils::detail

constexpr void utils::LinSeqCopy(void* dst, void const* src, size_t elem_size,
                                 size_t dst_elem_stride, size_t src_elem_stride,
                                 size_t cnt) {
    detail::LinSeqCopy_<size_t>(dst, src, elem_size, dst_elem_stride,
                                src_elem_stride, cnt);
}

constexpr void utils::LinSeqCopy(void* dst, void const* src, size_t elem_size,
                                 ptrdiff_t dst_elem_stride,
                                 ptrdiff_t src_elem_stride, size_t cnt) {
    detail::LinSeqCopy_<ptrdiff_t>(dst, src, elem_size, dst_elem_stride,
                                   src_elem_stride, cnt);
}

namespace utils::detail {

template <integral::IsIntegral StrideIntegral,
          lifecycle::IsDataTransferOpLike DataTransferOpLike, typename DstElem,
          typename SrcElem>
constexpr void DisjointLinSeqTransfer_(DataTransferOpLike data_transfer_op_like,
                                       DstElem* dst, SrcElem* src,
                                       StrideIntegral dst_elem_stride,
                                       StrideIntegral src_elem_stride,
                                       size_t cnt) {
    if (dst == src || cnt == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        dst_elem_stride == 0 ||
        sizeof(DstElem) <=
            static_cast<size_t>(integral_math::Abs(dst_elem_stride)));

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        src_elem_stride == 0 ||
        sizeof(SrcElem) <=
            static_cast<size_t>(integral_math::Abs(src_elem_stride)));

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <= 1 || dst_elem_stride != 0 ||
                                            src_elem_stride == 0);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(src != nullptr);

    if (dst_elem_stride == 0) {  // src_elem_stride == 0
        cnt = 1;
    }

    for (; 0 < cnt--; dst = (PtrInc)(dst, dst_elem_stride),
                      src = (PtrInc)(src, src_elem_stride)) {
        lifecycle::DataTransfer(data_transfer_op_like, dst, *src);
    }
}

}  // namespace utils::detail

template <lifecycle::IsDataTransferOpLike DataTransferOpLike, typename DstElem,
          typename SrcElem>
constexpr void utils::DisjointLinSeqTransfer(
    DataTransferOpLike data_transfer_op_like, DstElem* dst, SrcElem* src,
    size_t dst_elem_stride, size_t src_elem_stride, size_t cnt) {
    detail::DisjointLinSeqTransfer_<size_t>(
        data_transfer_op_like, dst, src, dst_elem_stride, src_elem_stride, cnt);
}

template <lifecycle::IsDataTransferOpLike DataTransferOpLike, typename DstElem,
          typename SrcElem>
constexpr void utils::DisjointLinSeqTransfer(
    DataTransferOpLike data_transfer_op_like, DstElem* dst, SrcElem* src,
    ptrdiff_t dst_elem_stride, ptrdiff_t src_elem_stride, size_t cnt) {
    detail::DisjointLinSeqTransfer_<ptrdiff_t>(
        data_transfer_op_like, dst, src, dst_elem_stride, src_elem_stride, cnt);
}

namespace utils::detail {

template <integral::IsIntegral StrideIntegral>
constexpr void EquistrideLinSeqMove_(void* dst_, void const* src_,
                                     size_t elem_size,
                                     StrideIntegral elem_stride, size_t cnt) {
    char* dst{ static_cast<char*>(dst_) };
    char const* src{ static_cast<char const*>(src_) };

    if (dst == src || elem_size == 0 || cnt == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(src != nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        elem_stride == 0 ||
        elem_size <= static_cast<size_t>(integral_math::Abs(elem_stride)));

    if (elem_stride == 0 || cnt == 1) {
        (MemCopy)(dst, src, elem_size);
        return;
    }

    StrideIntegral shift{ elem_stride * static_cast<StrideIntegral>(cnt - 1) };

    if (elem_stride < 0) {
        dst += shift;
        src += shift;

        elem_stride = -elem_stride;
        shift = -shift;
    }

    if (elem_size == static_cast<size_t>(elem_stride)) {
        (MemMove)(dst, src, static_cast<size_t>(shift + elem_stride));
        return;
    }

    ptrdiff_t diff{ dst - src };

    /*

    (-inf, -elem_stride] -> FORWARD_COPY
    (-elem_stride, 0) -> FORWARD_MOVE
    (0, elem_stride) -> BACKWARD_MOVE
    [elem_stride, shift + elem_size) -> BACKWARD_COPY
    [shift + elem_size, +inf) -> FORWARD_COPY

    */

    if (diff <= -static_cast<ptrdiff_t>(elem_stride) ||
        static_cast<ptrdiff_t>(shift) + static_cast<ptrdiff_t>(elem_size) <=
            diff) {
        for (size_t i{ 0 }; i < cnt;
             ++i, dst += elem_stride, src += elem_stride) {
            (MemCopy)(dst, src, elem_size);
        }

        return;
    }

    if (diff < 0) {
        for (size_t i{ 0 }; i < cnt;
             ++i, dst += elem_stride, src += elem_stride) {
            (MemMove)(dst, src, elem_size);
        }

        return;
    }

    dst += shift;
    src += shift;

    if (diff < static_cast<ptrdiff_t>(elem_stride)) {
        for (size_t i{ 0 }; i < cnt;
             ++i, dst -= elem_stride, src -= elem_stride) {
            (MemCopy)(dst, src, elem_size);
        }
    } else {
        for (size_t i{ 0 }; i < cnt;
             ++i, dst -= elem_stride, src -= elem_stride) {
            (MemMove)(dst, src, elem_size);
        }
    }
}

}  // namespace utils::detail

constexpr void utils::EquistrideLinSeqMove(void* dst, void const* src,
                                           size_t elem_size, size_t elem_stride,
                                           size_t cnt) {
    detail::EquistrideLinSeqMove_<size_t>(dst, src, elem_size, elem_stride,
                                          cnt);
}

constexpr void utils::EquistrideLinSeqMove(void* dst_, void const* src_,
                                           size_t elem_size,
                                           ptrdiff_t elem_stride, size_t cnt) {
    detail::EquistrideLinSeqMove_<ptrdiff_t>(dst_, src_, elem_size, elem_stride,
                                             cnt);
}

constexpr void utils::LinSeqMove(void* dst_, void const* src_, size_t elem_size,
                                 size_t dst_elem_stride, size_t src_elem_stride,
                                 size_t cnt) {
    auto* dst{ static_cast<char*>(dst_) };
    auto const* src{ static_cast<char const*>(src_) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_elem_stride == 0 ||
                                            elem_size <= dst_elem_stride);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(src_elem_stride == 0 ||
                                            elem_size <= src_elem_stride);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <= 1 || dst_elem_stride != 0 ||
                                            src_elem_stride == 0);
    // can not process when multiple src elems move to single dst elem.

    if (elem_size == 0 || cnt == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(src != nullptr);

    if (dst_elem_stride == 0 || cnt == 1) {  // src_elem_stride == 0
        (MemMove)(dst, src, elem_size);
        return;
    }

    // dst_elem_stride != 0 && 1 < cnt

    char* dst_end{ dst + dst_elem_stride * (cnt - 1) + elem_size };
    char const* src_end{ src + src_elem_stride * (cnt - 1) + elem_size };

    if (src_elem_stride == 0) {
        (MemMove)(dst, src, elem_size);

        for (char* dst_iter{ dst + dst_elem_stride }; dst_iter != dst_end;
             dst_iter += dst_elem_stride) {
            (MemCopy)(dst_iter, dst, elem_size);
        }

        return;
    }

    if (dst_end <= src || src_end <= dst) {
        (LinSeqCopy)(dst, src, elem_size, dst_elem_stride, src_elem_stride,
                     cnt);
        return;
    }

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

        char* dst_mid{ dst + dst_elem_stride * (cur_beg + cur_l_cnt) };
        char const* src_mid{ src + src_elem_stride * (cur_beg + cur_l_cnt) };

        if (dst_mid <= src_mid) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(buffer_i < buffer_capacity);

            begs[buffer_i] = cur_beg + cur_l_cnt;
            cnts[buffer_i] = cur_r_cnt;
            ++buffer_i;

            ZETA_Core_DebugUtils_Diag_PromiseAssert(buffer_i < buffer_capacity);

            begs[buffer_i] = cur_beg;
            cnts[buffer_i] = cur_l_cnt;
            ++buffer_i;
        } else {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(buffer_i < buffer_capacity);

            begs[buffer_i] = cur_beg;
            cnts[buffer_i] = cur_l_cnt;
            ++buffer_i;

            ZETA_Core_DebugUtils_Diag_PromiseAssert(buffer_i < buffer_capacity);

            begs[buffer_i] = cur_beg + cur_l_cnt;
            cnts[buffer_i] = cur_r_cnt;
            ++buffer_i;
        }
    }
}

constexpr void* utils::LinSeqRotate(void* data_, size_t width, size_t stride,
                                    size_t l_size, size_t r_size) {
    auto* data{ static_cast<char*>(data_) };

    if (width == 0 || stride == 0) { return data; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(width <= stride);

    if (width == stride) {
        return (MemRotate)(data, stride * l_size, stride * r_size);
    }

    if (l_size == 0 && r_size == 0) { return data; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(data != nullptr);

    char* ret{ data + stride * r_size };

    char* end{ data + stride * (l_size + r_size) };

    for (char* iter{ data }; 0 < l_size && 0 < r_size;) {
        char* jter{ iter + l_size };

        for (; jter != end; iter += stride, jter += stride) {
            (MemSwap)(iter, jter, width);
        }

        r_size %= l_size;
        l_size -= r_size;
    }

    return ret;
}

template <typename UnsignedIntegral>
constexpr UnsignedIntegral utils::SimpleUnsignedIntegralHash(
    UnsignedIntegral x, UnsignedIntegral salt) {
    static_assert(integral::IsUnsignedIntegral<UnsignedIntegral>);

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

constexpr unsigned long long utils::SimpleRandomRotate(
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

constexpr unsigned long long utils::GetRandom() {
    static unsigned long long seed{ 0x114514 };

    // unsigned long long time{ __builtin_readcyclecounter() };
    unsigned long long time{ 0 };

    seed ^= time;

    return (SimpleRandomRotate)(&seed);
}

constexpr int utils::Choose2(bool cond0, bool cond1,
                             unsigned long long* random_seed) {
    switch (static_cast<int>(cond1) * 0b10 + static_cast<int>(cond0) * 0b01) {
    case 0b00: return -1;
    case 0b01: return 0;
    case 0b10: return 1;
    case 0b11: return static_cast<int>((SimpleRandomRotate)(random_seed) % 2);
    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }
}

constexpr int utils::Choose3(bool cond0, bool cond1, bool cond2,
                             unsigned long long* random_seed) {
    switch (static_cast<int>(cond2) * 0b100 +  //
            static_cast<int>(cond1) * 0b010 +  //
            static_cast<int>(cond0) * 0b001) {
    case 0b000: return -1;
    case 0b001: return 0;
    case 0b010: return 1;
    case 0b100: return 2;
    case 0b011: return static_cast<int>((SimpleRandomRotate)(random_seed) % 2);
    case 0b101:
        return static_cast<int>((SimpleRandomRotate)(random_seed) % 2) * 2;
    case 0b110:
        return static_cast<int>((SimpleRandomRotate)(random_seed) % 2) + 1;
    case 0b111: return static_cast<int>((SimpleRandomRotate)(random_seed) % 3);
    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }
}

template <typename Value, typename Reason>
template <typename... Args>
constexpr utils::TryResult<Value, Reason>::TryResult(TryResultValueTag,
                                                     Args&&... args)
    : type{ TryResultType::Value }, value{ meta::Forward<Args>(args)... } {}

template <typename Value, typename Reason>
template <typename... Args>
constexpr utils::TryResult<Value, Reason>::TryResult(TryResultReasonTag,
                                                     Args&&... args)
    : type{ TryResultType::Reason }, reason{ meta::Forward<Args>(args)... } {}

template <typename Value, typename Reason>
constexpr utils::TryResultType utils::TryResult<Value, Reason>::GetType(
    this TryResult const& self) {
    return self.type;
}

template <typename Value, typename Reason>
constexpr bool utils::TryResult<Value, Reason>::HasValue(
    this TryResult const& self) {
    return self.type == TryResultType::Value;
}

template <typename Value, typename Reason>
constexpr bool utils::TryResult<Value, Reason>::HasReason(
    this TryResult const& self) {
    return self.type == TryResultType::Reason;
}

template <typename Value, typename Reason>
constexpr void utils::TryResult<Value, Reason>::CheckHasValue(
    this TryResult const& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.HasValue());
}

template <typename Value, typename Reason>
constexpr void utils::TryResult<Value, Reason>::CheckHasReason(
    this TryResult const& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.HasReason());
}

template <typename Value, typename Reason>
constexpr Value const& utils::TryResult<Value, Reason>::GetValue(
    this TryResult const& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.type == TryResultType::Value);
    return self.value;
}

template <typename Value, typename Reason>
constexpr Value& utils::TryResult<Value, Reason>::GetValue(
    this TryResult& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.type == TryResultType::Value);
    return self.value;
}

template <typename Value, typename Reason>
constexpr Reason const& utils::TryResult<Value, Reason>::GetReason(
    this TryResult const& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.type == TryResultType::Reason);
    return self.reason;
}

template <typename Value, typename Reason>
constexpr Reason& utils::TryResult<Value, Reason>::GetReason(
    this TryResult& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.type == TryResultType::Reason);
    return self.reason;
}

template <typename Value, typename Reason>
constexpr void utils::TryResult<Value, Reason>::Discard(this TryResult const&) {
}

}  // namespace zeta::core
