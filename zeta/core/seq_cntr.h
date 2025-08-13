#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_SeqCntr_VTable);
ZETA_Core_DeclareStruct(Zeta_Core_SeqCntr);

/**
 * @brief The interface of sequential container.
 */
struct Zeta_Core_SeqCntr_VTable {
    /**
     * @brief The number of bytes occupied by cursor.
     */
    size_t cursor_size;

    /**
     * @brief Deinitialize the container. Concrete container should implement
     *        this function.
     *
     * @param context The context of the container.
     */
    void (*Deinit)(void* context);

    /**
     * @brief The byte offset of two adjacent elements in an element array.
     *        Concrete container should implement this function.
     *
     * @param context The context of the container.
     */
    size_t (*GetWidth)(void const* context);

    /**
     * @brief The number of element in the container. Concrete container should
     *        implement this function.
     *
     * @param context The context of the container.
     */
    size_t (*GetSize)(void const* context);

    /**
     * @brief The maximum number of element can be contained by the container.
     *        Concrete container should implement this function.
     *
     * @param context The context of the container.
     */
    size_t (*GetCapacity)(void const* context);

    /**
     * @brief Get the left cursor. Concrete container can implement this
     *        function optionally.
     *
     * @param context The context of the container.
     * @param dst_cursor The destination of returned cursor.
     */
    void (*GetLBCursor)(void const* context, void* dst_cursor);

    /**
     * @brief Get the right cursor. Concrete container should implement this
     *        function.
     *
     * @param context The context of the container.
     * @param dst_cursor The destination of returned cursor.
     */
    void (*GetRBCursor)(void const* context, void* dst_cursor);

    /**
     * @brief Peek the left-most element. Concrete container can implement this
     *        function optionally.
     *
     * @param context The context of the container.
     * @param dst_cursor Optional. The destination of returned cursor.
     * @param dst_elem Optional. The destination of returned element.
     *
     * @return The pointer to target element if it can be referred. Otherwise,
     * NULL.
     */
    void* (*PeekL)(void* context, void* dst_cursor, void* dst_elem);

    /**
     * @brief Peek the right-most element. Concrete container should implement
     *        this function.
     *
     * @param context The context of the container.
     * @param dst_cursor Optional. The destination of returned cursor.
     * @param dst_elem Optional. The destination of returned element.
     *
     * @return The pointer to target element if it can be referred. Otherwise,
     * NULL.
     */
    void* (*PeekR)(void* context, void* dst_cursor, void* dst_elem);

    /**
     * @brief Access element by index. Concrete container can implement this
     *        function optionally.
     *
     * @param context The context of the container.
     * @param idx The index of target element.
     * @param dst_cursor Optional. The destination of returned cursor.
     * @param dst_elem Optional. The destination of returned element.
     *
     * @return The pointer to target element if it can be referred. Otherwise,
     * NULL.
     */
    void* (*Access)(void* context, size_t idx, void* dst_cursor,
                    void* dst_elem);

    /**
     * @brief Get the pointer of element pointed by cursor. Concrete container
     *        should implement this function.
     *
     * @param context The context of the container.
     * @param pos_cursor The cursor pointing to target element.
     *
     * @return The pointer to target element if it can be referred. Otherwise,
     *         NULL.
     */
    void* (*Refer)(void* context, void const* pos_cursor);

    /**
     * @brief Read \p cnt elements from cursor. Concrete container should
     *        implement this function.
     *
     * @param context The context of the container.
     * @param pos_cursor The cursor pointing to target element. Will be renewed
     *                   to prevent invalid.
     * @param cnt The number of read elements.
     * @param dst The destination of read elements.
     * @param dst_stride The stride of read elements.
     * @param dst_cursor Optional. The destination of returned cursor pointing
     *                   right next of last read elements.
     */
    void (*Read)(void const* context, void const* pos_cursor, size_t cnt,
                 void* dst, size_t dst_stride, void* dst_cursor);

