#pragma once

#include <stddef.h>

#include <iterator>
#include <zeta/core/type_traits.hpp>

namespace zeta::core_test {

template <typename Value>
struct PtrIter {
    using difference_type = ptrdiff_t;
    using value_type = Value;
    using pointer_type = Value*;
    using reference = Value&;
    using iterator_category = std::random_access_iterator_tag;

    core::Conditional<core::IsConst<Value>, void const*, void*> ptr;
    ptrdiff_t stride;

    Value* operator->() const;

    Value& operator[](ptrdiff_t idx) const;
};

template <typename ValueA, typename ValueB>
bool operator==(PtrIter<ValueA> const& ptr_iter_a,
                PtrIter<ValueB> const& ptr_iter_b);

template <typename ValueA, typename ValueB>
bool operator!=(PtrIter<ValueA> const& ptr_iter_a,
                PtrIter<ValueB> const& ptr_iter_b);

template <typename ValueA, typename ValueB>
bool operator<(PtrIter<ValueA> const& ptr_iter_a,
               PtrIter<ValueB> const& ptr_iter_b);

template <typename ValueA, typename ValueB>
bool operator>(PtrIter<ValueA> const& ptr_iter_a,
               PtrIter<ValueB> const& ptr_iter_b);

template <typename ValueA, typename ValueB>
bool operator<=(PtrIter<ValueA> const& ptr_iter_a,
                PtrIter<ValueB> const& ptr_iter_b);

template <typename ValueA, typename ValueB>
bool operator>=(PtrIter<ValueA> const& ptr_iter_a,
                PtrIter<ValueB> const& ptr_iter_b);

template <typename Value>
PtrIter<Value>& operator++(PtrIter<Value>& ptr_iter);

template <typename Value>
PtrIter<Value>& operator--(PtrIter<Value>& ptr_iter);

template <typename Value>
PtrIter<Value> operator++(PtrIter<Value>& ptr_iter, int);

template <typename Value>
PtrIter<Value> operator--(PtrIter<Value>& ptr_iter, int);

template <typename Value>
PtrIter<Value> operator+(PtrIter<Value> const& ptr_iter, ptrdiff_t step);

template <typename Value>
PtrIter<Value> operator-(PtrIter<Value> const& ptr_iter, ptrdiff_t step);

template <typename Value>
PtrIter<Value>& operator+=(PtrIter<Value> const& ptr_iter, ptrdiff_t step);

template <typename Value>
PtrIter<Value>& operator-=(PtrIter<Value> const& ptr_iter, ptrdiff_t step);

template <typename ValueA, typename ValueB>
ptrdiff_t operator-(PtrIter<ValueA> const& ptr_iter_a,
                    PtrIter<ValueB> const& ptr_iter_b);

template <typename Value>
Value& operator*(PtrIter<Value> const& ptr_iter);

}  // namespace zeta::core_test
