#pragma once

#include <zeta/core/circular_array.hpp>
#include <zeta/core/circular_array.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/seg_utils.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

inline void seg_utils::SegShoveL(circular_array::Cntr* l_ca,
                                 circular_array::Cntr* r_ca, size_t shove_cnt) {
    ZETA_Core_DebugAssert(l_ca != nullptr);
    ZETA_Core_DebugAssert(r_ca != nullptr);

    ZETA_Core_DebugAssert(l_ca->elem_size == r_ca->elem_size);

    size_t l_vac{ l_ca->elem_capacity - l_ca->elem_cnt };

    ZETA_Core_DebugAssert(shove_cnt <= l_vac);
    ZETA_Core_DebugAssert(shove_cnt <= r_ca->elem_cnt);

    if (shove_cnt == 0) { return; }

    size_t l_elem_cnt{ l_ca->elem_cnt };

    circular_array::PushR(l_ca, shove_cnt, seq_cntr::EmptyWriter{}, nullptr);

    circular_array::AssignFromCircularArray(l_ca, l_elem_cnt, r_ca, 0,
                                            shove_cnt);

    circular_array::PopL(r_ca, shove_cnt);
}

inline void seg_utils::SegShoveR(circular_array::Cntr* l_ca,
                                 circular_array::Cntr* r_ca, size_t shove_cnt) {
    ZETA_Core_DebugAssert(l_ca != nullptr);
    ZETA_Core_DebugAssert(r_ca != nullptr);

    ZETA_Core_DebugAssert(l_ca->elem_size == r_ca->elem_size);

    size_t r_vac{ r_ca->elem_capacity - r_ca->elem_cnt };

    ZETA_Core_DebugAssert(shove_cnt <= r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= l_ca->elem_cnt);

    if (shove_cnt == 0) { return; }

    size_t l_elem_cnt{ l_ca->elem_cnt };

    circular_array::PushL(r_ca, shove_cnt, seq_cntr::EmptyWriter{}, nullptr);

    circular_array::AssignFromCircularArray(r_ca, 0, l_ca,
                                            l_elem_cnt - shove_cnt, shove_cnt);

    circular_array::PopR(l_ca, shove_cnt);
}

