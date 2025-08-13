#include <zeta/core/circular_array.h>
#include <zeta/core/debugger.h>
#include <zeta/core/dynamic_vector.h>
#include <zeta/core/utils.h>

#if ZETA_Core_EnableDebug

#define CheckCntr_(dv) Zeta_Core_DynamicVector_Check((dv))

#define CheckCursor_(dv, cursor) \
    Zeta_Core_DynamicVector_Cursor_Check((dv), (cursor))

#else

#define CheckCntr_(dv) ZETA_Core_Unused((dv))

#define CheckCursor_(dv, cursor)    \
    {                               \
        ZETA_Core_Unused((dv));     \
        ZETA_Core_Unused((cursor)); \
    }                               \
    ZETA_Core_StaticAssert(TRUE)

#endif

static size_t GetNxtCapacity_(size_t cur_capacity) {
    return ZETA_Core_CeilIntDiv(ZETA_Core_GetMaxOf(8, cur_capacity) * 3, 2);
}

static void TryMigrate_(Zeta_Core_DynamicVector* dv,
                        Zeta_Core_CircularArray* cur_ca,
                        Zeta_Core_CircularArray* nxt_ca, size_t op_cnt) {
    size_t move_cnt = ZETA_Core_GetMinOf(8, op_cnt * 2, dv->size_b);

    if (nxt_ca->data == NULL || move_cnt == 0) { return; }

    unsigned long long random_seed = Zeta_Core_GetRandom();

    if (Zeta_Choose2(dv->size_c <= dv->size_a, dv->size_a <= dv->size_c,
                     &random_seed) == 0) {
        Zeta_Core_CircularArray_Assign(nxt_ca, cur_ca, dv->size_a, 0, move_cnt);
        Zeta_Core_CircularArray_PopL(cur_ca, move_cnt);

        dv->size_a += move_cnt;
        dv->size_b -= move_cnt;
    } else {
        Zeta_Core_CircularArray_Assign(nxt_ca, cur_ca,
                                       dv->size_a + dv->size_b - move_cnt,
                                       dv->size_b - move_cnt, move_cnt);

        Zeta_Core_CircularArray_PopR(cur_ca, move_cnt);

        dv->size_b -= move_cnt;
        dv->size_c += move_cnt;
    }

    if (dv->size_b == 0) {
        dv->size_a = 0;
        dv->size_b = nxt_ca->size;
        dv->size_c = 0;

        ZETA_Core_Allocator_Deallocate(dv->data_allocator, cur_ca->data);

        cur_ca->data = nxt_ca->data;
        cur_ca->offset = nxt_ca->offset;
        cur_ca->size = nxt_ca->size;
        cur_ca->capacity = nxt_ca->capacity;

        nxt_ca->data = NULL;
        nxt_ca->offset = 0;
        nxt_ca->size = 0;
        nxt_ca->capacity = 0;
    }
}

static void UpdateCA_(Zeta_Core_DynamicVector* dv,
                      Zeta_Core_CircularArray* cur_ca,
                      Zeta_Core_CircularArray* nxt_ca) {
    dv->cur_data = cur_ca->data;
    dv->cur_offset = cur_ca->offset;
    dv->cur_capacity = cur_ca->capacity;

    dv->nxt_data = nxt_ca->data;
    dv->nxt_offset = nxt_ca->offset;
    dv->nxt_capacity = nxt_ca->capacity;
}

static Zeta_Core_DynamicVector_Cursor Access_(Zeta_Core_DynamicVector* dv,
                                              size_t idx) {
    if (idx == (size_t)(-1) || idx == dv->size) {
        return (Zeta_Core_DynamicVector_Cursor){ dv, idx, NULL };
    }

    Zeta_Core_CircularArray cur_ca;
    cur_ca.data = dv->cur_data;
    cur_ca.width = sizeof(void*);
    cur_ca.offset = dv->cur_offset;
    cur_ca.size = dv->size_b;
    cur_ca.capacity = dv->cur_capacity;

    Zeta_Core_CircularArray nxt_ca;
    nxt_ca.data = dv->nxt_data;
    nxt_ca.width = sizeof(void*);
    nxt_ca.offset = dv->nxt_offset;
    nxt_ca.size = dv->size_a + dv->size_b + dv->size_c;
    nxt_ca.capacity = dv->nxt_capacity;

    TryMigrate_(dv, &cur_ca, &nxt_ca, 1);

    UpdateCA_(dv, &cur_ca, &nxt_ca);

    size_t offset = dv->offset + idx;

    size_t seg_idx = offset / dv->seg_capacity;
    size_t ele_idx = offset % dv->seg_capacity;

    return (Zeta_Core_DynamicVector_Cursor){
        dv, idx,
        *(void**)((dv->size_a <= seg_idx && seg_idx < dv->size_a + dv->size_b
                       ? Zeta_Core_CircularArray_Access(
                             &cur_ca, seg_idx - dv->size_a, NULL, NULL)
                       : Zeta_Core_CircularArray_Access(&nxt_ca, seg_idx, NULL,
                                                        NULL))) +
            dv->width * ele_idx
    };
}

