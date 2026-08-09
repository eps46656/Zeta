#pragma once

#include <zeta/core/elem_stream.hpp>
#include <zeta/core/fn_comparison.hpp>
#include <zeta/core/fn_elem_stream.hpp>
#include <zeta/core/fn_hash.hpp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core::assoc_cntr {

constexpr size_t max_max_elem_cnt{ integral::RangeMaxOf<size_t> / 2 };

template <typename Reader>
concept IsReader = elem_stream::acceptor::IsAcceptor<Reader>;

template <typename Writer>
concept IsWriter = elem_stream::provider::IsProvider<Writer>;

template <typename ReaderWriter>
concept IsReaderWriter = elem_stream::provider::IsProvider<ReaderWriter>;

using FnWriter = fn_elem_stream::Provider;

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
#define SatisfiesMethodMacro(cap, method, ret, ...)                            \
    requires(CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag() &             \
             (static_cast<capability::Flag>(1)                                 \
              << meta::ToUnderlying(capability::Kind::cap))) != 0 ||           \
                    requires {                                                 \
                        requires meta::IsMatched<                              \
                            meta::RemoveRef<decltype(CntrTraits<Cntr>::method( \
                                __VA_ARGS__))>,                                \
                            decltype(ret)>;                                    \
                    }

template <typename Cntr>
concept IsAssocCntr = requires(
    Cntr& cntr, bool bool_val, void* void_ptr, void const* const_void_ptr,
    size_t size_val, ElemPtrView* elem_ptr_view_ptr,
    hash::ArchetHasher key_hasher, comparison::ArchetComparator key_elem_cmptr,
    elem_stream::acceptor::ArchetAcceptor reader,
    elem_stream::provider::ArchetProvider writer,
    elem_stream::provider::ArchetProvider reader_writer,
    comparison::Ordering three_way_result_value,
    meta::AlwaysMatchedTag unused) {
    requires requires {
        requires meta::IsSame<
            meta::RemoveRef<decltype(CntrTraits<Cntr>::GetReferedInstPtr(
                cntr))>,
            void*>;

        requires meta::IsSame<
            meta::RemoveRef<
                decltype(CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag())>,
            capability::Flag>;

        requires(CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag() &
                 capability::empty_capability_flag) ==
                    capability::empty_capability_flag;

        requires(CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag() |
                 capability::full_capability_flag) ==
                    capability::full_capability_flag;

        requires meta::IsSame<
            meta::RemoveRef<
                decltype(CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag())>,
            capability::Flag>;

        requires(CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag() &
                 capability::empty_capability_flag) ==
                    capability::empty_capability_flag;

        requires(CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag() |
                 capability::full_capability_flag) ==
                    capability::full_capability_flag;

        requires meta::IsSame<
            meta::RemoveRef<
                decltype(CntrTraits<Cntr>::GetDynamicEnabledCapabilityFlag(
                    cntr))>,
            capability::Flag>;

        requires meta::IsSame<
            meta::RemoveRef<
                decltype(CntrTraits<Cntr>::GetDynamicDisabledCapabilityFlag(
                    cntr))>,
            capability::Flag>;

        requires(CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag() &
                 CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag()) ==
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
        size_val           // cnt
    );

    SatisfiesMethodMacro(  //
        PopR,              // capability
        PopR,              // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        size_val           // cnt
    );

    SatisfiesMethodMacro(  //
        Erase,             // capability
        Erase,             // method
                           //
        unused,            // ret
                           //
        cntr,              // cntr
        void_ptr           // pos_cursor
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

    SatisfiesMethodMacro(        //
        CompareCursor,           // capability
        CompareCursor,           // method
                                 //
        three_way_result_value,  //
                                 // ret
                                 //
        cntr,                    // cntr
        void_ptr,                // cursor_a
        void_ptr                 // cursor_b
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

template <typename Cntr, hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator>
constexpr decltype(auto) Find(Cntr& cntr, void const* key, KeyHasher key_hasher,
                              KeyElemComparator key_elem_cmptr,
                              bool lazy_copy_elem,
                              ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                              void* dst_elem);

template <typename Cntr, hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator,
          IsWriter Writer>
constexpr decltype(auto) Insert(Cntr& cntr, void const* key,
                                KeyHasher key_hasher,
                                KeyElemComparator key_elem_cmptr,
                                Writer&& writer, void* dst_cursor);

template <typename Cntr>
constexpr decltype(auto) PopL(Cntr& cntr, size_t cnt);

template <typename Cntr>
constexpr decltype(auto) PopR(Cntr& cntr, size_t cnt);

template <typename Cntr>
constexpr decltype(auto) Erase(Cntr& cntr, void* pos_cursor);

template <typename Cntr>
constexpr decltype(auto) EraseAll(Cntr& cntr);

template <typename Cntr>
constexpr decltype(auto) CopyCursor(Cntr& cntr, void* src_cursor,
                                    void* dst_cursor);

template <typename Cntr>
constexpr decltype(auto) AreEqualCursor(Cntr& cntr, void const* cursor_a,
                                        void const* cursor_b);

template <typename Cntr>
constexpr decltype(auto) CompareCursor(Cntr& cntr, void const* cursor_a,
                                       void const* cursor_b);

template <typename Cntr>
constexpr decltype(auto) GetCursorDist(Cntr& cntr, void const* cursor_a,
                                       void const* cursor_b);

template <typename Cntr>
constexpr decltype(auto) GetCursorIdx(Cntr& cntr, void const* cursor);

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
    size_t (*GetElemCnt)(void* cntr);

    size_t (*GetMaxElemCnt)(void* cntr);

    void (*GetLBCursor)(void* cntr, void* dst_cursor);

    void (*GetRBCursor)(void* cntr, void* dst_cursor);

    void (*PeekL)(void* cntr, bool lazy_copy_elem,
                  assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                  void* dst_elem);

    void (*PeekR)(void* cntr, bool lazy_copy_elem,
                  assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                  void* dst_elem);

    void (*Derefer)(void* cntr, void* pos_cursor, bool lazy_copy_elem,
                    assoc_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem);

    void (*Find_Fn)(void* cntr, void const* key,
                    fn_hash::FnHasher const& key_hasher,
                    fn_comparison::FnComparator const& key_elem_cmptr,
                    bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    void (*Insert_Fn)(void* cntr, void const* key,
                      fn_hash::FnHasher const& key_hasher,
                      fn_comparison::FnComparator const& key_elem_cmptr,
                      FnWriter const& writer, void* dst_cursor);

    void (*PopL)(void* cntr, size_t cnt);

    void (*PopR)(void* cntr, size_t cnt);

    void (*Erase)(void* cntr, void* pos_cursor);

    void (*EraseAll)(void* cntr);

    void (*CopyCursor)(void* cntr, void* src_cursor, void* dst_cursor);

    bool (*AreEqualCursor)(void* cntr, void const* cursor_a,
                           void const* cursor_b);

    comparison::Ordering (*CompareCursor)(void* cntr, void const* cursor_a,
                                          void const* cursor_b);

    size_t (*GetCursorDist)(void* cntr, void const* cursor_a,
                            void const* cursor_b);

    size_t (*GetCursorIdx)(void* cntr, void const* cursor);

    void (*CursorStepL)(void* cntr, void* cursor);

    void (*CursorStepR)(void* cntr, void* cursor);

    void (*CursorAdvanceL)(void* cntr, void* cursor, size_t step);

    void (*CursorAdvanceR)(void* cntr, void* cursor, size_t step);
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
