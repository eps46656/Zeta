/*
@file zeta/core/seq_cntr.hpp
*/

#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/elem_stream.hpp>
#include <zeta/core/elem_stream_ref.hpp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/lin_seq_elem_stream.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core::seq_cntr {

constexpr size_t max_max_elem_cnt{ integral::RangeMaxOf<size_t> / 2 };

template <typename Reader>
concept IsReader = elem_stream::acceptor::IsAcceptor<Reader>;

template <typename Writer>
concept IsWriter = elem_stream::provider::IsProvider<Writer>;

template <typename ReaderWriter>
concept IsReaderWriter = elem_stream::provider::IsProvider<ReaderWriter>;

using EmptyReader = elem_stream::acceptor::EmptyAcceptor;
using EmptyWriter = elem_stream::provider::EmptyProvider;
using EmptyReaderWriter = elem_stream::provider::EmptyProvider;

using LinSeqReader = lin_seq_elem_stream::Acceptor;
using LinSeqWriter = lin_seq_elem_stream::Provider;

using FnReader = elem_stream_ref::Acceptor;
using FnWriter = elem_stream_ref::Provider;
using FnReaderWriter = elem_stream_ref::Provider;

namespace capability {

// clang-format off

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_SeqCntr_CapabilityWithoutAlwaysNever_XMacro(func, sep)       \
    func(GetCursorSize, 2, true) sep                                           \
                                                                               \
    func(GetElemSize, 3, true) sep                                             \
    func(GetElemCnt, 4, true) sep                                              \
    func(GetMaxElemCnt, 5, true) sep                                           \
                                                                               \
    func(GetLBCursor, 6, true) sep                                             \
    func(GetRBCursor, 7, true) sep                                             \
                                                                               \
    func(PeekL, 8, true) sep                                                   \
    func(PeekR, 9, true) sep                                                   \
                                                                               \
    func(Refer, 10, true) sep                                                  \
    func(Derefer, 11, true) sep                                                \
                                                                               \
    func(Read, 12, true) sep                                                   \
    func(Write, 13, false) sep                                                 \
    func(ReadWrite, 14, false) sep                                             \
                                                                               \
    func(PushL, 15, false) sep                                                 \
    func(PushR, 16, false) sep                                                 \
    func(Insert, 17, false) sep                                                \
                                                                               \
    func(PopL, 18, false) sep                                                  \
    func(PopR, 19, false) sep                                                  \
    func(Erase, 20, false) sep                                                 \
    func(EraseAll, 21, false) sep                                              \
                                                                               \
    func(CopyCursor, 22, true) sep                                             \
                                                                               \
    func(AreEqualCursor, 23, true) sep                                         \
    func(CompareCursor, 24, true) sep                                          \
    func(GetCursorDist, 25, true) sep                                          \
    func(GetCursorIdx, 26, true) sep                                           \
                                                                               \
    func(CursorStepL, 27, true) sep                                            \
    func(CursorStepR, 28, true) sep                                            \
                                                                               \
    func(CursorAdvanceL, 29, true) sep                                         \
    func(CursorAdvanceR, 30, true)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_SeqCntr_Capability_XMacro(func, sep)                         \
    func(Always, 0, false) sep                                                 \
    func(Never, 1, false) sep                                                  \
    ZETA_Core_SeqCntr_CapabilityWithoutAlwaysNever_XMacro(func, sep)

// clang-format on

enum struct Kind : unsigned char {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) name = num,

    ZETA_Core_SeqCntr_Capability_XMacro(F, )

#pragma pop_macro("F")
};

using Flag = unsigned;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
struct FlagBuilder {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) bool const name;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    ZETA_Core_SeqCntr_CapabilityWithoutAlwaysNever_XMacro(F, );

#pragma pop_macro("F")

    constexpr Flag operator()() const {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) \
    (static_cast<Flag>(this->name) << meta::ToUnderlying(Kind::name))

        return (static_cast<Flag>(1) << meta::ToUnderlying(Kind::Always)) |
               ZETA_Core_SeqCntr_CapabilityWithoutAlwaysNever_XMacro(F, |);

