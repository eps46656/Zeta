#pragma once

#include <zeta/core/circular_array.hpp>
#include <zeta/core/circular_array.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seg_utils.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

constexpr void seg_utils::SegShoveL(circular_array::Cntr& l_ca,
                                    circular_array::Cntr& r_ca,
                                    size_t shove_cnt) {
    ZETA_Core_DebugAssert(l_ca.elem_size == r_ca.elem_size);

    size_t l_vac{ l_ca.slot_cnt - l_ca.elem_cnt };

    ZETA_Core_DebugAssert(shove_cnt <= l_vac);
    ZETA_Core_DebugAssert(shove_cnt <= r_ca.elem_cnt);

    if (shove_cnt == 0) { return; }

    size_t l_elem_cnt{ l_ca.elem_cnt };

    seq_cntr::PushR(l_ca, shove_cnt, elem_stream::provider::EmptyProvider{},
                    nullptr);

    l_ca.AssignFromCircularArray(l_elem_cnt, r_ca, 0, shove_cnt);

    seq_cntr::PopL(r_ca, shove_cnt, elem_stream::acceptor::EmptyAcceptor{});
}

constexpr void seg_utils::SegShoveR(circular_array::Cntr& l_ca,
                                    circular_array::Cntr& r_ca,
                                    size_t shove_cnt) {
    ZETA_Core_DebugAssert(l_ca.elem_size == r_ca.elem_size);

    size_t r_vac{ r_ca.slot_cnt - r_ca.elem_cnt };

    ZETA_Core_DebugAssert(shove_cnt <= r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= l_ca.elem_cnt);

    if (shove_cnt == 0) { return; }

    size_t l_elem_cnt{ l_ca.elem_cnt };

    seq_cntr::PushL(r_ca, shove_cnt, elem_stream::provider::EmptyProvider{},
                    nullptr);

    r_ca.AssignFromCircularArray(0, l_ca, l_elem_cnt - shove_cnt, shove_cnt);

    seq_cntr::PopR(l_ca, shove_cnt, elem_stream::acceptor::EmptyAcceptor{});
}

