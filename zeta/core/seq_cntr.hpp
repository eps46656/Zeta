/*
@file zeta/core/seq_cntr.hpp
*/

#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/elem_stream.hpp>
#include <zeta/core/fn_elem_stream.hpp>
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

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_SeqCntr_AllocaCursor(cntr)             \
    ({                                                   \
        __builtin_alloca_with_align(                     \
            ::zeta::core::seq_cntr::GetCursorSize(cntr), \
            __CHAR_BIT__ * alignof(max_align_t));        \
    })

// clang-format off

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_SeqCntr_CapabilityWithoutAlwaysNever_XMacro(func, sep) \
    func(GetCursorSize, 2) sep                                                 \
                                                                               \
    func(GetElemSize, 3) sep                                                   \
    func(GetElemCnt, 4) sep                                                    \
    func(GetMaxElemCnt, 5) sep                                                 \
                                                                               \
    func(GetLBCursor, 6) sep                                                   \
    func(GetRBCursor, 7) sep                                                   \
                                                                               \
    func(PeekL, 8) sep                                                         \
    func(PeekR, 9) sep                                                         \
                                                                               \
    func(Refer, 10) sep                                                        \
    func(Derefer, 11) sep                                                      \
                                                                               \
    func(Read, 12) sep                                                         \
    func(Write, 13) sep                                                        \
    func(ReadWrite, 14) sep                                                    \
                                                                               \
    func(PushL, 15) sep                                                        \
    func(PushR, 16) sep                                                        \
    func(Insert, 17) sep                                                       \
                                                                               \
    func(PopL, 18) sep                                                         \
    func(PopR, 19) sep                                                         \
    func(Erase, 20) sep                                                        \
    func(EraseAll, 21) sep                                                     \
                                                                               \
    func(CopyCursor, 22) sep                                                   \
                                                                               \
    func(AreEqualCursor, 23) sep                                               \
    func(CompareCursor, 24) sep                                                \
    func(GetCursorDist, 25) sep                                                \
    func(GetCursorIdx, 26) sep                                                 \
                                                                               \
    func(CursorStepL, 27) sep                                                  \
    func(CursorStepR, 28) sep                                                  \
                                                                               \
    func(CursorAdvanceL, 29) sep                                               \
    func(CursorAdvanceR, 30)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_SeqCntr_Capability_XMacro(func, sep)                         \
    func(Always, 0) sep                                                        \
    func(Never, 1) sep                                                         \
    ZETA_Core_SeqCntr_CapabilityWithoutAlwaysNever_XMacro(func, sep)

// clang-format on

using CapabilityFlag = unsigned;

struct CapabilityEnum {
    using Type = unsigned;

#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num)                                                        \
    struct name {                                                           \
        static constexpr Type value{ num };                                 \
        ZETA_Core_StaticAssert(value <= integral::WidthOf<CapabilityFlag>); \
    };

    ZETA_Core_SeqCntr_Capability_XMacro(F, );

#pragma pop_macro("F")
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
struct CapabilityFlagBuilder {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num) bool const name;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    ZETA_Core_SeqCntr_CapabilityWithoutAlwaysNever_XMacro(F, );

#pragma pop_macro("F")

    constexpr CapabilityFlag operator()() const {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num) \
    (static_cast<CapabilityFlag>(this->name) << CapabilityEnum::name::value)

