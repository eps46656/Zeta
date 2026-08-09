#pragma once

#include <zeta/core/elem_stream.hpp>

namespace zeta::core::lin_seq_elem_stream {

struct Provider {
    void const* data;
    size_t elem_size;
    size_t elem_stride;
    size_t elem_cnt;

    size_t GetElemSize(this Provider const& self);

    bool IsEnd(this Provider const& self);

    size_t Transfer(this Provider& self, void* dst, size_t dst_elem_size,
                    size_t dst_elem_stride, size_t cnt);
};

struct Acceptor {
    void* data;
    size_t elem_size;
    size_t elem_stride;
    size_t elem_cnt;

    size_t GetElemSize(this Acceptor const& self);

    bool IsEnd(this Acceptor const& self);

    size_t Transfer(this Acceptor& self, void const* src, size_t src_elem_size,
                    size_t src_elem_stride, size_t cnt);
};

}  // namespace zeta::core::lin_seq_elem_stream

namespace zeta::core {

template <>
struct elem_stream::provider::ProviderTraits<lin_seq_elem_stream::Provider>
    : elem_stream::provider::MemberFuncProviderTraitsAdapter<
          lin_seq_elem_stream::Provider> {};

template <>
struct elem_stream::acceptor::AcceptorTraits<lin_seq_elem_stream::Acceptor>
    : elem_stream::acceptor::MemberFuncAcceptorTraitsAdapter<
          lin_seq_elem_stream::Acceptor> {};

}  // namespace zeta::core