template <typename Writer>
void seg_utils::SegInsertShoveL(circular_array::Cntr* l_ca,
                                circular_array::Cntr* r_ca, size_t rl_cnt,
                                size_t ins_cnt, size_t shove_cnt,
                                Writer&& writer) {
    ZETA_Core_DebugAssert(l_ca != nullptr);
    ZETA_Core_DebugAssert(r_ca != nullptr);

    ZETA_Core_DebugAssert(l_ca->elem_size == r_ca->elem_size);

    size_t l_vac{ l_ca->elem_capacity - l_ca->elem_cnt };
    size_t r_vac{ r_ca->elem_capacity - r_ca->elem_cnt };

    ZETA_Core_DebugAssert(rl_cnt <= r_ca->elem_cnt);
    ZETA_Core_DebugAssert(ins_cnt <= l_vac + r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= l_vac);
    ZETA_Core_DebugAssert(shove_cnt <= r_ca->elem_cnt + ins_cnt);
    ZETA_Core_DebugAssert(r_ca->elem_cnt + ins_cnt - shove_cnt <=
                          r_ca->elem_capacity);

    if (ins_cnt == 0 && shove_cnt == 0) { return; }

    size_t cnt_a{ utils::Min(rl_cnt, shove_cnt) };
    size_t cnt_b{ utils::Min(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    size_t l_elem_cnt{ l_ca->elem_cnt };

    circular_array::PushR(l_ca, shove_cnt, seq_cntr::EmptyWriter{}, nullptr);

    if (0 < cnt_a) {
        circular_array::AssignFromCircularArray(l_ca, l_elem_cnt, r_ca, 0,
                                                cnt_a);
    }

    if (0 < cnt_b) {
        circular_array::IdxWrite(l_ca, l_elem_cnt + cnt_a, cnt_b,
                                 meta::Forward<Writer>(writer));
    }

    if (0 < cnt_c) {
        circular_array::AssignFromCircularArray(
            l_ca, l_elem_cnt + cnt_a + cnt_b, r_ca, cnt_a, cnt_c);
    }

    circular_array::PopL(r_ca, cnt_a + cnt_c);

    if (0 < ins_cnt - cnt_b) {
        circular_array::IdxInsert(r_ca, rl_cnt - cnt_a, ins_cnt - cnt_b,
                                  meta::Forward<Writer>(writer));
    }
}

template <typename Writer>
void seg_utils::SegInsertShoveR(circular_array::Cntr* l_ca,
                                circular_array::Cntr* r_ca, size_t lr_cnt,
                                size_t ins_cnt, size_t shove_cnt,
                                Writer&& writer) {
    ZETA_Core_DebugAssert(l_ca != nullptr);
    ZETA_Core_DebugAssert(r_ca != nullptr);

    ZETA_Core_DebugAssert(l_ca->elem_size == r_ca->elem_size);

    size_t l_vac{ l_ca->elem_capacity - l_ca->elem_cnt };
    size_t r_vac{ r_ca->elem_capacity - r_ca->elem_cnt };

    ZETA_Core_DebugAssert(lr_cnt <= l_ca->elem_cnt);
    ZETA_Core_DebugAssert(ins_cnt <= l_vac + r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= l_ca->elem_cnt + ins_cnt);
    ZETA_Core_DebugAssert(l_ca->elem_cnt + ins_cnt - shove_cnt <=
                          l_ca->elem_capacity);

    if (ins_cnt == 0 && shove_cnt == 0) { return; }

    size_t cnt_a{ utils::Min(lr_cnt, shove_cnt) };
    size_t cnt_b{ utils::Min(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    size_t l_elem_cnt{ l_ca->elem_cnt };

    circular_array::PushL(r_ca, shove_cnt, seq_cntr::EmptyWriter{}, nullptr);

    if (0 < cnt_c) {
        circular_array::AssignFromCircularArray(
            r_ca, 0, l_ca, l_elem_cnt - cnt_a - cnt_c, cnt_c);
    }

    if (0 < cnt_b) {
        circular_array::IdxWrite(r_ca, cnt_c, cnt_b,
                                 meta::Forward<Writer>(writer));
    }

    if (0 < cnt_a) {
        circular_array::AssignFromCircularArray(r_ca, cnt_c + cnt_b, l_ca,
                                                l_elem_cnt - cnt_a, cnt_a);
    }

    circular_array::PopR(l_ca, cnt_c + cnt_a);

    if (0 < ins_cnt - cnt_b) {
        circular_array::IdxInsert(l_ca, l_elem_cnt - lr_cnt, ins_cnt - cnt_b,
                                  meta::Forward<Writer>(writer));
    }
}

inline void seg_utils::SegEraseShoveL(circular_array::Cntr* l_ca,
                                      circular_array::Cntr* r_ca, size_t rl_cnt,
                                      size_t ers_cnt, size_t shove_cnt) {
    ZETA_Core_DebugAssert(l_ca != nullptr);
    ZETA_Core_DebugAssert(r_ca != nullptr);

    ZETA_Core_DebugAssert(l_ca->elem_size == r_ca->elem_size);

    size_t l_vac{ l_ca->elem_capacity - l_ca->elem_cnt };

    ZETA_Core_DebugAssert(rl_cnt <= r_ca->elem_cnt);
    ZETA_Core_DebugAssert(ers_cnt <= r_ca->elem_cnt - rl_cnt);
    ZETA_Core_DebugAssert(shove_cnt <= l_vac);
    ZETA_Core_DebugAssert(shove_cnt <= r_ca->elem_cnt - ers_cnt);

    if (ers_cnt == 0 && shove_cnt == 0) { return; }

    size_t cnt_a{ utils::Min(rl_cnt, shove_cnt) };
    size_t cnt_b{ ers_cnt };
    size_t cnt_c{ shove_cnt - cnt_a };

    size_t l_elem_cnt{ l_ca->elem_cnt };

    circular_array::PushR(l_ca, shove_cnt, seq_cntr::EmptyWriter{}, nullptr);

    if (0 < cnt_a) {
        circular_array::AssignFromCircularArray(l_ca, l_elem_cnt, r_ca, 0,
                                                cnt_a);
    }

    if (0 < cnt_c) {
        circular_array::AssignFromCircularArray(l_ca, l_elem_cnt + cnt_a, r_ca,
                                                cnt_a + cnt_b, cnt_c);
    }

    circular_array::PopL(r_ca, cnt_a + cnt_b + cnt_c);
}

inline void seg_utils::SegEraseShoveR(circular_array::Cntr* l_ca,
                                      circular_array::Cntr* r_ca, size_t lr_cnt,
                                      size_t ers_cnt, size_t shove_cnt) {
    ZETA_Core_DebugAssert(l_ca != nullptr);
    ZETA_Core_DebugAssert(r_ca != nullptr);

    ZETA_Core_DebugAssert(l_ca->elem_size == r_ca->elem_size);

    size_t r_vac{ r_ca->elem_capacity - r_ca->elem_cnt };

    ZETA_Core_DebugAssert(lr_cnt <= l_ca->elem_cnt);
    ZETA_Core_DebugAssert(ers_cnt <= l_ca->elem_cnt - lr_cnt);
    ZETA_Core_DebugAssert(shove_cnt <= r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= l_ca->elem_cnt - ers_cnt);

    if (ers_cnt == 0 && shove_cnt == 0) { return; }

    size_t cnt_a{ utils::Min(lr_cnt, shove_cnt) };
    size_t cnt_b{ ers_cnt };
    size_t cnt_c{ shove_cnt - cnt_a };

    circular_array::PushL(r_ca, shove_cnt, seq_cntr::EmptyWriter{}, nullptr);

    if (0 < cnt_c) {
        circular_array::AssignFromCircularArray(
            r_ca, 0, l_ca, l_ca->elem_cnt - cnt_a - cnt_b - cnt_c, cnt_c);
    }

    if (0 < cnt_a) {
        circular_array::AssignFromCircularArray(r_ca, cnt_c, l_ca,
                                                l_ca->elem_cnt - cnt_a, cnt_a);
    }

    circular_array::PopR(l_ca, cnt_a + cnt_b + cnt_c);
}

}  // namespace zeta::core
