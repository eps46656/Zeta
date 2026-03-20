#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>
#include <zeta/core/value_wrapper.ipp>

namespace zeta::core::seq_cntr {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_SeqCntr_AllocaCursor(cntr)             \
    ({                                                   \
        __builtin_alloca_with_align(                     \
            ::zeta::core::seq_cntr::GetCursorSize(cntr), \
            __CHAR_BIT__ * alignof(max_align_t));        \
    })

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
    func(GetElemSize) sep                                                      \
    func(GetElemCnt) sep                                                       \
    func(GetMaxElemCnt) sep                                                    \
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

ZETA_Core_StaticAssert(AbilityEnum::Total <= integral::WidthOf<AbilityFlag>);

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
    .GetElemSize = false,
    .GetElemCnt = false,
    .GetMaxElemCnt = false,
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
    .GetElemSize = true,
    .GetElemCnt = true,
    .GetMaxElemCnt = true,
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

ZETA_Core_StaticAssert((non_const_ability_flag | const_ability_flag) ==
                       full_ability_flag);

struct EmptyReader {
    constexpr void operator()(void const*, size_t, size_t) const;
};

struct EmptyWriter {
    constexpr void operator()(void*, size_t, size_t) const;
};

struct MemReader {
    void* data;
    size_t elem_size;
    size_t elem_stride;

    void operator()(void const* src, size_t src_elem_stride, size_t cnt);
};

struct MemWriter {
    void const* data;
    size_t elem_size;
    size_t elem_stride;

    void operator()(void* dst, size_t dst_elem_stride, size_t cnt);
};

template <typename Cntr, typename = void>
struct CntrTraits;

constexpr bool CheckAbilityFlags(AbilityFlag static_enabled_ability_flag,
                                 AbilityFlag static_disabled_ability_flag);

bool CheckAbilityFlags(AbilityFlag static_enabled_ability_flag,
                       AbilityFlag static_disabled_ability_flag,
                       AbilityFlag dynamic_enabled_ability_flag,
                       AbilityFlag dynamic_disabled_ability_flag);

template <typename CntrLike>
void* GetReferedInstPtr(CntrLike&& cntr);

template <typename Cntr>
constexpr AbilityFlag GetStaticEnabledAbilityFlag();

template <typename Cntr>
constexpr AbilityFlag GetStaticDisabledAbilityFlag();

template <typename CntrLike>
AbilityFlag GetDynamicEnabledAbilityFlag(CntrLike&& cntr);

template <typename CntrLike>
AbilityFlag GetDynamicDisabledAbilityFlag(CntrLike&& cntr);

template <typename CntrLike>
size_t GetCursorSize(CntrLike&& cntr);

template <typename CntrLike>
size_t GetElemSize(CntrLike&& cntr);

template <typename CntrLike>
size_t GetElemCnt(CntrLike&& cntr);

template <typename CntrLike>
size_t GetMaxElemCnt(CntrLike&& cntr);

template <typename CntrLike>
void GetLBCursor(CntrLike&& cntr, void* dst_cursor);

template <typename CntrLike>
void GetRBCursor(CntrLike&& cntr, void* dst_cursor);

template <typename CntrLike>
void* PeekL(CntrLike&& cntr, bool lazy_copy_elem, void* dst_cursor,
            void* dst_elem);

template <typename CntrLike>
void* PeekR(CntrLike&& cntr, bool lazy_copy_elem, void* dst_cursor,
            void* dst_elem);

template <typename CntrLike>
void* Access(CntrLike&& cntr, size_t idx, bool lazy_copy_elem, void* dst_cursor,
             void* dst_elem);

template <typename CntrLike>
void* Derefer(CntrLike&& cntr, void const* pos_cursor, bool lazy_copy_elem,
              void* dst_elem);

template <typename CntrLike, typename Reader>
void Read(CntrLike&& cntr, void const* pos_cursor, size_t cnt, Reader&& reader,
          void* dst_cursor);

template <typename CntrLike, typename Writer>
void Write(CntrLike&& cntr, void* pos_cursor, size_t cnt, Writer&& writer,
           void* dst_cursor);

template <typename CntrLike, typename ReaderWriter>
void ReadWrite(CntrLike&& cntr, void* pos_cursor, size_t cnt,
               ReaderWriter&& reader_writer, void* dst_cursor);

template <typename CntrLike, typename Writer>
void* PushL(CntrLike&& cntr, size_t cnt, Writer&& writer, void* dst_cursor);

template <typename CntrLike, typename Writer>
void* PushR(CntrLike&& cntr, size_t cnt, Writer&& writer, void* dst_cursor);

template <typename CntrLike, typename Writer>
void* Insert(CntrLike&& cntr, void* pos_cursor, size_t cnt, Writer&& writer,
             void* dst_cursor);

template <typename CntrLike>
void PopL(CntrLike&& cntr, size_t cnt);

template <typename CntrLike>
void PopR(CntrLike&& cntr, size_t cnt);

template <typename CntrLike>
void Erase(CntrLike&& cntr, void* pos_cursor, size_t cnt);

template <typename CntrLike>
void EraseAll(CntrLike&& cntr);

template <typename CntrLike>
void CopyCursor(CntrLike&& cntr, void const* src_cursor, void* dst_cursor);

template <typename CntrLike>
bool AreEqualCursor(CntrLike&& cntr, void const* cursor_a,
                    void const* cursor_b);

template <typename CntrLike>
int CompareCursor(CntrLike&& cntr, void const* cursor_a, void const* cursor_b);

template <typename CntrLike>
size_t GetCursorDist(CntrLike&& cntr, void const* cursor_a,
                     void const* cursor_b);

template <typename CntrLike>
size_t GetCursorIdx(CntrLike&& cntr, void const* cursor);

template <typename CntrLike>
void CursorStepL(CntrLike&& cntr, void* cursor);

template <typename CntrLike>
void CursorStepR(CntrLike&& cntr, void* cursor);

template <typename CntrLike>
void CursorAdvanceL(CntrLike&& cntr, void* cursor, size_t step);

template <typename CntrLike>
void CursorAdvanceR(CntrLike&& cntr, void* cursor, size_t step);

template <typename CntrLike>
void CheckContract(CntrLike&& cntr);

struct VTable {
    unsigned long long custom_tags[4];