static void* Push_(void* dv_, void* dst_cursor_, int dir, size_t cnt) {
    Zeta_Core_DynamicVector* dv = dv_;
    CheckCntr_(dv);

    Zeta_Core_DynamicVector_Cursor* dst_cursor = dst_cursor_;

    if (cnt == 0) {
        return dir == 0 ? Zeta_Core_DynamicVector_PeekL(dv, dst_cursor, NULL)
                        : Zeta_Core_DynamicVector_PeekR(dv, dst_cursor, NULL);
    }

    size_t width = dv->width;
    size_t seg_capacity = dv->seg_capacity;

    size_t offset = dv->offset;
    size_t size = dv->size;

    size_t seg_cnt =
        dir == 0
            ? ZETA_Core_CeilIntDiv(cnt + seg_capacity - offset, seg_capacity) -
                  1
            : ZETA_Core_CeilIntDiv(offset + size + cnt, seg_capacity) -
                  ZETA_Core_CeilIntDiv(offset + size, seg_capacity);

    Zeta_Core_CircularArray cur_ca;
    cur_ca.data = dv->cur_data;
    cur_ca.width = sizeof(void*);
    cur_ca.width = sizeof(void*);
    cur_ca.offset = dv->cur_offset;
    cur_ca.size = dv->size_b;
    cur_ca.capacity = dv->cur_capacity;

    Zeta_Core_CircularArray nxt_ca;
    nxt_ca.data = dv->nxt_data;
    nxt_ca.width = sizeof(void*);
    nxt_ca.width = sizeof(void*);
    nxt_ca.offset = dv->nxt_offset;
    nxt_ca.size = dv->size_a + dv->size_b + dv->size_c;
    nxt_ca.capacity = dv->nxt_capacity;

    if (seg_cnt == 0) { goto RET; }

    size_t need_capacity = nxt_ca.size + seg_cnt;
    size_t new_capacity = GetNxtCapacity_(need_capacity);

    if (nxt_ca.data == NULL) {
        if (cur_ca.data == NULL) {
            cur_ca.data = ZETA_Core_Allocator_SafeAllocate(
                dv->data_allocator, 1, sizeof(void*) * new_capacity);
            cur_ca.capacity = new_capacity;
        }

        if (need_capacity <= cur_ca.capacity) {
            if (dir == 0) {
                Zeta_Core_CircularArray_PushL(&cur_ca, seg_cnt, NULL);

                for (size_t i = 0; i < seg_cnt; ++i) {
                    *(void**)Zeta_Core_CircularArray_Access(&cur_ca, i, NULL,
                                                            NULL) =
                        ZETA_Core_Allocator_SafeAllocate(dv->seg_allocator, 1,
                                                         width * seg_capacity);
                }
            } else {
                Zeta_Core_CircularArray_PushR(&cur_ca, seg_cnt, NULL);

                for (size_t i = 0; i < seg_cnt; ++i) {
                    *(void**)Zeta_Core_CircularArray_Access(
                        &cur_ca, cur_ca.size - 1 - i, NULL, NULL) =
                        ZETA_Core_Allocator_SafeAllocate(dv->seg_allocator, 1,
                                                         width * seg_capacity);
                }
            }

            dv->cur_data = cur_ca.data;
            dv->cur_offset = cur_ca.offset;
            dv->cur_capacity = cur_ca.capacity;

            dv->size_b = cur_ca.size;

            goto PUSH_SEG_END;
        }

        nxt_ca.data = ZETA_Core_Allocator_SafeAllocate(
            dv->data_allocator, 1, sizeof(void*) * new_capacity);
        nxt_ca.offset = 0;
        nxt_ca.size = dv->size_b;
        nxt_ca.capacity = new_capacity;
    }

    if (nxt_ca.capacity < need_capacity) {
        Zeta_Core_CircularArray_Assign(&nxt_ca, &cur_ca, dv->size_a, 0,
                                       dv->size_b);

        ZETA_Core_Allocator_Deallocate(dv->data_allocator, cur_ca.data);

        cur_ca.data = nxt_ca.data;
        cur_ca.offset = nxt_ca.offset;
        cur_ca.size = nxt_ca.size;
        cur_ca.capacity = nxt_ca.capacity;

        nxt_ca.data = ZETA_Core_Allocator_SafeAllocate(
            dv->data_allocator, 1, sizeof(void*) * new_capacity);
        nxt_ca.offset = 0;
        nxt_ca.capacity = new_capacity;

        dv->size_a = 0;
        dv->size_b = cur_ca.size;
        dv->size_c = 0;
    }

    if (dir == 0) {
        Zeta_Core_CircularArray_PushL(&nxt_ca, seg_cnt, NULL);

        for (size_t i = 0; i < seg_cnt; ++i) {
            *(void**)Zeta_Core_CircularArray_Access(&nxt_ca, i, NULL, NULL) =
                ZETA_Core_Allocator_SafeAllocate(dv->seg_allocator, 1,
                                                 width * seg_capacity);
        }

        dv->size_a += seg_cnt;
    } else {
        Zeta_Core_CircularArray_PushR(&nxt_ca, seg_cnt, NULL);

        for (size_t i = 0; i < seg_cnt; ++i) {
            *(void**)Zeta_Core_CircularArray_Access(
                &nxt_ca, nxt_ca.size - 1 - i, NULL, NULL) =
                ZETA_Core_Allocator_SafeAllocate(dv->seg_allocator, 1,
                                                 width * seg_capacity);
        }

        dv->size_c += seg_cnt;
    }

PUSH_SEG_END:

    TryMigrate_(dv, &cur_ca, &nxt_ca, seg_cnt);

    UpdateCA_(dv, &cur_ca, &nxt_ca);

RET:

    if (dir == 0) {
        dv->offset = offset =
            (offset + seg_capacity - cnt % seg_capacity) % seg_capacity;
    }

    dv->size = size += cnt;

    size_t ret_offset = dir == 0 ? offset : offset + size - cnt;

    size_t ret_seg_idx = ret_offset / seg_capacity;
    size_t ret_ele_idx = ret_offset % seg_capacity;

    void* ref =
        *(void**)((dv->size_a <= ret_seg_idx &&
                           ret_seg_idx < dv->size_a + dv->size_b
                       ? Zeta_Core_CircularArray_Access(
                             &cur_ca, ret_seg_idx - dv->size_a, NULL, NULL)
                       : Zeta_Core_CircularArray_Access(&nxt_ca, ret_seg_idx,
                                                        NULL, NULL))) +
        dv->width * ret_ele_idx;

    if (dst_cursor != NULL) {
        dst_cursor->dv = dv;
        dst_cursor->idx = ret_offset - offset;
        dst_cursor->ref = ref;
    }

    return ref;
}