#pragma pop_macro("F")
    }
};

constexpr Flag empty_capability_flag{ FlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) .name = false,

    ZETA_Core_SeqCntr_CapabilityWithoutAlwaysNever_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr Flag full_capability_flag{ FlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) .name = true,

    ZETA_Core_SeqCntr_CapabilityWithoutAlwaysNever_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr Flag non_const_capability_flag{ FlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) .name = !is_const,

    ZETA_Core_SeqCntr_CapabilityWithoutAlwaysNever_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr Flag const_capability_flag{ FlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) .name = is_const,

    ZETA_Core_SeqCntr_CapabilityWithoutAlwaysNever_XMacro(F, )

#pragma pop_macro("F")
}() };

ZETA_Core_StaticAssert((non_const_capability_flag & const_capability_flag) ==
                       empty_capability_flag);

ZETA_Core_StaticAssert((non_const_capability_flag | const_capability_flag) ==
                       full_capability_flag);

constexpr bool CheckFlags(capability::Flag static_enabled_capability_flag,
                          capability::Flag static_disabled_capability_flag);

constexpr bool CheckFlags(capability::Flag static_enabled_capability_flag,
                          capability::Flag static_disabled_capability_flag,
                          capability::Flag dynamic_enabled_capability_flag,
                          capability::Flag dynamic_disabled_capability_flag);

}  // namespace capability

struct ElemPtrView {
    enum struct AliasabilityEnum : unsigned char {
        Null = 0,
        ReadOnly = 1,
        ReadWrite = 2,
    };

    void* ptr;
    AliasabilityEnum aliasability;

    bool operator==(ElemPtrView const&) const = default;
    bool operator!=(ElemPtrView const&) const = default;
};

struct CursorLimit {
    void* content[8];
} __attribute__((aligned(alignof(max_align_t))));

/**
@page zeta__core__seq_cntr__CntrTraits CntrTraits
*/
template <typename Cntr>
struct CntrTraits;  // IWYU pragma: export

#pragma push_macro("SatisfiesMethodMacro")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SatisfiesMethodMacro(cap, method, ret, ...)                          \
    requires(CntrTraits<                                                     \
                 meta::RemoveRef<Cntr>>::GetStaticDisabledCapabilityFlag() & \
             (static_cast<capability::Flag>(1)                               \
              << meta::ToUnderlying(capability::Kind::cap))) != 0 ||         \
                    meta::IsMatched<                                         \
                        meta::RemoveRef<                                     \
                            decltype(CntrTraits<meta::RemoveRef<Cntr>>::     \
                                         method(__VA_ARGS__))>,              \
                        decltype(ret)>

