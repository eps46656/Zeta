#include <zeta/core/seg_utils.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core::seg_utils {

template <typename Writer>
void SegShoveL(CircularArray* l_ca, CircularArray* r_ca, size_t rl_cnt,
               size_t ins_cnt, size_t shove_cnt, Writer&& writer) {
    ZETA_Core_DebugAssert(l_ca != nullptr);
    ZETA_Core_DebugAssert(r_ca != nullptr);

    ZETA_Core_DebugAssert(l_ca->width == r_ca->width);

    ZETA_Core_DebugAssert(shove_cnt <= l_ca->capacity - l_ca->size);
    ZETA_Core_DebugAssert(shove_cnt <= r_ca->size + ins_cnt);
    ZETA_Core_DebugAssert(r_ca->size + ins_cnt - shove_cnt <= r_ca->capacity);

    size_t cnt_a{ Min(rl_cnt, shove_cnt) };
    size_t cnt_b{ Min(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    size_t l_size{ l_ca->size };

    CircularArray::TplPushR(
        l_ca, shove_cnt, [](void*, size_t, size_t) {}, nullptr);

    if (0 < cnt_a) {
        CircularArray::AssignFromCircularArray(l_ca, l_size, r_ca, 0, cnt_a);
    }

    if (0 < cnt_b) {
        CircularArray::IdxTplWrite(l_ca, l_size + cnt_a, cnt_b, writer);
    }

    if (0 < cnt_c) {
        CircularArray::AssignFromCircularArray(l_ca, l_size + cnt_a + cnt_b,
                                               r_ca, cnt_a, cnt_c);
    }

    CircularArray::PopL(r_ca, cnt_a + cnt_c);

    if (0 < ins_cnt - cnt_b) {
        CircularArray::IdxTplInsert(r_ca, rl_cnt - cnt_a, ins_cnt - cnt_b,
                                    writer);
    }
}

template <typename Writer>
void SegShoveR(CircularArray* l_ca, CircularArray* r_ca, size_t lr_cnt,
               size_t ins_cnt, size_t shove_cnt, Writer&& writer) {
    ZETA_Core_DebugAssert(l_ca != nullptr);
    ZETA_Core_DebugAssert(r_ca != nullptr);

    ZETA_Core_DebugAssert(l_ca->width == r_ca->width);

    ZETA_Core_DebugAssert(r_ca->size + shove_cnt <= r_ca->capacity);
    ZETA_Core_DebugAssert(shove_cnt <= l_ca->size + ins_cnt);

    size_t cnt_a{ Min(lr_cnt, shove_cnt) };
    size_t cnt_b{ Min(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    size_t l_size{ l_ca->size };

    CircularArray::TplPushL(
        r_ca, shove_cnt, [](void*, size_t, size_t) {}, nullptr);

    if (0 < cnt_c) {
        CircularArray::AssignFromCircularArray(r_ca, 0, l_ca,
                                               l_size - cnt_a - cnt_c, cnt_c);
    }

    if (0 < cnt_a) {
        CircularArray::AssignFromCircularArray(r_ca, cnt_c + cnt_b, l_ca,
                                               l_size - cnt_a, cnt_a);
    }

    CircularArray::PopR(l_ca, cnt_c + cnt_a);

    if (0 < ins_cnt - cnt_b) {
        CircularArray::IdxTplInsert(l_ca, l_size - lr_cnt, ins_cnt - cnt_b,
                                    writer);
    }

    if (0 < cnt_b) { CircularArray::IdxTplWrite(r_ca, cnt_c, cnt_b, writer); }
}

}  // namespace zeta::core::seg_utils