static void Pop_(void* dv_, int dir, size_t cnt) {
    Zeta_Core_DynamicVector* dv = dv_;
    CheckCntr_(dv);

    size_t seg_capacity = dv->seg_capacity;

    size_t offset = dv->offset;
    size_t size = dv->size;

    ZETA_Core_DebugAssert(cnt <= size);

    size_t seg_cnt =
        dir == 0 ? (offset + cnt) / seg_capacity
                 : ZETA_Core_CeilIntDiv(offset + size, seg_capacity) -
                       ZETA_Core_CeilIntDiv(offset + size - cnt, seg_capacity);

    Zeta_Core_CircularArray cur_ca;
    cur_ca.data = dv->cur_data;
    cur_ca.width = sizeof(void*);
    cur_ca.width = sizeof(void*);
    cur_ca.offset = dv->cur_offset;
    cur_ca.size = dv->size_b;
    cur_ca.capacity = dv->cur_capacity;

    Zeta_Core_CircularArray nxt_ca;
    nxt_ca.data = dv->nxt_data;
    nxt_ca.width = sizeof(void*);
    nxt_ca.width = sizeof(void*);
    nxt_ca.offset = dv->nxt_offset;
    nxt_ca.size = dv->size_a + dv->size_b + dv->size_c;
    nxt_ca.capacity = dv->nxt_capacity;

    if (seg_cnt == 0) { goto POP_SEG_END; }

    size_t origin_seg_cnt = seg_cnt;

    if (nxt_ca.data != NULL && 0 < seg_cnt) {
        if (dir == 0) {
            size_t cur_cnt = ZETA_Core_GetMinOf(seg_cnt, dv->size_a);
            seg_cnt -= cur_cnt;

            dv->size_a -= cur_cnt;

            for (size_t i = 0; i < cur_cnt; ++i) {
                ZETA_Core_Allocator_Deallocate(
                    dv->seg_allocator, *(void**)Zeta_Core_CircularArray_Access(
                                           &nxt_ca, i, NULL, NULL));
            }

            Zeta_Core_CircularArray_PopL(&nxt_ca, cur_cnt);
        } else {
            size_t cur_seg_cnt = ZETA_Core_GetMinOf(seg_cnt, dv->size_c);
            seg_cnt -= cur_seg_cnt;

            dv->size_c -= cur_seg_cnt;

            for (size_t i = 0; i < cur_seg_cnt; ++i) {
                ZETA_Core_Allocator_Deallocate(
                    dv->seg_allocator,
                    *(void**)Zeta_Core_CircularArray_Access(
                        &nxt_ca, nxt_ca.size - 1 - i, NULL, NULL));
            }

            Zeta_Core_CircularArray_PopR(&nxt_ca, cur_seg_cnt);
        }
    }

    if (0 < seg_cnt) {
        size_t cur_seg_cnt = ZETA_Core_GetMinOf(seg_cnt, dv->size_b);
        seg_cnt -= cur_seg_cnt;

        dv->size_b -= cur_seg_cnt;

        if (dir == 0) {
            for (size_t i = 0; i < cur_seg_cnt; ++i) {
                ZETA_Core_Allocator_Deallocate(
                    dv->seg_allocator, *(void**)Zeta_Core_CircularArray_Access(
                                           &cur_ca, i, NULL, NULL));
            }

            Zeta_Core_CircularArray_PopL(&cur_ca, cur_seg_cnt);

            if (nxt_ca.data != NULL) {
                Zeta_Core_CircularArray_PopL(&nxt_ca, cur_seg_cnt);
            }
        } else {
            for (size_t i = 0; i < cur_seg_cnt; ++i) {
                ZETA_Core_Allocator_Deallocate(
                    dv->seg_allocator,
                    *(void**)Zeta_Core_CircularArray_Access(
                        &cur_ca, cur_ca.size - 1 - i, NULL, NULL));
            }

            Zeta_Core_CircularArray_PopR(&cur_ca, cur_seg_cnt);

            if (nxt_ca.data != NULL) {
                Zeta_Core_CircularArray_PopR(&nxt_ca, cur_seg_cnt);
            }
        }
    }

    if (nxt_ca.data != NULL && 0 < seg_cnt) {
        if (dir == 0) {
            dv->size_a -= seg_cnt;

            for (size_t i = 0; i < seg_cnt; ++i) {
                ZETA_Core_Allocator_Deallocate(
                    dv->seg_allocator, *(void**)Zeta_Core_CircularArray_Access(
                                           &nxt_ca, i, NULL, NULL));
            }

            Zeta_Core_CircularArray_PopL(&nxt_ca, seg_cnt);
        } else {
            dv->size_c -= seg_cnt;

            for (size_t i = 0; i < seg_cnt; ++i) {
                ZETA_Core_Allocator_Deallocate(
                    dv->seg_allocator,
                    *(void**)Zeta_Core_CircularArray_Access(
                        &nxt_ca, nxt_ca.size - 1 - i, NULL, NULL));
            }

            Zeta_Core_CircularArray_PopR(&nxt_ca, seg_cnt);
        }
    }

    if (nxt_ca.data == NULL) {
    } else if (dir == 0) {
        size_t move_cnt = ZETA_Core_GetMinOf(2 * origin_seg_cnt, dv->size_b);

        Zeta_Core_CircularArray_Assign(&nxt_ca, &cur_ca, dv->size_a, 0,
                                       move_cnt);
        Zeta_Core_CircularArray_PopL(&cur_ca, move_cnt);

        dv->size_a += move_cnt;
        dv->size_b -= move_cnt;
    } else {
        size_t move_cnt = ZETA_Core_GetMinOf(2 * origin_seg_cnt, dv->size_b);

        Zeta_Core_CircularArray_Assign(&nxt_ca, &cur_ca,
                                       dv->size_a + dv->size_b - move_cnt,
                                       dv->size_b - move_cnt, move_cnt);
        Zeta_Core_CircularArray_PopR(&cur_ca, move_cnt);

        dv->size_b -= move_cnt;
        dv->size_c += move_cnt;
    }

    if (dv->size_b == 0) {
        dv->size_b = dv->size_a + dv->size_c;
        dv->size_a = 0;
        dv->size_c = 0;

        ZETA_Core_Allocator_Deallocate(dv->data_allocator, cur_ca.data);

        dv->cur_data = nxt_ca.data;
        dv->cur_offset = nxt_ca.offset;
        dv->cur_capacity = nxt_ca.capacity;

        dv->nxt_data = NULL;
        dv->nxt_offset = 0;
        dv->nxt_capacity = 0;
    } else {
        dv->cur_offset = cur_ca.offset;
        dv->nxt_offset = nxt_ca.offset;
    }

POP_SEG_END:

    if (dir == 1) { dv->offset = (offset + cnt) % seg_capacity; }

    dv->size -= cnt;
}