template <typename Cntr>
concept IsSeqCntr = requires(
    Cntr& cntr, bool bool_val, void* void_ptr, void const* const_void_ptr,
    int int_val, size_t size_val, ElemPtrView* elem_ptr_view_ptr,
    elem_stream::acceptor::ArchetAcceptor reader,
    elem_stream::provider::ArchetProvider writer,
    elem_stream::provider::ArchetProvider reader_writer,
    comparison::Ordering three_way_result_value,
    meta::AlwaysMatchedTag unused) {
    requires requires {
        requires meta::IsSame<
            meta::RemoveRef<decltype(CntrTraits<meta::RemoveRef<Cntr>>::
                                         GetReferedInstPtr(cntr))>,
            void*>;

        requires meta::IsSame<
            meta::RemoveRef<decltype(CntrTraits<meta::RemoveRef<Cntr>>::
                                         GetStaticEnabledCapabilityFlag())>,
            capability::Flag>;

        requires(CntrTraits<
                     meta::RemoveRef<Cntr>>::GetStaticEnabledCapabilityFlag() &
                 capability::empty_capability_flag) ==
                    capability::empty_capability_flag;

        requires(CntrTraits<
                     meta::RemoveRef<Cntr>>::GetStaticEnabledCapabilityFlag() |
                 capability::full_capability_flag) ==
                    capability::full_capability_flag;

        requires meta::IsSame<
            meta::RemoveRef<decltype(CntrTraits<meta::RemoveRef<Cntr>>::
                                         GetStaticDisabledCapabilityFlag())>,
            capability::Flag>;

        requires(CntrTraits<
                     meta::RemoveRef<Cntr>>::GetStaticDisabledCapabilityFlag() &
                 capability::empty_capability_flag) ==
                    capability::empty_capability_flag;

        requires(CntrTraits<
                     meta::RemoveRef<Cntr>>::GetStaticDisabledCapabilityFlag() |
                 capability::full_capability_flag) ==
                    capability::full_capability_flag;

        requires meta::IsSame<
            meta::RemoveRef<decltype(CntrTraits<meta::RemoveRef<Cntr>>::
                                         GetDynamicEnabledCapabilityFlag(
                                             cntr))>,
            capability::Flag>;

        requires meta::IsSame<
            meta::RemoveRef<decltype(CntrTraits<meta::RemoveRef<Cntr>>::
                                         GetDynamicDisabledCapabilityFlag(
                                             cntr))>,
            capability::Flag>;

        requires(CntrTraits<
                     meta::RemoveRef<Cntr>>::GetStaticEnabledCapabilityFlag() &
                 CntrTraits<meta::RemoveRef<Cntr>>::
                     GetStaticDisabledCapabilityFlag()) ==
                    capability::empty_capability_flag;
    };

    SatisfiesMethodMacro(  //
        GetCursorSize,     // capability
        GetCursorSize,     // method
                           //
        size_val,          // ret
                           //
        cntr               // cntr
    );

    SatisfiesMethodMacro(  //
        GetElemSize,       // capability
        GetElemSize,       // method
                           //
        size_val,          // ret
                           //
        cntr               // cntr
    );

    SatisfiesMethodMacro(  //
        GetElemCnt,        // capability
        GetElemCnt,        // method
                           //
        size_val,          // ret
                           //
        cntr               // cntr
    );

    SatisfiesMethodMacro(  //
        GetMaxElemCnt,     // capability
        GetMaxElemCnt,     // method
                           //
        size_val,          // ret
                           //
        cntr               // cntr
    );

    SatisfiesMethodMacro(  //
        GetLBCursor,       // capability
        GetLBCursor,       // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        void_ptr           // cursor
    );

    SatisfiesMethodMacro(  //
        GetRBCursor,       // capability
        GetRBCursor,       // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        void_ptr           // cursor
    );

    SatisfiesMethodMacro(   //
        PeekL,              // capability
        PeekL,              // method
                            //
        unused,             // ret
                            //
        cntr,               // cntr
        bool_val,           // lazy_copy_elem
        elem_ptr_view_ptr,  // dst_elem_ptr_view
        void_ptr,           // dst_cursor, optional
        void_ptr            // dst_elem, optional
    );

    SatisfiesMethodMacro(   //
        PeekR,              // capability
        PeekR,              // method
                            //
        unused,             // ret
                            //
        cntr,               // cntr
        bool_val,           // lazy_copy_elem
        elem_ptr_view_ptr,  // dst_elem_ptr_view
        void_ptr,           // dst_cursor, optional
        void_ptr            // dst_elem, optional
    );

    SatisfiesMethodMacro(   //
        Refer,              // capability
        Refer,              // method
                            //
        unused,             // ret
                            //
        cntr,               // cntr
        size_val,           // idx
        bool_val,           // lazy_copy_elem
        elem_ptr_view_ptr,  // dst_elem_ptr_view
        void_ptr,           // dst_cursor, optional
        void_ptr            // dst_elem, optional
    );

    SatisfiesMethodMacro(   //
        Derefer,            // capability
        Derefer,            // method
                            //
        unused,             // ret
                            //
        cntr,               // cntr
        void_ptr,           // pos_cursor
        bool_val,           // lazy_copy_elem
        elem_ptr_view_ptr,  // dst_elem_ptr_view
        void_ptr            // dst_elem, optional
    );

    SatisfiesMethodMacro(  //
        Read,              // capability
        Read,              // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        void_ptr,          // pos_cursor
        size_val,          // cnt
        reader,            // reader
        void_ptr           // dst_cursor
    );

    SatisfiesMethodMacro(  //
        Write,             // capability
        Write,             // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        void_ptr,          // pos_cursor, point to original position
        size_val,          // cnt
        writer,            // writer
        void_ptr           // dst_cursor, optional, point to final position
                           // after write
    );

    SatisfiesMethodMacro(  //
        ReadWrite,         // capability
        ReadWrite,         // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        void_ptr,          // pos_cursor
        size_val,          // cnt
        reader_writer,     // reader_writer
        void_ptr           // dst_cursor
    );

    SatisfiesMethodMacro(  //
        PushL,             // capability
        PushL,             // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        size_val,          // cnt
        writer,            // writer
        void_ptr           // dst_cursor
    );

    SatisfiesMethodMacro(  //
        PushR,             // capability
        PushR,             // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        size_val,          // cnt
        writer,            //
        void_ptr           // dst_cursor
    );

    SatisfiesMethodMacro(  //
        Insert,            // capability
        Insert,            // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        void_ptr,          // pos_cursor
        size_val,          // cnt
        writer,            // writer
        void_ptr           // dst_cursor
    );

    SatisfiesMethodMacro(  //
        PopL,              // capability
        PopL,              // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        size_val,          // cnt
        reader             // reader
    );

    SatisfiesMethodMacro(  //
        PopR,              // capability
        PopR,              // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        size_val,          // cnt
        reader             // reader
    );

    SatisfiesMethodMacro(  //
        Erase,             // capability
        Erase,             // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        void_ptr,          // pos_cursor
        size_val,          // cnt
        reader             // reader
    );

    SatisfiesMethodMacro(  //
        EraseAll,          // capability
        EraseAll,          // method
                           //
        unused,            // ret
                           //
        cntr               // cntr
    );

    SatisfiesMethodMacro(  //
        CopyCursor,        // capability
        CopyCursor,        // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        void_ptr,          // src_cursor
        void_ptr           // dst_cursor
    );

    SatisfiesMethodMacro(  //
        AreEqualCursor,    // capability
        AreEqualCursor,    // method
                           //
        bool_val,          //
                           //
        cntr,              // cntr
        void_ptr,          // cursor_a
        void_ptr           // cursor_b
    );

    SatisfiesMethodMacro(        //
        CompareCursor,           // capability
        CompareCursor,           // method
                                 //
        three_way_result_value,  //
                                 //
        cntr,                    // cntr
        void_ptr,                // cursor_a
        void_ptr                 // cursor_b
    );

    SatisfiesMethodMacro(  //
        GetCursorDist,     // capability
        GetCursorDist,     // method
                           //
        size_val,          //
                           //
        cntr,              // cntr
        void_ptr,          // cursor_a
        void_ptr           // cursor_b
    );

    SatisfiesMethodMacro(  //
        GetCursorIdx,      // capability
        GetCursorIdx,      // method
                           //
        size_val,          //
                           //
        cntr,              // cntr
        void_ptr           // cursor
    );

    SatisfiesMethodMacro(  //
        CursorStepL,       // capability
        CursorStepL,       // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        void_ptr           // cursor
    );

    SatisfiesMethodMacro(  //
        CursorStepR,       // capability
        CursorStepR,       // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        void_ptr           // cursor
    );

    SatisfiesMethodMacro(  //
        CursorAdvanceL,    // capability
        CursorAdvanceL,    // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        void_ptr,          // cursor
        size_val           // step
    );

    SatisfiesMethodMacro(  //
        CursorAdvanceR,    // capability
        CursorAdvanceR,    // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        void_ptr,          // cursor
        size_val           // step
    );
};

