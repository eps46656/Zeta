#pragma once

#include <zeta/core/comparison.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/hash_ref.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/lin_seq_endpoint.hpp>
#include <zeta/core/poly_comparison.hpp>
#include <zeta/core/poly_seq_endpoint.hpp>
#include <zeta/core/quad_cap_sys.hpp>
#include <zeta/core/resolving.ipp>
#include <zeta/core/seq_endpoint.hpp>
#include <zeta/core/seq_endpoint.ipp>
#include <zeta/core/utils.hpp>

namespace zeta::core::assoc_cntr {

constexpr size_t max_max_elem_cnt{ integral::RangeMaxOf<size_t> / 2 };

namespace capability {

// clang-format off

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_AssocCntr_CapabilityWithoutAlwaysNever_XMacro(func, sep)     \
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

using Flag = unsigned;

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
    (static_cast<Flag>(this->name) << meta::EnumToUnderlying(Kind::name))

        return (static_cast<Flag>(1) << meta::EnumToUnderlying(Kind::Always)) |
               ZETA_Core_AssocCntr_CapabilityWithoutAlwaysNever_XMacro(F, |);

#pragma pop_macro("F")
    }
};

constexpr Flag empty_cap_flag{ FlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) .name = false,

    ZETA_Core_AssocCntr_CapabilityWithoutAlwaysNever_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr Flag full_cap_flag{ FlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) .name = true,

    ZETA_Core_AssocCntr_CapabilityWithoutAlwaysNever_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr Flag non_const_cap_flag{ FlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) .name = !is_const,

    ZETA_Core_AssocCntr_CapabilityWithoutAlwaysNever_XMacro(F, )

#pragma pop_macro("F")
}() };

constexpr Flag const_cap_flag{ FlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) .name = is_const,

    ZETA_Core_AssocCntr_CapabilityWithoutAlwaysNever_XMacro(F, )

#pragma pop_macro("F")
}() };

static_assert((non_const_cap_flag & const_cap_flag) == empty_cap_flag);

static_assert((non_const_cap_flag | const_cap_flag) == full_cap_flag);

constexpr quad_cap_sys::Config<Flag, empty_cap_flag, full_cap_flag> config;

}  // namespace capability

struct CursorLimit {
    void* content[8];
} __attribute__((aligned(alignof(max_align_t))));

struct Tag {};

#pragma push_macro("Elem")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define Elem(CntrType)                                                      \
    zeta::core::meta::GetTypeWrapperType<                                   \
        decltype(zeta::core::meta::RemoveConstVolatileReference<CntrType>:: \
                     GetElemType(zeta::core::assoc_cntr::Tag{},             \
                                 zeta::core::meta::TypeWrapper<CntrType>{}))>

#pragma push_macro("Cursor")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define Cursor(CntrType)                                                    \
    zeta::core::meta::GetTypeWrapperType<                                   \
        decltype(zeta::core::meta::RemoveConstVolatileReference<CntrType>:: \
                     GetCursorType(                                         \
                         zeta::core::assoc_cntr::Tag{},                     \
                         zeta::core::meta::TypeWrapper<CntrType>{}))>

#pragma push_macro("RequiresMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define RequiresMethod(cap, method, ret, ...)                                  \
    requires {                                                                 \
        requires(                                                              \
            (meta::RemoveReferenceFromType<Cntr>::                             \
                 GetStaticDisabledCapabilityFlag(tag,                          \
                                                 meta::TypeWrapper<Cntr>{}) &  \
             (static_cast<capability::Flag>(1)                                 \
              << meta::EnumToUnderlying(capability::Kind::cap))) != 0 ||       \
            meta::IsMatched<meta::RemoveReferenceFromType<                     \
                                decltype(static_cast<meta::Conditional<        \
                                             meta::IsRightReferenceType<Cntr>, \
                                             Cntr&&, Cntr&>>(cntr)             \
                                             .method(tag, __VA_ARGS__))>,      \
                            decltype(ret)>);                                   \
    }

