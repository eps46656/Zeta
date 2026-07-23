#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/lin_seq_elem_stream.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

size_t lin_seq_elem_stream::Provider::GetElemSize(this Provider const& self) {
    return self.elem_size;
}

void lin_seq_elem_stream::Provider::Transfer(this Provider& self, void* dst,
                                             size_t dst_elem_size,
                                             size_t dst_elem_stride,
                                             size_t cnt) {
    ZETA_Core_DebugAssert(cnt <= self.elem_cnt);

    size_t transfer_elem_size{ comparison_utils::BasicMin(self.elem_size,
                                                          dst_elem_size) };

    utils::ElemCopy(dst, self.data, transfer_elem_size, dst_elem_stride,
                    self.elem_stride, cnt);

    self.data = static_cast<char const*>(self.data) + self.elem_stride * cnt;
    self.elem_cnt -= cnt;
}

size_t lin_seq_elem_stream::Acceptor::GetElemSize(this Acceptor const& self) {
    return self.elem_size;
}

void lin_seq_elem_stream::Acceptor::Transfer(this Acceptor& self,
                                             void const* src,
                                             size_t src_elem_size,
                                             size_t src_elem_stride,
                                             size_t cnt) {
    ZETA_Core_DebugAssert(cnt <= self.elem_cnt);

    size_t transfer_elem_size{ comparison_utils::BasicMin(self.elem_size,
                                                          src_elem_size) };

    utils::ElemCopy(self.data, src, transfer_elem_size, self.elem_stride,
                    src_elem_stride, cnt);

    self.data = static_cast<char*>(self.data) + self.elem_stride * cnt;
    self.elem_cnt -= cnt;
}

}  // namespace zeta::core