#pragma pop_macro("SatisfiesMethodMacro")

template <IsSeqCntr Cntr>
constexpr decltype(auto) GetReferedInstPtr(Cntr& cntr);

template <IsSeqCntr Cntr>
constexpr decltype(auto) GetStaticEnabledCapabilityFlag();

template <IsSeqCntr Cntr>
constexpr decltype(auto) GetStaticDisabledCapabilityFlag();

template <IsSeqCntr Cntr>
constexpr decltype(auto) GetDynamicEnabledCapabilityFlag(Cntr& cntr);

template <IsSeqCntr Cntr>
constexpr decltype(auto) GetDynamicDisabledCapabilityFlag(Cntr& cntr);

template <IsSeqCntr Cntr>
constexpr decltype(auto) GetCursorSize(Cntr& cntr);

template <IsSeqCntr Cntr>
constexpr decltype(auto) GetElemSize(Cntr& cntr);

template <IsSeqCntr Cntr>
constexpr decltype(auto) GetElemCnt(Cntr& cntr);

template <IsSeqCntr Cntr>
constexpr decltype(auto) GetMaxElemCnt(Cntr& cntr);

template <IsSeqCntr Cntr>
constexpr decltype(auto) GetLBCursor(Cntr& cntr, void* dst_cursor);

