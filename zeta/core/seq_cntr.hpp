/*
@file zeta/core/seq_cntr.hpp
*/

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

constexpr size_t max_max_elem_cnt{ integral::RangeMaxOf<size_t> / 2 };

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_SeqCntr_AllocaCursor(cntr)             \
    ({                                                   \
        __builtin_alloca_with_align(                     \
            ::zeta::core::seq_cntr::GetCursorSize(cntr), \
            __CHAR_BIT__ * alignof(max_align_t));        \
    })

using FnReader = function_ref::Ref<void(  //
    void const*,                          // src
    size_t,                               // src_stride
    size_t                                // cnt
    )>;

using FnWriter = function_ref::Ref<void(  //
    void*,                                // dst
    size_t,                               // dst_stride
    size_t                                // cnt
    )>;

using FnReaderWriter = function_ref::Ref<void(void*,   // data
                                              size_t,  // data_stride
                                              size_t   // cnt
                                              )>;

// clang-format off
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_SeqCntr_Capability_XMacro(func, sep) \
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
    func(Refer) sep                                                            \
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

struct CapabilityEnum {
    using Type = unsigned;

    static constexpr Type NumBase{ __COUNTER__ + 1 };

    struct Never {
        static constexpr Type value{ __COUNTER__ - NumBase };
    };

    struct Always {
        static constexpr Type value{ __COUNTER__ - NumBase };
    };

#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name)                                               \
    struct name {                                             \
        static constexpr Type value{ __COUNTER__ - NumBase }; \
    };

    ZETA_Core_SeqCntr_Capability_XMacro(F, );

#pragma pop_macro("F")

    static constexpr Type Total{ __COUNTER__ - NumBase };
};

using CapabilityFlag = unsigned;

ZETA_Core_StaticAssert(CapabilityEnum::Total <=
                       integral::WidthOf<CapabilityFlag>);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
struct CapabilityFlagBuilder {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) bool const name;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    ZETA_Core_SeqCntr_Capability_XMacro(F, );

#pragma pop_macro("F")

    constexpr CapabilityFlag operator()() const {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name) \
    (static_cast<CapabilityFlag>(this->name) << CapabilityEnum::name::value)

        return (static_cast<CapabilityFlag>(1)
                << CapabilityEnum::Always::value) |
               ZETA_Core_SeqCntr_Capability_XMacro(F, |);

#pragma pop_macro("F")
    }
};

