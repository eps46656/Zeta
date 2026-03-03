#pragma once

#include <zeta/core_test/ptr_iter.hpp>

namespace zeta::core_test {

template <typename Value>
Value* PtrIter<Value>::operator->() const {
    return static_cast<Value*>(this->ptr);
}

template <typename Value>
Value& PtrIter<Value>::operator[](ptrdiff_t idx) const {
    return *static_cast<Value*>(this->ptr + this->stride * idx);
}

template <typename ValueA, typename ValueB>
bool operator==(PtrIter<ValueA> const& ptr_iter_a,
                PtrIter<ValueB> const& ptr_iter_b) {
    return ptr_iter_a.ptr == ptr_iter_b.ptr;
}

template <typename ValueA, typename ValueB>
bool operator!=(PtrIter<ValueA> const& ptr_iter_a,
                PtrIter<ValueB> const& ptr_iter_b) {
    return ptr_iter_a.ptr != ptr_iter_b.ptr;
}

template <typename ValueA, typename ValueB>
bool operator<(PtrIter<ValueA> const& ptr_iter_a,
               PtrIter<ValueB> const& ptr_iter_b) {
    return ptr_iter_a.ptr <= ptr_iter_b.ptr;
}

template <typename ValueA, typename ValueB>
bool operator>(PtrIter<ValueA> const& ptr_iter_a,
               PtrIter<ValueB> const& ptr_iter_b) {
    return ptr_iter_a.ptr >= ptr_iter_b.ptr;
}

template <typename ValueA, typename ValueB>
bool operator<=(PtrIter<ValueA> const& ptr_iter_a,
                PtrIter<ValueB> const& ptr_iter_b) {
    return ptr_iter_a.ptr <= ptr_iter_b.ptr;
}

template <typename ValueA, typename ValueB>
bool operator>=(PtrIter<ValueA> const& ptr_iter_a,
                PtrIter<ValueB> const& ptr_iter_b) {
    return ptr_iter_a.ptr >= ptr_iter_b.ptr;
}

template <typename Value>
PtrIter<Value>& operator++(PtrIter<Value>& ptr_iter) {
    ptr_iter.ptr =
        static_cast<core::meta::Conditional<core::meta::IsConst<Value>,
                                            char const*, char*>>(ptr_iter.ptr) +
        ptr_iter.stride;

    return ptr_iter;
}

template <typename Value>
PtrIter<Value>& operator--(PtrIter<Value>& ptr_iter) {
    ptr_iter.ptr =
        static_cast<core::meta::Conditional<core::meta::IsConst<Value>,
                                            char const*, char*>>(ptr_iter.ptr) -
        ptr_iter.stride;

    return ptr_iter;
}

template <typename Value>
PtrIter<Value> operator++(PtrIter<Value>& ptr_iter, int) {
    PtrIter ret{ ptr_iter };
    ++ptr_iter;
    return ret;
}

template <typename Value>
PtrIter<Value> operator--(PtrIter<Value>& ptr_iter, int) {
    PtrIter ret{ ptr_iter };
    --ptr_iter;
    return ret;
}

template <typename Value>
PtrIter<Value> operator+(PtrIter<Value> const& ptr_iter, ptrdiff_t step) {
    return { static_cast<core::meta::Conditional<core::meta::IsConst<Value>,
                                                 char const*, char*>>(
                 ptr_iter.ptr) +
                 ptr_iter.stride * step,
             ptr_iter.stride };
}

template <typename Value>
PtrIter<Value> operator-(PtrIter<Value> const& ptr_iter, ptrdiff_t step) {
    return { static_cast<core::meta::Conditional<core::meta::IsConst<Value>,
                                                 char const*, char*>>(
                 ptr_iter.ptr) -
                 ptr_iter.stride * step,
             ptr_iter.stride };
}

template <typename Value>
PtrIter<Value>& operator+=(PtrIter<Value> const& ptr_iter, ptrdiff_t step) {
    ptr_iter.ptr =
        static_cast<core::meta::Conditional<core::meta::IsConst<Value>,
                                            char const*, char*>>(ptr_iter.ptr) +
        ptr_iter.stride * step;

    return ptr_iter;
}

template <typename Value>
PtrIter<Value>& operator-=(PtrIter<Value> const& ptr_iter, ptrdiff_t step) {
    ptr_iter.ptr =
        static_cast<core::meta::Conditional<core::meta::IsConst<Value>,
                                            char const*, char*>>(ptr_iter.ptr) -
        ptr_iter.stride * step;

    return ptr_iter;
}

template <typename ValueA, typename ValueB>
ptrdiff_t operator-(PtrIter<ValueA> const& ptr_iter_a,
                    PtrIter<ValueB> const& ptr_iter_b) {
    ZETA_Core_DebugAssert(ptr_iter_a.stride == ptr_iter_b.stride);

    ptrdiff_t diff{
        static_cast<core::meta::Conditional<core::meta::IsConst<ValueB>,
                                            char const*, char*>>(
            ptr_iter_b.ptr) -
        static_cast<core::meta::Conditional<core::meta::IsConst<ValueA>,
                                            char const*, char*>>(ptr_iter_a.ptr)
    };

    ZETA_Core_DebugAssert(diff % ptr_iter_a.stride == 0);

    return diff / ptr_iter_a.stride;
}

template <typename Value>
Value& operator*(PtrIter<Value> const& ptr_iter) {
    return *static_cast<Value*>(ptr_iter.ptr);
}

}  // namespace zeta::core_test
