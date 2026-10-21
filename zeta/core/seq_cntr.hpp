/*
@file zeta/core/seq_cntr.hpp
*/

#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/lin_seq_endpoint.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/poly_seq_endpoint.ipp>
#include <zeta/core/utils.hpp>

namespace zeta::core::seq_cntr {

constexpr size_t max_max_elem_cnt{ integral::RangeMaxOf<size_t> / 2 };

namespace capability {

// clang-format off

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_SeqCntr_CapabilityWithoutAlwaysNever_XMacro(func, sep)       \
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
    (static_cast<Flag>(this->name) << meta::EnumToUnderlying(Kind::name))

        return (static_cast<Flag>(1) << meta::EnumToUnderlying(Kind::Always)) |
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

static_assert((non_const_capability_flag & const_capability_flag) ==
              empty_capability_flag);

static_assert((non_const_capability_flag | const_capability_flag) ==
              full_capability_flag);

constexpr bool CheckFlags(capability::Flag se_cap_flag,
                          capability::Flag sd_cap_flag);

constexpr bool CheckFlags(capability::Flag se_cap_flag,
                          capability::Flag sd_cap_flag,
                          capability::Flag de_cap_flag,
                          capability::Flag dd_cap_flag);

}  // namespace capability

struct alignas(max_align_t) CursorLimit {
    void* content[8];
};

struct Tag {};

#pragma push_macro("Elem")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define Elem(CntrType)                                                 \
    zeta::core::meta::GetTypeWrapperType<                              \
        decltype(zeta::core::meta::RemoveCVRef<CntrType>::GetElemType( \
            zeta::core::seq_cntr::Tag{},                               \
            zeta::core::meta::TypeWrapper<CntrType>{}))>

#pragma push_macro("Cursor")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define Cursor(CntrType)                                                 \
    zeta::core::meta::GetTypeWrapperType<                                \
        decltype(zeta::core::meta::RemoveCVRef<CntrType>::GetCursorType( \
            zeta::core::seq_cntr::Tag{},                                 \
            zeta::core::meta::TypeWrapper<CntrType>{}))>

#pragma push_macro("RequiresMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define RequiresMethod(cap, method, ret, ...)                              \
    requires {                                                             \
        requires(                                                          \
            (meta::RemoveRef<Cntr>::GetStaticDisabledCapabilityFlag(       \
                 tag, meta::TypeWrapper<Cntr>{}) &                         \
             (static_cast<capability::Flag>(1)                             \
              << meta::EnumToUnderlying(capability::Kind::cap))) != 0 ||   \
            meta::IsMatched<                                               \
                meta::RemoveRef<                                           \
                    decltype(static_cast<meta::Conditional<                \
                                 meta::IsRRef<Cntr>, Cntr&&, Cntr&>>(cntr) \
                                 .method(tag, __VA_ARGS__))>,              \
                decltype(ret)>);                                           \
    }

template <typename Cntr>
concept IsSeqCntr = requires(
    Cntr cntr, Tag tag, meta::TypeWrapper<Cntr> cntr_type_wrapper,
    bool bool_val, int int_val, size_t size_val, Cursor(Cntr) * cursor_ptr,
    lifecycle::ConstructingMemToObjView<Elem(Cntr)>
        constructing_mem_to_obj_view,
    lifecycle::DestructingObjToMemView<Elem(Cntr)> destructing_obj_to_mem_view,
    lifecycle::LeftObjView<Elem(Cntr)> left_obj_view,
    lifecycle::ConstLeftObjView<Elem(Cntr)> const_left_obj_view,
    lifecycle::RightObjView<Elem(Cntr)> right_obj_view,
    lifecycle::ConstRightObjView<Elem(Cntr)> const_right_obj_view,
    lifecycle::DynamicStorageView<Elem(Cntr)> dynamic_obj_view,
    Elem(Cntr) * elem_ptr, seq_endpoint::acceptor::ArchetypeAcceptor acceptor,
    seq_endpoint::provider::ArchetypeProvider provider,
    comparison::Ordering ordering_val, meta::AlwaysMatchedTag unused) {
    requires meta::IsRef<Cntr>;

    requires requires {
        requires meta::IsSame<
            meta::RemoveRef<decltype(cntr.GetReferedInstPtr(tag))>, void*>;

        requires meta::IsSame<
            meta::RemoveRef<
                decltype(meta::RemoveRef<Cntr>::GetStaticEnabledCapabilityFlag(
                    tag, cntr_type_wrapper))>,
            capability::Flag>;

        requires(meta::RemoveRef<Cntr>::GetStaticEnabledCapabilityFlag(
                     tag, cntr_type_wrapper) &
                 capability::empty_capability_flag) ==
                    capability::empty_capability_flag;

        requires(meta::RemoveRef<Cntr>::GetStaticEnabledCapabilityFlag(
                     tag, cntr_type_wrapper) |
                 capability::full_capability_flag) ==
                    capability::full_capability_flag;

        requires meta::IsSame<
            meta::RemoveRef<
                decltype(meta::RemoveRef<Cntr>::GetStaticDisabledCapabilityFlag(
                    tag, cntr_type_wrapper))>,
            capability::Flag>;

        requires(meta::RemoveRef<Cntr>::GetStaticDisabledCapabilityFlag(
                     tag, cntr_type_wrapper) &
                 capability::empty_capability_flag) ==
                    capability::empty_capability_flag;

        requires(meta::RemoveRef<Cntr>::GetStaticDisabledCapabilityFlag(
                     tag, cntr_type_wrapper) |
                 capability::full_capability_flag) ==
                    capability::full_capability_flag;

        requires meta::IsSame<
            meta::RemoveRef<decltype(cntr.GetDynamicEnabledCapabilityFlag(
                tag))>,
            capability::Flag>;

        requires meta::IsSame<
            meta::RemoveRef<decltype(cntr.GetDynamicDisabledCapabilityFlag(
                tag))>,
            capability::Flag>;

        requires(meta::RemoveRef<Cntr>::GetStaticEnabledCapabilityFlag(
                     tag, cntr_type_wrapper) &
                 meta::RemoveRef<Cntr>::GetStaticDisabledCapabilityFlag(
                     tag, cntr_type_wrapper)) ==
                    capability::empty_capability_flag;

        requires meta::IsObj<meta::GetTypeWrapperType<
            decltype(meta::RemoveRef<Cntr>::GetElemType(Tag{},
                                                        cntr_type_wrapper))>>;

        requires meta::IsTypeWrapper<
            decltype(meta::RemoveRef<Cntr>::GetCursorType(tag,
                                                          cntr_type_wrapper))>;
    };

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

    requires RequiresMethod(          //
        PeekL,                        // capability
        PeekL,                        // method
                                      //
        unused,                       // ret
                                      //
        bool_val,                     // lazy_copy_elem
        &dynamic_obj_view,            // dst_elem_view
        cursor_ptr,                   // dst_cursor, optional
        constructing_mem_to_obj_view  // dst_elem, optional
    );

    requires RequiresMethod(  //
        PeekL,                // capability
        PeekL,                // method
                              //
        unused,               // ret
                              //
        bool_val,             // lazy_copy_elem
        &dynamic_obj_view,    // dst_elem_view
        cursor_ptr,           // dst_cursor, optional
        left_obj_view         // dst_elem, optional
    );

    requires RequiresMethod(  //
        PeekL,                // capability
        PeekL,                // method
                              //
        unused,               // ret
                              //
        bool_val,             // lazy_copy_elem
        &dynamic_obj_view,    // dst_elem_view
        cursor_ptr,           // dst_cursor, optional
        right_obj_view        // dst_elem, optional
    );

    // -------------------------------------------------------------------------

    requires RequiresMethod(          //
        PeekR,                        // capability
        PeekR,                        // method
                                      //
        unused,                       // ret
                                      //
        bool_val,                     // lazy_copy_elem
        &dynamic_obj_view,            // dst_elem_view
        cursor_ptr,                   // dst_cursor, optional
        constructing_mem_to_obj_view  // dst_elem, optional
    );

    requires RequiresMethod(  //
        PeekR,                // capability
        PeekR,                // method
                              //
        unused,               // ret
                              //
        bool_val,             // lazy_copy_elem
        &dynamic_obj_view,    // dst_elem_view
        cursor_ptr,           // dst_cursor, optional
        left_obj_view         // dst_elem, optional
    );

    requires RequiresMethod(  //
        PeekR,                // capability
        PeekR,                // method
                              //
        unused,               // ret
                              //
        bool_val,             // lazy_copy_elem
        &dynamic_obj_view,    // dst_elem_view
        cursor_ptr,           // dst_cursor, optional
        right_obj_view        // dst_elem, optional
    );

    // -------------------------------------------------------------------------

    requires RequiresMethod(          //
        Refer,                        // capability
        Refer,                        // method
                                      //
        unused,                       // ret
                                      //
        size_val,                     // idx
        bool_val,                     // lazy_copy_elem
        &dynamic_obj_view,            // dst_elem_view
        cursor_ptr,                   // dst_cursor, optional
        constructing_mem_to_obj_view  // dst_elem, optional
    );

    requires RequiresMethod(  //
        Refer,                // capability
        Refer,                // method
                              //
        unused,               // ret
                              //
        size_val,             // idx
        bool_val,             // lazy_copy_elem
        &dynamic_obj_view,    // dst_elem_view
        cursor_ptr,           // dst_cursor, optional
        left_obj_view         // dst_elem, optional
    );

    requires RequiresMethod(  //
        Refer,                // capability
        Refer,                // method
                              //
        unused,               // ret
                              //
        size_val,             // idx
        bool_val,             // lazy_copy_elem
        &dynamic_obj_view,    // dst_elem_view
        cursor_ptr,           // dst_cursor, optional
        right_obj_view        // dst_elem, optional
    );

    // -------------------------------------------------------------------------

    requires RequiresMethod(          //
        Derefer,                      // capability
        Derefer,                      // method
                                      //
        unused,                       // ret
                                      //
        cursor_ptr,                   // pos_cursor
        bool_val,                     // lazy_copy_elem
        &dynamic_obj_view,            // dst_elem_view
        constructing_mem_to_obj_view  // dst_elem, optional
    );

    requires RequiresMethod(  //
        Derefer,              // capability
        Derefer,              // method
                              //
        unused,               // ret
                              //
        cursor_ptr,           // pos_cursor
        bool_val,             // lazy_copy_elem
        &dynamic_obj_view,    // dst_elem_view
        left_obj_view         // dst_elem, optional
    );

    requires RequiresMethod(  //
        Derefer,              // capability
        Derefer,              // method
                              //
        unused,               // ret
                              //
        cursor_ptr,           // pos_cursor
        bool_val,             // lazy_copy_elem
        &dynamic_obj_view,    // dst_elem_view
        right_obj_view        // dst_elem, optional
    );

    // -------------------------------------------------------------------------

    requires RequiresMethod(  //
        Read,                 // capability
        Read,                 // method
                              //
        unused,               // ret
                              //
        cursor_ptr,           // pos_cursor
        size_val,             // cnt
        acceptor,             // acceptor
        cursor_ptr            // dst_cursor
    );

    requires meta::Implies<                //
        meta::IsNonConstWithoutRef<Cntr>,  //
        RequiresMethod(                    //
            Write,                         // capability
            Write,                         // method
                                           //
            unused,                        // ret
                                           //
            cursor_ptr,                    // pos_cursor, point to original
                                           // position
            size_val,                      // cnt
            provider,                      // provider
            cursor_ptr                     // dst_cursor, optional, point
                                           // to final position after write
            )>;

    requires meta::Implies<                //
        meta::IsNonConstWithoutRef<Cntr>,  //
        RequiresMethod(                    //
            ReadWrite,                     // capability
            ReadWrite,                     // method
                                           //
            unused,                        // ret
                                           //
            cursor_ptr,                    // pos_cursor
            size_val,                      // cnt
            acceptor,                      // acceptor
            cursor_ptr                     // dst_cursor
            )>;

    requires meta::Implies<                //
        meta::IsNonConstWithoutRef<Cntr>,  //
        RequiresMethod(                    //
            PushL,                         // capability
            PushL,                         // method
                                           //
            unused,                        // ret
                                           //
            size_val,                      // cnt
            provider,                      // provider
            cursor_ptr,                    // dst_beg_cursor
            cursor_ptr                     // dst_end_cursor
            )>;

    requires meta::Implies<                //
        meta::IsNonConstWithoutRef<Cntr>,  //
        RequiresMethod(                    //
            PushR,                         // capability
            PushR,                         // method
                                           //
            unused,                        // ret
                                           //
            size_val,                      // cnt
            provider,                      // provider
            cursor_ptr,                    // dst_beg_cursor
            cursor_ptr                     // dst_end_cursor
            )>;

    requires meta::Implies<                //
        meta::IsNonConstWithoutRef<Cntr>,  //
        RequiresMethod(                    //
            Insert,                        // capability
            Insert,                        // method
                                           //
            unused,                        // ret
                                           //
            cursor_ptr,                    // pos_cursor
            size_val,                      // cnt
            provider,                      // provider
            cursor_ptr                     // dst_cursor
            )>;

    requires meta::Implies<                //
        meta::IsNonConstWithoutRef<Cntr>,  //
        RequiresMethod(                    //
            PopL,                          // capability
            PopL,                          // method
                                           //
            unused,                        // ret
                                           //
            size_val,                      // cnt
            acceptor,                      // acceptor
            cursor_ptr                     // dst_cursor, optional
            )>;

    requires meta::Implies<                //
        meta::IsNonConstWithoutRef<Cntr>,  //
        RequiresMethod(                    //
            PopR,                          // capability
            PopR,                          // method
                                           //
            unused,                        // ret
                                           //
            size_val,                      // cnt
            acceptor,                      // acceptor
            cursor_ptr                     // dst_cursor, optional
            )>;

    requires meta::Implies<                //
        meta::IsNonConstWithoutRef<Cntr>,  //
        RequiresMethod(                    //
            Erase,                         // capability
            Erase,                         // method
                                           //
            unused,                        // ret
                                           //
            cursor_ptr,                    // pos_cursor
            size_val,                      // cnt
            acceptor                       // acceptor
            )>;

    requires meta::Implies<                //
        meta::IsNonConstWithoutRef<Cntr>,  //
        RequiresMethod(                    //
            EraseAll,                      // capability
            EraseAll,                      // method
                                           //
            unused,                        // ret
                                           //
            acceptor                       // acceptor
            )>;

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
        bool_val,             //
                              //
        cursor_ptr,           // cursor_a
        cursor_ptr            // cursor_b
    );