void Zeta_Core_DynamicVector_Init(void* dv_) {
    Zeta_Core_DynamicVector* dv = dv_;
    ZETA_Core_DebugAssert(dv != NULL);

    size_t width = dv->width;
    size_t seg_capacity = dv->seg_capacity;

    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(0 < seg_capacity);

    ZETA_Core_Allocator_Check(dv->data_allocator, alignof(void*));
    ZETA_Core_Allocator_Check(dv->seg_allocator, 1);

    dv->size_a = 0;
    dv->size_b = 0;
    dv->size_c = 0;

    dv->cur_data = NULL;
    dv->cur_offset = 0;
    dv->cur_capacity = 0;

    dv->nxt_data = NULL;
    dv->nxt_offset = 0;
    dv->nxt_capacity = 0;

    dv->offset = 0;
    dv->size = 0;
}

void Zeta_Core_DynamicVector_Deinit(void* dv_) {
    Zeta_Core_DynamicVector* dv = dv_;
    CheckCntr_(dv);

    Zeta_Core_CircularArray cur_ca;
    cur_ca.data = dv->cur_data;
    cur_ca.width = sizeof(void*);
    cur_ca.width = sizeof(void*);
    cur_ca.offset = dv->cur_offset;
    cur_ca.size = dv->size_b;
    cur_ca.capacity = dv->cur_capacity;

    Zeta_Core_CircularArray nxt_ca;
    nxt_ca.data = dv->nxt_data;
    nxt_ca.width = sizeof(void*);
    nxt_ca.width = sizeof(void*);
    nxt_ca.offset = dv->nxt_offset;
    nxt_ca.size = dv->size_a + dv->size_b + dv->size_c;
    nxt_ca.capacity = dv->nxt_capacity;

    if (nxt_ca.data != NULL) {
        for (size_t i = 0; i < dv->size_a; ++i) {
            ZETA_Core_Allocator_Deallocate(
                dv->seg_allocator, *(void**)Zeta_Core_CircularArray_Access(
                                       &nxt_ca, i, NULL, NULL));
        }
    }

    for (size_t i = 0; i < dv->size_b; ++i) {
        ZETA_Core_Allocator_Deallocate(
            dv->seg_allocator,
            *(void**)Zeta_Core_CircularArray_Access(&cur_ca, i, NULL, NULL));
    }

    if (nxt_ca.data != NULL) {
        for (size_t i = 0; i < dv->size_c; ++i) {
            ZETA_Core_Allocator_Deallocate(
                dv->seg_allocator,
                *(void**)Zeta_Core_CircularArray_Access(
                    &cur_ca, dv->size_a + dv->size_b + i, NULL, NULL));
        }
    }

    if (dv->cur_data != NULL) {
        ZETA_Core_Allocator_Deallocate(dv->data_allocator, dv->cur_data);
    }

    if (dv->nxt_data != NULL) {
        ZETA_Core_Allocator_Deallocate(dv->data_allocator, dv->nxt_data);
    }
}

size_t Zeta_Core_DynamicVector_GetWidth(void const* dv_) {
    Zeta_Core_DynamicVector const* dv = dv_;
    CheckCntr_(dv);

    return dv->width;
}

size_t Zeta_Core_DynamicVector_GetSize(void const* dv_) {
    Zeta_Core_DynamicVector const* dv = dv_;
    CheckCntr_(dv);

    return dv->size;
}

void* Zeta_Core_DynamicVector_Refer(void* dv_, void const* pos_cursor_) {
    Zeta_Core_DynamicVector* dv = dv_;
    Zeta_Core_DynamicVector_Cursor const* pos_cursor = pos_cursor_;

    CheckCursor_(dv, pos_cursor);

    return pos_cursor->ref;
}

void Zeta_Core_DynamicVector_GetLBCursor(void const* dv_, void* dst_cursor) {
    Zeta_Core_DynamicVector const* dv = dv_;
    CheckCntr_(dv);

    Zeta_Core_DynamicVector_Access((void*)dv, -1, dst_cursor, NULL);
}

void Zeta_Core_DynamicVector_GetRBCursor(void const* dv_, void* dst_cursor) {
    Zeta_Core_DynamicVector const* dv = dv_;
    CheckCntr_(dv);

    Zeta_Core_DynamicVector_Access((void*)dv, dv->size, dst_cursor, NULL);
}

void* Zeta_Core_DynamicVector_PeekL(void* dv_, void* dst_cursor,
                                    void* dst_elem) {
    Zeta_Core_DynamicVector* dv = dv_;
    CheckCntr_(dv);

    return Zeta_Core_DynamicVector_Access(dv, 0, dst_cursor, dst_elem);
}

void* Zeta_Core_DynamicVector_PeekR(void* dv_, void* dst_cursor,
                                    void* dst_elem) {
    Zeta_Core_DynamicVector* dv = dv_;
    CheckCntr_(dv);

    return Zeta_Core_DynamicVector_Access(dv, dv->size - 1, dst_cursor,
                                          dst_elem);
}

void* Zeta_Core_DynamicVector_Access(void* dv_, size_t idx, void* dst_cursor_,
                                     void* dst_elem) {
    Zeta_Core_DynamicVector* dv = dv_;
    CheckCntr_(dv);

    ZETA_Core_DebugAssert(idx + 1 < dv->size + 2);

    Zeta_Core_DynamicVector_Cursor* dst_cursor = dst_cursor_;

    Zeta_Core_DynamicVector_Cursor pos_cursor =
        Access_((Zeta_Core_DynamicVector*)dv, idx);

    if (dst_cursor != NULL) {
        dst_cursor->dv = pos_cursor.dv;
        dst_cursor->idx = pos_cursor.idx;
        dst_cursor->ref = pos_cursor.ref;
    }

    if (pos_cursor.ref != NULL && dst_elem != NULL) {
        Zeta_Core_MemCopy(dst_elem, pos_cursor.ref, dv->width);
    }

    return pos_cursor.ref;
}