template <IsSeqCntr Cntr>
constexpr decltype(auto) GetRBCursor(Cntr& cntr, void* dst_cursor);

template <IsSeqCntr Cntr>
constexpr decltype(auto) PeekL(Cntr& cntr, bool lazy_copy_elem,
                               ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                               void* dst_elem);

template <IsSeqCntr Cntr>
constexpr decltype(auto) PeekR(Cntr& cntr, bool lazy_copy_elem,
                               ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                               void* dst_elem);

template <IsSeqCntr Cntr>
constexpr decltype(auto) Refer(Cntr& cntr, size_t idx, bool lazy_copy_elem,
                               ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                               void* dst_elem);

template <IsSeqCntr Cntr>
constexpr decltype(auto) Derefer(Cntr& cntr, void* pos_cursor,
                                 bool lazy_copy_elem,
                                 ElemPtrView* dst_elem_ptr_view,
                                 void* dst_elem);

template <IsSeqCntr Cntr, IsReader Reader>
constexpr decltype(auto) Read(Cntr& cntr, void* pos_cursor, size_t cnt,
                              Reader&& reader, void* dst_cursor);

template <IsSeqCntr Cntr, IsWriter Writer>
constexpr decltype(auto) Write(Cntr& cntr, void* pos_cursor, size_t cnt,
                               Writer&& writer, void* dst_cursor);

template <IsSeqCntr Cntr, IsReaderWriter ReaderWriter>
constexpr decltype(auto) ReadWrite(Cntr& cntr, void* pos_cursor, size_t cnt,
                                   ReaderWriter&& reader_writer,
                                   void* dst_cursor);

template <IsSeqCntr Cntr, IsWriter Writer>
constexpr decltype(auto) PushL(Cntr& cntr, size_t cnt, Writer&& writer,
                               void* dst_cursor);

template <IsSeqCntr Cntr, IsWriter Writer>
constexpr decltype(auto) PushR(Cntr& cntr, size_t cnt, Writer&& writer,
                               void* dst_cursor);

template <IsSeqCntr Cntr, IsWriter Writer>
constexpr decltype(auto) Insert(Cntr& cntr, void* pos_cursor, size_t cnt,
                                Writer&& writer, void* dst_cursor);

template <IsSeqCntr Cntr, IsReader Reader>
constexpr decltype(auto) PopL(Cntr& cntr, size_t cnt, Reader&& reader);

template <IsSeqCntr Cntr, IsReader Reader>
constexpr decltype(auto) PopR(Cntr& cntr, size_t cnt, Reader&& reader);

template <IsSeqCntr Cntr, IsReader Reader>
constexpr decltype(auto) Erase(Cntr& cntr, void* pos_cursor, size_t cnt,
                               Reader&& reader);

