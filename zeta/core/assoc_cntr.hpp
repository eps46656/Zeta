#pragma once

#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

#define ZETA_Core_AssocCntr_AllocaCursor_(tmp_cntr, cntr)                 \
    ({                                                                    \
        auto tmp_cntr{ cntr };                                            \
        ZETA_Core_DebugAssert(tmp_cntr != nullptr);                       \
                                                                          \
        __builtin_alloca_with_align(tmp_cntr->cursor_size,                \
                                    __CHAR_BIT__ * alignof(max_align_t)); \
    })

#define ZETA_Core_AssocCntr_AllocaCursor(cntr) \
    ZETA_Core_AssocCntr_AllocaCursor_(ZETA_Core_TmpName, cntr)

namespace zeta::core::assoc_cntr {

using FnHash = FunctionRef<unsigned long long(void const*, unsigned long long)>;
using FnCompare = FunctionRef<int(void const*, void const*)>;

// -----------------------------------------------------------------------------

template <typename AssocCntr>
bool CheckAssocCntr();

// -----------------------------------------------------------------------------

struct AssocCntrVTable {
    void (*Deinit)(void* cntr);

    // -------------------------------------------------------------------------

    size_t (*GetSize)(void const* cntr);

    size_t (*GetCapacity)(void const* cntr);

    // -------------------------------------------------------------------------

    void (*GetLBCursor)(void const* cntr, void* dst_cursor);

    void (*GetRBCursor)(void const* cntr, void* dst_cursor);

    // -------------------------------------------------------------------------

    void* (*PeekL)(void* cntr, void* dst_cursor, void* dst_elem);

    void const* (*ConstPeekL)(void const* cntr, void* dst_cursor,
                              void* dst_elem);

    void* (*PeekR)(void* cntr, void* dst_cursor, void* dst_elem);

    void const* (*ConstPeekR)(void const* cntr, void* dst_cursor,
                              void* dst_elem);

    void* (*Refer)(void* cntr, void const* pos_cursor);

    void const* (*ConstRefer)(void const* cntr, void const* pos_cursor);

    // -------------------------------------------------------------------------

    void* (*Find)(void* cntr, void const* key, FnHash const& key_hash,
                  FnCompare const& key_elem_compare, void* dst_cursor);

    void const* (*ConstFind)(void const* cntr, void const* key,
                             FnHash const& key_hash,
                             FnCompare const& key_elem_compare,
                             void* dst_cursor);

    void* (*Insert)(void* cntr, void const* elem, void* dst_cursor);

    void (*Erase)(void* cntr, void* pos_cursor);

    void (*EraseAll)(void* cntr);

    // -------------------------------------------------------------------------

    void (*CopyCursor)(void const* cntr, void const* cursor, void* dst_cursor);

    bool (*AreEqualCursor)(void const* cntr, void const* cursor_a,
                           void const* cursor_b);

    int (*CompareCursor)(void const* cntr, void const* cursor_a,
                         void const* cursor_b);

    size_t (*GetCursorDist)(void const* cntr, void const* cursor_a,
                            void const* cursor_b);

    size_t (*GetCursorIdx)(void const* cntr, void const* cursor);

    void (*CursorStepL)(void const* cntr, void* cursor);

    void (*CursorStepR)(void const* cntr, void* cursor);

    void (*CursorAdvanceL)(void const* cntr, void* cursor, size_t step);

    void (*CursorAdvanceR)(void const* cntr, void* cursor, size_t step);

    // -------------------------------------------------------------------------

    template <typename AssocCntrImpl>
    static AssocCntrVTable const& Make();
};

// -----------------------------------------------------------------------------

template <typename IsConst>
struct AssocCntrRefTpl {
    ZETA_Core_StaticAssert(value_wrapper::IsStaticValueWrapper<IsConst>);
    ZETA_Core_StaticAssert(IsAnyOf<decltype(IsConst::value), bool const>);

    // -------------------------------------------------------------------------

    void* inst;

    unsigned short cursor_size;

    size_t width;
    size_t capacity;

    AssocCntrVTable const* vtable;

    // -------------------------------------------------------------------------

    AssocCntrRefTpl() = default;

    template <typename OtherIsConst,
              typename = EnableIf<IsConst::value || !OtherIsConst::value, void>>
    AssocCntrRefTpl(AssocCntrRefTpl<OtherIsConst> const& other_assoc_cntr_ref);

    // -------------------------------------------------------------------------

    template <typename _ = void>
    EnableIf<!IsConst::value, void, _> Deinit(void* cntr);

    // -------------------------------------------------------------------------

    static size_t GetSize(void const* cntr);

    static size_t GetCapacity(void const* cntr);

    // -------------------------------------------------------------------------

    static void GetLBCursor(void const* cntr, void* dst_cursor);

    static void GetRBCursor(void const* cntr, void* dst_cursor);

    // -------------------------------------------------------------------------

    template <typename _ = void>
    static EnableIf<!IsConst::value, void*, _> PeekL(void* cntr,
                                                     void* dst_cursor,
                                                     void* dst_elem);

    static void const* ConstPeekL(void const* cntr, void* dst_cursor,
                                  void* dst_elem);

    template <typename _ = void>
    static EnableIf<!IsConst::value, void*, _> PeekR(void* cntr,
                                                     void* dst_cursor,
                                                     void* dst_elem);

    static void const* ConstPeekR(void const* cntr, void* dst_cursor,
                                  void* dst_elem);

    template <typename _ = void>
    static EnableIf<!IsConst::value, void*, _> Refer(void* cntr,
                                                     void const* pos_cursor);

    static void const* ConstRefer(void const* cntr, void const* pos_cursor);

    // -------------------------------------------------------------------------

    template <typename _ = void>
    static EnableIf<!IsConst::value, void*, _> Find(
        void* cntr, void const* key, FnHash const& key_hash,
        FnCompare const& key_elem_compare, void* dst_cursor);

    static void const* ConstFind(void const* cntr, void const* key,
                                 FnHash const& key_hash,
                                 FnCompare const& key_elem_compare,
                                 void* dst_cursor);

    template <typename _ = void>
    static EnableIf<!IsConst::value, void*, _> Insert(void* cntr,
                                                      void const* elem,
                                                      void* dst_cursor);

    template <typename _ = void>
    static EnableIf<!IsConst::value, void, _> Erase(void* cntr,
                                                    void* pos_cursor);

    template <typename _ = void>
    static EnableIf<!IsConst::value, void, _> EraseAll(void* cntr);

    // -------------------------------------------------------------------------

    static void CopyCursor(void const* cntr, void const* cursor,
                           void* dst_cursor);

    static bool AreEqualCursor(void const* cntr, void const* cursor_a,
                               void const* cursor_b);

    static int CompareCursor(void const* cntr, void const* cursor_a,
                             void const* cursor_b);

    static size_t GetCursorDist(void const* cntr, void const* cursor_a,
                                void const* cursor_b);

    static size_t GetCursorIdx(void const* cntr, void const* cursor);

    static void CursorStepL(void const* cntr, void* cursor);

    static void CursorStepR(void const* cntr, void* cursor);

    static void CursorAdvanceL(void const* cntr, void* cursor, size_t step);

    static void CursorAdvanceR(void const* cntr, void* cursor, size_t step);

    // -------------------------------------------------------------------------

    static bool CheckCntr(void const* cntr);
};

using AssocCntrRef = AssocCntrRefTpl<value_wrapper::StaticValueWrapper<false>>;

using ConstAssocCntrRef =
    AssocCntrRefTpl<value_wrapper::StaticValueWrapper<true>>;

}  // namespace zeta::core::assoc_cntr
