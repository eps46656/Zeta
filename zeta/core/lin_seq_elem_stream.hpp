#pragma once

#include <zeta/core/elem_stream.hpp>

namespace zeta::core::lin_seq_elem_stream {

struct Provider {
    void const* data;
    size_t elem_size;
    size_t elem_stride;
    size_t elem_cnt;

    size_t GetElemSize(this Provider const& self);

    void Transfer(this Provider& self, void* dst, size_t dst_elem_size,
                  size_t dst_elem_stride, size_t cnt);
};

struct Acceptor {
    void* data;
    size_t elem_size;
    size_t elem_stride;
    size_t elem_cnt;

    size_t GetElemSize(this Acceptor const& self);

    void Transfer(this Acceptor& self, void const* src, size_t src_elem_size,
                  size_t src_elem_stride, size_t cnt);
};

}  // namespace zeta::core::lin_seq_elem_stream