template <IsSeqCntr Cntr>
constexpr decltype(auto) EraseAll(Cntr& cntr);

template <IsSeqCntr Cntr>
constexpr decltype(auto) CopyCursor(Cntr& cntr, void* src_cursor,
                                    void* dst_cursor);

template <IsSeqCntr Cntr>
constexpr decltype(auto) AreEqualCursor(Cntr& cntr, void* cursor_a,
                                        void* cursor_b);

template <IsSeqCntr Cntr>
constexpr decltype(auto) CompareCursor(Cntr& cntr, void* cursor_a,
                                       void* cursor_b);

template <IsSeqCntr Cntr>
constexpr decltype(auto) GetCursorDist(Cntr& cntr, void* cursor_a,
                                       void* cursor_b);

template <IsSeqCntr Cntr>
constexpr decltype(auto) GetCursorIdx(Cntr& cntr, void* cursor);

template <IsSeqCntr Cntr>
constexpr decltype(auto) CursorStepL(Cntr& cntr, void* cursor);

template <IsSeqCntr Cntr>
constexpr decltype(auto) CursorStepR(Cntr& cntr, void* cursor);

template <IsSeqCntr Cntr>
constexpr decltype(auto) CursorAdvanceL(Cntr& cntr, void* cursor, size_t step);

template <IsSeqCntr Cntr>
constexpr decltype(auto) CursorAdvanceR(Cntr& cntr, void* cursor, size_t step);

template <typename Cntr, typename Cursor>
struct MemberFuncCntrTraitsAdapter {
    static constexpr decltype(auto) GetReferedInstPtr(Cntr& cntr);

    static constexpr decltype(auto) GetStaticEnabledCapabilityFlag();

    static constexpr decltype(auto) GetStaticDisabledCapabilityFlag();

    static constexpr decltype(auto) GetDynamicEnabledCapabilityFlag(Cntr& cntr);

    static constexpr decltype(auto) GetDynamicDisabledCapabilityFlag(
        Cntr& cntr);

    static constexpr decltype(auto) GetCursorSize(Cntr& cntr);

    static constexpr decltype(auto) GetElemSize(Cntr& cntr);

    static constexpr decltype(auto) GetElemCnt(Cntr& cntr);

    static constexpr decltype(auto) GetMaxElemCnt(Cntr& cntr);

    static constexpr decltype(auto) GetLBCursor(Cntr& cntr, void* dst_cursor);

    static constexpr decltype(auto) GetRBCursor(Cntr& cntr, void* dst_cursor);

    static constexpr decltype(auto) PeekL(Cntr& cntr, bool lazy_copy_elem,
                                          ElemPtrView* dst_elem_ptr_view,
                                          void* dst_cursor, void* dst_elem);

    static constexpr decltype(auto) PeekR(Cntr& cntr, bool lazy_copy_elem,
                                          ElemPtrView* dst_elem_ptr_view,
                                          void* dst_cursor, void* dst_elem);

    static constexpr decltype(auto) Refer(Cntr& cntr, size_t idx,
                                          bool lazy_copy_elem,
                                          ElemPtrView* dst_elem_ptr_view,
                                          void* dst_cursor, void* dst_elem);

    static constexpr decltype(auto) Derefer(Cntr& cntr, void* pos_cursor,
                                            bool lazy_copy_elem,
                                            ElemPtrView* dst_elem_ptr_view,
                                            void* dst_elem);

    template <IsReader Reader>
    static constexpr decltype(auto) Read(Cntr& cntr, void* pos_cursor,
                                         size_t cnt, Reader&& reader,
                                         void* dst_cursor);

    template <IsWriter Writer>
    static constexpr decltype(auto) Write(Cntr& cntr, void* pos_cursor,
                                          size_t cnt, Writer&& writer,
                                          void* dst_cursor);

    template <IsReaderWriter ReaderWriter>
    static constexpr decltype(auto) ReadWrite(Cntr& cntr, void* pos_cursor,
                                              size_t cnt,
                                              ReaderWriter&& reader_writer,
                                              void* dst_cursor);

