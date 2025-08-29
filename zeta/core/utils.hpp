#pragma once

#include <zeta/core/define.h>

#include <zeta/core/integer.hpp>

#define ZETA_Core_AreOverlapped(a_beg, a_end, b_beg, b_end) \
    (!((a_end) <= (b_beg) || (b_end) <= (a_beg)))

namespace zeta::core {

struct Monostate {};

// -----------------------------------------------------------------------------

template <typename T>
T Declval();

// -----------------------------------------------------------------------------

template <typename T>
struct identity {
    using type = T;
};

template <typename T>
using identity_t = typename identity<T>::type;

// -----------------------------------------------------------------------------

template <typename T1, typename T2>
struct is_same_ {
    static constexpr bool value{ false };
};

template <typename T>
struct is_same_<T, T> {
    static constexpr bool value{ true };
};

template <typename T1, typename T2>
constexpr bool is_same{ is_same_<T1, T2>::value };

// -----------------------------------------------------------------------------

template <typename T>
struct is_const_ {
    static constexpr bool value{ false };
};

template <typename T>
struct is_const_<const T> {
    static constexpr bool value{ true };
};

template <typename T>
constexpr bool is_const{ is_const_<T>::value };

// -----------------------------------------------------------------------------

template <typename T>
struct is_pointer_ {
    static constexpr bool value{ false };
};

template <typename T>
struct is_pointer_<T*> {
    static constexpr bool value{ true };
};

template <typename T>
constexpr bool is_pointer{ is_pointer_<T>::value };

// -----------------------------------------------------------------------------

template <bool Cond, typename T1, typename T2>
struct conditional;

template <typename T1, typename T2>
struct conditional<true, T1, T2> {
    using type = T1;
};

template <typename T1, typename T2>
struct conditional<false, T1, T2> {
    using type = T2;
};

template <bool Cond, typename T1, typename T2>
using conditional_t = typename conditional<Cond, T1, T2>::type;

// -----------------------------------------------------------------------------

template <typename T>
struct remove_const {
    using type = T;
};

template <typename T>
struct remove_const<const T> {
    using type = T;
};

template <typename T>
using remove_const_t = typename remove_const<T>::type;

// -----------------------------------------------------------------------------

template <typename T>
struct remove_reference {
    using type = T;
};

template <typename T>
struct remove_reference<T&> {
    using type = T;
};

template <typename T>
struct remove_reference<T&&> {
    using type = T;
};

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

// -----------------------------------------------------------------------------

template <typename T>
using remove_const_reference_t = remove_const_t<remove_reference_t<T>>;

// -----------------------------------------------------------------------------

template <typename Base, typename Derived>
struct is_base_of_ {
    static constexpr bool test(Base const*) { return true; }
    static constexpr bool test(...) { return false; }