        return (static_cast<CapabilityFlag>(1)
                << CapabilityEnum::Always::value) |
               ZETA_Core_SeqCntr_CapabilityWithoutAlwaysNever_XMacro(F, |);

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

/**
@page zeta__core__seq_cntr__CntrTraits CntrTraits
*/
template <typename Cntr, typename = void>
struct CntrTraits;  // IWYU pragma: export

namespace detail {

template <typename Cntr>
concept HasCapabilitySystem_ = requires(Cntr& cntr) {
    requires meta::IsSame<
        meta::RemoveCVRef<decltype(CntrTraits<Cntr>::GetReferedInstPtr(cntr))>,
        void*>;

    requires meta::IsSame<
        meta::RemoveCVRef<
            decltype(CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag())>,
        CapabilityFlag>;

    requires(CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag() &
             empty_capability_flag) == empty_capability_flag;

    requires(CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag() |
             full_capability_flag) == full_capability_flag;

    requires meta::IsSame<
        meta::RemoveCVRef<
            decltype(CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag())>,
        CapabilityFlag>;

    requires(CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag() &
             empty_capability_flag) == empty_capability_flag;

    requires(CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag() |
             full_capability_flag) == full_capability_flag;

    requires meta::IsSame<
        meta::RemoveCVRef<
            decltype(CntrTraits<Cntr>::GetDynamicEnabledCapabilityFlag(cntr))>,
        CapabilityFlag>;

    requires meta::IsSame<
        meta::RemoveCVRef<
            decltype(CntrTraits<Cntr>::GetDynamicDisabledCapabilityFlag(cntr))>,
        CapabilityFlag>;

    requires(CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag() &
             CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag()) ==
                empty_capability_flag;
};

#pragma push_macro("MethodConceptGenMacro")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MethodConceptGenMacro(capability, method, ret, ...)                    \
    template <typename Cntr>                                                   \
    concept Satisfies##method##Contract_ = requires(                           \
        Cntr& cntr, bool bool_val, void* void_ptr, void const* const_void_ptr, \
        int int_val, size_t size_val, ElemPtrView* elem_ptr_view_ptr,          \
        elem_stream::acceptor::ArchetAcceptor reader,                          \
        elem_stream::provider::ArchetProvider writer,                          \
        elem_stream::provider::ArchetProvider reader_writer,                   \
        meta::AlwaysMatchedTag unused) {                                       \
        requires HasCapabilitySystem_<Cntr>;                                   \
                                                                               \
        requires(CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag() &         \
                 (static_cast<CapabilityFlag>(1)                               \
                  << CapabilityEnum::capability::value)) != 0 ||               \
                        requires {                                             \
                            requires meta::IsMatched<                          \
                                meta::RemoveCVRef<                             \
                                    decltype(CntrTraits<Cntr>::method(         \
                                        __VA_ARGS__))>,                        \
                                decltype(ret)>;                                \
                        };                                                     \
    };                                                                         \
    ZETA_Core_StaticAssert(true)

MethodConceptGenMacro(  //
    GetCursorSize,      // capability
    GetCursorSize,      // method
                        //
    size_val,           // ret
                        //
    cntr                // cntr
);

MethodConceptGenMacro(  //
    GetElemSize,        // capability
    GetElemSize,        // method
                        //
    size_val,           // ret
                        //
    cntr                // cntr
);

MethodConceptGenMacro(  //
    GetElemCnt,         // capability
    GetElemCnt,         // method
                        //
    size_val,           // ret
                        //
    cntr                // cntr
);

MethodConceptGenMacro(  //
    GetMaxElemCnt,      // capability
    GetMaxElemCnt,      // method
                        //
    size_val,           // ret
                        //
    cntr                // cntr
);

MethodConceptGenMacro(  //
    GetLBCursor,        // capability
    GetLBCursor,        // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    void_ptr            // cursor
);

MethodConceptGenMacro(  //
    GetRBCursor,        // capability
    GetRBCursor,        // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    void_ptr            // cursor
);

MethodConceptGenMacro(  //
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

MethodConceptGenMacro(  //
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

MethodConceptGenMacro(  //
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

MethodConceptGenMacro(  //
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

MethodConceptGenMacro(  //
    Read,               // capability
    Read,               // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    void_ptr,           // pos_cursor
    size_val,           // cnt
    reader,             // reader
    void_ptr            // dst_cursor
);

MethodConceptGenMacro(  //
    Write,              // capability
    Write,              // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    void_ptr,           // pos_cursor, point to original position
    size_val,           // cnt
    writer,             // writer
    void_ptr            // dst_cursor, optional, point to final position
                        // after write
);

MethodConceptGenMacro(  //
    ReadWrite,          // capability
    ReadWrite,          // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    void_ptr,           // pos_cursor
    size_val,           // cnt
    reader_writer,      // reader_writer
    void_ptr            // dst_cursor
);

MethodConceptGenMacro(  //
    PushL,              // capability
    PushL,              // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    size_val,           // cnt
    writer,             // writer
    void_ptr            // dst_cursor
);

MethodConceptGenMacro(  //
    PushR,              // capability
    PushR,              // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    size_val,           // cnt
    writer,             //
    void_ptr            // dst_cursor
);

MethodConceptGenMacro(  //
    Insert,             // capability
    Insert,             // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    void_ptr,           // pos_cursor
    size_val,           // cnt
    writer,             // writer
    void_ptr            // dst_cursor
);

MethodConceptGenMacro(  //
    PopL,               // capability
    PopL,               // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    size_val,           // cnt
    reader              // reader
);

MethodConceptGenMacro(  //
    PopR,               // capability
    PopR,               // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    size_val,           // cnt
    reader              // reader
);

MethodConceptGenMacro(  //
    Erase,              // capability
    Erase,              // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    void_ptr,           // pos_cursor
    size_val,           // cnt
    reader              // reader
);

MethodConceptGenMacro(  //
    EraseAll,           // capability
    EraseAll,           // method
                        //
    unused,             // ret
                        //
    cntr                // cntr
);

MethodConceptGenMacro(  //
    CopyCursor,         // capability
    CopyCursor,         // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    void_ptr,           // src_cursor
    void_ptr            // dst_cursor
);

MethodConceptGenMacro(  //
    AreEqualCursor,     // capability
    AreEqualCursor,     // method
                        //
    bool_val,           //
                        //
    cntr,               // cntr
    void_ptr,           // cursor_a
    void_ptr            // cursor_b
);

MethodConceptGenMacro(  //
    CompareCursor,      // capability
    CompareCursor,      // method
                        //
    int_val,            //
                        //
    cntr,               // cntr
    void_ptr,           // cursor_a
    void_ptr            // cursor_b
);

MethodConceptGenMacro(  //
    GetCursorDist,      // capability
    GetCursorDist,      // method
                        //
    size_val,           //
                        //
    cntr,               // cntr
    void_ptr,           // cursor_a
    void_ptr            // cursor_b
);

MethodConceptGenMacro(  //
    GetCursorIdx,       // capability
    GetCursorIdx,       // method
                        //
    size_val,           //
                        //
    cntr,               // cntr
    void_ptr            // cursor
);

MethodConceptGenMacro(  //
    CursorStepL,        // capability
    CursorStepL,        // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    void_ptr            // cursor
);

MethodConceptGenMacro(  //
    CursorStepR,        // capability
    CursorStepR,        // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    void_ptr            // cursor
);

MethodConceptGenMacro(  //
    CursorAdvanceL,     // capability
    CursorAdvanceL,     // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    void_ptr,           // cursor
    size_val            // step
);

MethodConceptGenMacro(  //
    CursorAdvanceR,     // capability
    CursorAdvanceR,     // method
                        //
    unused,             // ret
                        //
    cntr,               // cntr
    void_ptr,           // cursor
    size_val            // step
);

#pragma pop_macro("MethodConceptGenMacro")

}  // namespace detail

template <typename Cntr>
concept IsSeqCntr = requires(Cntr& cntr) {
    requires detail::HasCapabilitySystem_<Cntr>;
    requires detail::SatisfiesGetCursorSizeContract_<Cntr>;
    requires detail::SatisfiesGetElemSizeContract_<Cntr>;
    requires detail::SatisfiesGetElemCntContract_<Cntr>;
    requires detail::SatisfiesGetMaxElemCntContract_<Cntr>;
    requires detail::SatisfiesGetLBCursorContract_<Cntr>;
    requires detail::SatisfiesGetRBCursorContract_<Cntr>;
    requires detail::SatisfiesPeekLContract_<Cntr>;
    requires detail::SatisfiesPeekRContract_<Cntr>;
    requires detail::SatisfiesReferContract_<Cntr>;
    requires detail::SatisfiesDereferContract_<Cntr>;
    requires detail::SatisfiesReadContract_<Cntr>;
    requires detail::SatisfiesWriteContract_<Cntr>;
    requires detail::SatisfiesReadWriteContract_<Cntr>;
    requires detail::SatisfiesPushLContract_<Cntr>;
    requires detail::SatisfiesPushRContract_<Cntr>;
    requires detail::SatisfiesInsertContract_<Cntr>;
    requires detail::SatisfiesPopLContract_<Cntr>;
    requires detail::SatisfiesPopRContract_<Cntr>;
    requires detail::SatisfiesEraseContract_<Cntr>;
    requires detail::SatisfiesEraseAllContract_<Cntr>;
    requires detail::SatisfiesCopyCursorContract_<Cntr>;
    requires detail::SatisfiesAreEqualCursorContract_<Cntr>;
    requires detail::SatisfiesCompareCursorContract_<Cntr>;
    requires detail::SatisfiesGetCursorDistContract_<Cntr>;
    requires detail::SatisfiesGetCursorIdxContract_<Cntr>;
    requires detail::SatisfiesCursorStepLContract_<Cntr>;
    requires detail::SatisfiesCursorStepRContract_<Cntr>;
    requires detail::SatisfiesCursorAdvanceLContract_<Cntr>;
    requires detail::SatisfiesCursorAdvanceRContract_<Cntr>;
};

template <typename Cntr>
struct DefaultCntrTraits {
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

constexpr bool CheckCapabilityFlags(
    CapabilityFlag static_enabled_capability_flag,
    CapabilityFlag static_disabled_capability_flag);

bool CheckCapabilityFlags(CapabilityFlag static_enabled_capability_flag,
                          CapabilityFlag static_disabled_capability_flag,
                          CapabilityFlag dynamic_enabled_capability_flag,
                          CapabilityFlag dynamic_disabled_capability_flag);

template <IsSeqCntr Cntr>
void* GetReferedInstPtr(Cntr& cntr);

template <IsSeqCntr Cntr>
constexpr CapabilityFlag GetStaticEnabledCapabilityFlag();

template <IsSeqCntr Cntr>
constexpr CapabilityFlag GetStaticDisabledCapabilityFlag();

template <IsSeqCntr Cntr>
CapabilityFlag GetDynamicEnabledCapabilityFlag(Cntr& cntr);

template <IsSeqCntr Cntr>
CapabilityFlag GetDynamicDisabledCapabilityFlag(Cntr& cntr);

template <IsSeqCntr Cntr>
size_t GetCursorSize(Cntr& cntr);

template <IsSeqCntr Cntr>
size_t GetElemSize(Cntr& cntr);

template <IsSeqCntr Cntr>
size_t GetElemCnt(Cntr& cntr);

template <IsSeqCntr Cntr>
size_t GetMaxElemCnt(Cntr& cntr);

template <IsSeqCntr Cntr>
void GetLBCursor(Cntr& cntr, void* dst_cursor);

template <IsSeqCntr Cntr>
void GetRBCursor(Cntr& cntr, void* dst_cursor);

template <IsSeqCntr Cntr>
void PeekL(Cntr& cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
           void* dst_cursor, void* dst_elem);

template <IsSeqCntr Cntr>
void PeekR(Cntr& cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
           void* dst_cursor, void* dst_elem);

template <IsSeqCntr Cntr>
void Refer(Cntr& cntr, size_t idx, bool lazy_copy_elem,
           ElemPtrView* dst_elem_ptr_view, void* dst_cursor, void* dst_elem);

template <IsSeqCntr Cntr>
void Derefer(Cntr& cntr, void* pos_cursor, bool lazy_copy_elem,
             ElemPtrView* dst_elem_ptr_view, void* dst_elem);

template <IsSeqCntr Cntr, IsReader Reader>
void Read(Cntr& cntr, void* pos_cursor, size_t cnt, Reader&& reader,
          void* dst_cursor);

template <IsSeqCntr Cntr, IsWriter Writer>
void Write(Cntr& cntr, void* pos_cursor, size_t cnt, Writer&& writer,
           void* dst_cursor);

template <IsSeqCntr Cntr, IsReaderWriter ReaderWriter>
void ReadWrite(Cntr& cntr, void* pos_cursor, size_t cnt,
               ReaderWriter&& reader_writer, void* dst_cursor);

template <IsSeqCntr Cntr, IsWriter Writer>
void PushL(Cntr& cntr, size_t cnt, Writer&& writer, void* dst_cursor);

template <IsSeqCntr Cntr, IsWriter Writer>
void PushR(Cntr& cntr, size_t cnt, Writer&& writer, void* dst_cursor);

template <IsSeqCntr Cntr, IsWriter Writer>
void Insert(Cntr& cntr, void* pos_cursor, size_t cnt, Writer&& writer,
            void* dst_cursor);

template <IsSeqCntr Cntr, IsReader Reader>
void PopL(Cntr& cntr, size_t cnt, Reader&& reader);

template <IsSeqCntr Cntr, IsReader Reader>
void PopR(Cntr& cntr, size_t cnt, Reader&& reader);

template <IsSeqCntr Cntr, IsReader Reader>
void Erase(Cntr& cntr, void* pos_cursor, size_t cnt, Reader&& reader);

template <IsSeqCntr Cntr>
void EraseAll(Cntr& cntr);

template <IsSeqCntr Cntr>
void CopyCursor(Cntr& cntr, void* src_cursor, void* dst_cursor);

template <IsSeqCntr Cntr>
bool AreEqualCursor(Cntr& cntr, void* cursor_a, void* cursor_b);

template <IsSeqCntr Cntr>
int CompareCursor(Cntr& cntr, void* cursor_a, void* cursor_b);

template <IsSeqCntr Cntr>
size_t GetCursorDist(Cntr& cntr, void* cursor_a, void* cursor_b);

template <IsSeqCntr Cntr>
size_t GetCursorIdx(Cntr& cntr, void* cursor);

template <IsSeqCntr Cntr>
void CursorStepL(Cntr& cntr, void* cursor);

template <IsSeqCntr Cntr>
void CursorStepR(Cntr& cntr, void* cursor);

template <IsSeqCntr Cntr>
void CursorAdvanceL(Cntr& cntr, void* cursor, size_t step);

template <IsSeqCntr Cntr>
void CursorAdvanceR(Cntr& cntr, void* cursor, size_t step);

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

    void (*Read_EmptyAcceptor)(void* cntr, void* pos_cursor, size_t cnt,
                               elem_stream::provider::EmptyProvider reader,
                               void* dst_cursor);

    void (*Read_LinSeqAcceptor)(void* cntr, void* pos_cursor, size_t cnt,
                                lin_seq_elem_stream::Acceptor& reader,
                                void* dst_cursor);

    void (*Read_FnAcceptor)(void* cntr, void* pos_cursor, size_t cnt,
                            fn_elem_stream::Acceptor reader, void* dst_cursor);

    void (*Write_EmptyProvider)(void* cntr, void* pos_cursor, size_t cnt,
                                elem_stream::provider::EmptyProvider writer,
                                void* dst_cursor);

    void (*Write_LinSeqProvider)(void* cntr, void* pos_cursor, size_t cnt,
                                 lin_seq_elem_stream::Provider& writer,
                                 void* dst_cursor);

    void (*Write_FnProvider)(void* cntr, void* pos_cursor, size_t cnt,
                             fn_elem_stream::Provider writer, void* dst_cursor);

    void (*ReadWrite_FnProvider)(void* cntr, void* pos_cursor, size_t cnt,
                                 fn_elem_stream::Provider reader_writer,
                                 void* dst_cursor);

    void (*PushL_EmptyProvider)(void* cntr, size_t cnt,
                                elem_stream::provider::EmptyProvider writer,
                                void* dst_cursor);

    void (*PushL_LinSeqProvider)(void* cntr, size_t cnt,
                                 lin_seq_elem_stream::Provider& writer,
                                 void* dst_cursor);

    void (*PushL_FnProvider)(void* cntr, size_t cnt,
                             fn_elem_stream::Provider writer, void* dst_cursor);

    void (*PushR_EmptyProvider)(void* cntr, size_t cnt,
                                elem_stream::provider::EmptyProvider writer,
                                void* dst_cursor);

    void (*PushR_LinSeqProvider)(void* cntr, size_t cnt,
                                 lin_seq_elem_stream::Provider& writer,
                                 void* dst_cursor);

    void (*PushR_FnProvider)(void* cntr, size_t cnt,
                             fn_elem_stream::Provider writer, void* dst_cursor);

    void (*Insert_EmptyProvider)(void* cntr, void* pos_cursor, size_t cnt,
                                 elem_stream::provider::EmptyProvider writer,
                                 void* dst_cursor);

    void (*Insert_LinSeqProvider)(void* cntr, void* pos_cursor, size_t cnt,
                                  lin_seq_elem_stream::Provider& writer,
                                  void* dst_cursor);

    void (*Insert_FnProvider)(void* cntr, void* pos_cursor, size_t cnt,
                              fn_elem_stream::Provider writer,
                              void* dst_cursor);

    void (*PopL_EmptyAcceptor)(void* cntr, size_t cnt,
                               elem_stream::acceptor::EmptyAcceptor reader);

    void (*PopL_LinSeqAcceptor)(void* cntr, size_t cnt,
                                lin_seq_elem_stream::Acceptor& reader);

    void (*PopL_FnAcceptor)(void* cntr, size_t cnt,
                            fn_elem_stream::Acceptor reader);

    void (*PopR_EmptyAcceptor)(void* cntr, size_t cnt,
                               elem_stream::acceptor::EmptyAcceptor reader);

    void (*PopR_LinSeqAcceptor)(void* cntr, size_t cnt,
                                lin_seq_elem_stream::Acceptor& reader);

    void (*PopR_FnAcceptor)(void* cntr, size_t cnt,
                            fn_elem_stream::Acceptor reader);

    void (*Erase_EmptyAcceptor)(void* cntr, void* pos_cursor, size_t cnt,
                                elem_stream::acceptor::EmptyAcceptor reader);

    void (*Erase_LinSeqAcceptor)(void* cntr, void* pos_cursor, size_t cnt,
                                 lin_seq_elem_stream::Acceptor& reader);

    void (*Erase_FnAcceptor)(void* cntr, void* pos_cursor, size_t cnt,
                             fn_elem_stream::Acceptor reader);

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

template <IsSeqCntr Cntr>
constexpr VTable BuildVTableBasic();

template <IsSeqCntr Cntr, typename = void>
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