    /**
     * @brief Write \p cnt elements from cursor. Concrete container should
     *        implement this function.
     *
     * @param context The context of the container.
     * @param pos_cursor The cursor pointing to target element. Will be renewed
     *                   to prevent invalid.
     * @param cnt The number of written elements.
     * @param src The source of written elements.
     * @param src_stride The stride of written elements.
     * @param dst_cursor Optional. The destination of returned cursor pointing
     *                   right next of last written elements.
     */
    void (*Write)(void* context, void* pos_cursor, size_t cnt, void const* src,
                  size_t src_stride, void* dst_cursor);

    /**
     * @brief Push \p cnt elements from left end. Concrete container can
     *        implement this function optionally.
     *
     * @param context The context of the container.
     * @param cnt The number of pushing elements.
     * @param dst_cursor Optional. The destination of returned cursor which
     *                   points to the first pushed element.
     *
     * @return The pointer pointing the first pushed element if it can be
     *         referred. Otherwise, NULL.
     */
    void* (*PushL)(void* context, size_t cnt, void* dst_cursor);

    /**
     * @brief Push \p cnt elements from right end. Concrete container should
     *        implement this function.
     *
     * @param context The context of the container.
     * @param cnt The number of pushing elements.
     * @param dst_cursor Optional. The destination of returned cursor which
     *                   points to the first pushed element.
     *
     * @return The pointer pointing the first pushed element if it can be
     *         referred. Otherwise, NULL.
     */
    void* (*PushR)(void* context, size_t cnt, void* dst_cursor);

    /**
     * @brief Insert \p cnt elements at \p pos_cursor. Concrete container can
     *        implement this function optionally.
     *
     * @param context The context of the container.
     * @param pos_cursor The cursor pointing to inserting point. Will be renewed
     *                   to prevent invalid.
     * @param cnt The number of inserting elements.
     *
     * @return The pointer pointing the first inserted element if it can be
     *         referred. Otherwise, NULL.
     */
    void* (*Insert)(void* context, void* pos_cursor, size_t cnt);

    /**
     * @brief Pop \p cnt elements from left end. Concrete container can
     *        implement this function optionally.
     *
     * @param context The context of the container.
     * @param cnt The number of popped elements.
     */
    void (*PopL)(void* context, size_t cnt);

    /**
     * @brief Pop \p cnt elements from right end. Concrete container should
     *        implement this function.
     *
     * @param context The context of the container.
     * @param cnt The number of popped elements.
     */
    void (*PopR)(void* context, size_t cnt);

    /**
     * @brief Erase \p cnt elements at \p pos_cursor. Concrete container can
     *        implement this function optionally.
     *
     * @param context The context of the container.
     * @param pos_cursor The cursor pointing to erasing point. Will be renewed
     *                   to prevent invalid.
     * @param cnt The number of erasing elements.
     */
    void (*Erase)(void* context, void* pos_cursor, size_t cnt);

    /**
     * @brief Erase all elements in the container. Concrete container should
     *        implement this function.
     *
     * @param context The context of the container.
     */
    void (*EraseAll)(void* context);

    /**
     * @brief Determine if \p cursor_a and \p cursor_b point to the same
     *        position. Concrete container should implement this function.
     *
     * @param context The context of the container.
     * @param cursor_a The first target cursor.
     * @param cursor_b The second target cursor.
     *
     * @return If \p cursor_a and \p cursor_b point to the same position.
     */
    bool_t (*Cursor_AreEqual)(void const* context, void const* cursor_a,
                              void const* cursor_b);

    /**
     * @brief Compare the direction between \p cursor_a and \p cursor_b.
     *        Concrete container can implement this function optionally.
     *
     * @param context The context of the container.
     * @param cursor_a The first target cursor.
     * @param cursor_b The second target cursor.
     *
     * @return Negative, if \p cursor_a is to the left of \p cursor_b. Positive,
     *         if \p cursor_a is to the right of \p cursor_b. Zero, if
     *         \p cursor_a and \p cursor_b point to the same position.
     */
    int (*Cursor_Compare)(void const* context, void const* cursor_a,
                          void const* cursor_b);

    /**
     * @brief Get the distance from \p cursor_a to \p cursor_b. Concrete
     *        container can implement this function optionally.
     *
     * @param context The context of the container.
     * @param cursor_a The first target cursor.
     * @param cursor_b The second target cursor.
     *
     * @return The distance from \p cursor_a to \p cursor_b.
     */
    size_t (*Cursor_GetDist)(void const* context, void const* cursor_a,
                             void const* cursor_b);

