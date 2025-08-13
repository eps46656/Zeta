#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_AssocCntr_VTable);
ZETA_Core_DeclareStruct(Zeta_Core_AssocCntr);

/**
 * @brief The interface of associative container.
 */
struct Zeta_Core_AssocCntr_VTable {
    /**
     * @brief The number of bytes occupied by cursor.
     */
    size_t cursor_size;

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
    void* (*PeekL)(void* context, void* dst_cursor);

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
    void* (*PeekR)(void* context, void* dst_cursor);

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

    void* (*Find)(void* context, void const* key, Zeta_Core_Hash KeyHash,
                  void const* key_hash_context,
                  Zeta_Core_Compare KeyElemCompare,
                  void const* key_elem_cmp_context, void* dst_cursor);

    void* (*Insert)(void* context, void const* elem, void* dst_cursor);

    void (*Erase)(void* context, void* pos_cursor);

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

struct Zeta_Core_AssocCntr {
    Zeta_Core_AssocCntr_VTable const* vtable;
    void* context;
};

#define ZETA_Core_AssocCntr_Call__(tmp_assoc_cntr, assoc_cntr, member_func, \
                                   ...)                                     \
    ({                                                                      \
        ZETA_Core_AutoVar(tmp_assoc_cntr, (assoc_cntr));                    \
        ZETA_Core_FuncPtrTable_Call(tmp_assoc_cntr.vtable, member_func,     \
                                    tmp_assoc_cntr.context, __VA_ARGS__);   \
    })

#define ZETA_Core_AssocCntr_Call_(assoc_cntr, member_func, ...)              \
    ZETA_Core_AssocCntr_Call__(ZETA_Core_TmpName, (assoc_cntr), member_func, \
                               __VA_ARGS__)

#define ZETA_Core_AssocCntr_AllocaCursor_(tmp_assoc_cntr, assoc_cntr)     \
    ({                                                                    \
        ZETA_Core_AutoVar(tmp_assoc_cntr, (assoc_cntr));                  \
                                                                          \
        ZETA_Core_DebugAssert(tmp_assoc_cntr.vtable != NULL);             \
                                                                          \
        __builtin_alloca_with_align(tmp_assoc_cntr.vtable->cursor_size,   \
                                    __CHAR_BIT__ * alignof(max_align_t)); \
    })

#define ZETA_Core_AssocCntr_AllocaCursor(assoc_cntr) \
    ZETA_Core_AssocCntr_AllocaCursor_(ZETA_Core_TmpName, (assoc_cntr))

#define ZETA_Core_AssocCntr_Deinit(assoc_cntr) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), Deinit)

#define ZETA_Core_AssocCntr_GetWidth(assoc_cntr) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), GetWidth)

#define ZETA_Core_AssocCntr_GetSize(assoc_cntr) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), GetSize)

#define ZETA_Core_AssocCntr_GetCapacity(assoc_cntr) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), GetCapacity)

#define ZETA_Core_AssocCntr_GetLBCursor(assoc_cntr, dst_cursor)         \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), GetLBCursor, (dst_cursor)); \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_AssocCntr_GetRBCursor(assoc_cntr, dst_cursor)         \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), GetRBCursor, (dst_cursor)); \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_AssocCntr_PeekL(assoc_cntr, dst_cursor) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), PeekL, (dst_cursor))

#define ZETA_Core_AssocCntr_ConstPeekL(assoc_cntr, dst_cursor) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), PeekL, (dst_cursor))

#define ZETA_Core_AssocCntr_PeekR(assoc_cntr, dst_cursor) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), PeekR, (dst_cursor))

#define ZETA_Core_AssocCntr_ConstPeekR(assoc_cntr, dst_cursor) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), PeekR, (dst_cursor))

#define ZETA_Core_AssocCntr_Refer(assoc_cntr, pos_cursor) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), Refer, (pos_cursor))

#define ZETA_Core_AssocCntr_ConstRefer(assoc_cntr, pos_cursor) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), Refer, (pos_cursor))

#define ZETA_Core_AssocCntr_Find(assoc_cntr, key, KeyHash, key_hash_context, \
                                 KeyElemCompare, key_elem_cmp_context,       \
                                 dst_cursor)                                 \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), Find, (key), (KeyHash),          \
                              (key_hash_context), (KeyElemCompare),          \
                              (key_elem_cmp_context), (dst_cursor))

#define ZETA_Core_AssocCntr_ConstFind(assoc_cntr, key, dst_cursor) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), Find, (key), (dst_cursor))

#define ZETA_Core_AssocCntr_Insert(assoc_cntr, elem, dst_cursor) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), Insert, (elem), (dst_cursor))

#define ZETA_Core_AssocCntr_Erase(assoc_cntr, pos_cursor)         \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), Erase, (pos_cursor)); \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_AssocCntr_EraseAll(assoc_cntr)       \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), EraseAll); \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_AssocCntr_Cursor_AreEqual(assoc_cntr, cursor_a, cursor_b) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), Cursor_AreEqual, (cursor_a),    \
                              (cursor_b))

#define ZETA_Core_AssocCntr_Cursor_Compare(assoc_cntr, cursor_a, cursor_b) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), Cursor_Compare, (cursor_a),    \
                              (cursor_b))

#define ZETA_Core_AssocCntr_Cursor_GetDist(assoc_cntr, cursor_a, cursor_b) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), Cursor_GetDist, (cursor_a),    \
                              (cursor_b))

#define ZETA_Core_AssocCntr_Cursor_GetIdx(assoc_cntr, cursor) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), Cursor_GetIdx, (cursor))

#define ZETA_Core_AssocCntr_Cursor_StepL(assoc_cntr, cursor) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), Cursor_StepL, (cursor))

#define ZETA_Core_AssocCntr_Cursor_StepR(assoc_cntr, cursor) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), Cursor_StepR, (cursor))

#define ZETA_Core_AssocCntr_Cursor_AdvanceL(assoc_cntr, cursor, idx) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), Cursor_AdvanceL, (cursor), (idx))

#define ZETA_Core_AssocCntr_Cursor_AdvanceR(assoc_cntr, cursor, idx) \
    ZETA_Core_AssocCntr_Call_((assoc_cntr), Cursor_AdvanceR, (cursor), (idx))

ZETA_Core_ExternC_End;