    requires RequiresMethod(  //
        CompareCursor,        // capability
        CompareCursor,        // method
                              //
        ordering_val,         //
                              //
        cursor_ptr,           // cursor_a
        cursor_ptr            // cursor_b
    );

    requires RequiresMethod(  //
        GetCursorDist,        // capability
        GetCursorDist,        // method
                              //
        size_val,             //
                              //
        cursor_ptr,           // cursor_a
        cursor_ptr            // cursor_b
    );

    requires RequiresMethod(  //
        GetCursorIdx,         // capability
        GetCursorIdx,         // method
                              //
        size_val,             //
                              //
        cursor_ptr            // cursor
    );

    requires RequiresMethod(  //
        CursorStepL,          // capability
        CursorStepL,          // method
                              //
        unused,               // ret
                              //
        cursor_ptr            // cursor
    );

    requires RequiresMethod(  //
        CursorStepR,          // capability
        CursorStepR,          // method
                              //
        unused,               // ret
                              //
        cursor_ptr            // cursor
    );

    requires RequiresMethod(  //
        CursorAdvanceL,       // capability
        CursorAdvanceL,       // method
                              //
        unused,               // ret
                              //
        cursor_ptr,           // cursor
        size_val              // step
    );

    requires RequiresMethod(  //
        CursorAdvanceR,       // capability
        CursorAdvanceR,       // method
                              //
        unused,               // ret
                              //
        cursor_ptr,           // cursor
        size_val              // step
    );
};

