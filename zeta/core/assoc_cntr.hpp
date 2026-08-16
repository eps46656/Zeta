#pragma once

#include <zeta/core/comparison.ipp>
#include <zeta/core/comparison_ref.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/elem_stream.ipp>
#include <zeta/core/elem_stream_ref.hpp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/hash_ref.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/lin_seq_elem_stream.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core::assoc_cntr {

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

using FnReader = elem_stream_ref::acceptor::Acceptor;
using FnWriter = elem_stream_ref::provider::Provider;
using FnReaderWriter = elem_stream_ref::provider::Provider;

namespace capability {

// clang-format off

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_AssocCntr_CapabilityWithoutAlwaysNever_XMacro(func, sep)     \
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
    func(Derefer, 10, true) sep                                                \
                                                                               \
    func(Find, 11, true) sep                                                   \
                                                                               \
    func(Insert, 12, false) sep                                                \
                                                                               \
    func(PopL, 13, false) sep                                                  \
    func(PopR, 14, false) sep                                                  \
    func(Erase, 15, false) sep                                                 \
    func(EraseAll, 16, false) sep                                              \
                                                                               \
    func(CopyCursor, 17, true) sep                                             \
                                                                               \
    func(AreEqualCursor, 18, true) sep                                         \
    func(CompareCursor, 19, true) sep                                          \
    func(GetCursorDist, 20, true) sep                                          \
    func(GetCursorIdx, 21, true) sep                                           \
                                                                               \
    func(CursorStepL, 22, true) sep                                            \
    func(CursorStepR, 23, true) sep                                            \
                                                                               \
    func(CursorAdvanceL, 24, true) sep                                         \
    func(CursorAdvanceR, 25, true)


// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_AssocCntr_Capability_XMacro(func, sep)                       \
    func(Always, 0, false) sep                                                 \
    func(Never, 1, false) sep                                                  \
    ZETA_Core_AssocCntr_CapabilityWithoutAlwaysNever_XMacro(func, sep)

// clang-format on

enum struct Kind : unsigned char {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) name = num,

    ZETA_Core_AssocCntr_Capability_XMacro(F, )

#pragma pop_macro("F")
};

using Flag = unsigned int;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
struct FlagBuilder {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) bool const name;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    ZETA_Core_AssocCntr_CapabilityWithoutAlwaysNever_XMacro(F, );

#pragma pop_macro("F")

    constexpr Flag operator()() const {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) \
    (static_cast<Flag>(this->name) << meta::ToUnderlying(Kind::name))

        return (static_cast<Flag>(1) << meta::ToUnderlying(Kind::Always)) |
               ZETA_Core_AssocCntr_CapabilityWithoutAlwaysNever_XMacro(F, |);

#pragma pop_macro("F")
    }
};

constexpr Flag empty_capability_flag{ FlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) .name = false,

    ZETA_Core_AssocCntr_CapabilityWithoutAlwaysNever_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr Flag full_capability_flag{ FlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) .name = true,

    ZETA_Core_AssocCntr_CapabilityWithoutAlwaysNever_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr Flag non_const_capability_flag{ FlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) .name = !is_const,

    ZETA_Core_AssocCntr_CapabilityWithoutAlwaysNever_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr Flag const_capability_flag{ FlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) .name = is_const,

    ZETA_Core_AssocCntr_CapabilityWithoutAlwaysNever_XMacro(F, )

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

template <typename Cntr>
struct CntrTraits;

#pragma push_macro("SatisfiesMethodMacro")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SatisfiesMethodMacro(cap, method, ret, ...)                           \
    requires((CntrTraits<                                                     \
                  meta::RemoveRef<Cntr>>::GetStaticDisabledCapabilityFlag() & \
              (static_cast<capability::Flag>(1)                               \
               << meta::ToUnderlying(capability::Kind::cap))) != 0 ||         \
             meta::IsMatched<                                                 \
                 meta::RemoveRef<decltype(CntrTraits<meta::RemoveRef<Cntr>>:: \
                                              method(__VA_ARGS__))>,          \
                 decltype(ret)>)

