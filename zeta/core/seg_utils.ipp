#pragma once

#include <zeta/core/circular_array.hpp>
#include <zeta/core/circular_array.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seg_utils.hpp>
#include <zeta/core/seq_cntr_utils.ipp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

template <meta::IsContainerElem Elem>
constexpr void seg_utils::SegShoveL(circular_array::Cntr<Elem>& l_ca,
                                    circular_array::Cntr<Elem>& r_ca,
                                    size_t shove_cnt) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(l_ca.elem_size == r_ca.elem_size);

    size_t l_vac{ l_ca.slot_cnt - l_ca.elem_cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <= l_vac);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <= r_ca.elem_cnt);

    if (shove_cnt == 0) { return; }

    circular_array::Cursor r_cursor;

    seq_cntr::Refer(r_ca, 0, true, nullptr, &r_cursor,
                    lifecycle::DataLifeState::Mem, nullptr);

    seq_cntr::PushR(
        l_ca, shove_cnt,
        seq_cntr_utils::provider::Provider<circular_array::Cntr<Elem>&&>(
            meta::Move(r_ca), r_cursor, shove_cnt),
        nullptr);

    seq_cntr::PopL(r_ca, shove_cnt, seq_endpoint::acceptor::BasicAcceptor{});
}

template <meta::IsContainerElem Elem>
constexpr void seg_utils::SegShoveR(circular_array::Cntr<Elem>& l_ca,
                                    circular_array::Cntr<Elem>& r_ca,
                                    size_t shove_cnt) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(l_ca.elem_size == r_ca.elem_size);

    size_t r_vac{ r_ca.slot_cnt - r_ca.elem_cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <= r_vac);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <= l_ca.elem_cnt);

    if (shove_cnt == 0) { return; }

    circular_array::Cursor l_cursor;

    seq_cntr::Refer(l_ca, l_ca.elem_cnt - shove_cnt, true, nullptr, &l_cursor,
                    lifecycle::DataLifeState::Mem, nullptr);

    seq_cntr::PushL(
        r_ca, shove_cnt,
        seq_cntr_utils::provider::Provider<circular_array::Cntr<Elem>&&>(
            meta::Move(l_ca), l_cursor, shove_cnt),
        nullptr);

    seq_cntr::PopR(l_ca, shove_cnt, seq_endpoint::acceptor::BasicAcceptor{});
}