    static constexpr bool value{ test(static_cast<Derived const*>(nullptr)) };
};

template <typename Base, typename Derived>
constexpr bool is_base_of{ is_base_of_<Base, Derived>::value };

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <typename T>
struct StaticValueWrapperBase;

template <typename T>
struct DynamicValueWrapperBase;

template <typename T, T Value>
struct StaticValueWrapper;

template <typename T>
struct DynamicValueWrapper;

// -----------------------------------------------------------------------------

template <typename First, typename Second>
struct Pair;

template <typename First, typename Second, typename Third>
struct Triple;

// -----------------------------------------------------------------------------

template <typename AlignWrapper, typename WidthWrapper, typename StrideWrapper>
struct TypeTrait;

// -----------------------------------------------------------------------------

template <typename SizeValueWrapper>
struct MemBlock;

template <typename WidthWrapper, typename StrideWrapper>
struct MemIterator;

// -----------------------------------------------------------------------------

template <typename T, T Value>
decltype(auto) MergeValueWrapper(StaticValueWrapper<T, Value> const& x,
                                 StaticValueWrapper<T, Value> const& y);

template <typename T, T Value>
decltype(auto) MergeValueWrapper(StaticValueWrapper<T, Value> const& x,
                                 DynamicValueWrapper<T> const& y);

template <typename T, T Value>
decltype(auto) MergeValueWrapper(DynamicValueWrapper<T> const& x,
                                 StaticValueWrapper<T, Value> const& y);

template <typename T, T Value>
decltype(auto) MergeValueWrapper(DynamicValueWrapper<T> const& x,
                                 DynamicValueWrapper<T> const& y);

template <typename ValueWrapperX, typename ValueWrapperY,
          typename ValueWrapperZ, typename... ValueWrappers>
decltype(auto) MergeValueWrapper(ValueWrapperX const& x, ValueWrapperY const& y,
                                 ValueWrapperZ const& z,
                                 ValueWrappers const&... value_wrappers);

// -----------------------------------------------------------------------------

template <typename WidthWrapper, typename StrideWrapper>
int ThreeWayCompare(const MemIterator<WidthWrapper, StrideWrapper>& x,
                    const MemIterator<WidthWrapper, StrideWrapper>& y);

// -----------------------------------------------------------------------------

template <typename T>
remove_reference_t<T>&& Move(T&& t);

template <typename T>
constexpr T&& Forward(remove_reference_t<T>& t);

template <typename X, typename Y>
void Swap(X&& x, Y&& y);

// -----------------------------------------------------------------------------

template <typename X, typename Y, typename... Args>
decltype(auto) Min(X&& x, Y&& y, Args&&... args);

template <typename X, typename Y, typename... Args>
decltype(auto) Max(X&& x, Y&& y, Args&&... args);

// -----------------------------------------------------------------------------

template <typename X, typename Y>
int ThreeWayCompare(const X& x, const Y& y);

template <typename Node, typename GetLinkFunc>
Pair<Node*, size_t> GetMostLink(Node* n, const GetLinkFunc get_link);

// -----------------------------------------------------------------------------

int MemCompare(void const* a, void const* b, size_t size);

void MemCopy(void* dst, void const* src, size_t size);

void MemMove(void* dst, void const* src, size_t size);

void* MemRotate(void* beg, void* mid, void* end);

// -----------------------------------------------------------------------------

void ElemCopy(void* dst, void const* src, size_t width, size_t dst_stride,
              size_t src_stride, size_t cnt);

void ElemMove(void* dst, void const* src, size_t width, size_t dst_stride,
              size_t src_stride, size_t cnt);

// -----------------------------------------------------------------------------

template <typename XIterator, typename YIterator>
int SeqCompare(XIterator x, YIterator y, size_t size);

template <typename DstIterator, typename SrcIterator>
void SeqCopy(DstIterator dst, SrcIterator src, size_t size);

template <typename DstIterator, typename SrcIterator>
void SeqMove(DstIterator dst, SrcIterator src, size_t size);

template <typename Iterator>
Iterator SeqRotate(Iterator beg, Iterator mid, Iterator end);

// -----------------------------------------------------------------------------

constexpr int FloorLog2(unsigned long long x);

constexpr int CeilLog2(unsigned long long x);

// -----------------------------------------------------------------------------

unsigned long long ULLHash(unsigned long long x, unsigned long long salt);
unsigned long long SLLHash(long long x, unsigned long long salt);

unsigned long long GetRandom();

unsigned long long SimpleRandomRotate(unsigned long long* x);

// -----------------------------------------------------------------------------

int Choose2(bool cond0, bool cond1, unsigned long long* random_seed);

int Choose3(bool cond0, bool cond1, bool cond2,
            unsigned long long* random_seed);

// -----------------------------------------------------------------------------

template <typename T>
struct StaticValueWrapperBase;

template <typename T>
struct DynamicValueWrapperBase;

template <typename T, T Value>
struct StaticValueWrapper : public StaticValueWrapperBase<T> {
    constexpr T& operator()() const;
};

template <typename T>
struct DynamicValueWrapper : public DynamicValueWrapperBase<T> {
    const T& value;

    const T& operator()() const;
};

// -----------------------------------------------------------------------------

template <typename First, typename Second>
struct Pair {
    First first;
    Second second;
};

template <typename First, typename Second, typename Third>
struct Triple {
    First first;
    Second second;
    Third third;
};

// -----------------------------------------------------------------------------

template <typename AlignWrapper, typename WidthWrapper, typename StrideWrapper>
struct TypeTrait {
    ZETA_Core_StaticAssert(
        (is_base_of<StaticValueWrapperBase, AlignWrapper> ||
         is_base_of<DynamicValueWrapperBase, AlignWrapper>) &&
        (is_base_of<StaticValueWrapperBase, WidthWrapper> ||
         is_base_of<DynamicValueWrapperBase, WidthWrapper>) &&
        (is_base_of<StaticValueWrapperBase, StrideWrapper> ||
         is_base_of<DynamicValueWrapperBase, StrideWrapper>));

    AlignWrapper align;
    WidthWrapper width;
    StrideWrapper stride;
};

template <typename T>
using type_trait_t = TypeTrait<StaticValueWrapper<size_t, alignof(T)>,
                               StaticValueWrapper<size_t, sizeof(T)>,
                               StaticValueWrapper<size_t, sizeof(T)>>;

// -----------------------------------------------------------------------------

template <typename SizeValueWrapper>
struct MemBlock {
    void* ptr;
    SizeValueWrapper size;
};

// -----------------------------------------------------------------------------

template <typename WidthWrapper, typename StrideWrapper>
struct MemIterator {
    ZETA_Core_StaticAssert(
        (is_base_of<StaticValueWrapperBase, WidthWrapper> ||
         is_base_of<DynamicValueWrapperBase, WidthWrapper>) &&
        (is_base_of<StaticValueWrapperBase, StrideWrapper> ||
         is_base_of<DynamicValueWrapperBase, StrideWrapper>));