    size_t (*GetElemCnt)(void* cntr);

    size_t (*GetMaxElemCnt)(void* cntr);

    void (*GetLBCursor)(void* cntr, void* dst_cursor);

    void (*GetRBCursor)(void* cntr, void* dst_cursor);

    void* (*PeekL)(void* cntr, bool lazy_copy_elem, void* dst_cursor,
                   void* dst_elem);

    void* (*PeekR)(void* cntr, bool lazy_copy_elem, void* dst_cursor,
                   void* dst_elem);

    void* (*Access)(void* cntr, size_t idx, bool lazy_copy_elem,
                    void* dst_cursor, void* dst_elem);

    void* (*Derefer)(void* cntr, void const* pos_cursor, bool lazy_copy_elem,
                     void* dst_elem);

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

    void (*CopyCursor)(void* cntr, void const* src_cursor, void* dst_cursor);

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

    unsigned long long (*CustomMethods[4])(void* cntr, unsigned long long arg0,
                                           unsigned long long arg1,
                                           unsigned long long arg2,
                                           unsigned long long arg3);
};

template <typename Cntr>
constexpr VTable BuildVTableBasic();

template <typename Cntr, typename = void>
struct BuildVTableImpl {
    static constexpr VTable Call();
};

template <typename Cntr>
constexpr VTable BuildVTable();

template <typename Cntr>
constexpr VTable const& GetVTable();

constexpr bool IsReferable(size_t idx, size_t cnt, size_t size);

constexpr bool IsDereferable(size_t idx, size_t cnt, size_t size);

constexpr bool IsInsertable(size_t idx, size_t cnt, size_t size,
                            size_t capacity);

constexpr bool IsErasable(size_t idx, size_t cnt, size_t size);

template <typename DstSeqCntrLike, typename SrcSeqCntrLike>
void RangeAssign(DstSeqCntrLike&& dst_cntr_, SrcSeqCntrLike&& src_cntr_,
                 size_t dst_beg, size_t src_beg, size_t cnt);

template <typename DstSeqCntrLike, typename SrcSeqCntrLike>
void Assign(DstSeqCntrLike&& dst_cntr, SrcSeqCntrLike&& src_cntr);

}  // namespace zeta::core::seq_cntr