template <typename Cntr>
concept IsAssocCntr = requires(
    Cntr& cntr, Tag tag, meta::TypeWrapper<Cntr> cntr_type_wrapper,
    bool bool_val, size_t size_val, Elem(Cntr) const& elem_const_ref,
    Cursor(Cntr) * cursor_ptr,
    resolving::Packet<meta::RemoveConst<Elem(Cntr)>,
                      meta::RemoveConst<Elem(Cntr)>>* resolving_packet_ptr,
    hash::ArchetypeHasher key_hasher,
    comparison::ArchetypeComparator key_elem_cmptr,
    seq_endpoint::acceptor::ArchetypeAcceptor acceptor,
    seq_endpoint::provider::ArchetypeProvider provider,
    comparison::Ordering ordering_val, meta::AlwaysMatchedTag unused) {
    requires meta::IsSame<meta::RemoveReferenceFromType<
                              decltype(meta::RemoveReferenceFromType<Cntr>::
                                           GetStaticEnabledCapabilityFlag(
                                               tag, cntr_type_wrapper))>,
                          capability::Flag>;

    requires(meta::RemoveReferenceFromType<
                 Cntr>::GetStaticEnabledCapabilityFlag(tag, cntr_type_wrapper) &
             capability::empty_cap_flag) == capability::empty_cap_flag;

    requires(meta::RemoveReferenceFromType<
                 Cntr>::GetStaticEnabledCapabilityFlag(tag, cntr_type_wrapper) |
             capability::full_cap_flag) == capability::full_cap_flag;

    requires meta::IsSame<meta::RemoveReferenceFromType<
                              decltype(meta::RemoveReferenceFromType<Cntr>::
                                           GetStaticDisabledCapabilityFlag(
                                               tag, cntr_type_wrapper))>,
                          capability::Flag>;

    requires(Cntr::GetStaticDisabledCapabilityFlag(tag, cntr_type_wrapper) &
             capability::empty_cap_flag) == capability::empty_cap_flag;

    requires(Cntr::GetStaticDisabledCapabilityFlag(tag, cntr_type_wrapper) |
             capability::full_cap_flag) == capability::full_cap_flag;

    requires meta::IsSame<
        meta::RemoveReferenceFromType<
            decltype(cntr.GetDynamicEnabledCapabilityFlag(tag))>,
        capability::Flag>;

    requires meta::IsSame<
        meta::RemoveReferenceFromType<
            decltype(cntr.GetDynamicDisabledCapabilityFlag(tag))>,
        capability::Flag>;

    requires(meta::RemoveReferenceFromType<
                 Cntr>::GetStaticEnabledCapabilityFlag(tag, cntr_type_wrapper) &
             meta::RemoveReferenceFromType<
                 Cntr>::GetStaticDisabledCapabilityFlag(tag,
                                                        cntr_type_wrapper)) ==
                capability::empty_cap_flag;

    // -------------------------------------------------------------------------

    requires meta::IsObjectType<meta::GetTypeWrapperType<
        decltype(meta::RemoveReferenceFromType<Cntr>::GetElemType(
            Tag{}, cntr_type_wrapper))>>;

    requires meta::IsTypeWrapper<
        decltype(meta::RemoveReferenceFromType<Cntr>::GetCursorType(
            tag, cntr_type_wrapper))>;

    // -------------------------------------------------------------------------

    requires meta::IsSame<
        meta::RemoveReferenceFromType<decltype(cntr.GetReferedInstPtr(tag))>,
        void*>;

    // -------------------------------------------------------------------------

    requires RequiresMethod(  //
        GetElemCnt,           // capability
        GetElemCnt,           // method
                              //
        size_val              // ret
    );

    requires RequiresMethod(  //
        GetMaxElemCnt,        // capability
        GetMaxElemCnt,        // method
                              //
        size_val              // ret
    );

    // -------------------------------------------------------------------------

    requires RequiresMethod(  //
        GetLBCursor,          // capability
        GetLBCursor,          // method
                              //
        unused,               // ret
                              //
        cursor_ptr            // cursor
    );

    requires RequiresMethod(  //
        GetRBCursor,          // capability
        GetRBCursor,          // method
                              //
        unused,               // ret
                              //
        cursor_ptr            // cursor
    );

    // -------------------------------------------------------------------------

    requires RequiresMethod(  //
        PeekL,                // capability
        PeekL,                // method
                              //
        unused,               // ret
                              //
        cursor_ptr,           // dst_cursor, optional
        resolving_packet_ptr  // dst_resolving_packet, optional
    );

    requires RequiresMethod(  //
        PeekR,                // capability
        PeekR,                // method
                              //
        unused,               // ret
                              //
        cursor_ptr,           // dst_cursor, optional
        resolving_packet_ptr  // dst_resolving_packet, optional
    );

    // -------------------------------------------------------------------------

    requires RequiresMethod(  //
        Derefer,              // capability
        Derefer,              // method
                              //
        unused,               // ret
                              //
        cursor_ptr,           // pos_cursor
        resolving_packet_ptr  // dst_resolving_packet, optional
    );

    // -------------------------------------------------------------------------

    requires RequiresMethod(  //
        Find,                 // capability
        Find,                 // method
                              //
        unused,               // ret
                              //
        elem_const_ref,       // elem
        cursor_ptr,           // dst_cursor
        resolving_packet_ptr  // resolving_packet
    );

    // -------------------------------------------------------------------------

    requires RequiresMethod(  //
        Insert,               // capability
        Insert,               // method
                              //
        unused,               // ret
                              //
        elem_const_ref,       // elem
        provider,             // provider
        cursor_ptr            // dst_cursor
    );

    RequiresMethod(      //
        Insert,          // capability
        Insert,          // method
                         //
        unused,          // ret
                         //
        const_void_ptr,  // key
        key_hasher,      // key_hasher
        key_elem_cmptr,  // key_elem_cmptr
        writer,          // writer
        void_ptr         // dst_cursor
    );

    // -------------------------------------------------------------------------

    requires RequiresMethod(  //
        PopL,                 // capability
        PopL,                 // method
                              //
        unused,               // ret
                              //
        size_val,             // cnt
        acceptor              // acceptor
    );

    requires RequiresMethod(  //
        PopR,                 // capability
        PopR,                 // method
                              //
        unused,               // ret
                              //
        size_val,             // cnt
        acceptor              // acceptor
    );

    requires RequiresMethod(  //
        Erase,                // capability
        Erase,                // method
                              //
        unused,               // ret
                              //
        cursor_ptr,           // pos_cursor
        size_val,             // cnt
        acceptor              // acceptor
    );

    requires RequiresMethod(  //
        EraseAll,             // capability
        EraseAll,             // method
                              //
        unused,               // ret
                              //
        acceptor              // acceptor
    );

    // -------------------------------------------------------------------------

    requires RequiresMethod(  //
        CopyCursor,           // capability
        CopyCursor,           // method
                              //
        unused,               // ret
                              //
        cursor_ptr,           // src_cursor
        cursor_ptr            // dst_cursor
    );

    requires RequiresMethod(  //
        AreEqualCursor,       // capability
        AreEqualCursor,       // method
                              //
        bool_val,             // ret
                              //
        cursor_ptr,           // cursor_a
        cursor_ptr            // cursor_b
    );

    requires RequiresMethod(  //
        CompareCursor,        // capability
        CompareCursor,        // method
                              //
        ordering_val,         // ret
                              //
        cursor_ptr,           // cursor_a
        cursor_ptr            // cursor_b
    );

    requires RequiresMethod(  //
        GetCursorDist,        // capability
        GetCursorDist,        // method
                              //
        size_val,             // ret
                              //
        cursor_ptr,           // cursor_a
        cursor_ptr            // cursor_b
    );

    RequiresMethod(    //
        GetCursorIdx,  // capability
        GetCursorIdx,  // method
                       //
        size_val,      // ret
                       //
        cursor_ptr     // cursor
    );

    RequiresMethod(   //
        CursorStepL,  // capability
        CursorStepL,  // method
                      //
        unused,       // ret
                      //
        cursor_ptr    // cursor
    );

    RequiresMethod(   //
        CursorStepR,  // capability
        CursorStepR,  // method
                      //
        unused,       // ret
                      //
        void_ptr      // cursor
    );

    RequiresMethod(      //
        CursorAdvanceL,  // capability
        CursorAdvanceL,  // method
                         //
        unused,          // ret
                         //
        cursor_ptr,      // cursor
        size_val         // step
    );

    RequiresMethod(      //
        CursorAdvanceR,  // capability
        CursorAdvanceR,  // method
                         //
        unused,          // ret
                         //
        cursor_ptr,      // cursor
        size_val         // step
    );
};