#define Transform_                                                             \
    Zeta_Core_DynamicVector* dv = dv_;                                         \
    Zeta_Core_DynamicVector_Cursor const* pos_cursor = pos_cursor_;            \
                                                                               \
    CheckCursor_(dv, pos_cursor);                                              \
                                                                               \
    ZETA_Core_DebugAssert(pos_cursor->idx <= dv->size);                        \
    ZETA_Core_DebugAssert(cnt <= dv->size - pos_cursor->idx);                  \
                                                                               \
    Zeta_Core_DynamicVector_Cursor* dst_cursor = dst_cursor_;                  \
                                                                               \
    if (cnt == 0) {                                                            \
        if (dst_cursor != NULL) {                                              \
            dst_cursor->dv = pos_cursor->dv;                                   \
            dst_cursor->idx = pos_cursor->idx;                                 \
            dst_cursor->ref = pos_cursor->ref;                                 \
        }                                                                      \
                                                                               \
        return;                                                                \
    }                                                                          \
                                                                               \
    size_t width = dv->width;                                                  \
    size_t seg_capacity = dv->seg_capacity;                                    \
                                                                               \
    Zeta_Core_CircularArray cur_ca;                                            \
    cur_ca.data = dv->cur_data;                                                \
    cur_ca.width = sizeof(void*);                                              \
    cur_ca.width = sizeof(void*);                                              \
    cur_ca.offset = dv->cur_offset;                                            \
    cur_ca.size = dv->size_b;                                                  \
    cur_ca.capacity = dv->cur_capacity;                                        \
                                                                               \
    Zeta_Core_CircularArray nxt_ca;                                            \
    nxt_ca.data = dv->nxt_data;                                                \
    nxt_ca.width = sizeof(void*);                                              \
    nxt_ca.width = sizeof(void*);                                              \
    nxt_ca.offset = dv->nxt_offset;                                            \
    nxt_ca.size = dv->size_a + dv->size_b + dv->size_c;                        \
    nxt_ca.capacity = dv->nxt_capacity;                                        \
                                                                               \
    TryMigrate_(dv, &cur_ca, &nxt_ca, cnt);                                    \
                                                                               \
    UpdateCA_(dv, &cur_ca, &nxt_ca);                                           \
                                                                               \
    size_t offset = dv->offset + pos_cursor->idx;                              \
                                                                               \
    while (0 < cnt) {                                                          \
        size_t seg_idx = offset / seg_capacity;                                \
        size_t ele_idx = offset % seg_capacity;                                \
                                                                               \
        size_t cur_cnt = ZETA_Core_GetMinOf(cnt, seg_capacity - ele_idx);      \
        cnt -= cur_cnt;                                                        \
                                                                               \
        void* seg = *(                                                         \
            void**)(dv->size_a <= seg_idx && seg_idx < dv->size_a + dv->size_b \
                        ? Zeta_Core_CircularArray_Access(                      \
                              &cur_ca, seg_idx - dv->size_a, NULL, NULL)       \
                        : Zeta_Core_CircularArray_Access(&nxt_ca, seg_idx,     \
                                                         NULL, NULL));         \
                                                                               \
        ElemArrTransform(elem_arr_transform_context, seg + width * ele_idx,    \
                         width, width, cur_cnt);                               \
                                                                               \
        offset += cur_cnt;                                                     \
    }                                                                          \
                                                                               \
    if (dst_cursor != NULL) {                                                  \
        dst_cursor->dv = dv;                                                   \
        dst_cursor->idx = offset - dv->offset;                                 \
                                                                               \
        if (dst_cursor->idx == dv->size) {                                     \
            dst_cursor->ref = NULL;                                            \
        } else {                                                               \
            size_t seg_idx = offset / dv->seg_capacity;                        \
            size_t ele_idx = offset % dv->seg_capacity;                        \
                                                                               \
            dst_cursor->ref =                                                  \
                *(void**)((                                                    \
                    dv->size_a <= seg_idx && seg_idx < dv->size_a + dv->size_b \
                        ? Zeta_Core_CircularArray_Access(                      \
                              &cur_ca, seg_idx - dv->size_a, NULL, NULL)       \
                        : Zeta_Core_CircularArray_Access(&nxt_ca, seg_idx,     \
                                                         NULL, NULL))) +       \
                width * ele_idx;                                               \
        }                                                                      \
    }

