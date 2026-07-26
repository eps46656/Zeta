#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/lin_seq_elem_stream.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

size_t lin_seq_elem_stream::Provider::GetElemSize(this Provider const& self) {
    return self.elem_size;
}

bool lin_seq_elem_stream::Provider::IsEnd(this Provider const& self) {
    return self.elem_cnt == 0;
}

size_t lin_seq_elem_stream::Provider::Transfer(this Provider& self, void* dst,
                                               size_t dst_elem_size,
                                               size_t dst_elem_stride,
                                               size_t cnt) {
    size_t transfer_elem_size{ comparison_utils::BasicMin(self.elem_size,
                                                          dst_elem_size) };

    size_t transfer_elem_cnt{ comparison_utils::BasicMin(self.elem_cnt, cnt) };

    utils::ElemCopy(dst, self.data, transfer_elem_size, dst_elem_stride,
                    self.elem_stride, transfer_elem_cnt);

    self.data = static_cast<char const*>(self.data) +
                self.elem_stride * transfer_elem_cnt;

    self.elem_cnt -= transfer_elem_cnt;

    return transfer_elem_cnt;
}

size_t lin_seq_elem_stream::Acceptor::GetElemSize(this Acceptor const& self) {
    return self.elem_size;
}

bool lin_seq_elem_stream::Acceptor::IsEnd(this Acceptor const& self) {
    return self.elem_cnt == 0;
}

size_t lin_seq_elem_stream::Acceptor::Transfer(this Acceptor& self,
                                               void const* src,
                                               size_t src_elem_size,
                                               size_t src_elem_stride,
                                               size_t cnt) {
    size_t transfer_elem_size{ comparison_utils::BasicMin(self.elem_size,
                                                          src_elem_size) };

    size_t transfer_elem_cnt{ comparison_utils::BasicMin(self.elem_cnt, cnt) };

    utils::ElemCopy(self.data, src, transfer_elem_size, self.elem_stride,
                    src_elem_stride, transfer_elem_cnt);

    self.data =
        static_cast<char*>(self.data) + self.elem_stride * transfer_elem_cnt;

    self.elem_cnt -= transfer_elem_cnt;

    return transfer_elem_cnt;
}

}  // namespace zeta::core
