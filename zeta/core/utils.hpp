#pragma once

#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/pair.hpp>

#define ZETA_Core_AreOverlapped(a_beg, a_end, b_beg, b_end) \
    (((a_beg) < (b_end)) && ((b_beg) < (a_end)))

#define ZETA_Core_Unreachable()                    \
    ZETA_Core_WhenEnableDebug(ZETA_Core_Identity({ \
        ZETA_Core_DebugAssert(false);              \
        __builtin_unreachable();                   \
    }))

namespace zeta::core::utils {

template <typename X, typename Y>
void Swap(X&& x, Y&& y);

template <typename T0, typename... Ts>
constexpr decltype(auto) Sum(T0&& x0, Ts&&... xs);

template <typename Node, typename GetLinkFunc>
pair::Pair<Node*, size_t> GetMostLink(Node* n, GetLinkFunc const& get_link);

constexpr int MemCompare(void const* a, void const* b, size_t size);

constexpr void MemSwap(void* x, void* y, size_t size);

constexpr void MemCopy(void* dst, void const* src, size_t size);

constexpr void MemMove(void* dst, void const* src, size_t size);

constexpr void* MemRotate(void* data, size_t l_size, size_t r_size);

constexpr int ElemCompare(void const* a, void const* b, size_t elem_size,
                          size_t a_elem_stride, size_t b_elem_stride,
                          size_t cnt);

constexpr void ElemCopy(void* dst, void const* src, size_t elem_size,
                        size_t dst_elem_stride, size_t src_elem_stride,
                        size_t cnt);

constexpr void ElemMove(void* dst, void const* src, size_t elem_size,
                        size_t dst_elem_stride, size_t src_stride, size_t cnt);

constexpr void* ElemRotate(void* data, size_t elem_size, size_t elem_stride,
                           size_t l_size, size_t r_size);

template <typename XIterator, typename YIterator>
constexpr int SeqCompare(XIterator x, YIterator y, size_t size);

template <typename DstIterator, typename SrcIterator>
constexpr void SeqCopy(DstIterator dst, SrcIterator src, size_t size);

template <typename DstIterator, typename SrcIterator>
constexpr void SeqMove(DstIterator dst, SrcIterator src, size_t size);

template <typename Iterator>
constexpr Iterator SeqRotate(Iterator beg, Iterator mid, Iterator end);

template <typename UnsignedIntegral>
constexpr UnsignedIntegral SimpleUnsignedIntegralHash(UnsignedIntegral x,
                                                      UnsignedIntegral salt);

constexpr unsigned long long GetRandom();

constexpr unsigned long long SimpleRandomRotate(
    unsigned long long* random_seed);

constexpr int Choose2(bool cond0, bool cond1, unsigned long long* random_seed);

constexpr int Choose3(bool cond0, bool cond1, bool cond2,
                      unsigned long long* random_seed);

constexpr int Choose4(bool cond0, bool cond1, bool cond2, bool cond3,
                      unsigned long long* random_seed);

}  // namespace zeta::core::utils
