#pragma once

#include <zeta/core/compare.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/hash.hpp>

#define ZETA_Core_AreOverlapped(a_beg, a_end, b_beg, b_end) \
    (((a_beg) < (b_end)) && ((b_beg) < (a_end)))

namespace zeta::core::utils {

template <typename T, size_t N>
struct Array {
    T elems[N];
};

}  // namespace zeta::core::utils

namespace zeta::core::utils {

template <typename First, typename Second>
struct Pair {
    First first;
    Second second;
};

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool operator==(Pair<XFirst, XSecond> const& x, Pair<YFirst, YSecond> const& y);

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool operator!=(Pair<XFirst, XSecond> const& x, Pair<YFirst, YSecond> const& y);

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool operator<(Pair<XFirst, XSecond> const& x, Pair<YFirst, YSecond> const& y);

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool operator<=(Pair<XFirst, XSecond> const& x, Pair<YFirst, YSecond> const& y);

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool operator>(Pair<XFirst, XSecond> const& x, Pair<YFirst, YSecond> const& y);

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
bool operator>=(Pair<XFirst, XSecond> const& x, Pair<YFirst, YSecond> const& y);

}  // namespace zeta::core::utils

namespace zeta::core {

template <typename First, typename Second>
struct hash::ops::BasicHashImpl<utils::Pair<First, Second>> {
    unsigned long long operator()(utils::Pair<First, Second> const& x,
                                  unsigned long long salt) const;
};

template <typename XFirst, typename XSecond, typename YFirst, typename YSecond>
struct compare::BasicCompareImpl<utils::Pair<XFirst, XSecond>,
                                 utils::Pair<YFirst, YSecond>> {
    int operator()(utils::Pair<XFirst, XSecond> const& x,
                   utils::Pair<YFirst, YSecond> const& y) const;
};

}  // namespace zeta::core

namespace zeta::core::utils {

template <typename First, typename Second, typename Third>
struct Triplet {
    First first;
    Second second;
    Third third;
};

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool operator==(Triplet<XFirst, XSecond, XThird> const& x,
                Triplet<YFirst, YSecond, YThird> const& y);

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool operator!=(Triplet<XFirst, XSecond, XThird> const& x,
                Triplet<YFirst, YSecond, YThird> const& y);

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool operator<(Triplet<XFirst, XSecond, XThird> const& x,
               Triplet<YFirst, YSecond, YThird> const& y);

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool operator<=(Triplet<XFirst, XSecond, XThird> const& x,
                Triplet<YFirst, YSecond, YThird> const& y);

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool operator>(Triplet<XFirst, XSecond, XThird> const& x,
               Triplet<YFirst, YSecond, YThird> const& y);

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
bool operator>=(Triplet<XFirst, XSecond, XThird> const& x,
                Triplet<YFirst, YSecond, YThird> const& y);

}  // namespace zeta::core::utils

namespace zeta::core {

template <typename First, typename Second, typename Third>
struct hash::ops::BasicHashImpl<utils::Triplet<First, Second, Third>> {
    unsigned long long operator()(utils::Triplet<First, Second, Third> const& x,
                                  unsigned long long salt) const;
};

template <typename XFirst, typename XSecond, typename XThird, typename YFirst,
          typename YSecond, typename YThird>
struct compare::BasicCompareImpl<utils::Triplet<XFirst, XSecond, XThird>,
                                 utils::Triplet<YFirst, YSecond, YThird>> {
    int operator()(utils::Triplet<XFirst, XSecond, XThird> const& x,
                   utils::Triplet<YFirst, YSecond, YThird> const& y) const;
};

}  // namespace zeta::core

