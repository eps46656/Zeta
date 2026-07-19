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

int MemCompare(void const* a, void const* b, size_t size);

void MemSwap(void* x, void* y, size_t size);

void MemCopy(void* dst, void const* src, size_t size);

void MemMove(void* dst, void const* src, size_t size);

void* MemRotate(void* data, size_t l_size, size_t r_size);

int ElemCompare(void const* a, void const* b, size_t elem_size,
                size_t a_elem_stride, size_t b_elem_stride, size_t cnt);

void ElemCopy(void* dst, void const* src, size_t elem_size,
              size_t dst_elem_stride, size_t src_elem_stride, size_t cnt);

void ElemMove(void* dst, void const* src, size_t elem_size,
              size_t dst_elem_stride, size_t src_stride, size_t cnt);

void* ElemRotate(void* data, size_t elem_size, size_t elem_stride,
                 size_t l_size, size_t r_size);

template <typename XIterator, typename YIterator>
int SeqCompare(XIterator x, YIterator y, size_t size);

template <typename DstIterator, typename SrcIterator>
void SeqCopy(DstIterator dst, SrcIterator src, size_t size);

template <typename DstIterator, typename SrcIterator>
void SeqMove(DstIterator dst, SrcIterator src, size_t size);

template <typename Iterator>
Iterator SeqRotate(Iterator beg, Iterator mid, Iterator end);

template <typename UnsignedIntegral>
UnsignedIntegral SimpleUnsignedIntegralHash(UnsignedIntegral x,
                                            UnsignedIntegral salt);

unsigned long long GetRandom();

unsigned long long SimpleRandomRotate(unsigned long long* random_seed);

int Choose2(bool cond0, bool cond1, unsigned long long* random_seed);

int Choose3(bool cond0, bool cond1, bool cond2,
            unsigned long long* random_seed);

int Choose4(bool cond0, bool cond1, bool cond2, bool cond3,
            unsigned long long* random_seed);

template <size_t N>
unsigned _BitInt(8 * N) ReadLittleEndian(void const* data);

template <size_t N>
unsigned _BitInt(8 * N) ReadBigEndian(void const* data);

template <size_t N>
void WriteLittleEndian(void* data, unsigned _BitInt(8 * N) value);

template <size_t N>
void WriteBigEndian(void* data, unsigned _BitInt(8 * N) value);

}  // namespace zeta::core::utils