void Zeta_Core_DynamicVector_Read(void const* dv_, void const* pos_cursor_,
                                  size_t cnt, void* dst, void* dst_cursor_) {
    Zeta_Core_DynamicVector* dv = (Zeta_Core_DynamicVector*)dv_;
    Zeta_Core_DynamicVector_Cursor const* pos_cursor = pos_cursor_;

    CheckCursor_(dv, pos_cursor);

    ZETA_Core_DebugAssert(pos_cursor->idx <= dv->size);
    ZETA_Core_DebugAssert(cnt <= dv->size - pos_cursor->idx);

    Zeta_Core_DynamicVector_Cursor* dst_cursor = dst_cursor_;

    if (cnt == 0) {
        if (dst_cursor != NULL) {
            dst_cursor->dv = pos_cursor->dv;
            dst_cursor->idx = pos_cursor->idx;
            dst_cursor->ref = pos_cursor->ref;
        }

        return;
    }

    size_t width = dv->width;
    size_t seg_capacity = dv->seg_capacity;

    Zeta_Core_CircularArray cur_ca;
    cur_ca.data = dv->cur_data;
    cur_ca.width = sizeof(void*);
    cur_ca.width = sizeof(void*);
    cur_ca.offset = dv->cur_offset;
    cur_ca.size = dv->size_b;
    cur_ca.capacity = dv->cur_capacity;

    Zeta_Core_CircularArray nxt_ca;
    nxt_ca.data = dv->nxt_data;
    nxt_ca.width = sizeof(void*);
    nxt_ca.width = sizeof(void*);
    nxt_ca.offset = dv->nxt_offset;
    nxt_ca.size = dv->size_a + dv->size_b + dv->size_c;
    nxt_ca.capacity = dv->nxt_capacity;

    TryMigrate_(dv, &cur_ca, &nxt_ca, cnt);

    UpdateCA_(dv, &cur_ca, &nxt_ca);

    ZETA_Core_DebugAssert(dst != NULL);

    size_t offset = dv->offset + pos_cursor->idx;

    while (0 < cnt) {
        size_t seg_idx = offset / seg_capacity;
        size_t ele_idx = offset % seg_capacity;

        size_t cur_cnt = ZETA_Core_GetMinOf(cnt, seg_capacity - ele_idx);
        cnt -= cur_cnt;

        void* seg =
            *(void**)(dv->size_a <= seg_idx && seg_idx < dv->size_a + dv->size_b
                          ? Zeta_Core_CircularArray_Access(
                                &cur_ca, seg_idx - dv->size_a, NULL, NULL)
                          : Zeta_Core_CircularArray_Access(&nxt_ca, seg_idx,
                                                           NULL, NULL));

        Zeta_Core_MemCopy(dst, seg + width * ele_idx, width * cur_cnt);

        dst += width * cur_cnt;
        offset += cur_cnt;
    }

    if (dst_cursor != NULL) {
        dst_cursor->dv = dv;
        dst_cursor->idx = offset - dv->offset;

        if (dst_cursor->idx == dv->size) {
            dst_cursor->ref = NULL;
        } else {
            size_t seg_idx = offset / dv->seg_capacity;
            size_t ele_idx = offset % dv->seg_capacity;

            dst_cursor->ref =
                *(void**)((dv->size_a <= seg_idx &&
                                   seg_idx < dv->size_a + dv->size_b
                               ? Zeta_Core_CircularArray_Access(
                                     &cur_ca, seg_idx - dv->size_a, NULL, NULL)
                               : Zeta_Core_CircularArray_Access(
                                     &nxt_ca, seg_idx, NULL, NULL))) +
                width * ele_idx;
        }
    }
}

void Zeta_Core_DynamicVector_Write(void* dv_, void* pos_cursor_, size_t cnt,
                                   void const* src, void* dst_cursor_) {
    Zeta_Core_DynamicVector* dv = dv_;
    Zeta_Core_DynamicVector_Cursor const* pos_cursor = pos_cursor_;

    CheckCursor_(dv, pos_cursor);

    ZETA_Core_DebugAssert(pos_cursor->idx <= dv->size);
    ZETA_Core_DebugAssert(cnt <= dv->size - pos_cursor->idx);

    Zeta_Core_DynamicVector_Cursor* dst_cursor = dst_cursor_;

    if (cnt == 0) {
        if (dst_cursor != NULL) {
            dst_cursor->dv = pos_cursor->dv;
            dst_cursor->idx = pos_cursor->idx;
            dst_cursor->ref = pos_cursor->ref;
        }

        return;
    }

    size_t width = dv->width;
    size_t seg_capacity = dv->seg_capacity;

    Zeta_Core_CircularArray cur_ca;
    cur_ca.data = dv->cur_data;
    cur_ca.width = sizeof(void*);
    cur_ca.width = sizeof(void*);
    cur_ca.offset = dv->cur_offset;
    cur_ca.size = dv->size_b;
    cur_ca.capacity = dv->cur_capacity;

    Zeta_Core_CircularArray nxt_ca;
    nxt_ca.data = dv->nxt_data;
    nxt_ca.width = sizeof(void*);
    nxt_ca.width = sizeof(void*);
    nxt_ca.offset = dv->nxt_offset;
    nxt_ca.size = dv->size_a + dv->size_b + dv->size_c;
    nxt_ca.capacity = dv->nxt_capacity;

    TryMigrate_(dv, &cur_ca, &nxt_ca, cnt);

    UpdateCA_(dv, &cur_ca, &nxt_ca);

    ZETA_Core_DebugAssert(src != NULL);

    size_t offset = dv->offset + pos_cursor->idx;

    while (0 < cnt) {
        size_t seg_idx = offset / seg_capacity;
        size_t ele_idx = offset % seg_capacity;

        size_t cur_cnt = ZETA_Core_GetMinOf(cnt, seg_capacity - ele_idx);
        cnt -= cur_cnt;

        void* seg =
            *(void**)(dv->size_a <= seg_idx && seg_idx < dv->size_a + dv->size_b
                          ? Zeta_Core_CircularArray_Access(
                                &cur_ca, seg_idx - dv->size_a, NULL, NULL)
                          : Zeta_Core_CircularArray_Access(&nxt_ca, seg_idx,
                                                           NULL, NULL));

        Zeta_Core_MemCopy(seg + width * ele_idx, src, width * cur_cnt);

        src += width * cur_cnt;
        offset += cur_cnt;
    }

    if (dst_cursor != NULL) {
        dst_cursor->dv = dv;
        dst_cursor->idx = offset - dv->offset;

        if (dst_cursor->idx == dv->size) {
            dst_cursor->ref = NULL;
        } else {
            size_t seg_idx = offset / dv->seg_capacity;
            size_t ele_idx = offset % dv->seg_capacity;

            dst_cursor->ref =
                *(void**)((dv->size_a <= seg_idx &&
                                   seg_idx < dv->size_a + dv->size_b
                               ? Zeta_Core_CircularArray_Access(
                                     &cur_ca, seg_idx - dv->size_a, NULL, NULL)
                               : Zeta_Core_CircularArray_Access(
                                     &nxt_ca, seg_idx, NULL, NULL))) +
                width * ele_idx;
        }
    }
}

