#include <zeta/core/debugger.h>
#include <zeta/core/seq_cntr.h>
#include <zeta/core/utils.h>

void Zeta_Core_SeqCntr_ResizeL(Zeta_Core_SeqCntr seq_cntr, size_t size) {
    ZETA_Core_DebugAssert(seq_cntr.context != NULL);

    size_t origin_size = ZETA_Core_SeqCntr_GetSize(seq_cntr);

    if (origin_size < size) {
        ZETA_Core_SeqCntr_PushL(seq_cntr, size - origin_size, NULL);
    } else if (size < origin_size) {
        ZETA_Core_SeqCntr_PopL(seq_cntr, origin_size - size);
    }
}

void Zeta_Core_SeqCntr_ResizeR(Zeta_Core_SeqCntr seq_cntr, size_t size) {
    ZETA_Core_DebugAssert(seq_cntr.context != NULL);

    size_t origin_size = ZETA_Core_SeqCntr_GetSize(seq_cntr);

    if (origin_size < size) {
        ZETA_Core_SeqCntr_PushR(seq_cntr, size - origin_size, NULL);
    } else if (size < origin_size) {
        ZETA_Core_SeqCntr_PopR(seq_cntr, origin_size - size);
    }
}

void Zeta_Core_SeqCntr_RangeAssign(Zeta_Core_SeqCntr dst_seq_cntr,
                                   Zeta_Core_SeqCntr src_seq_cntr,
                                   size_t dst_idx, size_t src_idx, size_t cnt) {
    ZETA_Core_DebugAssert(dst_seq_cntr.context != NULL);
    ZETA_Core_DebugAssert(src_seq_cntr.context != NULL);

    ZETA_Core_DebugAssert(ZETA_Core_SeqCntr_GetWidth(dst_seq_cntr) ==
                          ZETA_Core_SeqCntr_GetWidth(src_seq_cntr));

    size_t width = ZETA_Core_SeqCntr_GetWidth(dst_seq_cntr);

    size_t buffer_capacity =
        ZETA_Core_GetMaxOf(1ULL, (sizeof(void*) * 64) / width);

    size_t dst_size = ZETA_Core_SeqCntr_GetSize(dst_seq_cntr);
    size_t src_size = ZETA_Core_SeqCntr_GetSize(src_seq_cntr);

    ZETA_Core_DebugAssert(dst_idx <= dst_size);
    ZETA_Core_DebugAssert(src_idx <= src_size);

    ZETA_Core_DebugAssert(cnt <= dst_size - dst_idx);
    ZETA_Core_DebugAssert(cnt <= src_size - src_idx);

    if (cnt == 0 ||
        (dst_seq_cntr.context == src_seq_cntr.context && dst_idx == src_idx)) {
        return;
    }

    size_t dst_end = dst_idx + cnt;
    size_t src_end = src_idx + cnt;

    void* buffer = __builtin_alloca_with_align(
        width * buffer_capacity, __CHAR_BIT__ * alignof(max_align_t));

    void* dst_cursor = ZETA_Core_SeqCntr_AllocaCursor(dst_seq_cntr);
    void* src_cursor = ZETA_Core_SeqCntr_AllocaCursor(src_seq_cntr);

    if (dst_seq_cntr.context != src_seq_cntr.context || dst_idx <= src_idx ||
        src_end <= dst_idx) {
        ZETA_Core_SeqCntr_Access(dst_seq_cntr, dst_idx, dst_cursor, NULL);
        ZETA_Core_SeqCntr_Access(src_seq_cntr, src_idx, src_cursor, NULL);

        while (0 < cnt) {
            size_t cur_cnt = ZETA_Core_GetMinOf(buffer_capacity, cnt);
            cnt -= cur_cnt;

            ZETA_Core_SeqCntr_Read(src_seq_cntr, src_cursor, cur_cnt, buffer,
                                   width, src_cursor);
            ZETA_Core_SeqCntr_Write(dst_seq_cntr, dst_cursor, cur_cnt, buffer,
                                    width, dst_cursor);
        }

        return;
    }

    ZETA_Core_SeqCntr_Access(dst_seq_cntr, dst_end, dst_cursor, NULL);
    ZETA_Core_SeqCntr_Access(src_seq_cntr, src_end, src_cursor, NULL);

    while (0 < cnt) {
        size_t cur_cnt = ZETA_Core_GetMinOf(buffer_capacity, cnt);
        cnt -= cur_cnt;

        ZETA_Core_SeqCntr_Cursor_AdvanceL(dst_seq_cntr, dst_cursor, cur_cnt);
        ZETA_Core_SeqCntr_Cursor_AdvanceL(src_seq_cntr, src_cursor, cur_cnt);

        ZETA_Core_SeqCntr_Read(src_seq_cntr, src_cursor, cur_cnt, buffer, width,
                               NULL);
        ZETA_Core_SeqCntr_Write(dst_seq_cntr, dst_cursor, cur_cnt, buffer,
                                width, NULL);
    }
}

void Zeta_Core_SeqCntr_Assign(Zeta_Core_SeqCntr dst_seq_cntr,
                              Zeta_Core_SeqCntr src_seq_cntr) {
    size_t dst_size = ZETA_Core_SeqCntr_GetSize(dst_seq_cntr);
    size_t src_size = ZETA_Core_SeqCntr_GetSize(src_seq_cntr);

    if (dst_size < src_size) {
        if (dst_seq_cntr.vtable->PushR != NULL) {
            ZETA_Core_SeqCntr_PushR(dst_seq_cntr, src_size - dst_size, NULL);
        } else {
            ZETA_Core_SeqCntr_PushL(dst_seq_cntr, src_size - dst_size, NULL);
        }
    } else if (src_size < dst_size) {
        if (dst_seq_cntr.vtable->PopR != NULL) {
            ZETA_Core_SeqCntr_PopR(dst_seq_cntr, dst_size - src_size);
        } else {
            ZETA_Core_SeqCntr_PopL(dst_seq_cntr, dst_size - src_size);
        }
    }

    Zeta_Core_SeqCntr_RangeAssign(dst_seq_cntr, src_seq_cntr, 0, 0, src_size);
}

void Zeta_Core_SeqCntr_NaiveInsert(Zeta_Core_SeqCntr seq_cntr, size_t idx,
                                   size_t cnt) {
    if (cnt == 0) { return; }

    bool_t push_l_en = seq_cntr.vtable->PushL != NULL;
    bool_t push_r_en = seq_cntr.vtable->PushL != NULL;

    ZETA_Core_DebugAssert(push_l_en || push_r_en);

    size_t size = ZETA_Core_SeqCntr_GetSize(seq_cntr);
    ZETA_Core_DebugAssert(idx <= size);

    size_t l_size = idx;
    size_t r_size = size - l_size;

    void* dst_cursor = ZETA_Core_SeqCntr_AllocaCursor(seq_cntr);

    if (!push_l_en) { goto PUSH_R; }
    if (!push_r_en) { goto PUSH_L; }

    if (l_size < r_size) {
        goto PUSH_L;
    } else {
        goto PUSH_R;
    }

PUSH_L: {
    ZETA_Core_SeqCntr_PushL(seq_cntr, cnt, dst_cursor);
    Zeta_Core_SeqCntr_RangeAssign(seq_cntr, seq_cntr, 0, cnt + l_size, l_size);
    return;
}

PUSH_R: {
    ZETA_Core_SeqCntr_PushR(seq_cntr, cnt, dst_cursor);
    Zeta_Core_SeqCntr_RangeAssign(seq_cntr, seq_cntr, size, l_size, r_size);
    return;
}
}