#pragma pop_macro("RequiresMethod")

template <typename Cntr>
concept IsSeqCntrWithLRef = IsSeqCntr<Cntr&>;

template <typename Cntr>
concept IsSeqCntrWithRRef = IsSeqCntr<Cntr&&>;

template <typename Cntr>
concept IsSeqCntrLike = IsSeqCntr<Cntr> || IsSeqCntr<meta::RemovePointer<Cntr>>;

template <IsSeqCntrWithRRef Cntr>
constexpr void CheckCapabilityFlags(Cntr&& cntr);

template <IsSeqCntrWithRRef Cntr>
constexpr void* GetReferedInstPtr(Cntr&& cntr);

template <IsSeqCntr Cntr>
constexpr capability::Flag GetStaticEnabledCapabilityFlag(
    meta::TypeWrapper<Cntr>);

template <IsSeqCntr Cntr>
constexpr capability::Flag GetStaticDisabledCapabilityFlag(
    meta::TypeWrapper<Cntr>);

template <IsSeqCntrWithRRef Cntr>
constexpr capability::Flag GetDynamicEnabledCapabilityFlag(Cntr&& cntr);

template <IsSeqCntrWithRRef Cntr>
constexpr capability::Flag GetDynamicDisabledCapabilityFlag(Cntr&& cntr);