template <typename Cntr>
concept IsAssocCntr = requires(
    Cntr& cntr, bool bool_val, void* void_ptr, void const* const_void_ptr,
    size_t size_val, ElemPtrView* elem_ptr_view_ptr,
    hash::ArchetHasher key_hasher, comparison::ArchetComparator key_elem_cmptr,
    elem_stream::acceptor::ArchetAcceptor reader,
    elem_stream::provider::ArchetProvider writer,
    elem_stream::provider::ArchetProvider reader_writer,
    comparison::Ordering ordering_val, meta::AlwaysMatchedTag unused) {
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

        size_val,  // ret
                   //
        cntr       // cntr
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
        unused,            //
                           //
        cntr,              // cntr
        void_ptr           // cursor
    );

    SatisfiesMethodMacro(  //
        GetRBCursor,       // capability
        GetRBCursor,       // method
                           //
        unused,            //
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
        void_ptr,           // dst_cursor
        void_ptr            // dst_elem
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
        void_ptr,           // dst_cursor
        void_ptr            // dst_elem
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
        void_ptr            // dst_elem
    );

    SatisfiesMethodMacro(   //
        Find,               // capability
        Find,               // method
                            //
        unused,             // ret
                            //
        cntr,               // cntr
        const_void_ptr,     // elem
        bool_val,           // lazy_copy_elem
        elem_ptr_view_ptr,  // dst_elem_ptr_view
        void_ptr,           // dst_cursor
        void_ptr            // dst_elem
    );

    SatisfiesMethodMacro(   //
        Find,               // capability
        Find,               // method
                            //
        unused,             // ret
                            //
        cntr,               // cntr
        const_void_ptr,     // key
        key_hasher,         // key_hasher
        key_elem_cmptr,     // key_elem_cmptr
        bool_val,           // lazy_copy_elem
        elem_ptr_view_ptr,  // dst_elem_ptr_view
        void_ptr,           // dst_cursor
        void_ptr            // dst_elem
    );

    SatisfiesMethodMacro(  //
        Insert,            // capability
        Insert,            // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        const_void_ptr,    // elem
        writer,            // writer
        void_ptr           // dst_cursor
    );

    SatisfiesMethodMacro(  //
        Insert,            // capability
        Insert,            // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        const_void_ptr,    // key
        key_hasher,        // key_hasher
        key_elem_cmptr,    // key_elem_cmptr
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
        bool_val,          // ret
                           //
        cntr,              // cntr
        void_ptr,          // cursor_a
        void_ptr           // cursor_b
    );

    SatisfiesMethodMacro(  //
        CompareCursor,     // capability
        CompareCursor,     // method
                           //
        ordering_val,      //
                           // ret
                           //
        cntr,              // cntr
        void_ptr,          // cursor_a
        void_ptr           // cursor_b
    );

    SatisfiesMethodMacro(  //
        GetCursorDist,     // capability
        GetCursorDist,     // method
                           //
        size_val,          // ret
                           //
        cntr,              // cntr
        void_ptr,          // cursor_a
        void_ptr           // cursor_b
    );

    SatisfiesMethodMacro(  //
        GetCursorIdx,      // capability
        GetCursorIdx,      // method
                           //
        size_val,          // ret
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

template <typename Cntr, typename Cursr>
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

    static constexpr decltype(auto) Derefer(Cntr& cntr, void* pos_cursor,
                                            bool lazy_copy_elem,
                                            ElemPtrView* dst_elem_ptr_view,
                                            void* dst_elem);

    static constexpr decltype(auto) Find(Cntr& cntr, void const* elem,
                                         bool lazy_copy_elem,
                                         ElemPtrView* dst_elem_ptr_view,
                                         void* dst_cursor, void* dst_elem);

    template <
        hash::CanHash<void const*> KeyHasher,
        comparison::CanCompare<void const*, void const*> KeyElemComparator>
    static constexpr decltype(auto) Find(
        Cntr& cntr, void const* key, KeyHasher const& key_hasher,
        KeyElemComparator const& key_elem_cmptr, bool lazy_copy_elem,
        ElemPtrView* dst_elem_ptr_view, void* dst_cursor, void* dst_elem);

    template <IsReader Reader>
    static constexpr decltype(auto) Read(Cntr& cntr, void* pos_cursor,
                                         size_t cnt, Reader&& reader,
                                         void* dst_cursor);

    template <IsWriter Writer>
    static constexpr decltype(auto) Insert(Cntr& cntr, void const* elem,
                                           Writer&& writer, void* dst_cursor);

    template <
        hash::CanHash<void const*> KeyHasher,
        comparison::CanCompare<void const*, void const*> KeyElemComparator,
        IsWriter Writer>
    static constexpr decltype(auto) Insert(
        Cntr& cntr, void const* key, KeyHasher const& key_hasher,
        KeyElemComparator const& key_elem_cmptr, Writer&& writer,
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

template <typename Cntr>
constexpr decltype(auto) GetReferedInstPtr(Cntr& cntr);

template <typename Cntr>
constexpr decltype(auto) GetStaticEnabledCapabilityFlag();

template <typename Cntr>
constexpr decltype(auto) GetStaticDisabledCapabilityFlag();

template <typename Cntr>
constexpr decltype(auto) GetDynamicEnabledCapabilityFlag(Cntr& cntr);

template <typename Cntr>
constexpr decltype(auto) GetDynamicDisabledCapabilityFlag(Cntr& cntr);

template <typename Cntr>
constexpr decltype(auto) GetCursorSize(Cntr& cntr);

template <typename Cntr>
constexpr decltype(auto) GetElemSize(Cntr& cntr);

template <typename Cntr>
constexpr decltype(auto) GetElemCnt(Cntr& cntr);

template <typename Cntr>
constexpr decltype(auto) GetMaxElemCnt(Cntr& cntr);

template <typename Cntr>
constexpr decltype(auto) GetLBCursor(Cntr& cntr, void* dst_cursor);

template <typename Cntr>
constexpr decltype(auto) GetRBCursor(Cntr& cntr, void* dst_cursor);

template <typename Cntr>
constexpr decltype(auto) PeekL(Cntr& cntr, bool lazy_copy_elem,
                               assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                               void* dst_cursor, void* dst_elem);

template <typename Cntr>
constexpr decltype(auto) PeekR(Cntr& cntr, bool lazy_copy_elem,
                               assoc_cntr::ElemPtrView* dst_elem_ptr_view,
                               void* dst_cursor, void* dst_elem);

template <typename Cntr>
constexpr decltype(auto) Derefer(Cntr& cntr, void* pos_cursor,
                                 bool lazy_copy_elem,
                                 ElemPtrView* dst_elem_ptr_view,
                                 void* dst_elem);

template <typename Cntr>
constexpr decltype(auto) Find(Cntr& cntr, void const* elem, bool lazy_copy_elem,
                              ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                              void* dst_elem);

template <typename Cntr, hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator>
constexpr decltype(auto) Find(Cntr& cntr, void const* key,
                              KeyHasher const& key_hasher,
                              KeyElemComparator const& key_elem_cmptr,
                              bool lazy_copy_elem,
                              ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                              void* dst_elem);

template <typename Cntr, IsWriter Writer>
constexpr decltype(auto) Insert(Cntr& cntr, void const* elem, Writer&& writer,
                                void* dst_cursor);

template <typename Cntr, hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator,
          IsWriter Writer>
constexpr decltype(auto) Insert(Cntr& cntr, void const* key,
                                KeyHasher key_hasher,
                                KeyElemComparator key_elem_cmptr,
                                Writer&& writer, void* dst_cursor);

template <typename Cntr, assoc_cntr::IsReader Reader>
constexpr decltype(auto) PopL(Cntr& cntr, size_t cnt, Reader&& reader);

template <typename Cntr, assoc_cntr::IsReader Reader>
constexpr decltype(auto) PopR(Cntr& cntr, size_t cnt, Reader&& reader);

template <typename Cntr, assoc_cntr::IsReader Reader>
constexpr decltype(auto) Erase(Cntr& cntr, void* pos_cursor, size_t cnt,
                               Reader&& reader);

template <typename Cntr>
constexpr decltype(auto) EraseAll(Cntr& cntr);

template <typename Cntr>
constexpr decltype(auto) CopyCursor(Cntr& cntr, void* src_cursor,
                                    void* dst_cursor);

template <typename Cntr>
constexpr decltype(auto) AreEqualCursor(Cntr& cntr, void* cursor_a,
                                        void* cursor_b);

template <typename Cntr>
constexpr decltype(auto) CompareCursor(Cntr& cntr, void* cursor_a,
                                       void* cursor_b);

template <typename Cntr>
constexpr decltype(auto) GetCursorDist(Cntr& cntr, void* cursor_a,
                                       void* cursor_b);

template <typename Cntr>
constexpr decltype(auto) GetCursorIdx(Cntr& cntr, void* cursor);

template <typename Cntr>
constexpr decltype(auto) CursorStepL(Cntr& cntr, void* cursor);

template <typename Cntr>
constexpr decltype(auto) CursorStepR(Cntr& cntr, void* cursor);

template <typename Cntr>
constexpr decltype(auto) CursorAdvanceL(Cntr& cntr, void* cursor, size_t step);

template <typename Cntr>
constexpr decltype(auto) CursorAdvanceR(Cntr& cntr, void* cursor, size_t step);

struct TypeErasedWrapper {
    void const* ptr;
};

struct VTable {
    size_t (*get_elem_cnt)(void* cntr);

    size_t (*get_max_elem_cnt)(void* cntr);

    void (*get_lb_cursor)(void* cntr, void* dst_cursor);

    void (*get_rb_cursor)(void* cntr, void* dst_cursor);

    void (*peek_l)(void* cntr, bool lazy_copy_elem,
                   assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                   void* dst_elem);

    void (*peek_r)(void* cntr, bool lazy_copy_elem,
                   assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                   void* dst_elem);

    void (*derefer)(void* cntr, void* pos_cursor, bool lazy_copy_elem,
                    assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem);

    struct {
        struct {
            void (*fn)(void* cntr, void const* elem, bool lazy_copy_elem,
                       ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                       void* dst_elem);
        } with_elem;

        struct {
            void (*fn)(void* cntr, void const* key,
                       hash_ref::Hasher const& key_hasher,
                       comparison_ref::Comparator const& key_elem_cmptr,
                       bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
                       void* dst_cursor, void* dst_elem);
        } with_key;
    } find;

    struct {
        struct {
            void (*empty)(void* cntr, void const* elem, EmptyWriter writer,
                          void* dst_cursor);
            void (*fn)(void* cntr, void const* elem, FnWriter writer,
                       void* dst_cursor);
        } with_elem;

        struct {
            void (*empty)(void* cntr, void const* key,
                          hash_ref::Hasher const& key_hasher,
                          comparison_ref::Comparator const& key_elem_cmptr,
                          EmptyWriter writer, void* dst_cursor);
            void (*fn)(void* cntr, void const* key,
                       hash_ref::Hasher const& key_hasher,
                       comparison_ref::Comparator const& key_elem_cmptr,
                       FnWriter writer, void* dst_cursor);
        } with_key;
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

}  // namespace zeta::core::assoc_cntr
