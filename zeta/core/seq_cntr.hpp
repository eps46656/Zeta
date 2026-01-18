#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/type_traits.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>
#include <zeta/core/value_wrapper.ipp>

namespace zeta::core::seq_cntr {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_SeqCntr_AllocaCursor_(tmp_cntr, cntr)                      \
    ({                                                                       \
        auto tmp_cntr{ cntr };                                               \
        ZETA_Core_StaticAssert(::zeta::core::IsPointer<decltype(tmp_cntr)>); \
                                                                             \
        ZETA_Core_DebugAssert(tmp_cntr != nullptr);                          \
        ::zeta::core::seq_cntr::CheckContract(tmp_cntr);                     \
                                                                             \
        __builtin_alloca_with_align(                                         \
            ::zeta::core::RemovePointer<decltype(tmp_cntr)>::GetCursorSize(  \
                tmp_cntr),                                                   \
            __CHAR_BIT__ * alignof(max_align_t));                            \
    })

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_SeqCntr_AllocaCursor(cntr) \
    ZETA_Core_SeqCntr_AllocaCursor_(ZETA_Core_TmpName, cntr)

using FnReader = FunctionRef<void(  //
    void const*,                    // src
    size_t,                         // src_stride
    size_t                          // cnt
    )>;

using FnWriter = FunctionRef<void(  //
    void*,                          // dst
    size_t,                         // dst_stride
    size_t                          // cnt
    )>;

using FnReaderWriter = FunctionRef<void(void*,   // data
                                        size_t,  // data_stride
                                        size_t   // cnt
                                        )>;

// clang-format off
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_SeqCntr_Ability_XMacro(func, sep) \
    func(GetCursorSize) sep                                                    \
                                                                               \
    func(GetWidth) sep                                                         \
    func(GetSize) sep                                                          \
    func(GetCapacity) sep                                                      \
                                                                               \
    func(GetLBCursor) sep                                                      \
    func(GetRBCursor) sep                                                      \
                                                                               \
    func(PeekL) sep                                                            \
    func(PeekR) sep                                                            \
                                                                               \
    func(Access) sep                                                           \
    func(Derefer) sep                                                          \
                                                                               \
    func(Read) sep                                                             \
    func(Write) sep                                                            \
    func(ReadWrite) sep                                                        \
                                                                               \
    func(PushL) sep                                                            \
    func(PushR) sep                                                            \
    func(Insert) sep                                                           \
                                                                               \
    func(PopL) sep                                                             \
    func(PopR) sep                                                             \
    func(Erase) sep                                                            \
    func(EraseAll) sep                                                         \
                                                                               \
    func(CopyCursor) sep                                                       \
                                                                               \
    func(AreEqualCursor) sep                                                   \
    func(CompareCursor) sep                                                    \
    func(GetCursorDist) sep                                                    \
    func(GetCursorIdx) sep                                                     \
                                                                               \
    func(CursorStepL) sep                                                      \
    func(CursorStepR) sep                                                      \
                                                                               \
    func(CursorAdvanceL) sep                                                   \
    func(CursorAdvanceR)
// clang-format on

struct AbilityEnum {
    static constexpr size_t NumBase{ __COUNTER__ + 1 };

    static constexpr size_t Never{ __COUNTER__ - NumBase };
    static constexpr size_t Always{ __COUNTER__ - NumBase };

#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) static constexpr size_t name{ __COUNTER__ - NumBase };

    ZETA_Core_SeqCntr_Ability_XMacro(F, );

#pragma pop_macro("F")

    static constexpr size_t Total{ __COUNTER__ - NumBase };
};

using AbilityFlag = unsigned;

ZETA_Core_StaticAssert(AbilityEnum::Total <= WidthOf<AbilityFlag>);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
struct AbilityFlagBuilder {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) bool const name;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    ZETA_Core_SeqCntr_Ability_XMacro(F, );

#pragma pop_macro("F")

    constexpr AbilityFlag operator()() const {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) (static_cast<AbilityFlag>(this->name) << AbilityEnum::name)

        return (static_cast<AbilityFlag>(1) << AbilityEnum::Always) |
               ZETA_Core_SeqCntr_Ability_XMacro(F, |);

#pragma pop_macro("F")
    }
};

constexpr AbilityFlag empty_ability_flag{ AbilityFlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) .name = false,

    ZETA_Core_SeqCntr_Ability_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr AbilityFlag full_ability_flag{ AbilityFlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) .name = true,

    ZETA_Core_SeqCntr_Ability_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr AbilityFlag non_const_ability_flag{ AbilityFlagBuilder{
    .GetCursorSize = false,
    .GetWidth = false,
    .GetSize = false,
    .GetCapacity = false,
    .GetLBCursor = false,
    .GetRBCursor = false,
    .PeekL = false,
    .PeekR = false,
    .Access = false,
    .Derefer = false,
    .Read = false,
    .Write = true,
    .ReadWrite = true,
    .PushL = true,
    .PushR = true,
    .Insert = true,
    .PopL = true,
    .PopR = true,
    .Erase = true,
    .EraseAll = true,
    .CopyCursor = false,
    .AreEqualCursor = false,
    .CompareCursor = false,
    .GetCursorDist = false,
    .GetCursorIdx = false,
    .CursorStepL = false,
    .CursorStepR = false,
    .CursorAdvanceL = false,
    .CursorAdvanceR = false,
}() };