template <IsSeqCntrWithRRef Cntr>
constexpr meta::TypeWrapper<Elem(Cntr)> GetElemType(meta::TypeWrapper<Cntr>);

template <IsSeqCntrWithRRef Cntr>
constexpr meta::TypeWrapper<Cursor(Cntr)> GetCursorType(
    meta::TypeWrapper<Cntr>);

template <IsSeqCntrWithRRef Cntr>
constexpr size_t GetElemCnt(Cntr&& cntr);

template <IsSeqCntrWithRRef Cntr>
constexpr size_t GetMaxElemCnt(Cntr&& cntr);

template <IsSeqCntrWithRRef Cntr>
constexpr decltype(auto) GetLBCursor(Cntr&& cntr, Cursor(Cntr&&) * dst_cursor);

template <IsSeqCntrWithRRef Cntr>
constexpr decltype(auto) GetRBCursor(Cntr&& cntr, Cursor(Cntr&&) * dst_cursor);

template <IsSeqCntrWithRRef Cntr, lifecycle::IsDstStorageView DstElemView>
constexpr decltype(auto) PeekL(
    Cntr&& cntr, bool lazy_copy_elem,
    lifecycle::DynamicStorageView<Elem(Cntr&&)>* dst_elem_view,
    Cursor(Cntr&&) * dst_cursor, DstElemView dst_elem);

