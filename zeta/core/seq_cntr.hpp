#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>
#include <zeta/core/value_wrapper.ipp>

namespace zeta::core::seq_cntr {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_SeqCntr_AllocaCursor(cntr)                  \
    ({                                                        \
        __builtin_alloca_with_align(                          \
            ::zeta::core::seq_cntr::ops::GetCursorSize(cntr), \
            __CHAR_BIT__ * alignof(max_align_t));             \
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

ZETA_Core_StaticAssert((non_const_ability_flag | const_ability_flag) ==
                       full_ability_flag);

namespace ops {

constexpr bool CheckAbilityFlags(AbilityFlag static_enabled_ability_flag,
                                 AbilityFlag static_disabled_ability_flag);

bool CheckAbilityFlags(AbilityFlag static_enabled_ability_flag,
                       AbilityFlag static_disabled_ability_flag,
                       AbilityFlag dynamic_enabled_ability_flag,
                       AbilityFlag dynamic_disabled_ability_flag);

template <typename SeqCntrLike>
void CheckContract(SeqCntrLike&& cntr);

}  // namespace ops

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

struct VTable {
    unsigned long long custom_tags[4];

    size_t (*GetSize)(void* cntr);

    size_t (*GetCapacity)(void* cntr);

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

template <typename SeqCntr>
struct BasicVTableBuilder {
    static constexpr VTable Build();

    static size_t GetSize(void* cntr);

    static constexpr size_t GetCapacity(void* cntr);

    static void GetLBCursor(void* cntr, void* dst_cursor);

    static void GetRBCursor(void* cntr, void* dst_cursor);

    static void* PeekL(void* cntr, bool lazy_copy_elem, void* dst_cursor,
                       void* dst_elem);

    static void* PeekR(void* cntr, bool lazy_copy_elem, void* dst_cursor,
                       void* dst_elem);

    static void* Access(void* cntr, size_t idx, bool lazy_copy_elem,
                        void* dst_cursor, void* dst_elem);

    static void* Derefer(void* cntr, void const* pos_cursor,
                         bool lazy_copy_elem, void* dst_elem);

    static void FnRead(void* cntr, void const* pos_cursor, size_t cnt,
                       FnReader reader, void* dst_cursor);

    static void FnWrite(void* cntr, void* pos_cursor, size_t cnt,
                        FnWriter writer, void* dst_cursor);

    static void FnReadWrite(void* cntr, void* pos_cursor, size_t cnt,
                            FnReaderWriter reader_writer, void* dst_cursor);

    static void MemRead(void* cntr, void const* pos_cursor, size_t cnt,
                        MemReader reader, void* dst_cursor);

    static void MemWrite(void* cntr, void* pos_cursor, size_t cnt,
                         MemWriter writer, void* dst_cursor);

    static void* FnPushL(void* cntr, size_t cnt, FnWriter writer,
                         void* dst_cursor);

    static void* FnPushR(void* cntr, size_t cnt, FnWriter writer,
                         void* dst_cursor);

    static void* FnInsert(void* cntr, void* pos_cursor, size_t cnt,
                          FnWriter writer, void* dst_cursor);

    static void* MemPushL(void* cntr, size_t cnt, MemWriter writer,
                          void* dst_cursor);

    static void* MemPushR(void* cntr, size_t cnt, MemWriter writer,
                          void* dst_cursor);

    static void* MemInsert(void* cntr, void* pos_cursor, size_t cnt,
                           MemWriter writer, void* dst_cursor);

    static void PopL(void* cntr, size_t cnt);

    static void PopR(void* cntr, size_t cnt);

    static void Erase(void* cntr, void* pos_cursor, size_t cnt);

    static void EraseAll(void* cntr);

    static void CopyCursor(void* cntr, void const* src_cursor,
                           void* dst_cursor);

    static bool AreEqualCursor(void* cntr, void const* cursor_a,
                               void const* cursor_b);

    static int CompareCursor(void* cntr, void const* cursor_a,
                             void const* cursor_b);

    static size_t GetCursorDist(void* cntr, void const* cursor_a,
                                void const* cursor_b);

    static size_t GetCursorIdx(void* cntr, void const* cursor);

    static void CursorStepL(void* cntr, void* cursor);

    static void CursorStepR(void* cntr, void* cursor);

    static void CursorAdvanceL(void* cntr, void* cursor, size_t step);

    static void CursorAdvanceR(void* cntr, void* cursor, size_t step);
};

template <typename SeqCntr>
struct VTableBuilder {
    static constexpr VTable Build();
};

template <typename SeqCntr>
constexpr VTable const& GetVTable();

struct Ref {
    unsigned short cursor_size;

    size_t width;
    size_t capacity;

    AbilityFlag dynamic_enabled_ability_flag;
    AbilityFlag dynamic_disabled_ability_flag;

    VTable const* vtable;

    void* cntr;

    static constexpr AbilityFlag GetStaticEnabledAbilityFlag(
        type_wrapper::TypeWrapper<Ref*>);

    static constexpr AbilityFlag GetStaticEnabledAbilityFlag(
        type_wrapper::TypeWrapper<Ref const*>);

    static constexpr AbilityFlag GetStaticDisabledAbilityFlag(
        type_wrapper::TypeWrapper<Ref*>);

    static constexpr AbilityFlag GetStaticDisabledAbilityFlag(
        type_wrapper::TypeWrapper<Ref const*>);

    static AbilityFlag GetDynamicEnabledAbilityFlag(Ref*);

    static AbilityFlag GetDynamicEnabledAbilityFlag(Ref const*);

    static AbilityFlag GetDynamicDisabledAbilityFlag(Ref*);

    static AbilityFlag GetDynamicDisabledAbilityFlag(Ref const*);

    static size_t GetCursorSize(Ref const*);

    static size_t GetWidth(Ref const* ref);

    static size_t GetSride(Ref const* ref);

    static size_t GetOffset(Ref const* ref);

    static size_t GetSize(Ref const* ref);

    static size_t GetCapacity(Ref const* ref);

    static void GetLBCursor(Ref const* ref, void* dst_cursor);

    static void GetRBCursor(Ref const* ref, void* dst_cursor);

    static void* PeekL(Ref* ref, bool lazy_copy_elem, void* dst_cursor,
                       void* dst_elem);

    static void const* PeekL(Ref const* ref, bool lazy_copy_elem,
                             void* dst_cursor, void* dst_elem);

    static void* PeekR(Ref* ref, bool lazy_copy_elem, void* dst_cursor,
                       void* dst_elem);

    static void const* PeekR(Ref const* ref, bool lazy_copy_elem,
                             void* dst_cursor, void* dst_elem);

    static void* Access(Ref* ref, size_t idx, bool lazy_copy_elem,
                        void* dst_cursor, void* dst_elem);

    static void const* Access(Ref const* ref, size_t idx, bool lazy_copy_elem,
                              void* dst_cursor, void* dst_elem);

    static void* Derefer(Ref* ref, void const* pos_cursor, bool lazy_copy_elem,
                         void* dst_elem);

    static void const* Derefer(Ref const* ref, void const* pos_cursor,
                               bool lazy_copy_elem, void* dst_elem);

    template <typename Reader>
    static void Read(Ref const* ref, void const* pos_cursor, size_t cnt,
                     Reader&& reader, void* dst_cursor);

    template <typename Writer>
    static void Write(Ref* ref, void* pos_cursor, size_t cnt, Writer&& writer,
                      void* dst_cursor);

    template <typename ReaderWriter>
    static void ReadWrite(Ref* ref, void* pos_cursor, size_t cnt,
                          ReaderWriter&& reader_writer, void* dst_cursor);

    static void Read(Ref const* ref, void const* pos_cursor, size_t cnt,
                     MemReader reader, void* dst_cursor);

    static void Write(Ref* ref, void* pos_cursor, size_t cnt, MemWriter writer,
                      void* dst_cursor);

    template <typename Writer>
    static void* PushL(Ref* ref, size_t cnt, Writer&& writer, void* dst_cursor);

    static void* PushL(Ref* ref, size_t cnt, MemWriter writer,
                       void* dst_cursor);

    template <typename Writer>
    static void* PushR(Ref* ref, size_t cnt, Writer&& writer, void* dst_cursor);

    static void* PushR(Ref* ref, size_t cnt, MemWriter writer,
                       void* dst_cursor);

    template <typename Writer>
    static void* Insert(Ref* ref, void* pos_cursor, size_t cnt, Writer&& writer,
                        void* dst_cursor);

    static void* Insert(Ref* ref, void* pos_cursor, size_t cnt,
                        MemWriter writer, void* dst_cursor);

    static void PopL(Ref* ref, size_t cnt);

    static void PopR(Ref* ref, size_t cnt);

    static void Erase(Ref* ref, void* pos_cursor, size_t cnt);

    static void EraseAll(Ref* ref);

    static void CopyCursor(Ref const* ref, void const* src_cursor,
                           void* dst_cursor);

    static bool AreEqualCursor(Ref const* ref, void const* cursor_a,
                               void const* cursor_b);

    static int CompareCursor(Ref const* ref, void const* cursor_a,
                             void const* cursor_b);

    static size_t GetCursorDist(Ref const* ref, void const* cursor_a,
                                void const* cursor_b);

    static size_t GetCursorIdx(Ref const* ref, void const* cursor);

    static void CursorStepL(Ref const* ref, void* cursor);

    static void CursorStepR(Ref const* ref, void* cursor);

    static void CursorAdvanceL(Ref const* ref, void* cursor, size_t step);

    static void CursorAdvanceR(Ref const* ref, void* cursor, size_t step);

    static void CheckCntr(Ref* ref);
};

namespace ops {

constexpr bool IsReferable(size_t idx, size_t cnt, size_t size);

constexpr bool IsDereferable(size_t idx, size_t cnt, size_t size);

constexpr bool IsInsertable(size_t idx, size_t cnt, size_t size,
                            size_t capacity);

constexpr bool IsErasable(size_t idx, size_t cnt, size_t size);

template <typename SeqCntrLike>
constexpr bool IsConst(type_wrapper::TypeWrapper<SeqCntrLike>);

template <typename SeqCntrLike>
constexpr AbilityFlag GetStaticEnabledAbilityFlag(
    type_wrapper::TypeWrapper<SeqCntrLike>);

template <typename SeqCntrLike>
constexpr AbilityFlag GetStaticDisabledAbilityFlag(
    type_wrapper::TypeWrapper<SeqCntrLike>);

template <typename SeqCntrLike>
AbilityFlag GetDynamicEnabledAbilityFlag(SeqCntrLike&& cntr);

template <typename SeqCntrLike>
AbilityFlag GetDynamicDisabledAbilityFlag(SeqCntrLike&& cntr);

template <typename SeqCntrLike>
decltype(auto) GetCursorSize(SeqCntrLike&& cntr);

template <typename SeqCntrLike>
size_t GetWidth(SeqCntrLike&& cntr);

template <typename SeqCntrLike>
size_t GetSize(SeqCntrLike&& cntr);

template <typename SeqCntrLike>
decltype(auto) GetCapacity(SeqCntrLike&& cntr);

template <typename SeqCntrLike>
void GetLBCursor(SeqCntrLike&& cntr, void* dst_cursor);

template <typename SeqCntrLike>
void GetRBCursor(SeqCntrLike&& cntr, void* dst_cursor);

template <typename SeqCntrLike>
auto PeekL(SeqCntrLike&& cntr, bool lazy_copy_elem, void* dst_cursor,
           void* dst_elem);

template <typename SeqCntrLike>
auto PeekR(SeqCntrLike&& cntr, bool lazy_copy_elem, void* dst_cursor,
           void* dst_elem);

template <typename SeqCntrLike>
auto Access(SeqCntrLike&& cntr, size_t idx, bool lazy_copy_elem,
            void* dst_cursor, void* dst_elem);

template <typename SeqCntrLike>
auto Derefer(SeqCntrLike&& cntr, void const* pos_cursor, bool lazy_copy_elem,
             void* dst_elem);

template <typename SeqCntrLike, typename Reader>
void Read(SeqCntrLike&& cntr, void const* pos_cursor, size_t cnt,
          Reader&& reader, void* dst_cursor);

template <typename SeqCntrLike, typename Writer>
void Write(SeqCntrLike&& cntr, void* pos_cursor, size_t cnt, Writer&& writer,
           void* dst_cursor);

template <typename SeqCntrLike, typename ReaderWriter>
void ReadWrite(SeqCntrLike&& cntr, void* pos_cursor, size_t cnt,
               ReaderWriter&& reader_writer, void* dst_cursor);

template <typename SeqCntrLike, typename Writer>
void* PushL(SeqCntrLike&& cntr, size_t cnt, Writer&& writer, void* dst_cursor);

template <typename SeqCntrLike, typename Writer>
void* PushR(SeqCntrLike&& cntr, size_t cnt, Writer&& writer, void* dst_cursor);

template <typename SeqCntrLike, typename Writer>
void Insert(SeqCntrLike&& cntr, void* pos_cursor, size_t cnt, Writer&& writer,
            void* dst_cursor);

template <typename SeqCntrLike>
void PopL(SeqCntrLike&& cntr, size_t cnt);

template <typename SeqCntrLike>
void PopR(SeqCntrLike&& cntr, size_t cnt);

template <typename SeqCntrLike>
void Erase(SeqCntrLike&& cntr, void* pos_cursor, size_t cnt);

template <typename SeqCntrLike>
void EraseAll(SeqCntrLike&& cntr);

template <typename SeqCntrLike>
void CopyCursor(SeqCntrLike&& cntr, void const* src_cursor, void* dst_cursor);

template <typename SeqCntrLike>
bool AreEqualCursor(SeqCntrLike&& cntr, void const* cursor_a,
                    void const* cursor_b);

template <typename SeqCntrLike>
int CompareCursor(SeqCntrLike&& cntr, void const* cursor_a,
                  void const* cursor_b);

template <typename SeqCntrLike>
size_t GetCursorDist(SeqCntrLike&& cntr, void const* cursor_a,
                     void const* cursor_b);

template <typename SeqCntrLike>
size_t GetCursorIdx(SeqCntrLike&& cntr, void const* cursor);

template <typename SeqCntrLike>
void CursorStepL(SeqCntrLike&& cntr, void* cursor);

template <typename SeqCntrLike>
void CursorStepR(SeqCntrLike&& cntr, void* cursor);

template <typename SeqCntrLike>
void CursorAdvanceL(SeqCntrLike&& cntr, void* cursor, size_t step);

template <typename SeqCntrLike>
void CursorAdvanceR(SeqCntrLike&& cntr, void* cursor, size_t step);

template <typename DstSeqCntrLike, typename SrcSeqCntrLike>
void RangeAssign(DstSeqCntrLike&& dst_cntr_, SrcSeqCntrLike&& src_cntr_,
                 size_t dst_beg, size_t src_beg, size_t cnt);

template <typename DstSeqCntrLike, typename SrcSeqCntrLike>
void Assign(DstSeqCntrLike&& dst_cntr, SrcSeqCntrLike&& src_cntr);

template <typename SeqCntrLike>
Ref MakeRef(SeqCntrLike&& cntr);

}  // namespace ops

}  // namespace zeta::core::seq_cntr