constexpr CapabilityFlag empty_capability_flag{ CapabilityFlagBuilder{
    .GetCursorSize = false,
    .GetElemSize = false,
    .GetElemCnt = false,
    .GetMaxElemCnt = false,
    .GetLBCursor = false,
    .GetRBCursor = false,
    .PeekL = false,
    .PeekR = false,
    .Refer = false,
    .Derefer = false,
    .Read = false,
    .Write = false,
    .ReadWrite = false,
    .PushL = false,
    .PushR = false,
    .Insert = false,
    .PopL = false,
    .PopR = false,
    .Erase = false,
    .EraseAll = false,
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

constexpr CapabilityFlag full_capability_flag{ CapabilityFlagBuilder{
    .GetCursorSize = true,
    .GetElemSize = true,
    .GetElemCnt = true,
    .GetMaxElemCnt = true,
    .GetLBCursor = true,
    .GetRBCursor = true,
    .PeekL = true,
    .PeekR = true,
    .Refer = true,
    .Derefer = true,
    .Read = true,
    .Write = true,
    .ReadWrite = true,
    .PushL = true,
    .PushR = true,
    .Insert = true,
    .PopL = true,
    .PopR = true,
    .Erase = true,
    .EraseAll = true,
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

constexpr CapabilityFlag non_const_capability_flag{ CapabilityFlagBuilder{
    .GetCursorSize = false,
    .GetElemSize = false,
    .GetElemCnt = false,
    .GetMaxElemCnt = false,
    .GetLBCursor = false,
    .GetRBCursor = false,
    .PeekL = false,
    .PeekR = false,
    .Refer = false,
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

constexpr CapabilityFlag const_capability_flag{ CapabilityFlagBuilder{
    .GetCursorSize = true,
    .GetElemSize = true,
    .GetElemCnt = true,
    .GetMaxElemCnt = true,
    .GetLBCursor = true,
    .GetRBCursor = true,
    .PeekL = true,
    .PeekR = true,
    .Refer = true,
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

ZETA_Core_StaticAssert((non_const_capability_flag & const_capability_flag) ==
                       empty_capability_flag);

ZETA_Core_StaticAssert((non_const_capability_flag | const_capability_flag) ==
                       full_capability_flag);

struct ElemPtrView {
    struct AliasabilityEnum {
        using Type = unsigned;

        static constexpr Type Null{ 0 };
        static constexpr Type ReadOnly{ 1 };
        static constexpr Type ReadWrite{ 2 };
    };

    void* ptr;
    AliasabilityEnum::Type aliasability;

    bool operator==(ElemPtrView const&) const = default;
    bool operator!=(ElemPtrView const&) const = default;
};

struct CursorLimit {
    void* content[8];
} __attribute__((aligned(alignof(max_align_t))));

struct EmptyReader {
    constexpr void operator()(void const*, size_t, size_t) const;
};

constexpr EmptyReader empty_reader;

struct EmptyWriter {
    constexpr void operator()(void*, size_t, size_t) const;
};

constexpr EmptyWriter empty_writer;

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

/**
@page zeta__core__seq_cntr__CntrTraits CntrTraits
*/
template <typename Cntr, typename = void>
struct CntrTraits;  // IWYU pragma: export

constexpr bool CheckCapabilityFlags(
    CapabilityFlag static_enabled_capability_flag,
    CapabilityFlag static_disabled_capability_flag);

bool CheckCapabilityFlags(CapabilityFlag static_enabled_capability_flag,
                          CapabilityFlag static_disabled_capability_flag,
                          CapabilityFlag dynamic_enabled_capability_flag,
                          CapabilityFlag dynamic_disabled_capability_flag);

template <typename Cntr>
void* GetReferedInstPtr(Cntr& cntr);

template <typename Cntr>
constexpr CapabilityFlag GetStaticEnabledCapabilityFlag();

template <typename Cntr>
constexpr CapabilityFlag GetStaticDisabledCapabilityFlag();

template <typename Cntr>
CapabilityFlag GetDynamicEnabledCapabilityFlag(Cntr& cntr);

template <typename Cntr>
CapabilityFlag GetDynamicDisabledCapabilityFlag(Cntr& cntr);

template <typename Cntr>
size_t GetCursorSize(Cntr& cntr);

template <typename Cntr>
size_t GetElemSize(Cntr& cntr);

template <typename Cntr>
size_t GetElemCnt(Cntr& cntr);

template <typename Cntr>
size_t GetMaxElemCnt(Cntr& cntr);

template <typename Cntr>
void GetLBCursor(Cntr& cntr, void* dst_cursor);

template <typename Cntr>
void GetRBCursor(Cntr& cntr, void* dst_cursor);

template <typename Cntr>
void PeekL(Cntr& cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
           void* dst_cursor, void* dst_elem);

template <typename Cntr>
void PeekR(Cntr& cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
           void* dst_cursor, void* dst_elem);

template <typename Cntr>
void Refer(Cntr& cntr, size_t idx, bool lazy_copy_elem,
           ElemPtrView* dst_elem_ptr_view, void* dst_cursor, void* dst_elem);

template <typename Cntr>
void Derefer(Cntr& cntr, void* pos_cursor, bool lazy_copy_elem,
             ElemPtrView* dst_elem_ptr_view, void* dst_elem);

template <typename Cntr, typename Reader>
void Read(Cntr& cntr, void* pos_cursor, size_t cnt, Reader&& reader,
          void* dst_cursor);

template <typename Cntr, typename Writer>
void Write(Cntr& cntr, void* pos_cursor, size_t cnt, Writer&& writer,
           void* dst_cursor);

template <typename Cntr, typename ReaderWriter>
void ReadWrite(Cntr& cntr, void* pos_cursor, size_t cnt,
               ReaderWriter&& reader_writer, void* dst_cursor);

template <typename Cntr, typename Writer>
void PushL(Cntr& cntr, size_t cnt, Writer&& writer, void* dst_cursor);

template <typename Cntr, typename Writer>
void PushR(Cntr& cntr, size_t cnt, Writer&& writer, void* dst_cursor);

template <typename Cntr, typename Writer>
void Insert(Cntr& cntr, void* pos_cursor, size_t cnt, Writer&& writer,
            void* dst_cursor);

template <typename Cntr, typename Reader>
void PopL(Cntr& cntr, size_t cnt, Reader&& reader);

template <typename Cntr, typename Reader>
void PopR(Cntr& cntr, size_t cnt, Reader&& reader);

template <typename Cntr, typename Reader>
void Erase(Cntr& cntr, void* pos_cursor, size_t cnt, Reader&& reader);

template <typename Cntr>
void EraseAll(Cntr& cntr);

template <typename Cntr>
void CopyCursor(Cntr& cntr, void* src_cursor, void* dst_cursor);

template <typename Cntr>
bool AreEqualCursor(Cntr& cntr, void* cursor_a, void* cursor_b);

template <typename Cntr>
int CompareCursor(Cntr& cntr, void* cursor_a, void* cursor_b);

template <typename Cntr>
size_t GetCursorDist(Cntr& cntr, void* cursor_a, void* cursor_b);

template <typename Cntr>
size_t GetCursorIdx(Cntr& cntr, void* cursor);

template <typename Cntr>
void CursorStepL(Cntr& cntr, void* cursor);

template <typename Cntr>
void CursorStepR(Cntr& cntr, void* cursor);

template <typename Cntr>
void CursorAdvanceL(Cntr& cntr, void* cursor, size_t step);

template <typename Cntr>
void CursorAdvanceR(Cntr& cntr, void* cursor, size_t step);

template <typename Cntr>
void CheckContract(Cntr& cntr);

struct VTable {
    unsigned long long custom_tags[4];

    size_t (*GetElemCnt)(void* cntr);

    size_t (*GetMaxElemCnt)(void* cntr);

    void (*GetLBCursor)(void* cntr, void* dst_cursor);

    void (*GetRBCursor)(void* cntr, void* dst_cursor);

    void (*PeekL)(void* cntr, bool lazy_copy_elem,
                  ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                  void* dst_elem);

    void (*PeekR)(void* cntr, bool lazy_copy_elem,
                  ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                  void* dst_elem);

    void (*Refer)(void* cntr, size_t idx, bool lazy_copy_elem,
                  ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                  void* dst_elem);

    void (*Derefer)(void* cntr, void* pos_cursor, bool lazy_copy_elem,
                    ElemPtrView* dst_elem_ptr_view, void* dst_elem);

    void (*EmptyRead)(void* cntr, void* pos_cursor, size_t cnt,
                      EmptyReader reader, void* dst_cursor);

    void (*MemRead)(void* cntr, void* pos_cursor, size_t cnt, MemReader& reader,
                    void* dst_cursor);

    void (*FnRead)(void* cntr, void* pos_cursor, size_t cnt, FnReader reader,
                   void* dst_cursor);

    void (*EmptyWrite)(void* cntr, void* pos_cursor, size_t cnt,
                       EmptyWriter writer, void* dst_cursor);

    void (*MemWrite)(void* cntr, void* pos_cursor, size_t cnt,
                     MemWriter& writer, void* dst_cursor);

    void (*FnWrite)(void* cntr, void* pos_cursor, size_t cnt, FnWriter writer,
                    void* dst_cursor);

    void (*FnReadWrite)(void* cntr, void* pos_cursor, size_t cnt,
                        FnReaderWriter reader_writer, void* dst_cursor);

    void (*EmptyPushL)(void* cntr, size_t cnt, EmptyWriter writer,
                       void* dst_cursor);

    void (*MemPushL)(void* cntr, size_t cnt, MemWriter& writer,
                     void* dst_cursor);

    void (*FnPushL)(void* cntr, size_t cnt, FnWriter writer, void* dst_cursor);

    void (*EmptyPushR)(void* cntr, size_t cnt, EmptyWriter writer,
                       void* dst_cursor);

    void (*MemPushR)(void* cntr, size_t cnt, MemWriter& writer,
                     void* dst_cursor);

    void (*FnPushR)(void* cntr, size_t cnt, FnWriter writer, void* dst_cursor);

    void (*EmptyInsert)(void* cntr, void* pos_cursor, size_t cnt,
                        EmptyWriter writer, void* dst_cursor);

    void (*MemInsert)(void* cntr, void* pos_cursor, size_t cnt,
                      MemWriter& writer, void* dst_cursor);

    void (*FnInsert)(void* cntr, void* pos_cursor, size_t cnt, FnWriter writer,
                     void* dst_cursor);

    void (*EmptyPopL)(void* cntr, size_t cnt, EmptyReader reader);

    void (*MemPopL)(void* cntr, size_t cnt, MemReader& reader);

    void (*FnPopL)(void* cntr, size_t cnt, FnReader reader);

    void (*EmptyPopR)(void* cntr, size_t cnt, EmptyReader reader);

    void (*MemPopR)(void* cntr, size_t cnt, MemReader& reader);

    void (*FnPopR)(void* cntr, size_t cnt, FnReader reader);

    void (*EmptyErase)(void* cntr, void* pos_cursor, size_t cnt,
                       EmptyReader reader);

    void (*MemErase)(void* cntr, void* pos_cursor, size_t cnt,
                     MemReader& reader);

    void (*FnErase)(void* cntr, void* pos_cursor, size_t cnt, FnReader reader);

    void (*EraseAll)(void* cntr);

    void (*CopyCursor)(void* cntr, void* src_cursor, void* dst_cursor);

    bool (*AreEqualCursor)(void* cntr, void* cursor_a, void* cursor_b);

    int (*CompareCursor)(void* cntr, void* cursor_a, void* cursor_b);

    size_t (*GetCursorDist)(void* cntr, void* cursor_a, void* cursor_b);

    size_t (*GetCursorIdx)(void* cntr, void* cursor);

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

namespace check_operation {

constexpr bool CanRefer(size_t idx, size_t cnt, size_t elem_cnt);

constexpr bool CanDerefer(size_t idx, size_t cnt, size_t elem_cnt);

constexpr bool CanPushL(size_t cnt, size_t elem_cnt, size_t max_elem_cnt);

constexpr bool CanPushR(size_t cnt, size_t elem_cnt, size_t max_elem_cnt);

constexpr bool CanInsert(size_t idx, size_t cnt, size_t elem_cnt,
                         size_t max_elem_cnt);

constexpr bool CanPopL(size_t cnt, size_t elem_cnt);

constexpr bool CanPopR(size_t cnt, size_t elem_cnt);

constexpr bool CanErase(size_t idx, size_t cnt, size_t elem_cnt);

constexpr bool CanStepL(size_t idx, size_t elem_cnt);

constexpr bool CanStepR(size_t idx, size_t elem_cnt);

constexpr bool CanAdvancableL(size_t idx, size_t step, size_t elem_cnt);

constexpr bool CanAdvancableR(size_t idx, size_t step, size_t elem_cnt);

}  // namespace check_operation

template <typename DstSeqCntr, typename SrcSeqCntr>
void RangeAssign(DstSeqCntr& dst_cntr, SrcSeqCntr& src_cntr, size_t dst_beg,
                 size_t src_beg, size_t cnt);

template <typename DstSeqCntr, typename SrcSeqCntr>
void Assign(DstSeqCntr& dst_cntr, SrcSeqCntr& src_cntr);

}  // namespace zeta::core::seq_cntr