    template <IsWriter Writer>
    static constexpr decltype(auto) PushL(Cntr& cntr, size_t cnt,
                                          Writer&& writer, void* dst_cursor);

    template <IsWriter Writer>
    static constexpr decltype(auto) PushR(Cntr& cntr, size_t cnt,
                                          Writer&& writer, void* dst_cursor);

    template <IsWriter Writer>
    static constexpr decltype(auto) Insert(Cntr& cntr, void* pos_cursor,
                                           size_t cnt, Writer&& writer,
                                           void* dst_cursor);

    template <IsReader Reader>
    static constexpr decltype(auto) PopL(Cntr& cntr, size_t cnt,
                                         Reader&& reader);

    template <IsReader Reader>
    static constexpr decltype(auto) PopR(Cntr& cntr, size_t cnt,
                                         Reader&& reader);

    template <IsReader Reader>
    static constexpr decltype(auto) Erase(Cntr& cntr, void* pos_cursor,
                                          size_t cnt, Reader&& reader);

    static constexpr decltype(auto) EraseAll(Cntr& cntr);

    static constexpr decltype(auto) CopyCursor(Cntr& cntr, void* src_cursor,
                                               void* dst_cursor);

    static constexpr decltype(auto) AreEqualCursor(Cntr& cntr, void* cursor_a,
                                                   void* cursor_b);

    static constexpr decltype(auto) CompareCursor(Cntr& cntr, void* cursor_a,
                                                  void* cursor_b);

    static constexpr decltype(auto) GetCursorDist(Cntr& cntr, void* cursor_a,
                                                  void* cursor_b);

    static constexpr decltype(auto) GetCursorIdx(Cntr& cntr, void* cursor);

    static constexpr decltype(auto) CursorStepL(Cntr& cntr, void* cursor);

    static constexpr decltype(auto) CursorStepR(Cntr& cntr, void* cursor);

    static constexpr decltype(auto) CursorAdvanceL(Cntr& cntr, void* cursor,
                                                   size_t step);

    static constexpr decltype(auto) CursorAdvanceR(Cntr& cntr, void* cursor,
                                                   size_t step);
};

struct VTable {
    unsigned long long custom_tags[4];

    size_t (*get_elem_cnt)(void* cntr);

    size_t (*get_max_elem_cnt)(void* cntr);

    void (*get_lb_cursor)(void* cntr, void* dst_cursor);

    void (*get_rb_cursor)(void* cntr, void* dst_cursor);

    void (*peek_l)(void* cntr, bool lazy_copy_elem,
                   ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                   void* dst_elem);

    void (*peek_r)(void* cntr, bool lazy_copy_elem,
                   ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                   void* dst_elem);

    void (*refer)(void* cntr, size_t idx, bool lazy_copy_elem,
                  ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                  void* dst_elem);

    void (*derefer)(void* cntr, void* pos_cursor, bool lazy_copy_elem,
                    ElemPtrView* dst_elem_ptr_view, void* dst_elem);

    struct {
        void (*empty)(void* cntr, void* pos_cursor, size_t cnt,
                      EmptyReader reader, void* dst_cursor);
        void (*lin_seq)(void* cntr, void* pos_cursor, size_t cnt,
                        LinSeqReader& reader, void* dst_cursor);
        void (*fn)(void* cntr, void* pos_cursor, size_t cnt, FnReader reader,
                   void* dst_cursor);
    } read;

    struct {
        void (*empty)(void* cntr, void* pos_cursor, size_t cnt,
                      EmptyWriter writer, void* dst_cursor);
        void (*lin_seq)(void* cntr, void* pos_cursor, size_t cnt,
                        LinSeqWriter& writer, void* dst_cursor);
        void (*fn)(void* cntr, void* pos_cursor, size_t cnt, FnWriter writer,
                   void* dst_cursor);
    } write;