    /**
     * @brief Get the index pointed by \p cursor. Concrete container can
     *        implement this function optionally.
     *
     * @param context The context of the container.
     * @param cursor The target cursor.
     *
     * @return The index pointed by \p cursor.
     */
    size_t (*Cursor_GetIdx)(void const* context, void const* cursor);

    /**
     * @brief Step \p cursor left. Concrete container can implement this
     *        function optionally.
     *
     * @param context The context of the container.
     * @param cursor The target cursor.
     */
    void (*Cursor_StepL)(void const* context, void* cursor);

    /**
     * @brief Step \p cursor right. Concrete container should implement this
     *        function.
     *
     * @param context The context of the container.
     * @param cursor The target cursor.
     */
    void (*Cursor_StepR)(void const* context, void* cursor);

    /**
     * @brief Advance cursor left \p step. Concrete container can implement
     *        this function optionally.
     *
     * @param context The context of the container.
     * @param cursor The target cursor.
     * @param step The number of steps to advance cursor.
     */
    void (*Cursor_AdvanceL)(void const* context, void* cursor, size_t step);

    /**
     * @brief Advance \p cursor right \p step. Concrete container can implement
     *        this function optionally.
     *
     * @param context The context of the container.
     * @param cursor The target cursor.
     * @param step The number of steps to advance cursor.
     */
    void (*Cursor_AdvanceR)(void const* context, void* cursor, size_t step);
};

struct Zeta_Core_SeqCntr {
    Zeta_Core_SeqCntr_VTable const* vtable;
    void* context;
};

#define ZETA_Core_SeqCntr_Call__(tmp_seq_cntr, seq_cntr, member_func, ...) \
    ({                                                                     \
        ZETA_Core_AutoVar(tmp_seq_cntr, (seq_cntr));                       \
        ZETA_Core_FuncPtrTable_Call(tmp_seq_cntr.vtable, member_func,      \
                                    tmp_seq_cntr.context, __VA_ARGS__);    \
    })

#define ZETA_Core_SeqCntr_Call_(seq_cntr, member_func, ...)              \
    ZETA_Core_SeqCntr_Call__(ZETA_Core_TmpName, (seq_cntr), member_func, \
                             __VA_ARGS__)

#define ZETA_Core_SeqCntr_AllocaCursor_(tmp_seq_cntr, seq_cntr)           \
    ({                                                                    \
        ZETA_Core_AutoVar(tmp_seq_cntr, (seq_cntr));                      \
                                                                          \
        ZETA_Core_DebugAssert(tmp_seq_cntr.vtable != NULL);               \
                                                                          \
        __builtin_alloca_with_align(tmp_seq_cntr.vtable->cursor_size,     \
                                    __CHAR_BIT__ * alignof(max_align_t)); \
    })

#define ZETA_Core_SeqCntr_AllocaCursor(seq_cntr) \
    ZETA_Core_SeqCntr_AllocaCursor_(ZETA_Core_TmpName, (seq_cntr))

#define ZETA_Core_SeqCntr_Deinit(seq_cntr) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), Deinit)

#define ZETA_Core_SeqCntr_GetWidth(seq_cntr) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), GetWidth)

#define ZETA_Core_SeqCntr_GetSize(seq_cntr) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), GetSize)

#define ZETA_Core_SeqCntr_GetCapacity(seq_cntr) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), GetCapacity)

#define ZETA_Core_SeqCntr_GetLBCursor(seq_cntr, dst_cursor)         \
    ZETA_Core_SeqCntr_Call_((seq_cntr), GetLBCursor, (dst_cursor)); \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_SeqCntr_GetRBCursor(seq_cntr, dst_cursor)         \
    ZETA_Core_SeqCntr_Call_((seq_cntr), GetRBCursor, (dst_cursor)); \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_SeqCntr_PeekL(seq_cntr, dst_cursor, dst_elem) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), PeekL, (dst_cursor), (dst_elem))