#pragma pop_macro("RequiresMethod")

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

template <typename Cntr,
          seq_endpoint::provider::IsProvider<Elem(Cntr)> Provider>
constexpr decltype(auto) Insert(Cntr& cntr, void const* elem, Provider&& writer,
                                void* dst_cursor);

template <typename Cntr, hash::CanHash<void const*> KeyHasher,
          comparison::CanCompare<void const*, void const*> KeyElemComparator,
          seq_endpoint::provider::IsProvider<Elem(Cntr)> Provider>
constexpr decltype(auto) Insert(Cntr& cntr, void const* key,
                                KeyHasher key_hasher,
                                KeyElemComparator key_elem_cmptr,
                                Provider&& writer, void* dst_cursor);

template <typename Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr)> Acceptor>
constexpr decltype(auto) PopL(Cntr& cntr, size_t cnt, Acceptor&& acceptor);

template <typename Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr)> Acceptor>
constexpr decltype(auto) PopR(Cntr& cntr, size_t cnt, Acceptor&& acceptor);

template <typename Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr)> Acceptor>
constexpr decltype(auto) Erase(Cntr& cntr, void* pos_cursor, size_t cnt,
                               Acceptor&& acceptor);

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
                       poly_comparison::Comparator const& key_elem_cmptr,
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
                          poly_comparison::Comparator const& key_elem_cmptr,
                          EmptyWriter writer, void* dst_cursor);
            void (*fn)(void* cntr, void const* key,
                       hash_ref::Hasher const& key_hasher,
                       poly_comparison::Comparator const& key_elem_cmptr,
                       FnWriter writer, void* dst_cursor);
        } with_key;
    } insert;

    struct {
        void (*empty)(void* cntr, size_t cnt, EmptyReader acceptor);
        void (*lin_seq)(void* cntr, size_t cnt, LinSeqReader& acceptor);
        void (*fn)(void* cntr, size_t cnt, FnReader acceptor);
    } pop_l;

    struct {
        void (*empty)(void* cntr, size_t cnt, EmptyReader acceptor);
        void (*lin_seq)(void* cntr, size_t cnt, LinSeqReader& acceptor);
        void (*fn)(void* cntr, size_t cnt, FnReader acceptor);
    } pop_r;

    struct {
        void (*empty)(void* cntr, void* pos_cursor, size_t cnt,
                      EmptyReader acceptor);
        void (*lin_seq)(void* cntr, void* pos_cursor, size_t cnt,
                        LinSeqReader& acceptor);
        void (*fn)(void* cntr, void* pos_cursor, size_t cnt, FnReader acceptor);
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
