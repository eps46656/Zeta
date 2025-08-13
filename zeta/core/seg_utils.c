#include <zeta/core/debugger.h>
#include <zeta/core/seg_utils.h>
#include <zeta/core/utils.h>

void SegShoveL(Zeta_Core_CircularArray* l_ca, Zeta_Core_CircularArray* r_ca,
               size_t rl_cnt, size_t ins_cnt, size_t shove_cnt) {
    ZETA_Core_DebugAssert(l_ca != NULL);
    ZETA_Core_DebugAssert(r_ca != NULL);

    ZETA_Core_DebugAssert(l_ca->width == r_ca->width);

    ZETA_Core_DebugAssert(shove_cnt <= l_ca->capacity - l_ca->size);
    ZETA_Core_DebugAssert(shove_cnt <= r_ca->size + ins_cnt);
    ZETA_Core_DebugAssert(r_ca->size + ins_cnt - shove_cnt <= r_ca->capacity);

    size_t cnt_a = ZETA_Core_GetMinOf(rl_cnt, shove_cnt);
    size_t cnt_b = ZETA_Core_GetMinOf(ins_cnt, shove_cnt - cnt_a);
    size_t cnt_c = shove_cnt - cnt_a - cnt_b;

    size_t l_size = l_ca->size;

    Zeta_Core_CircularArray_PushR(l_ca, shove_cnt, NULL);

    if (0 < cnt_a) {
        Zeta_Core_CircularArray_Assign(l_ca, r_ca, l_size, 0, cnt_a);
    }

    if (0 < cnt_c) {
        Zeta_Core_CircularArray_Assign(l_ca, r_ca, l_size + cnt_a + cnt_b,
                                       cnt_a, cnt_c);
    }

    Zeta_Core_CircularArray_PopL(r_ca, cnt_a + cnt_c);

    if (0 < ins_cnt - cnt_b) {
        Zeta_Core_CircularArray_Cursor r_ca_cursor;

        Zeta_Core_CircularArray_Access(r_ca, rl_cnt - cnt_a, &r_ca_cursor,
                                       NULL);

        Zeta_Core_CircularArray_Insert(r_ca, &r_ca_cursor, ins_cnt - cnt_b);
    }
}

void SegShoveR(Zeta_Core_CircularArray* l_ca, Zeta_Core_CircularArray* r_ca,
               size_t lr_cnt, size_t ins_cnt, size_t shove_cnt) {
    ZETA_Core_DebugAssert(l_ca != NULL);
    ZETA_Core_DebugAssert(r_ca != NULL);

    ZETA_Core_DebugAssert(l_ca->width == r_ca->width);

    ZETA_Core_DebugAssert(r_ca->size + shove_cnt <= r_ca->capacity);
    ZETA_Core_DebugAssert(shove_cnt <= l_ca->size + ins_cnt);

    size_t cnt_a = ZETA_Core_GetMinOf(lr_cnt, shove_cnt);
    size_t cnt_b = ZETA_Core_GetMinOf(ins_cnt, shove_cnt - cnt_a);
    size_t cnt_c = shove_cnt - cnt_a - cnt_b;

    size_t l_size = l_ca->size;

    Zeta_Core_CircularArray_PushL(r_ca, shove_cnt, NULL);

    if (0 < cnt_c) {
        Zeta_Core_CircularArray_Assign(r_ca, l_ca, 0, l_size - cnt_a - cnt_c,
                                       cnt_c);
    }

    if (0 < cnt_a) {
        Zeta_Core_CircularArray_Assign(r_ca, l_ca, cnt_c + cnt_b,
                                       l_size - cnt_a, cnt_a);
    }

    Zeta_Core_CircularArray_PopR(l_ca, cnt_c + cnt_a);

    if (0 < ins_cnt - cnt_b) {
        Zeta_Core_CircularArray_Cursor l_ca_cursor;

        Zeta_Core_CircularArray_Access(l_ca, l_size - lr_cnt, &l_ca_cursor,
                                       NULL);

        Zeta_Core_CircularArray_Insert(l_ca, &l_ca_cursor, ins_cnt - cnt_b);
    }
}