#define ZETA_Core_SeqCntr_ConstPeekL(seq_cntr, dst_cursor, dst_elem)       \
    ((void const*)ZETA_Core_SeqCntr_Call_((seq_cntr), PeekL, (dst_cursor), \
                                          (dst_elem)))

#define ZETA_Core_SeqCntr_PeekR(seq_cntr, dst_cursor, dst_elem) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), PeekR, (dst_cursor), (dst_elem))

#define ZETA_Core_SeqCntr_ConstPeekR(seq_cntr, dst_cursor, dst_elem)       \
    ((void const*)ZETA_Core_SeqCntr_Call_((seq_cntr), PeekR, (dst_cursor), \
                                          (dst_elem)))

#define ZETA_Core_SeqCntr_Access(seq_cntr, idx, dst_cursor, dst_elem) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), Access, (idx), (dst_cursor), (dst_elem))

#define ZETA_Core_SeqCntr_ConstAccess(seq_cntr, idx, dst_cursor, dst_elem) \
    ((void const*)ZETA_Core_SeqCntr_Call_((seq_cntr), Access, (idx),       \
                                          (dst_cursor), (dst_elem)))

#define ZETA_Core_SeqCntr_Refer(seq_cntr, pos_cursor) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), Refer, (pos_cursor))

#define ZETA_Core_SeqCntr_ConstRefer(seq_cntr, pos_cursor) \
    ((void const*)ZETA_Core_SeqCntr_Call_((seq_cntr), Refer, (pos_cursor)))

#define ZETA_Core_SeqCntr_Read(seq_cntr, pos_cursor, cnt, dst, dst_stride, \
                               dst_cursor)                                 \
    ZETA_Core_SeqCntr_Call_((seq_cntr), Read, (pos_cursor), (cnt), (dst),  \
                            (dst_stride), (dst_cursor))

#define ZETA_Core_SeqCntr_Write(seq_cntr, pos_cursor, cnt, src, src_stride, \
                                dst_cursor)                                 \
    ZETA_Core_SeqCntr_Call_((seq_cntr), Write, (pos_cursor), (cnt), (src),  \
                            (src_stride), (dst_cursor))

#define ZETA_Core_SeqCntr_PushL(seq_cntr, cnt, dst_cursor) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), PushL, (cnt), (dst_cursor));

#define ZETA_Core_SeqCntr_PushR(seq_cntr, cnt, dst_cursor) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), PushR, (cnt), (dst_cursor));

#define ZETA_Core_SeqCntr_Insert(seq_cntr, pos_cursor, cnt) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), Insert, (pos_cursor), (cnt))

#define ZETA_Core_SeqCntr_PopL(seq_cntr, cnt)         \
    ZETA_Core_SeqCntr_Call_((seq_cntr), PopL, (cnt)); \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_SeqCntr_PopR(seq_cntr, cnt)         \
    ZETA_Core_SeqCntr_Call_((seq_cntr), PopR, (cnt)); \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_SeqCntr_Erase(seq_cntr, pos_cursor, cnt)           \
    ZETA_Core_SeqCntr_Call_((seq_cntr), Erase, (pos_cursor), (cnt)); \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_SeqCntr_EraseAll(seq_cntr)       \
    ZETA_Core_SeqCntr_Call_((seq_cntr), EraseAll); \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_SeqCntr_Cursor_AreEqual(seq_cntr, cursor_a, cursor_b) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), Cursor_AreEqual, (cursor_a), (cursor_b))

#define ZETA_Core_SeqCntr_Cursor_Compare(seq_cntr, cursor_a, cursor_b) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), Cursor_Compare, (cursor_a), (cursor_b))

#define ZETA_Core_SeqCntr_Cursor_GetDist(seq_cntr, cursor_a, cursor_b) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), Cursor_GetDist, (cursor_a), (cursor_b))

#define ZETA_Core_SeqCntr_Cursor_GetIdx(seq_cntr, cursor) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), Cursor_GetIdx, (cursor))

#define ZETA_Core_SeqCntr_Cursor_StepL(seq_cntr, cursor) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), Cursor_StepL, (cursor))

#define ZETA_Core_SeqCntr_Cursor_StepR(seq_cntr, cursor) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), Cursor_StepR, (cursor))