template <IsSeqCntrWithRRef Cntr, lifecycle::IsDstStorageView DstElemView>
constexpr decltype(auto) PeekR(
    Cntr&& cntr, bool lazy_copy_elem,
    lifecycle::DynamicStorageView<Elem(Cntr&&)>* dst_elem_view,
    Cursor(Cntr&&) * dst_cursor, DstElemView dst_elem);

template <IsSeqCntrWithRRef Cntr, lifecycle::IsDstStorageView DstElemView>
constexpr decltype(auto) Refer(
    Cntr&& cntr, size_t idx, bool lazy_copy_elem,
    lifecycle::DynamicStorageView<Elem(Cntr&&)>* dst_elem_view,
    Cursor(Cntr&&) * dst_cursor, DstElemView dst_elem);

template <IsSeqCntrWithRRef Cntr, lifecycle::IsDstStorageView DstElemView>
constexpr decltype(auto) Derefer(
    Cntr&& cntr, Cursor(Cntr&&) * pos_cursor, bool lazy_copy_elem,
    lifecycle::DynamicStorageView<Elem(Cntr&&)>* dst_elem_view,
    DstElemView dst_elem);

template <IsSeqCntrWithRRef Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr&&)> Acceptor>
constexpr decltype(auto) Read(Cntr&& cntr, Cursor(Cntr&&) * pos_cursor,
                              size_t cnt, Acceptor&& acceptor,
                              Cursor(Cntr&&) * dst_cursor);

template <IsSeqCntrWithRRef Cntr,
          seq_endpoint::provider::IsProvider<Elem(Cntr&&)> Provider>
constexpr decltype(auto) Write(Cntr&& cntr, Cursor(Cntr&&) * pos_cursor,
                               size_t cnt, Provider&& provider,
                               Cursor(Cntr&&) * dst_cursor);

template <IsSeqCntrWithRRef Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr&&)> Acceptor>
constexpr decltype(auto) ReadWrite(Cntr&& cntr, Cursor(Cntr&&) * pos_cursor,
                                   size_t cnt, Acceptor&& acceptor,
                                   Cursor(Cntr&&) * dst_cursor);