template <meta::IsContainerElem Elem,
          seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void seg_utils::SegInsertShoveL(circular_array::Cntr<Elem>& l_ca,
                                          circular_array::Cntr<Elem>& r_ca,
                                          size_t rl_cnt, size_t ins_cnt,
                                          size_t shove_cnt,
                                          Provider& provider) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(l_ca.elem_size == r_ca.elem_size);

    size_t l_vac{ l_ca.slot_cnt - l_ca.elem_cnt };
    size_t r_vac{ r_ca.slot_cnt - r_ca.elem_cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(rl_cnt <= r_ca.elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(ins_cnt <= l_vac + r_vac);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <= l_vac);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <=
                                            r_ca.elem_cnt + ins_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        r_ca.elem_cnt + ins_cnt - shove_cnt <= r_ca.slot_cnt);

    if (ins_cnt == 0 && shove_cnt == 0) { return; }

    size_t cnt_a{ comparison_utils::BasicMin(rl_cnt, shove_cnt) };
    size_t cnt_b{ comparison_utils::BasicMin(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    circular_array::Cursor r_cursor;

    seq_cntr::Refer(r_ca, 0, true, nullptr, &r_cursor,
                    lifecycle::DataLifeState::Mem, nullptr);

    seq_cntr_utils::provider::Provider<circular_array::Cntr<Elem>&&> r_provider{
        meta::Move(r_ca), r_cursor, shove_cnt
    };

    if (0 < cnt_a) {
        seq_cntr::PushR(l_ca, cnt_a, r_provider, nullptr, nullptr);
    }

    if (0 < cnt_b) { seq_cntr::PushR(l_ca, cnt_b, provider, nullptr, nullptr); }

    if (0 < cnt_c) {
        seq_cntr::PushR(l_ca, cnt_c, r_provider, nullptr, nullptr);
    }

    seq_cntr::PopL(r_ca, cnt_a + cnt_c,
                   seq_endpoint::acceptor::BasicAcceptor{});

    if (0 < ins_cnt - cnt_b) {
        r_ca.IdxInsert(rl_cnt - cnt_a, ins_cnt - cnt_b, provider);
    }
}

template <meta::IsContainerElem Elem,
          seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void seg_utils::SegInsertShoveR(circular_array::Cntr<Elem>& l_ca,
                                          circular_array::Cntr<Elem>& r_ca,
                                          size_t lr_cnt, size_t ins_cnt,
                                          size_t shove_cnt,
                                          Provider& provider) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(l_ca.elem_size == r_ca.elem_size);

    size_t l_vac{ l_ca.slot_cnt - l_ca.elem_cnt };
    size_t r_vac{ r_ca.slot_cnt - r_ca.elem_cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(lr_cnt <= l_ca.elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(ins_cnt <= l_vac + r_vac);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <= r_vac);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <=
                                            l_ca.elem_cnt + ins_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        l_ca.elem_cnt + ins_cnt - shove_cnt <= l_ca.slot_cnt);

    if (ins_cnt == 0 && shove_cnt == 0) { return; }

    size_t cnt_a{ comparison_utils::BasicMin(lr_cnt, shove_cnt) };
    size_t cnt_b{ comparison_utils::BasicMin(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    size_t ll_cnt{ l_ca.elem_cnt - lr_cnt };

    circular_array::Cursor l_cursor;

    if (0 < cnt_a) {
        seq_cntr::Refer(l_ca, l_ca.elem_cnt - cnt_a, true, nullptr, &l_cursor,
                        lifecycle::DataLifeState::Mem, nullptr);

        seq_cntr_utils::provider::Provider<circular_array::Cntr<Elem>&&>
            l_provider{ meta::Move(l_ca), l_cursor, cnt_a };

        seq_cntr::PushL(r_ca, cnt_a, l_provider, nullptr, nullptr);

        seq_cntr::PopR(l_ca, cnt_a, seq_endpoint::acceptor::BasicAcceptor{});
    }

    if (0 < ins_cnt - cnt_b) {
        l_ca.IdxInsert(ll_cnt, ins_cnt - cnt_b, provider);
    }

    if (0 < cnt_b) { seq_cntr::PushL(r_ca, cnt_b, provider, nullptr, nullptr); }

    if (0 < cnt_c) {
        seq_cntr::Refer(l_ca, ll_cnt - cnt_c, true, nullptr, &l_cursor,
                        lifecycle::DataLifeState::Mem, nullptr);

        seq_cntr_utils::provider::Provider<circular_array::Cntr<Elem>&&>
            l_provider{ meta::Move(l_ca), l_cursor, cnt_c };

        seq_cntr::PushL(r_ca, cnt_c, l_provider, nullptr, nullptr);

        seq_cntr::PopR(l_ca, cnt_c, seq_endpoint::acceptor::BasicAcceptor{});
    }
}

template <meta::IsContainerElem Elem,
          seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void seg_utils::SegEraseShoveL(circular_array::Cntr<Elem>& l_ca,
                                         circular_array::Cntr<Elem>& r_ca,
                                         size_t rl_cnt, size_t ers_cnt,
                                         size_t shove_cnt, Acceptor&& reader) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(l_ca.elem_size == r_ca.elem_size);

    size_t l_vac{ l_ca.slot_cnt - l_ca.elem_cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(rl_cnt <= r_ca.elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(ers_cnt <= r_ca.elem_cnt - rl_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <= l_vac);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <=
                                            r_ca.elem_cnt - ers_cnt);

    if (ers_cnt == 0 && shove_cnt == 0) { return; }

    size_t cnt_a{ comparison_utils::BasicMin(rl_cnt, shove_cnt) };
    size_t cnt_b{ ers_cnt };
    size_t cnt_c{ shove_cnt - cnt_a };

    circular_array::Cursor r_cursor;

    if (0 < cnt_a) {
        seq_cntr::Refer(r_ca, 0, true, nullptr, &r_cursor,
                        lifecycle::DataLifeState::Mem, nullptr);

        seq_cntr_utils::provider::Provider<circular_array::Cntr<Elem>&&>
            r_provider{ meta::Move(r_ca), r_cursor, cnt_a };

        seq_cntr::PushR(l_ca, cnt_a, r_provider, nullptr, nullptr);

        seq_cntr::PopL(r_ca, cnt_a, seq_endpoint::acceptor::BasicAcceptor{});
    }

    if (0 < cnt_b) {
        seq_cntr::Refer(r_ca, rl_cnt - cnt_a, true, nullptr, &r_cursor,
                        lifecycle::DataLifeState::Mem, nullptr);

        seq_cntr::Erase(r_ca, &r_cursor, cnt_b, reader);
    }

    if (0 < cnt_c) {
        seq_cntr::Refer(r_ca, 0, true, nullptr, &r_cursor,
                        lifecycle::DataLifeState::Mem, nullptr);

        seq_cntr_utils::provider::Provider<circular_array::Cntr<Elem>&&>
            r_provider{ meta::Move(r_ca), r_cursor, cnt_c };

        seq_cntr::PushR(l_ca, cnt_c, r_provider, nullptr, nullptr);

        seq_cntr::PopL(r_ca, cnt_c, seq_endpoint::acceptor::BasicAcceptor{});
    }
}

template <meta::IsContainerElem Elem,
          seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void seg_utils::SegEraseShoveR(circular_array::Cntr<Elem>& l_ca,
                                         circular_array::Cntr<Elem>& r_ca,
                                         size_t lr_cnt, size_t ers_cnt,
                                         size_t shove_cnt, Acceptor&& reader) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(l_ca.elem_size == r_ca.elem_size);

    size_t r_vac{ r_ca.slot_cnt - r_ca.elem_cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(lr_cnt <= l_ca.elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(ers_cnt <= l_ca.elem_cnt - lr_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <= r_vac);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <=
                                            l_ca.elem_cnt - ers_cnt);

    if (ers_cnt == 0 && shove_cnt == 0) { return; }

    size_t cnt_a{ comparison_utils::BasicMin(lr_cnt, shove_cnt) };
    size_t cnt_b{ ers_cnt };
    size_t cnt_c{ shove_cnt - cnt_a };

    size_t ll_cnt{ l_ca.elem_cnt - lr_cnt - ers_cnt };

    circular_array::Cursor r_cursor;

    if (0 < cnt_a) {
        seq_cntr::Refer(l_ca, l_ca.elem_cnt - cnt_a, true, nullptr, &r_cursor,
                        lifecycle::DataLifeState::Mem, nullptr);

        seq_cntr_utils::provider::Provider<circular_array::Cntr<Elem>&&>
            l_provider{ meta::Move(l_ca), r_cursor, cnt_a };

        seq_cntr::PushL(r_ca, cnt_a, l_provider, nullptr, nullptr);

        seq_cntr::PopR(l_ca, cnt_a, seq_endpoint::acceptor::BasicAcceptor{});
    }

    if (0 < cnt_b) {
        seq_cntr::Refer(l_ca, ll_cnt, true, nullptr, &r_cursor,
                        lifecycle::DataLifeState::Mem, nullptr);

        seq_cntr::Erase(l_ca, &r_cursor, cnt_b, reader);
    }

    if (0 < cnt_c) {
        seq_cntr::Refer(l_ca, ll_cnt - cnt_c, true, nullptr, &r_cursor,
                        lifecycle::DataLifeState::Mem, nullptr);

        seq_cntr_utils::provider::Provider<circular_array::Cntr<Elem>&&>
            l_provider{ meta::Move(l_ca), r_cursor, cnt_c };

        seq_cntr::PushL(r_ca, cnt_c, l_provider, nullptr, nullptr);

        seq_cntr::PopR(l_ca, cnt_c, seq_endpoint::acceptor::BasicAcceptor{});
    }
}

}  // namespace zeta::core
