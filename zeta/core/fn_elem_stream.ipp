#pragma once

#include <zeta/core/fn_elem_stream.hpp>

namespace zeta::core {

constexpr size_t fn_elem_stream::Acceptor::GetElemSize() const {
    return this->elem_size;
}

constexpr void fn_elem_stream::Acceptor::Transfer(void const* src,
                                                  size_t src_stride,
                                                  size_t cnt) {
    this->transfer(src, src_stride, cnt);
}

constexpr size_t fn_elem_stream::Provider::GetElemSize() const {
    return this->elem_size;
}

constexpr void fn_elem_stream::Provider::Transfer(void* dst, size_t dst_stride,
                                                  size_t cnt) {
    this->transfer(dst, dst_stride, cnt);
}

}  // namespace zeta::core