template <IsSeqCntrWithRRef Cntr,
          seq_endpoint::provider::IsProvider<Elem(Cntr&&)> Provider>
constexpr decltype(auto) PushL(Cntr&& cntr, size_t cnt, Provider&& provider,
                               Cursor(Cntr&&) * dst_beg_cursor,
                               Cursor(Cntr&&) * dst_end_cursor);

template <IsSeqCntrWithRRef Cntr,
          seq_endpoint::provider::IsProvider<Elem(Cntr&&)> Provider>
constexpr decltype(auto) PushR(Cntr&& cntr, size_t cnt, Provider&& provider,
                               Cursor(Cntr&&) * dst_beg_cursor,
                               Cursor(Cntr&&) * dst_end_cursor);

template <IsSeqCntrWithRRef Cntr,
          seq_endpoint::provider::IsProvider<Elem(Cntr&&)> Provider>
constexpr decltype(auto) Insert(Cntr&& cntr, Cursor(Cntr&&) * pos_cursor,
                                size_t cnt, Provider&& provider,
                                Cursor(Cntr&&) * dst_cursor);

template <IsSeqCntrWithRRef Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr&&)> Acceptor>
constexpr decltype(auto) PopL(Cntr&& cntr, size_t cnt, Acceptor&& acceptor,
                              Cursor(Cntr&&) * dst_cursor);

template <IsSeqCntrWithRRef Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr&&)> Acceptor>
constexpr decltype(auto) PopR(Cntr&& cntr, size_t cnt, Acceptor&& acceptor,
                              Cursor(Cntr&&) * dst_cursor);

template <IsSeqCntrWithRRef Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr&&)> Acceptor>
constexpr decltype(auto) Erase(Cntr&& cntr, Cursor(Cntr&&) * pos_cursor,
                               size_t cnt, Acceptor&& acceptor);

template <IsSeqCntrWithRRef Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr&&)> Acceptor>
constexpr decltype(auto) EraseAll(Cntr&& cntr, Acceptor&& acceptor);

template <IsSeqCntrWithRRef Cntr>
constexpr decltype(auto) CopyCursor(Cntr&& cntr, Cursor(Cntr&&) * src_cursor,
                                    Cursor(Cntr&&) * dst_cursor);

template <IsSeqCntrWithRRef Cntr>
constexpr bool AreEqualCursor(Cntr&& cntr, Cursor(Cntr&&) * cursor_a,
                              Cursor(Cntr&&) * cursor_b);

template <IsSeqCntrWithRRef Cntr>
constexpr comparison::Ordering CompareCursor(Cntr&& cntr,
                                             Cursor(Cntr&&) * cursor_a,
                                             Cursor(Cntr&&) * cursor_b);

template <IsSeqCntrWithRRef Cntr>
constexpr size_t GetCursorDist(Cntr&& cntr, Cursor(Cntr&&) * cursor_a,
                               Cursor(Cntr&&) * cursor_b);

template <IsSeqCntrWithRRef Cntr>
constexpr size_t GetCursorIdx(Cntr&& cntr, Cursor(Cntr&&) * cursor);

template <IsSeqCntrWithRRef Cntr>
constexpr decltype(auto) CursorStepL(Cntr&& cntr, Cursor(Cntr&&) * cursor);

template <IsSeqCntrWithRRef Cntr>
constexpr decltype(auto) CursorStepR(Cntr&& cntr, Cursor(Cntr&&) * cursor);

template <IsSeqCntrWithRRef Cntr>
constexpr decltype(auto) CursorAdvanceL(Cntr&& cntr, Cursor(Cntr&&) * cursor,
                                        size_t step);

template <IsSeqCntrWithRRef Cntr>
constexpr decltype(auto) CursorAdvanceR(Cntr&& cntr, Cursor(Cntr&&) * cursor,
                                        size_t step);

template <meta::IsObj Elem>
struct VTable {
    unsigned long long custom_tags[4];

    size_t (*get_elem_cnt)(void* cntr);

    size_t (*get_max_elem_cnt)(void* cntr);

    void (*get_lb_cursor)(void* cntr, seq_cntr::CursorLimit* dst_cursor);

    void (*get_rb_cursor)(void* cntr, seq_cntr::CursorLimit* dst_cursor);