void* Zeta_Core_DynamicVector_PushL(void* dv, size_t cnt, void* dst_cursor) {
    return Push_(dv, dst_cursor, 0, cnt);
}

void* Zeta_Core_DynamicVector_PushR(void* dv, size_t cnt, void* dst_cursor) {
    return Push_(dv, dst_cursor, 1, cnt);
}

void Zeta_Core_DynamicVector_PopL(void* dv, size_t cnt) { Pop_(dv, 0, cnt); }

void Zeta_Core_DynamicVector_PopR(void* dv, size_t cnt) { Pop_(dv, 1, cnt); }

void Zeta_Core_DynamicVector_EraseAll(void* dv_) {
    Zeta_Core_DynamicVector* dv = dv_;
    CheckCntr_(dv);

    Pop_(dv, 0, dv->size);
}

void Zeta_Core_DynamicVector_Check(void const* dv_) {
    Zeta_Core_DynamicVector const* dv = dv_;
    ZETA_Core_DebugAssert(dv != NULL);

    size_t width = dv->width;
    size_t seg_capacity = dv->seg_capacity;

    size_t offset = dv->offset;
    size_t size = dv->size;

    size_t size_a = dv->size_a;
    size_t size_b = dv->size_b;
    size_t size_c = dv->size_c;

    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(0 < seg_capacity);
    ZETA_Core_DebugAssert(offset < seg_capacity);

    ZETA_Core_DebugAssert(ZETA_Core_CeilIntDiv(offset + size, seg_capacity) ==
                          size_a + size_b + size_c);

    Zeta_Core_CircularArray cur_ca;
    cur_ca.data = dv->cur_data;
    cur_ca.width = sizeof(void*);
    cur_ca.width = sizeof(void*);
    cur_ca.offset = dv->cur_offset;
    cur_ca.size = dv->size_b;
    cur_ca.capacity = dv->cur_capacity;

    Zeta_Core_CircularArray nxt_ca;
    nxt_ca.data = dv->nxt_data;
    nxt_ca.width = sizeof(void*);
    nxt_ca.width = sizeof(void*);
    nxt_ca.offset = dv->nxt_offset;
    nxt_ca.size = dv->size_a + dv->size_b + dv->size_c;
    nxt_ca.capacity = dv->nxt_capacity;

    if (cur_ca.data != NULL) { Zeta_Core_CircularArray_Check(&cur_ca); }
    if (nxt_ca.data != NULL) { Zeta_Core_CircularArray_Check(&nxt_ca); }

    if (nxt_ca.data == NULL) {
        ZETA_Core_DebugAssert(size_a == 0);
        ZETA_Core_DebugAssert(size_c == 0);
    } else {
        size_t nxt_size = size_a + size_b + size_c;
        ZETA_Core_DebugAssert(nxt_size <= nxt_ca.capacity);

        ZETA_Core_DebugAssert(size_b <= (nxt_ca.capacity - nxt_size) * 2);
    }

    ZETA_Core_Allocator_Check(dv->data_allocator, alignof(void*));
    ZETA_Core_Allocator_Check(dv->seg_allocator, 1);
}

void Zeta_Core_DynamicVector_Sanitize(void* dv_,
                                      Zeta_Core_MemRecorder* dst_data,
                                      Zeta_Core_MemRecorder* dst_seg) {
    Zeta_Core_DynamicVector* dv = dv_;
    CheckCntr_(dv);

#if !ZETA_Core_EnableDebug
    ZETA_Core_Unused(dst_data);
    ZETA_Core_Unused(dst_seg);
#else
    if (dst_data != NULL) {
        if (dv->cur_data != NULL) {
            Zeta_Core_MemRecorder_Record(dst_data, dv->cur_data,
                                         sizeof(void*) * dv->cur_capacity);
        }

        if (dv->nxt_data != NULL) {
            Zeta_Core_MemRecorder_Record(dst_data, dv->nxt_data,
                                         sizeof(void*) * dv->nxt_capacity);
        }
    }

    size_t width = dv->width;
    size_t seg_capacity = dv->seg_capacity;

    Zeta_Core_CircularArray cur_ca;
    cur_ca.data = dv->cur_data;
    cur_ca.width = sizeof(void*);
    cur_ca.width = sizeof(void*);
    cur_ca.offset = dv->cur_offset;
    cur_ca.size = dv->size_b;
    cur_ca.capacity = dv->cur_capacity;

    Zeta_Core_CircularArray nxt_ca;
    nxt_ca.data = dv->nxt_data;
    nxt_ca.width = sizeof(void*);
    nxt_ca.width = sizeof(void*);
    nxt_ca.offset = dv->nxt_offset;
    nxt_ca.size = dv->size_a + dv->size_b + dv->size_c;
    nxt_ca.capacity = dv->nxt_capacity;

    if (dst_seg == NULL) { return; }

    if (nxt_ca.data != NULL) {
        for (size_t i = 0; i < dv->size_a; ++i) {
            Zeta_Core_MemRecorder_Record(
                dst_seg,
                *(void**)Zeta_Core_CircularArray_Access(&nxt_ca, i, NULL, NULL),
                width * seg_capacity);
        }
    }

    for (size_t i = 0; i < dv->size_b; ++i) {
        Zeta_Core_MemRecorder_Record(
            dst_seg,
            *(void**)Zeta_Core_CircularArray_Access(&cur_ca, i, NULL, NULL),
            width * seg_capacity);
    }

    if (nxt_ca.data != NULL) {
        for (size_t i = 0; i < dv->size_c; ++i) {
            Zeta_Core_MemRecorder_Record(
                dst_seg,
                *(void**)Zeta_Core_CircularArray_Access(
                    &nxt_ca, dv->size_a + dv->size_b + i, NULL, NULL),
                width * seg_capacity);
        }
    }
#endif
}

bool_t Zeta_Core_DynamicVector_Cursor_AreEqual(void const* dv_,
                                               void const* cursor_a_,
                                               void const* cursor_b_) {
    Zeta_Core_DynamicVector const* dv = dv_;

    Zeta_Core_DynamicVector_Cursor const* cursor_a = cursor_a_;
    Zeta_Core_DynamicVector_Cursor const* cursor_b = cursor_b_;

    CheckCursor_(dv, cursor_a);
    CheckCursor_(dv, cursor_b);

    return cursor_a->idx == cursor_b->idx;
}