#define ZETA_Core_SeqCntr_Cursor_AdvanceL(seq_cntr, cursor, step) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), Cursor_AdvanceL, (cursor), (step))

#define ZETA_Core_SeqCntr_Cursor_AdvanceR(seq_cntr, cursor, step) \
    ZETA_Core_SeqCntr_Call_((seq_cntr), Cursor_AdvanceR, (cursor), (step))

// -----------------------------------------------------------------------------

#define ZETA_Core_SeqCntr_CheckInsertable_(                             \
    tmp_idx, tmp_cnt, tmp_size, tmp_capacity, idx, cnt, size, capacity) \
    {                                                                   \
        size_t tmp_idx = (size_t)(idx);                                 \
        size_t tmp_cnt = (size_t)(cnt);                                 \
        size_t tmp_size = (size_t)(size);                               \
        size_t tmp_capacity = (size_t)(capacity);                       \
                                                                        \
        ZETA_Core_DebugAssert(tmp_idx <= tmp_size);                     \
        ZETA_Core_DebugAssert(tmp_size <= tmp_capacity);                \
        ZETA_Core_DebugAssert(tmp_cnt <= tmp_capacity - tmp_size);      \
    }                                                                   \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_SeqCntr_CheckInsertable(idx, cnt, size, capacity)          \
    ZETA_Core_SeqCntr_CheckInsertable_(ZETA_Core_TmpName, ZETA_Core_TmpName, \
                                       ZETA_Core_TmpName, ZETA_Core_TmpName, \
                                       (idx), (cnt), (size), (capacity))

#define ZETA_Core_SeqCntr_CheckErasable_(tmp_idx, tmp_cnt, tmp_size, idx, cnt, \
                                         size)                                 \
    {                                                                          \
        size_t tmp_idx = (size_t)(idx);                                        \
        size_t tmp_cnt = (size_t)(cnt);                                        \
        size_t tmp_size = (size_t)(size);                                      \
                                                                               \
        ZETA_Core_DebugAssert(tmp_idx <= tmp_size);                            \
        ZETA_Core_DebugAssert(tmp_cnt <= tmp_size - tmp_idx);                  \
    }                                                                          \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_SeqCntr_CheckErasable(idx, cnt, size)                    \
    ZETA_Core_SeqCntr_CheckErasable_(ZETA_Core_TmpName, ZETA_Core_TmpName, \
                                     ZETA_Core_TmpName, (idx), (cnt), (size))

// -----------------------------------------------------------------------------

/**
 * @brief Resize the container with push left or pop left.
 *
 * @param seq_cntr The target container.
 * @param size The number of elements in \p seq_cntr after resizing.
 */
void Zeta_Core_SeqCntr_ResizeL(Zeta_Core_SeqCntr seq_cntr, size_t size);

/**
 * @brief Resize the container with push right or pop right.
 *
 * @param seq_cntr The target container.
 * @param size The number of elements in \p seq_cntr after resizing.
 */
void Zeta_Core_SeqCntr_ResizeR(Zeta_Core_SeqCntr seq_cntr, size_t size);

/**
 * @brief Assign \p src_seq_cntr 's range to \p dst_seq_cntr 's range.
 *
 * @param dst_seq_cntr The destination container.
 * @param src_seq_cntr The source container.
 */
void Zeta_Core_SeqCntr_RangeAssign(Zeta_Core_SeqCntr dst_seq_cntr,
                                   Zeta_Core_SeqCntr src_seq_cntr,
                                   size_t dst_idx, size_t src_idx, size_t cnt);

/**
 * @brief Assign \p src_seq_cntr to \p dst_seq_cntr.
 *
 * @param dst_seq_cntr The destination container.
 * @param src_seq_cntr The source container.
 */
void Zeta_Core_SeqCntr_Assign(Zeta_Core_SeqCntr dst_seq_cntr,
                              Zeta_Core_SeqCntr src_seq_cntr);

void Zeta_Core_SeqCntr_NaiveInsert(Zeta_Core_SeqCntr seq_cntr, size_t idx,
                                   size_t cnt);

ZETA_Core_ExternC_End;