    struct {
        void (*constructing_mem_to_obj)(
            void* cntr, bool lazy_copy_elem,
            lifecycle::DynamicStorageView<Elem>* dst_elem_view,
            seq_cntr::CursorLimit* dst_cursor,
            lifecycle::ConstructingMemToObjView<Elem> dst_elem);

        void (*left_obj)(void* cntr, bool lazy_copy_elem,
                         lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                         seq_cntr::CursorLimit* dst_cursor,
                         lifecycle::LeftObjView<Elem> dst_elem);

        void (*right_obj)(void* cntr, bool lazy_copy_elem,
                          lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                          seq_cntr::CursorLimit* dst_cursor,
                          lifecycle::RightObjView<Elem> dst_elem);
    } peek_l;

    struct {
        void (*constructing_mem_to_obj)(
            void* cntr, bool lazy_copy_elem,
            lifecycle::DynamicStorageView<Elem>* dst_elem_view,
            seq_cntr::CursorLimit* dst_cursor,
            lifecycle::ConstructingMemToObjView<Elem> dst_elem);

        void (*left_obj)(void* cntr, bool lazy_copy_elem,
                         lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                         seq_cntr::CursorLimit* dst_cursor,
                         lifecycle::LeftObjView<Elem> dst_elem);

        void (*right_obj)(void* cntr, bool lazy_copy_elem,
                          lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                          seq_cntr::CursorLimit* dst_cursor,
                          lifecycle::RightObjView<Elem> dst_elem);
    } peek_r;

    struct {
        void (*constructing_mem_to_obj)(
            void* cntr, size_t idx, bool lazy_copy_elem,
            lifecycle::DynamicStorageView<Elem>* dst_elem_view,
            seq_cntr::CursorLimit* dst_cursor,
            lifecycle::ConstructingMemToObjView<Elem> dst_elem);

        void (*left_obj)(void* cntr, size_t idx, bool lazy_copy_elem,
                         lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                         seq_cntr::CursorLimit* dst_cursor,
                         lifecycle::LeftObjView<Elem> dst_elem);

        void (*right_obj)(void* cntr, size_t idx, bool lazy_copy_elem,
                          lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                          seq_cntr::CursorLimit* dst_cursor,
                          lifecycle::RightObjView<Elem> dst_elem);
    } refer;

    struct {
        void (*constructing_mem_to_obj)(
            void* cntr, seq_cntr::CursorLimit* pos_cursor, bool lazy_copy_elem,
            lifecycle::DynamicStorageView<Elem>* dst_elem_view,
            lifecycle::ConstructingMemToObjView<Elem> dst_elem);

        void (*left_obj)(void* cntr, seq_cntr::CursorLimit* pos_cursor,
                         bool lazy_copy_elem,
                         lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                         lifecycle::LeftObjView<Elem> dst_elem);

        void (*right_obj)(void* cntr, seq_cntr::CursorLimit* pos_cursor,
                          bool lazy_copy_elem,
                          lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                          lifecycle::RightObjView<Elem> dst_elem);
    } derefer;

    struct {
        void (*basic)(void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                      seq_endpoint::acceptor::BasicAcceptor acceptor,
                      seq_cntr::CursorLimit* dst_cursor);
        void (*poly)(void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                     poly_seq_endpoint::acceptor::Acceptor<Elem> acceptor,
                     seq_cntr::CursorLimit* dst_cursor);
    } read;

    struct {
        void (*basic)(void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                      seq_endpoint::provider::BasicProvider provider,
                      seq_cntr::CursorLimit* dst_cursor);
        void (*poly)(void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                     poly_seq_endpoint::provider::Provider<Elem> provider,
                     seq_cntr::CursorLimit* dst_cursor);
    } write;

    struct {
        void (*poly)(void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                     poly_seq_endpoint::acceptor::Acceptor<Elem> acceptor,
                     seq_cntr::CursorLimit* dst_cursor);
    } read_write;

    struct {
        void (*basic)(void* cntr, size_t cnt,
                      seq_endpoint::provider::BasicProvider provider,
                      seq_cntr::CursorLimit* dst_beg_cursor,
                      seq_cntr::CursorLimit* dst_end_cursor);
        void (*poly)(void* cntr, size_t cnt,
                     poly_seq_endpoint::provider::Provider<Elem> provider,
                     seq_cntr::CursorLimit* dst_beg_cursor,
                     seq_cntr::CursorLimit* dst_end_cursor);
    } push_l;