template <typename Writer>
constexpr void seg_utils::SegInsertShoveL(circular_array::Cntr& l_ca,
                                          circular_array::Cntr& r_ca,
                                          size_t rl_cnt, size_t ins_cnt,
                                          size_t shove_cnt, Writer& writer) {
    ZETA_Core_DebugAssert(l_ca.elem_size == r_ca.elem_size);

    size_t l_vac{ l_ca.slot_cnt - l_ca.elem_cnt };
    size_t r_vac{ r_ca.slot_cnt - r_ca.elem_cnt };

    ZETA_Core_DebugAssert(rl_cnt <= r_ca.elem_cnt);
    ZETA_Core_DebugAssert(ins_cnt <= l_vac + r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= l_vac);
    ZETA_Core_DebugAssert(shove_cnt <= r_ca.elem_cnt + ins_cnt);
    ZETA_Core_DebugAssert(r_ca.elem_cnt + ins_cnt - shove_cnt <= r_ca.slot_cnt);

    if (ins_cnt == 0 && shove_cnt == 0) { return; }

    size_t cnt_a{ comparison_utils::BasicMin(rl_cnt, shove_cnt) };
    size_t cnt_b{ comparison_utils::BasicMin(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    size_t l_elem_cnt{ l_ca.elem_cnt };

    l_ca.PushR(shove_cnt, elem_stream::provider::EmptyProvider{}, nullptr);

    if (0 < cnt_a) { l_ca.AssignFromCircularArray(l_elem_cnt, r_ca, 0, cnt_a); }

    if (0 < cnt_b) { l_ca.IdxWrite(l_elem_cnt + cnt_a, cnt_b, writer); }

    if (0 < cnt_c) {
        l_ca.AssignFromCircularArray(l_elem_cnt + cnt_a + cnt_b, r_ca, cnt_a,
                                     cnt_c);
    }

    r_ca.PopL(cnt_a + cnt_c, elem_stream::acceptor::EmptyAcceptor{});

    if (0 < ins_cnt - cnt_b) {
        r_ca.IdxInsert(rl_cnt - cnt_a, ins_cnt - cnt_b, writer);
    }
}

template <typename Writer>
constexpr void seg_utils::SegInsertShoveR(circular_array::Cntr& l_ca,
                                          circular_array::Cntr& r_ca,
                                          size_t lr_cnt, size_t ins_cnt,
                                          size_t shove_cnt, Writer& writer) {
    ZETA_Core_DebugAssert(l_ca.elem_size == r_ca.elem_size);

    size_t l_vac{ l_ca.slot_cnt - l_ca.elem_cnt };
    size_t r_vac{ r_ca.slot_cnt - r_ca.elem_cnt };

    ZETA_Core_DebugAssert(lr_cnt <= l_ca.elem_cnt);
    ZETA_Core_DebugAssert(ins_cnt <= l_vac + r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= l_ca.elem_cnt + ins_cnt);
    ZETA_Core_DebugAssert(l_ca.elem_cnt + ins_cnt - shove_cnt <= l_ca.slot_cnt);

    if (ins_cnt == 0 && shove_cnt == 0) { return; }

    size_t cnt_a{ comparison_utils::BasicMin(lr_cnt, shove_cnt) };
    size_t cnt_b{ comparison_utils::BasicMin(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    size_t l_elem_cnt{ l_ca.elem_cnt };

    r_ca.PushL(shove_cnt, elem_stream::provider::EmptyProvider{}, nullptr);

    if (0 < cnt_c) {
        r_ca.AssignFromCircularArray(0, l_ca, l_elem_cnt - cnt_a - cnt_c,
                                     cnt_c);
    }

    if (0 < cnt_b) { r_ca.IdxWrite(cnt_c, cnt_b, writer); }

    if (0 < cnt_a) {
        r_ca.AssignFromCircularArray(cnt_c + cnt_b, l_ca, l_elem_cnt - cnt_a,
                                     cnt_a);
    }

    l_ca.PopR(cnt_c + cnt_a, elem_stream::acceptor::EmptyAcceptor{});

    if (0 < ins_cnt - cnt_b) {
        l_ca.IdxInsert(l_elem_cnt - lr_cnt, ins_cnt - cnt_b, writer);
    }
}

template <typename Reader>
constexpr void seg_utils::SegEraseShoveL(circular_array::Cntr& l_ca,
                                         circular_array::Cntr& r_ca,
                                         size_t rl_cnt, size_t ers_cnt,
                                         size_t shove_cnt, Reader&& reader) {
    ZETA_Core_DebugAssert(l_ca.elem_size == r_ca.elem_size);

    size_t l_vac{ l_ca.slot_cnt - l_ca.elem_cnt };

    ZETA_Core_DebugAssert(rl_cnt <= r_ca.elem_cnt);
    ZETA_Core_DebugAssert(ers_cnt <= r_ca.elem_cnt - rl_cnt);
    ZETA_Core_DebugAssert(shove_cnt <= l_vac);
    ZETA_Core_DebugAssert(shove_cnt <= r_ca.elem_cnt - ers_cnt);

    if (ers_cnt == 0 && shove_cnt == 0) { return; }

    size_t cnt_a{ comparison_utils::BasicMin(rl_cnt, shove_cnt) };
    size_t cnt_b{ ers_cnt };
    size_t cnt_c{ shove_cnt - cnt_a };

    size_t l_elem_cnt{ l_ca.elem_cnt };

    seq_cntr::PushR(l_ca, shove_cnt, elem_stream::provider::EmptyProvider{},
                    nullptr);

    if (0 < cnt_a) { l_ca.AssignFromCircularArray(l_elem_cnt, r_ca, 0, cnt_a); }

    if (0 < cnt_c) {
        l_ca.AssignFromCircularArray(l_elem_cnt + cnt_a, r_ca, cnt_a + cnt_b,
                                     cnt_c);
    }

    if constexpr (!meta::IsSame<meta::RemoveCVRef<Reader>,
                                elem_stream::acceptor::EmptyAcceptor>) {
        r_ca.IdxRead(rl_cnt, ers_cnt, reader);
    }

    seq_cntr::PopL(r_ca, cnt_a + cnt_b + cnt_c,
                   elem_stream::acceptor::EmptyAcceptor{});
}

template <typename Reader>
constexpr void seg_utils::SegEraseShoveR(circular_array::Cntr& l_ca,
                                         circular_array::Cntr& r_ca,
                                         size_t lr_cnt, size_t ers_cnt,
                                         size_t shove_cnt, Reader&& reader) {
    ZETA_Core_DebugAssert(l_ca.elem_size == r_ca.elem_size);

    size_t r_vac{ r_ca.slot_cnt - r_ca.elem_cnt };

    ZETA_Core_DebugAssert(lr_cnt <= l_ca.elem_cnt);
    ZETA_Core_DebugAssert(ers_cnt <= l_ca.elem_cnt - lr_cnt);
    ZETA_Core_DebugAssert(shove_cnt <= r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= l_ca.elem_cnt - ers_cnt);

    if (ers_cnt == 0 && shove_cnt == 0) { return; }

    size_t cnt_a{ comparison_utils::BasicMin(lr_cnt, shove_cnt) };
    size_t cnt_b{ ers_cnt };
    size_t cnt_c{ shove_cnt - cnt_a };

    seq_cntr::PushL(r_ca, shove_cnt, elem_stream::provider::EmptyProvider{},
                    nullptr);

    if (0 < cnt_c) {
        r_ca.AssignFromCircularArray(
            0, l_ca, l_ca.elem_cnt - cnt_a - cnt_b - cnt_c, cnt_c);
    }

    if (0 < cnt_a) {
        r_ca.AssignFromCircularArray(cnt_c, l_ca, l_ca.elem_cnt - cnt_a, cnt_a);
    }

    if constexpr (!meta::IsSame<meta::RemoveCVRef<Reader>,
                                elem_stream::acceptor::EmptyAcceptor>) {
        l_ca.IdxRead(l_ca.elem_cnt - lr_cnt - ers_cnt, ers_cnt, reader);
    }

    seq_cntr::PopR(l_ca, cnt_a + cnt_b + cnt_c,
                   elem_stream::acceptor::EmptyAcceptor{});
}

}  // namespace zeta::core