constexpr AbilityFlag const_ability_flag{ AbilityFlagBuilder{
    .GetCursorSize = true,
    .GetWidth = true,
    .GetSize = true,
    .GetCapacity = true,
    .GetLBCursor = true,
    .GetRBCursor = true,
    .PeekL = true,
    .PeekR = true,
    .Access = true,
    .Derefer = true,
    .Read = true,
    .Write = false,
    .ReadWrite = false,
    .PushL = false,
    .PushR = false,
    .Insert = false,
    .PopL = false,
    .PopR = false,
    .Erase = false,
    .EraseAll = false,
    .CopyCursor = true,
    .AreEqualCursor = true,
    .CompareCursor = true,
    .GetCursorDist = true,
    .GetCursorIdx = true,
    .CursorStepL = true,
    .CursorStepR = true,
    .CursorAdvanceL = true,
    .CursorAdvanceR = true,
}() };

ZETA_Core_StaticAssert((non_const_ability_flag & const_ability_flag) ==
                       empty_ability_flag);

bool CheckAbilityFlags(AbilityFlag static_enabled_ability_flag,
                       AbilityFlag static_disabled_ability_flag,
                       AbilityFlag dynamic_enabled_ability_flag,
                       AbilityFlag dynamic_disabled_ability_flag);

struct MemReader {
    void* dst;
    size_t dst_width;
    size_t dst_stride;

    void operator()(void const* src, size_t src_stride, size_t cnt);
};

struct MemWriter {
    void const* src;
    size_t src_width;
    size_t src_stride;

    void operator()(void* dst, size_t dst_stride, size_t cnt);
};

template <typename SeqCntr>
void CheckContract(SeqCntr* cntr);

struct VTable {
    template <typename SeqCntr>
    static constexpr VTable const& Make();

    size_t (*GetSize)(void* cntr);

    size_t (*GetCapacity)(void* cntr);

    void (*GetLBCursor)(void* cntr, void* dst_cursor);

    void (*GetRBCursor)(void* cntr, void* dst_cursor);

    void* (*PeekL)(void* cntr, void* dst_cursor, void* dst_elem);

    void* (*PeekR)(void* cntr, void* dst_cursor, void* dst_elem);

    void* (*Access)(void* cntr, size_t idx, void* dst_cursor, void* dst_elem);

    void* (*Derefer)(void* cntr, void const* pos_cursor, void* dst_elem);

    void (*FnRead)(void* cntr, void const* pos_cursor, size_t cnt,
                   FnReader reader, void* dst_cursor);

    void (*FnWrite)(void* cntr, void* pos_cursor, size_t cnt, FnWriter writer,
                    void* dst_cursor);

    void (*FnReadWrite)(void* cntr, void* pos_cursor, size_t cnt,
                        FnReaderWriter reader_writer, void* dst_cursor);

    void (*MemRead)(void* cntr, void const* pos_cursor, size_t cnt,
                    MemReader reader, void* dst_cursor);

    void (*MemWrite)(void* cntr, void* pos_cursor, size_t cnt, MemWriter writer,
                     void* dst_cursor);

    void* (*FnPushL)(void* cntr, size_t cnt, FnWriter writer, void* dst_cursor);

    void* (*FnPushR)(void* cntr, size_t cnt, FnWriter writer, void* dst_cursor);

    void* (*FnInsert)(void* cntr, void* pos_cursor, size_t cnt, FnWriter writer,
                      void* dst_cursor);

    void* (*MemPushL)(void* cntr, size_t cnt, MemWriter writer,
                      void* dst_cursor);

    void* (*MemPushR)(void* cntr, size_t cnt, MemWriter writer,
                      void* dst_cursor);

    void* (*MemInsert)(void* cntr, void* pos_cursor, size_t cnt,
                       MemWriter writer, void* dst_cursor);

    void (*PopL)(void* cntr, size_t cnt);

    void (*PopR)(void* cntr, size_t cnt);

    void (*Erase)(void* cntr, void* pos_cursor, size_t cnt);

    void (*EraseAll)(void* cntr);

    void (*CopyCursor)(void* cntr, void* dst_cursor, void const* src_cursor);

    bool (*AreEqualCursor)(void* cntr, void const* cursor_a,
                           void const* cursor_b);

    int (*CompareCursor)(void* cntr, void const* cursor_a,
                         void const* cursor_b);

    size_t (*GetCursorDist)(void* cntr, void const* cursor_a,
                            void const* cursor_b);

    size_t (*GetCursorIdx)(void* cntr, void const* cursor);

    void (*CursorStepL)(void* cntr, void* cursor);

    void (*CursorStepR)(void* cntr, void* cursor);

    void (*CursorAdvanceL)(void* cntr, void* cursor, size_t step);

    void (*CursorAdvanceR)(void* cntr, void* cursor, size_t step);
};

template <typename SeqCntrType>
struct BasicVTableBuilder {
    static constexpr VTable Build();