    void* ptr;
    WidthWrapper width;
    StrideWrapper stride;

    MemIterator& operator++();
    MemIterator& operator--();

    MemIterator operator++(int);
    MemIterator operator--(int);
};

// -----------------------------------------------------------------------------

template <typename T, T Value>
constexpr T& StaticValueWrapper<T, Value>::operator()() const {
    return Value;
}

template <typename T>
const T& DynamicValueWrapper<T>::operator()() const {
    return this->value;
}

// -----------------------------------------------------------------------------

template <typename T, T Value>
decltype(auto) MergeValueWrapper(StaticValueWrapper<T, Value> const& x,
                                 StaticValueWrapper<T, Value> const& y) {
    ZETA_Core_StaticAssert(x.value == y.value);
    return x;
}

template <typename T, T Value>
decltype(auto) MergeValueWrapper(StaticValueWrapper<T, Value> const& x,
                                 DynamicValueWrapper<T> const& y) {
    ZETA_Core_DebugAssert(x.value == y.value);
    return x;
}

template <typename T, T Value>
decltype(auto) MergeValueWrapper(DynamicValueWrapper<T> const& x,
                                 StaticValueWrapper<T, Value> const& y) {
    ZETA_Core_DebugAssert(x.value == y.value);
    return y;
}

template <typename T, T Value>
decltype(auto) MergeValueWrapper(DynamicValueWrapper<T> const& x,
                                 DynamicValueWrapper<T> const& y) {
    ZETA_Core_DebugAssert(x.value == y.value);
    return x;
}

template <typename ValueWrapperX, typename ValueWrapperY,
          typename ValueWrapperZ, typename... ValueWrappers>
decltype(auto) MergeValueWrapper(ValueWrapperX const& x, ValueWrapperY const& y,
                                 ValueWrapperZ const& z,
                                 ValueWrappers const&... value_wrappers) {
    return MergeValueWrapper(MergeValueWrapper(x, y),
                             MergeValueWrapper(z, value_wrappers...));
}

// -----------------------------------------------------------------------------

template <typename WidthWrapper, typename StrideWrapper>
MemIterator<WidthWrapper, StrideWrapper>&
MemIterator<WidthWrapper, StrideWrapper>::operator++() {
    this->ptr = static_cast<char*>(this->ptr) + stride();
    return *this;
}

template <typename WidthWrapper, typename StrideWrapper>
MemIterator<WidthWrapper, StrideWrapper>&
MemIterator<WidthWrapper, StrideWrapper>::operator--() {
    this->ptr = static_cast<char*>(this->ptr) - stride();
    return *this;
}

template <typename WidthWrapper, typename StrideWrapper>
MemIterator<WidthWrapper, StrideWrapper>
MemIterator<WidthWrapper, StrideWrapper>::operator++(int) {
    MemIterator ret{ *this };
    ++(*this);
    return ret;
}

template <typename WidthWrapper, typename StrideWrapper>
MemIterator<WidthWrapper, StrideWrapper>
MemIterator<WidthWrapper, StrideWrapper>::operator--(int) {
    MemIterator ret{ *this };
    --(*this);
    return ret;
}

template <typename WidthWrapper, typename StrideWrapper>
int ThreeWayCompare(const MemIterator<WidthWrapper, StrideWrapper>& x,
                    const MemIterator<WidthWrapper, StrideWrapper>& y) {
    auto width{ MergeValueWrapper(x.width, y.width).value };
    auto stride{ MergeValueWrapper(x.stride, y.stride).value };

    ptrdiff_t diff{ static_cast<char*>(x.ptr) - static_cast<char*>(y.ptr) };

    ZETA_Core_DebugAssert(diff % stride == 0);

    return ThreeWayCompare(diff, 0);
}

// -----------------------------------------------------------------------------

template <typename T>
remove_reference_t<T>&& Move(T&& t) {
    return static_cast<remove_reference_t<T>&&>(t);
}

template <typename T>
constexpr T&& Forward(remove_reference_t<T>& t) {
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

template <typename X, typename Y, typename... Args>
decltype(auto) Min(X&& x, Y&& y, Args&&... args) {
    if constexpr (sizeof...(args) == 0) {
        return y < x ? Forward<Y>(y) : Forward<X>(x);
    } else {
        return Min(Min(Forward<X>(x), Forward<Y>(y)), Forward<Args>(args)...);
    }
}

template <typename X, typename Y, typename... Args>
decltype(auto) Max(X&& x, Y&& y, Args&&... args) {
    if constexpr (sizeof...(args) == 0) {
        return x < y ? Forward<Y>(y) : Forward<X>(x);
    } else {
        return Max(Max(Forward<X>(x), Forward<Y>(y)), Forward<Args>(args)...);
    }
}

// -----------------------------------------------------------------------------

template <typename X, typename Y>
int ThreeWayCompare(const X& x, const Y& y) {
    return (y < x) - (x < y);
}

template <typename Node, typename GetLinkFunc>
Pair<Node*, size_t> GetMostLink(Node* n, const GetLinkFunc get_link) {
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

    return SeqCompare(static_cast<const char*>(a), static_cast<const char*>(b),
                      size);
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

// -----------------------------------------------------------------------------

inline void ElemCopy(void* dst_, void const* src_, size_t width,
                     size_t dst_stride, size_t src_stride, size_t cnt) {
    auto dst{ static_cast<char*>(dst_) };
    auto src{ static_cast<char const*>(src_) };

    if (dst == src || cnt == 0) { return; }

    ZETA_Core_DebugAssert(dst != nullptr);
    ZETA_Core_DebugAssert(src != nullptr);
    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= dst_stride);
    ZETA_Core_DebugAssert(width <= src_stride);

    for (size_t i{ 0 }; i < cnt; ++i, dst += dst_stride, src += src_stride) {
        MemCopy(dst, src, width);
    }
}

inline void ElemMove(void* dst_, void const* src_, size_t width,
                     size_t dst_stride, size_t src_stride, size_t size) {
    auto dst{ static_cast<char*>(dst_) };
    auto src{ static_cast<char const*>(src_) };

    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(0 < dst_stride);
    ZETA_Core_DebugAssert(0 < src_stride);

    if (size == 0) { return; }

    width = Min(width, dst_stride, src_stride);

    if (width == dst_stride && width == src_stride) {
        MemMove(dst, src, width * size);
        return;
    }

    ZETA_Core_DebugAssert(dst != nullptr);
    ZETA_Core_DebugAssert(src != nullptr);

    void* dst_end{ dst + dst_stride * (size - 1) + width };
    void const* src_end{ src + src_stride * (size - 1) + width };

    if (dst_end <= src || src_end <= dst) {
        ElemCopy(dst, src, width, dst_stride, src_stride, size);
        return;
    }

    if (dst == src && dst_stride == src_stride) { return; }

    if (dst <= src && dst_end <= src_end) {
        for (; 0 < size--; dst += dst_stride, src += src_stride) {
            MemCopy(dst, src, width);
        }

        return;
    }

    if (src <= dst && src_end <= dst_end) {
        dst += dst_stride * size;
        src += src_stride * size;

        while (0 < size--) {
            MemCopy(dst -= dst_stride, src -= src_stride, width);
        }

        return;
    }

    size_t buffer_capacity{ FloorLog2(size) + 4 };

    auto begs{ static_cast<size_t*>(__builtin_alloca_with_align(
        sizeof(size_t) * buffer_capacity, __CHAR_BIT__ * alignof(size_t))) };

    auto cnts{ static_cast<size_t*>(__builtin_alloca_with_align(
        sizeof(size_t) * buffer_capacity, __CHAR_BIT__ * alignof(size_t))) };

    size_t buffer_i{ 0 };

    begs[buffer_i] = 0;
    cnts[buffer_i] = size;
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

            begs[buffer_i] = cur_beg + cur_l_cnt;
            cnts[buffer_i] = cur_r_cnt;
            ++buffer_i;

            ZETA_Core_DebugAssert(buffer_i < buffer_capacity);

            begs[buffer_i] = cur_beg;
            cnts[buffer_i] = cur_l_cnt;
            ++buffer_i;
        } else {
            ZETA_Core_DebugAssert(buffer_i < buffer_capacity);

            begs[buffer_i] = cur_beg;
            cnts[buffer_i] = cur_l_cnt;
            ++buffer_i;

            ZETA_Core_DebugAssert(buffer_i < buffer_capacity);

            begs[buffer_i] = cur_beg + cur_l_cnt;
            cnts[buffer_i] = cur_r_cnt;
            ++buffer_i;
        }
    }
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
    //
}

// -----------------------------------------------------------------------------

inline constexpr int FloorLog2(unsigned long long x) {
    ZETA_Core_DebugAssert(0 < x);
    return ZETA_Core_ullong_width - 1 - __builtin_clzll(x);
}

inline constexpr int CeilLog2(unsigned long long x) {
    return x <= 1 ? 0 : ZETA_Core_ullong_width - __builtin_clzll(x - 1);
}

}  // namespace zeta::core