namespace zeta::core::utils {

template <typename X, typename Y>
void Swap(X&& x, Y&& y);

template <typename Operation, typename T0, typename... Ts>
constexpr decltype(auto) LeftReduce(Operation const& opr, T0&& x0, Ts&&... xs);

template <typename Operation, typename T0, typename... Ts>
constexpr decltype(auto) RightReduce(Operation const& opr, T0&& x0, Ts&&... xs);

template <typename Operation, typename T0, typename... Ts>
constexpr decltype(auto) TreeReduce(Operation const& opr, T0&& x0, Ts&&... xs);

template <typename T0, typename... Ts>
constexpr decltype(auto) Min(T0&& x0, Ts&&... xs);

template <typename T0, typename... Ts>
constexpr decltype(auto) Max(T0&& x0, Ts&&... xs);

template <typename T0, typename... Ts>
constexpr decltype(auto) Sum(T0&& x0, Ts&&... xs);

template <typename Node, typename GetLinkFunc>
Pair<Node*, size_t> GetMostLink(Node* n, GetLinkFunc const& get_link);

using HashFunctionRef = FunctionRef<unsigned long long(void const*)>;

using CompareFunctionRef = FunctionRef<int(void const*, void const*)>;

int MemCompare(void const* a, void const* b, size_t size);

void MemSwap(void* x, void* y, size_t size);

void MemCopy(void* dst, void const* src, size_t size);

void MemMove(void* dst, void const* src, size_t size);

void* MemRotate(void* data, size_t l_size, size_t r_size);

int ElemCompare(void const* a, void const* b, size_t width, size_t a_stride,
                size_t b_stride, size_t cnt);

void ElemCopy(void* dst, void const* src, size_t width, size_t dst_stride,
              size_t src_stride, size_t cnt);

void ElemMove(void* dst, void const* src, size_t width, size_t dst_stride,
              size_t src_stride, size_t cnt);

void* ElemRotate(void* data, size_t width, size_t stride, size_t l_size,
                 size_t r_size);

template <typename XIterator, typename YIterator>
int SeqCompare(XIterator x, YIterator y, size_t size);

template <typename DstIterator, typename SrcIterator>
void SeqCopy(DstIterator dst, SrcIterator src, size_t size);

template <typename DstIterator, typename SrcIterator>
void SeqMove(DstIterator dst, SrcIterator src, size_t size);

template <typename Iterator>
Iterator SeqRotate(Iterator beg, Iterator mid, Iterator end);

template <typename UnsignedIntegral>
constexpr UnsignedIntegral UIntCeilDiv(UnsignedIntegral x, UnsignedIntegral y);

template <typename UnsignedIntegral>
constexpr UnsignedIntegral UIntAlignDown(UnsignedIntegral val,
                                         UnsignedIntegral align);

template <typename UnsignedIntegral>
constexpr UnsignedIntegral UIntAlignUp(UnsignedIntegral val,
                                       UnsignedIntegral align);

constexpr unsigned long long Power(unsigned long long base, unsigned exp);

constexpr unsigned FloorLog2(unsigned long long x);

constexpr unsigned CeilLog2(unsigned long long x);

constexpr unsigned FloorLog(unsigned long long x, unsigned base);

constexpr unsigned CeilLog(unsigned long long x, unsigned base);

constexpr int FindPrevOne(unsigned long long val, int pos);

constexpr int FindNextOne(unsigned long long val, int pos);

unsigned long long ULLHash(unsigned long long x, unsigned long long salt);
unsigned long long SLLHash(long long x, unsigned long long salt);

unsigned long long GetRandom();

unsigned long long SimpleRandomRotate(unsigned long long* random_seed);

int Choose2(bool cond0, bool cond1, unsigned long long* random_seed);

int Choose3(bool cond0, bool cond1, bool cond2,
            unsigned long long* random_seed);

unsigned long long GCD(unsigned long long x, unsigned long long y);

unsigned long long LCM(unsigned long long x, unsigned long long y);

template <typename T>
T& GetInstRef(T* inst);

template <typename T>
T& GetInstRef(T& inst);

template <typename T>
T* GetInstPtr(T* inst);

template <typename T>
T* GetInstPtr(T& inst);

template <typename T>
T* GetInstPtr(T&& inst) = delete;

}  // namespace zeta::core::utils
