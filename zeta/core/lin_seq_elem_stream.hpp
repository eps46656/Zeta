#pragma once

#include <zeta/core/elem_stream.hpp>

namespace zeta::core::lin_seq_elem_stream {

struct Provider {
    void const* data;
    size_t elem_size;
    ptrdiff_t elem_stride;
    size_t elem_cnt;

    constexpr size_t GetElemSize(this Provider const& self,
                                 elem_stream::provider::Tag);

    constexpr bool IsEnd(this Provider const& self, elem_stream::provider::Tag);

    constexpr size_t Transfer(this Provider& self, elem_stream::provider::Tag,
                              void* dst, size_t dst_elem_size,
                              ptrdiff_t dst_elem_stride, size_t cnt);
};

struct Acceptor {
    void* data;
    size_t elem_size;
    ptrdiff_t elem_stride;
    size_t elem_cnt;

    constexpr size_t GetElemSize(this Acceptor const& self,
                                 elem_stream::acceptor::Tag);

    constexpr bool IsEnd(this Acceptor const& self, elem_stream::acceptor::Tag);

    constexpr size_t Transfer(this Acceptor& self, elem_stream::acceptor::Tag,
                              void const* src, size_t src_elem_size,
                              ptrdiff_t src_elem_stride, size_t cnt);
};

}  // namespace zeta::core::lin_seq_elem_stream