int Zeta_Core_DynamicVector_Cursor_Compare(void const* dv_,
                                           void const* cursor_a_,
                                           void const* cursor_b_) {
    Zeta_Core_DynamicVector const* dv = dv_;

    Zeta_Core_DynamicVector_Cursor const* cursor_a = cursor_a_;
    Zeta_Core_DynamicVector_Cursor const* cursor_b = cursor_b_;

    CheckCursor_(dv, cursor_a);
    CheckCursor_(dv, cursor_b);

    return ZETA_Core_ThreeWayCompare(cursor_a->idx + 1, cursor_b->idx + 1);
}

size_t Zeta_Core_DynamicVector_Cursor_GetDist(void const* dv_,
                                              void const* cursor_a_,
                                              void const* cursor_b_) {
    Zeta_Core_DynamicVector const* dv = dv_;

    Zeta_Core_DynamicVector_Cursor const* cursor_a = cursor_a_;
    Zeta_Core_DynamicVector_Cursor const* cursor_b = cursor_b_;

    CheckCursor_(dv, cursor_a);
    CheckCursor_(dv, cursor_b);

    return cursor_b->idx - cursor_a->idx;
}

size_t Zeta_Core_DynamicVector_Cursor_GetIdx(void const* dv_,
                                             void const* cursor_) {
    Zeta_Core_DynamicVector const* dv = dv_;
    Zeta_Core_DynamicVector_Cursor const* cursor = cursor_;
    CheckCursor_(dv, cursor);

    return cursor->idx;
}

void Zeta_Core_DynamicVector_Cursor_StepL(void const* dv, void* cursor) {
    Zeta_Core_DynamicVector_Cursor_AdvanceL(dv, cursor, 1);
}

void Zeta_Core_DynamicVector_Cursor_StepR(void const* dv, void* cursor) {
    Zeta_Core_DynamicVector_Cursor_AdvanceR(dv, cursor, 1);
}

void Zeta_Core_DynamicVector_Cursor_AdvanceL(void const* dv_, void* cursor_,
                                             size_t step) {
    Zeta_Core_DynamicVector const* dv = dv_;
    Zeta_Core_DynamicVector_Cursor* cursor = cursor_;
    CheckCursor_(dv, cursor);

    if (step == 0) { return; }

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    Zeta_Core_DynamicVector_Access((void*)dv, cursor->idx - step, cursor, NULL);
}

void Zeta_Core_DynamicVector_Cursor_AdvanceR(void const* dv_, void* cursor_,
                                             size_t step) {
    Zeta_Core_DynamicVector const* dv = dv_;
    Zeta_Core_DynamicVector_Cursor* cursor = cursor_;

    CheckCursor_(dv, cursor);

    if (step == 0) { return; }

    ZETA_Core_DebugAssert(step <= dv->size - cursor->idx);

    Zeta_Core_DynamicVector_Access((void*)dv, cursor->idx + step, cursor, NULL);
}

void Zeta_Core_DynamicVector_Cursor_Check(void const* dv_,
                                          void const* cursor_) {
    Zeta_Core_DynamicVector const* dv = dv_;
    CheckCntr_(dv);

    Zeta_Core_DynamicVector_Cursor const* cursor = cursor_;
    ZETA_Core_DebugAssert(cursor != NULL);

    Zeta_Core_DynamicVector_Cursor re_cursor;
    Zeta_Core_DynamicVector_Access((void*)dv, cursor->idx, &re_cursor, NULL);

    ZETA_Core_DebugAssert(re_cursor.dv == cursor->dv);
    ZETA_Core_DebugAssert(re_cursor.idx == cursor->idx);
    ZETA_Core_DebugAssert(re_cursor.ref == cursor->ref);
}

Zeta_Core_SeqCntr_VTable const Zeta_Core_DynamicVector_seq_cntr_vtable = {
    .cursor_size = sizeof(Zeta_Core_DynamicVector_Cursor),

    .Deinit = Zeta_Core_DynamicVector_Deinit,

    .GetWidth = Zeta_Core_DynamicVector_GetWidth,

    .GetSize = Zeta_Core_DynamicVector_GetSize,

    .GetLBCursor = Zeta_Core_DynamicVector_GetLBCursor,

    .GetRBCursor = Zeta_Core_DynamicVector_GetRBCursor,

    .PeekL = Zeta_Core_DynamicVector_PeekL,

    .PeekR = Zeta_Core_DynamicVector_PeekR,

    .Access = Zeta_Core_DynamicVector_Access,

    .Refer = Zeta_Core_DynamicVector_Refer,

    .Read = Zeta_Core_DynamicVector_Read,

    .Write = Zeta_Core_DynamicVector_Write,

    .PushL = Zeta_Core_DynamicVector_PushL,

    .PushR = Zeta_Core_DynamicVector_PushR,

    .PopL = Zeta_Core_DynamicVector_PopL,

    .PopR = Zeta_Core_DynamicVector_PopR,

    .EraseAll = Zeta_Core_DynamicVector_EraseAll,

    .Cursor_AreEqual = Zeta_Core_DynamicVector_Cursor_AreEqual,

    .Cursor_Compare = Zeta_Core_DynamicVector_Cursor_Compare,

    .Cursor_GetDist = Zeta_Core_DynamicVector_Cursor_GetDist,

    .Cursor_GetIdx = Zeta_Core_DynamicVector_Cursor_GetIdx,

    .Cursor_StepL = Zeta_Core_DynamicVector_Cursor_StepL,

    .Cursor_StepR = Zeta_Core_DynamicVector_Cursor_StepR,

    .Cursor_AdvanceL = Zeta_Core_DynamicVector_Cursor_AdvanceL,

    .Cursor_AdvanceR = Zeta_Core_DynamicVector_Cursor_AdvanceR,
};