    struct {
        void (*basic)(void* cntr, size_t cnt,
                      seq_endpoint::provider::BasicProvider provider,
                      seq_cntr::CursorLimit* dst_beg_cursor,
                      seq_cntr::CursorLimit* dst_end_cursor);
        void (*poly)(void* cntr, size_t cnt,
                     poly_seq_endpoint::provider::Provider<Elem> provider,
                     seq_cntr::CursorLimit* dst_beg_cursor,
                     seq_cntr::CursorLimit* dst_end_cursor);
    } push_r;

    struct {
        void (*basic)(void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                      seq_endpoint::provider::BasicProvider provider,
                      seq_cntr::CursorLimit* dst_cursor);
        void (*poly)(void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                     poly_seq_endpoint::provider::Provider<Elem> provider,
                     seq_cntr::CursorLimit* dst_cursor);
    } insert;

    struct {
        void (*basic)(void* cntr, size_t cnt,
                      seq_endpoint::acceptor::BasicAcceptor acceptor,
                      seq_cntr::CursorLimit* dst_cursor);
        void (*poly)(void* cntr, size_t cnt,
                     poly_seq_endpoint::acceptor::Acceptor<Elem> acceptor,
                     seq_cntr::CursorLimit* dst_cursor);
    } pop_l;

    struct {
        void (*basic)(void* cntr, size_t cnt,
                      seq_endpoint::acceptor::BasicAcceptor acceptor,
                      seq_cntr::CursorLimit* dst_cursor);
        void (*poly)(void* cntr, size_t cnt,
                     poly_seq_endpoint::acceptor::Acceptor<Elem> acceptor,
                     seq_cntr::CursorLimit* dst_cursor);
    } pop_r;

    struct {
        void (*basic)(void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                      seq_endpoint::acceptor::BasicAcceptor acceptor);
        void (*poly)(void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                     poly_seq_endpoint::acceptor::Acceptor<Elem> acceptor);
    } erase;

    struct {
        void (*basic)(void* cntr,
                      seq_endpoint::acceptor::BasicAcceptor acceptor);
        void (*poly)(void* cntr,
                     poly_seq_endpoint::acceptor::Acceptor<Elem> acceptor);
    } erase_all;

    void (*copy_cursor)(void* cntr, seq_cntr::CursorLimit* src_cursor,
                        seq_cntr::CursorLimit* dst_cursor);

    bool (*are_equal_cursor)(void* cntr, seq_cntr::CursorLimit* cursor_a,
                             seq_cntr::CursorLimit* cursor_b);

    comparison::Ordering (*compare_cursor)(void* cntr,
                                           seq_cntr::CursorLimit* cursor_a,
                                           seq_cntr::CursorLimit* cursor_b);

    size_t (*get_cursor_dist)(void* cntr, seq_cntr::CursorLimit* cursor_a,
                              seq_cntr::CursorLimit* cursor_b);

    size_t (*get_cursor_idx)(void* cntr, seq_cntr::CursorLimit* cursor);

    void (*cursor_step_l)(void* cntr, seq_cntr::CursorLimit* cursor);

    void (*cursor_step_r)(void* cntr, seq_cntr::CursorLimit* cursor);

    void (*cursor_advance_l)(void* cntr, seq_cntr::CursorLimit* cursor,
                             size_t step);

    void (*cursor_advance_r)(void* cntr, seq_cntr::CursorLimit* cursor,
                             size_t step);

    unsigned long long (*custom_methods[4])(void* cntr, unsigned long long arg0,
                                            unsigned long long arg1,
                                            unsigned long long arg2,
                                            unsigned long long arg3);
};

template <IsSeqCntr Cntr>
constexpr VTable<Elem(Cntr)> const& GetVTable();

namespace op_check {

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

constexpr bool CanAdvanceL(size_t idx, size_t step, size_t elem_cnt);

constexpr bool CanAdvanceR(size_t idx, size_t step, size_t elem_cnt);

}  // namespace op_check

}  // namespace zeta::core::seq_cntr

#pragma pop_macro("Cursor")
#pragma pop_macro("Elem")