    static constexpr size_t GetSize(void* cntr);

    static constexpr size_t GetCapacity(void* cntr);

    static constexpr void GetLBCursor(void* cntr, void* dst_cursor);

    static constexpr void GetRBCursor(void* cntr, void* dst_cursor);

    static constexpr void* PeekL(void* cntr, void* dst_cursor, void* dst_elem);

    static constexpr void* PeekR(void* cntr, void* dst_cursor, void* dst_elem);

    static constexpr void* Access(void* cntr, size_t idx, void* dst_cursor,
                                  void* dst_elem);

    static constexpr void* Derefer(void* cntr, void const* pos_cursor,
                                   void* dst_elem);

    static constexpr void FnRead(void* cntr, void const* pos_cursor, size_t cnt,
                                 FnReader reader, void* dst_cursor);

    static constexpr void FnWrite(void* cntr, void* pos_cursor, size_t cnt,
                                  FnWriter writer, void* dst_cursor);

    static constexpr void FnReadWrite(void* cntr, void* pos_cursor, size_t cnt,
                                      FnReaderWriter reader_writer,
                                      void* dst_cursor);

    static constexpr void MemRead(void* cntr, void const* pos_cursor,
                                  size_t cnt, MemReader reader,
                                  void* dst_cursor);

    static constexpr void MemWrite(void* cntr, void* pos_cursor, size_t cnt,
                                   MemWriter writer, void* dst_cursor);

    static constexpr void* FnPushL(void* cntr, size_t cnt, FnWriter writer,
                                   void* dst_cursor);

    static constexpr void* FnPushR(void* cntr, size_t cnt, FnWriter writer,
                                   void* dst_cursor);

    static constexpr void* FnInsert(void* cntr, void* pos_cursor, size_t cnt,
                                    FnWriter writer, void* dst_cursor);

    static constexpr void* MemPushL(void* cntr, size_t cnt, MemWriter writer,
                                    void* dst_cursor);

    static constexpr void* MemPushR(void* cntr, size_t cnt, MemWriter writer,
                                    void* dst_cursor);

    static constexpr void* MemInsert(void* cntr, void* pos_cursor, size_t cnt,
                                     MemWriter writer, void* dst_cursor);

    static constexpr void PopL(void* cntr, size_t cnt);

    static constexpr void PopR(void* cntr, size_t cnt);

    static constexpr void Erase(void* cntr, void* pos_cursor, size_t cnt);

    static constexpr void EraseAll(void* cntr);

    static constexpr void CopyCursor(void* cntr, void* dst_cursor,
                                     void const* src_cursor);

    static constexpr bool AreEqualCursor(void* cntr, void const* cursor_a,
                                         void const* cursor_b);

    static constexpr int CompareCursor(void* cntr, void const* cursor_a,
                                       void const* cursor_b);

    static constexpr size_t GetCursorDist(void* cntr, void const* cursor_a,
                                          void const* cursor_b);

    static constexpr size_t GetCursorIdx(void* cntr, void const* cursor);

    static constexpr void CursorStepL(void* cntr, void* cursor);

    static constexpr void CursorStepR(void* cntr, void* cursor);

    static constexpr void CursorAdvanceL(void* cntr, void* cursor, size_t step);

    static constexpr void CursorAdvanceR(void* cntr, void* cursor, size_t step);
};

template <typename SeqCntr>
struct VTableBuilder {
    static constexpr VTable Build();
};

template <typename SeqCntr>
constexpr VTable const& GetVTable();

template <typename IsConst>
struct Ref {
    // -------------------------------------------------------------------------

    ZETA_Core_StaticAssert(
        IsAnyOf<IsConst, value_wrapper::FalseType, value_wrapper::TrueType>);

    unsigned short cursor_size;

    size_t width;
    size_t capacity;

    AbilityFlag dynamic_enabled_ability_flag;
    AbilityFlag dynamic_disabled_ability_flag;

    VTable const* vtable;

    void* cntr;
};

#pragma push_macro("TplDecl")

#define TplDecl                                                              \
    template <typename IsConst,                                              \
              typename = EnableIf<IsAnyOf<IsConst, value_wrapper::FalseType, \
                                          value_wrapper::TrueType>>>

#pragma pop_macro("TplDecl")

template <typename SC>
auto MakeRef(SC&& cntr_);

constexpr bool IsReferable(size_t idx, size_t cnt, size_t size);

constexpr bool IsDereferable(size_t idx, size_t cnt, size_t size);

constexpr bool IsInsertable(size_t idx, size_t cnt, size_t size,
                            size_t capacity);

constexpr bool IsErasable(size_t idx, size_t cnt, size_t size);

template <typename DstSeqCntr, typename SrcSeqCntr>
void RangeAssign(DstSeqCntr* dst_cntr, SrcSeqCntr* src_cntr, size_t dst_beg,
                 size_t src_beg, size_t cnt);

template <typename DstSeqCntr, typename SrcSeqCntr>
void Assign(DstSeqCntr* dst_cntr, SrcSeqCntr* src_cntr);

}  // namespace zeta::core::seq_cntr