    struct {
        void (*fn)(void* cntr, void* pos_cursor, size_t cnt,
                   FnReaderWriter reader_writer, void* dst_cursor);
    } read_write;

    struct {
        void (*empty)(void* cntr, size_t cnt, EmptyWriter writer,
                      void* dst_cursor);
        void (*lin_seq)(void* cntr, size_t cnt, LinSeqWriter& writer,
                        void* dst_cursor);
        void (*fn)(void* cntr, size_t cnt, FnWriter writer, void* dst_cursor);
    } push_l;

    struct {
        void (*empty)(void* cntr, size_t cnt, EmptyWriter writer,
                      void* dst_cursor);
        void (*lin_seq)(void* cntr, size_t cnt, LinSeqWriter& writer,
                        void* dst_cursor);
        void (*fn)(void* cntr, size_t cnt, FnWriter writer, void* dst_cursor);
    } push_r;

    struct {
        void (*empty)(void* cntr, void* pos_cursor, size_t cnt,
                      EmptyWriter writer, void* dst_cursor);
        void (*lin_seq)(void* cntr, void* pos_cursor, size_t cnt,
                        LinSeqWriter& writer, void* dst_cursor);
        void (*fn)(void* cntr, void* pos_cursor, size_t cnt, FnWriter writer,
                   void* dst_cursor);
    } insert;

    struct {
        void (*empty)(void* cntr, size_t cnt, EmptyReader reader);
        void (*lin_seq)(void* cntr, size_t cnt, LinSeqReader& reader);
        void (*fn)(void* cntr, size_t cnt, FnReader reader);
    } pop_l;

    struct {
        void (*empty)(void* cntr, size_t cnt, EmptyReader reader);
        void (*lin_seq)(void* cntr, size_t cnt, LinSeqReader& reader);
        void (*fn)(void* cntr, size_t cnt, FnReader reader);
    } pop_r;

    struct {
        void (*empty)(void* cntr, void* pos_cursor, size_t cnt,
                      EmptyReader reader);
        void (*lin_seq)(void* cntr, void* pos_cursor, size_t cnt,
                        LinSeqReader& reader);
        void (*fn)(void* cntr, void* pos_cursor, size_t cnt, FnReader reader);
    } erase;

    void (*erase_all)(void* cntr);

    void (*copy_cursor)(void* cntr, void* src_cursor, void* dst_cursor);

    bool (*are_equal_cursor)(void* cntr, void* cursor_a, void* cursor_b);

    comparison::Ordering (*compare_cursor)(void* cntr, void* cursor_a,
                                           void* cursor_b);

    size_t (*get_cursor_dist)(void* cntr, void* cursor_a, void* cursor_b);

    size_t (*get_cursor_idx)(void* cntr, void* cursor);

    void (*cursor_step_l)(void* cntr, void* cursor);

    void (*cursor_step_r)(void* cntr, void* cursor);

    void (*cursor_advance_l)(void* cntr, void* cursor, size_t step);

    void (*cursor_advance_r)(void* cntr, void* cursor, size_t step);

    unsigned long long (*CustomMethods[4])(void* cntr, unsigned long long arg0,
                                           unsigned long long arg1,
                                           unsigned long long arg2,
                                           unsigned long long arg3);
};

template <IsSeqCntr Cntr>
constexpr VTable BuildVTableBasic();

template <IsSeqCntr Cntr>
struct BuildVTableImpl {
    static constexpr VTable Call();
};

template <IsSeqCntr Cntr>
constexpr VTable BuildVTable();

template <IsSeqCntr Cntr>
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

template <IsSeqCntr DstSeqCntr, IsSeqCntr SrcSeqCntr>
void RangeAssign(DstSeqCntr& dst_cntr, SrcSeqCntr& src_cntr, size_t dst_beg,
                 size_t src_beg, size_t cnt);

template <IsSeqCntr DstSeqCntr, IsSeqCntr SrcSeqCntr>
void Assign(DstSeqCntr& dst_cntr, SrcSeqCntr& src_cntr);

}  // namespace zeta::core::seq_cntr
