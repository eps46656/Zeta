/**
@file zeta/core/seq_cntr.ipp
*/

#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/lifecycle.ipp>
#include <zeta/core/lin_seq_endpoint.ipp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

#pragma push_macro("TestCapability")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestCapability(cap_flag, cap_name)                                     \
    (((cap_flag) & (static_cast<capability::Flag>(1)                           \
                    << meta::EnumToUnderlying(capability::Kind::cap_name))) != \
     0)

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

constexpr bool seq_cntr::capability::CheckFlags(Flag se_cap_flag,
                                                Flag sd_cap_flag) {
    Flag cap_flags[]{ se_cap_flag, sd_cap_flag };

    for (Flag cap_flag : cap_flags) {
        if ((cap_flag & empty_capability_flag) != empty_capability_flag) {
            return false;
        }

        if ((cap_flag | full_capability_flag) != full_capability_flag) {
            return false;
        }
    }

    if ((se_cap_flag & sd_cap_flag) != empty_capability_flag) { return false; }

    return true;
}

constexpr bool seq_cntr::capability::CheckFlags(Flag se_cap_flag,
                                                Flag sd_cap_flag,
                                                Flag de_cap_flag,
                                                Flag dd_cap_flag) {
    Flag cap_flags[]{ se_cap_flag, sd_cap_flag, de_cap_flag, dd_cap_flag };

    for (Flag cap_flag : cap_flags) {
        if ((cap_flag & empty_capability_flag) != empty_capability_flag) {
            return false;
        }

        if ((cap_flag | full_capability_flag) != full_capability_flag) {
            return false;
        }
    }

    for (int i{ 0 }; i < 4; ++i) {
        for (int j{ i + 1 }; j < 4; ++j) {
            if ((cap_flags[i] & cap_flags[j]) != empty_capability_flag) {
                return false;
            }
        }
    }

    if ((se_cap_flag | sd_cap_flag | de_cap_flag | dd_cap_flag) !=
        full_capability_flag) {
        return false;
    }

    return true;
}

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(cntr_type, cap_name, method_name, ...)                      \
    (CheckCapabilityFlags)(meta::Forward<cntr_type>(cntr));                    \
                                                                               \
    if constexpr (!TestCapability(                                             \
                      (GetStaticEnabledCapabilityFlag)(meta::TypeWrapper<      \
                                                       cntr_type>{}),          \
                      cap_name)) {                                             \
        static_assert(!TestCapability(                                         \
            (GetStaticDisabledCapabilityFlag)(meta::TypeWrapper<cntr_type>{}), \
            cap_name));                                                        \
                                                                               \
        ZETA_Core_DebugUtils_Diag_PromiseAssert(TestCapability(                \
            (GetDynamicEnabledCapabilityFlag)(meta::Forward<cntr_type>(cntr)), \
            cap_name));                                                        \
    }                                                                          \
                                                                               \
    return meta::Forward<cntr_type>(cntr).method_name(Tag{}, __VA_ARGS__);     \
                                                                               \
    static_assert(true)

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr void seq_cntr::CheckCapabilityFlags(Cntr&& cntr) {
    constexpr capability::Flag se_cap_flag{ (
        GetStaticEnabledCapabilityFlag)(meta::TypeWrapper<Cntr&&>{}) };

    constexpr capability::Flag sd_cap_flag{ (
        GetStaticDisabledCapabilityFlag)(meta::TypeWrapper<Cntr&&>{}) };

    capability::CheckFlags(se_cap_flag, sd_cap_flag);

    capability::Flag de_cap_flag{ (
        GetDynamicEnabledCapabilityFlag)(meta::Forward<Cntr&&>(cntr)) };

    capability::Flag dd_cap_flag{ (
        GetDynamicDisabledCapabilityFlag)(meta::Forward<Cntr&&>(cntr)) };

    capability::CheckFlags(se_cap_flag, sd_cap_flag, de_cap_flag, dd_cap_flag);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr void* seq_cntr::GetReferedInstPtr(Cntr&& cntr) {
    return cntr.GetReferedInstPtr(Tag{});
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr::capability::Flag seq_cntr::GetStaticEnabledCapabilityFlag(
    meta::TypeWrapper<Cntr>) {
    return meta::RemoveRef<Cntr>::GetStaticEnabledCapabilityFlag(
        Tag{}, meta::TypeWrapper<Cntr>{});
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr::capability::Flag seq_cntr::GetStaticDisabledCapabilityFlag(
    meta::TypeWrapper<Cntr>) {
    return meta::RemoveRef<Cntr>::GetStaticDisabledCapabilityFlag(
        Tag{}, meta::TypeWrapper<Cntr>{});
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr seq_cntr::capability::Flag seq_cntr::GetDynamicEnabledCapabilityFlag(
    Cntr&& cntr) {
    return cntr.GetDynamicEnabledCapabilityFlag(Tag{});
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr seq_cntr::capability::Flag seq_cntr::GetDynamicDisabledCapabilityFlag(
    Cntr&& cntr) {
    return cntr.GetDynamicDisabledCapabilityFlag(Tag{});
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr meta::TypeWrapper<Elem(Cntr)> seq_cntr::GetElemType(
    meta::TypeWrapper<Cntr>) {
    return meta::RemoveCVRef<Cntr>::GetElemType(Tag{},
                                                meta::TypeWrapper<Cntr>{});
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr meta::TypeWrapper<Cursor(Cntr)> seq_cntr::GetCursorType(
    meta::TypeWrapper<Cntr>) {
    return meta::RemoveCVRef<Cntr>::GetCursorType(Tag{},
                                                  meta::TypeWrapper<Cntr>{});
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr size_t seq_cntr::GetElemCnt(Cntr&& cntr) {
    CallMethod(Cntr&&, GetElemCnt, GetElemCnt);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr size_t seq_cntr::GetMaxElemCnt(Cntr&& cntr) {
    CallMethod(Cntr&&, GetMaxElemCnt, GetMaxElemCnt);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr decltype(auto) seq_cntr::GetLBCursor(Cntr&& cntr,
                                               Cursor(Cntr&&) * dst_cursor) {
    CallMethod(Cntr&&, GetLBCursor, GetLBCursor, dst_cursor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr decltype(auto) seq_cntr::GetRBCursor(Cntr&& cntr,
                                               Cursor(Cntr&&) * dst_cursor) {
    CallMethod(Cntr&&, GetRBCursor, GetRBCursor, dst_cursor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr,
          lifecycle::IsDstStorageView DstElemView>
constexpr decltype(auto) seq_cntr::PeekL(
    Cntr&& cntr, bool lazy_copy_elem,
    lifecycle::DynamicStorageView<Elem(Cntr&&)>* dst_elem_view,
    Cursor(Cntr&&) * dst_cursor, DstElemView dst_elem) {
    CallMethod(Cntr&&, PeekL, PeekL, lazy_copy_elem, dst_elem_view, dst_cursor,
               dst_elem);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr,
          lifecycle::IsDstStorageView DstElemView>
constexpr decltype(auto) seq_cntr::PeekR(
    Cntr&& cntr, bool lazy_copy_elem,
    lifecycle::DynamicStorageView<Elem(Cntr&&)>* dst_elem_view,
    Cursor(Cntr&&) * dst_cursor, DstElemView dst_elem) {
    CallMethod(Cntr&&, PeekR, PeekR, lazy_copy_elem, dst_elem_view, dst_cursor,
               dst_elem);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr,
          lifecycle::IsDstStorageView DstElemView>
constexpr decltype(auto) seq_cntr::Refer(
    Cntr&& cntr, size_t idx, bool lazy_copy_elem,
    lifecycle::DynamicStorageView<Elem(Cntr&&)>* dst_elem_view,
    Cursor(Cntr&&) * dst_cursor, DstElemView dst_elem) {
    CallMethod(Cntr&&, Refer, Refer, idx, lazy_copy_elem, dst_elem_view,
               dst_cursor, dst_elem);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr,
          lifecycle::IsDstStorageView DstElemView>
constexpr decltype(auto) seq_cntr::Derefer(
    Cntr&& cntr, Cursor(Cntr&&) * pos_cursor, bool lazy_copy_elem,
    lifecycle::DynamicStorageView<Elem(Cntr&&)>* dst_elem_view,
    DstElemView dst_elem) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

    CallMethod(Cntr&&, Derefer, Derefer, pos_cursor, lazy_copy_elem,
               dst_elem_view, dst_elem);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr&&)> Acceptor>
constexpr decltype(auto) seq_cntr::Read(
    Cntr&& cntr, Cursor(Cntr&&) * pos_cursor, size_t cnt,
    Acceptor&& acceptor,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr&&) * dst_cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

    CallMethod(Cntr&&, Read, Read, pos_cursor, cnt, acceptor, dst_cursor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr,
          seq_endpoint::provider::IsProvider<Elem(Cntr&&)> Provider>
constexpr decltype(auto) seq_cntr::Write(
    Cntr&& cntr, Cursor(Cntr&&) * pos_cursor, size_t cnt,
    Provider&& provider,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr&&) * dst_cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

    CallMethod(Cntr&&, Write, Write, pos_cursor, cnt, provider, dst_cursor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr&&)> Acceptor>
constexpr decltype(auto) seq_cntr::ReadWrite(
    Cntr&& cntr, Cursor(Cntr&&) * pos_cursor, size_t cnt,
    Acceptor&& acceptor,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr&&) * dst_cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

    CallMethod(Cntr&&, ReadWrite, ReadWrite, pos_cursor, cnt, acceptor,
               dst_cursor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr,
          seq_endpoint::provider::IsProvider<Elem(Cntr&&)> Provider>
constexpr decltype(auto) seq_cntr::PushL(
    Cntr&& cntr, size_t cnt,
    Provider&& provider,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr&&) * dst_beg_cursor, Cursor(Cntr&&) * dst_end_cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_beg_cursor == nullptr ||
                                            dst_end_cursor == nullptr ||
                                            dst_beg_cursor != dst_end_cursor);

    CallMethod(Cntr&&, PushL, PushL, cnt, provider, dst_beg_cursor,
               dst_end_cursor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr,
          seq_endpoint::provider::IsProvider<Elem(Cntr&&)> Provider>
constexpr decltype(auto) seq_cntr::PushR(
    Cntr&& cntr, size_t cnt,
    Provider&& provider,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr&&) * dst_beg_cursor, Cursor(Cntr&&) * dst_end_cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_beg_cursor == nullptr ||
                                            dst_end_cursor == nullptr ||
                                            dst_beg_cursor != dst_end_cursor);

    CallMethod(Cntr&&, PushR, PushR, cnt, provider, dst_beg_cursor,
               dst_end_cursor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr,
          seq_endpoint::provider::IsProvider<Elem(Cntr&&)> Provider>
constexpr decltype(auto) seq_cntr::Insert(
    Cntr&& cntr, Cursor(Cntr&&) * pos_cursor, size_t cnt,
    Provider&& provider,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr&&) * dst_cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

    CallMethod(Cntr&&, Insert, Insert, pos_cursor, cnt, provider, dst_cursor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr&&)> Acceptor>
constexpr decltype(auto) seq_cntr::PopL(
    Cntr&& cntr, size_t cnt,
    Acceptor&& acceptor,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr&&) * dst_cursor) {
    CallMethod(Cntr&&, PopL, PopL, cnt, acceptor, dst_cursor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr&&)> Acceptor>
constexpr decltype(auto) seq_cntr::PopR(
    Cntr&& cntr, size_t cnt,
    Acceptor&& acceptor,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr&&) * dst_cursor) {
    CallMethod(Cntr&&, PopR, PopR, cnt, acceptor, dst_cursor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr&&)> Acceptor>
constexpr decltype(auto) seq_cntr::Erase(
    Cntr&& cntr, Cursor(Cntr&&) * pos_cursor, size_t cnt,
    Acceptor&& acceptor  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

    CallMethod(Cntr&&, Erase, Erase, pos_cursor, cnt, acceptor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr&&)> Acceptor>
constexpr decltype(auto) seq_cntr::EraseAll(Cntr&& cntr, Acceptor&& acceptor) {
    CallMethod(Cntr&&, EraseAll, EraseAll, acceptor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr decltype(auto) seq_cntr::CopyCursor(Cntr&& cntr,
                                              Cursor(Cntr&&) * src_cursor,
                                              Cursor(Cntr&&) * dst_cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(src_cursor != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr);

    CallMethod(Cntr&&, CopyCursor, CopyCursor, src_cursor, dst_cursor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr bool seq_cntr::AreEqualCursor(Cntr&& cntr, Cursor(Cntr&&) * cursor_a,
                                        Cursor(Cntr&&) * cursor_b) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_a != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_b != nullptr);

    CallMethod(Cntr&&, AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr comparison::Ordering seq_cntr::CompareCursor(
    Cntr&& cntr, Cursor(Cntr&&) * cursor_a, Cursor(Cntr&&) * cursor_b) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_a != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_b != nullptr);

    CallMethod(Cntr&&, CompareCursor, CompareCursor, cursor_a, cursor_b);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr size_t seq_cntr::GetCursorDist(Cntr&& cntr, Cursor(Cntr&&) * cursor_a,
                                         Cursor(Cntr&&) * cursor_b) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_a != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_b != nullptr);

    CallMethod(Cntr&&, GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr size_t seq_cntr::GetCursorIdx(Cntr&& cntr, Cursor(Cntr&&) * cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

    CallMethod(Cntr&&, GetCursorIdx, GetCursorIdx, cursor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr decltype(auto) seq_cntr::CursorStepL(Cntr&& cntr,
                                               Cursor(Cntr&&) * cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

    CallMethod(Cntr&&, CursorStepL, CursorStepL, cursor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr decltype(auto) seq_cntr::CursorStepR(Cntr&& cntr,
                                               Cursor(Cntr&&) * cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

    CallMethod(Cntr&&, CursorStepR, CursorStepR, cursor);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr decltype(auto) seq_cntr::CursorAdvanceL(Cntr&& cntr,
                                                  Cursor(Cntr&&) * cursor,
                                                  size_t step) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

    CallMethod(Cntr&&, CursorAdvanceL, CursorAdvanceL, cursor, step);
}

template <seq_cntr::IsSeqCntrWithRRef Cntr>
constexpr decltype(auto) seq_cntr::CursorAdvanceR(Cntr&& cntr,
                                                  Cursor(Cntr&&) * cursor,
                                                  size_t step) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

    CallMethod(Cntr&&, CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")

namespace seq_cntr::detail {

template <seq_cntr::IsSeqCntr Cntr>
constexpr VTable<Elem(Cntr)> BuildVTable_() {
#pragma push_macro("CntrRef")
#define CntrRef static_cast<Cntr&&>(*static_cast<meta::RemoveRef<Cntr>*>(cntr))

#pragma push_macro("LoadRealCursor")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LoadRealCursor(cursor_name, allow_null)                          \
    if constexpr (!allow_null) {                                         \
        ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_name != nullptr); \
    }                                                                    \
                                                                         \
    alignas(Cursor(Cntr)) unsigned char                                  \
        real_##cursor_name##_storage[sizeof(Cursor(Cntr))];              \
                                                                         \
    Cursor(Cntr) * real_##cursor_name;                                   \
                                                                         \
    if constexpr (!allow_null) {                                         \
        real_##cursor_name = reinterpret_cast<Cursor(Cntr)*>(            \
            new (real_##cursor_name##_storage)                           \
                Cursor(Cntr){ meta::Move(*cursor_name) });               \
    } else if (cursor_name == nullptr) {                                 \
        real_##cursor_name = nullptr;                                    \
    } else {                                                             \
        real_##cursor_name = reinterpret_cast<Cursor(Cntr)*>(            \
            new (real_##cursor_name##_storage)                           \
                Cursor(Cntr){ meta::Move(*cursor_name) });               \
    }                                                                    \
    static_assert(true)

#pragma push_macro("StoreRealCursor")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define StoreRealCursor(cursor_name, allow_null)          \
    if (!allow_null || cursor_name != nullptr) {          \
        *cursor_name = meta::Move(*real_##cursor_name);   \
                                                          \
        lifecycle::InvokeDestructor(*real_##cursor_name); \
    }                                                     \
    static_assert(true)

#pragma push_macro("LoadTwoRealCursor_PosDst")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LoadTwoRealCursor_PosDst(cursor_name_a, cursor_name_b)                 \
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_name_a != nullptr);         \
                                                                               \
    Cursor(Cntr) real_##cursor_name_a##_storage{ meta::Move(*cursor_name_a) }; \
    alignas(Cursor(Cntr)) unsigned char                                        \
        real_##cursor_name_b##_storage[sizeof(Cursor(Cntr))];                  \
                                                                               \
    Cursor(Cntr) * real_##cursor_name_a;                                       \
    Cursor(Cntr) * real_##cursor_name_b;                                       \
                                                                               \
    real_##cursor_name_a = &real_##cursor_name_a##_storage;                    \
                                                                               \
    if (cursor_name_b == nullptr) {                                            \
        real_##cursor_name_b = nullptr;                                        \
    } else if (cursor_name_a == cursor_name_b) {                               \
        real_##cursor_name_b = real_##cursor_name_a;                           \
    } else {                                                                   \
        real_##cursor_name_b = reinterpret_cast<Cursor(Cntr)*>(                \
            new (real_##cursor_name_b##_storage) Cursor(Cntr){});              \
    }                                                                          \
    static_assert(true)

#pragma push_macro("StoreTwoRealCursor_PosDst")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define StoreTwoRealCursor_PosDst(cursor_name_a, cursor_name_b)       \
    if (cursor_name_b != nullptr && cursor_name_a != cursor_name_b) { \
        *cursor_name_b = meta::Move(*real_##cursor_name_b);           \
                                                                      \
        lifecycle::InvokeDestructor(*real_##cursor_name_b);           \
    }                                                                 \
                                                                      \
    *cursor_name_a = meta::Move(*real_##cursor_name_a);               \
    static_assert(true)

#pragma push_macro("LoadTwoRealCursor_AB")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LoadTwoRealCursor_AB(cursor_name_a, cursor_name_b)                     \
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_name_a != nullptr);         \
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_name_b != nullptr);         \
                                                                               \
    Cursor(Cntr) real_##cursor_name_a##_storage{ meta::Move(*cursor_name_a) }; \
    alignas(Cursor(Cntr)) unsigned char                                        \
        real_##cursor_name_b##_storage[sizeof(Cursor(Cntr))];                  \
                                                                               \
    Cursor(Cntr) * real_##cursor_name_a;                                       \
    Cursor(Cntr) * real_##cursor_name_b;                                       \
                                                                               \
    real_##cursor_name_a = &real_##cursor_name_a##_storage;                    \
                                                                               \
    if (cursor_name_a == cursor_name_b) {                                      \
        real_##cursor_name_b = real_##cursor_name_a;                           \
    } else {                                                                   \
        real_##cursor_name_b = reinterpret_cast<Cursor(Cntr)*>(                \
            new (real_##cursor_name_b##_storage)                               \
                Cursor(Cntr){ meta::Move(*cursor_name_b) });                   \
    }                                                                          \
    static_assert(true)

#pragma push_macro("StoreTwoRealCursor_AB")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define StoreTwoRealCursor_AB(cursor_name_a, cursor_name_b) \
    if (cursor_name_a != cursor_name_b) {                   \
        *cursor_name_b = meta::Move(*real_##cursor_name_b); \
                                                            \
        lifecycle::InvokeDestructor(*real_##cursor_name_b); \
    }                                                       \
                                                            \
    *cursor_name_a = meta::Move(*real_##cursor_name_a);     \
    static_assert(true)

#pragma push_macro("F")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(cap, method)                                    \
    []() constexpr {                                      \
        if constexpr (TestCapability(sd_cap_flag, cap)) { \
            return nullptr;                               \
        } else {                                          \
            return method;                                \
        }                                                 \
    }()

    constexpr capability::Flag sd_cap_flag{ (
        GetStaticDisabledCapabilityFlag)(meta::TypeWrapper<Cntr>{}) };

    VTable<Elem(Cntr)> table;

    for (auto& custom_tag : table.custom_tags) { custom_tag = 0; }

    table.get_elem_cnt = F(  //
        GetElemCnt, [](void* cntr) { return (GetElemCnt)(CntrRef); });

    table.get_max_elem_cnt = F(  //
        GetMaxElemCnt, [](void* cntr) { return (GetMaxElemCnt)(CntrRef); });

    table.get_lb_cursor = F(  //
        GetLBCursor, [](void* cntr, seq_cntr::CursorLimit* dst_cursor) {
            LoadRealCursor(dst_cursor, true);

            (GetLBCursor)(CntrRef, real_dst_cursor);

            StoreRealCursor(dst_cursor, true);
        });

    table.get_rb_cursor = F(  //
        GetRBCursor, [](void* cntr, seq_cntr::CursorLimit* dst_cursor) {
            LoadRealCursor(dst_cursor, true);

            (GetRBCursor)(CntrRef, real_dst_cursor);

            StoreRealCursor(dst_cursor, true);
        });

    // -------------------------------------------------------------------------

    table.peek_l.constructing_mem_to_obj = F(  //
        PeekL, [](void* cntr, bool lazy_copy_elem,
                  lifecycle::DynamicStorageView<Elem(Cntr)>* dst_elem_view,
                  seq_cntr::CursorLimit* dst_cursor,
                  lifecycle::ConstructingMemToObjView<Elem(Cntr)> dst_elem) {
            LoadRealCursor(dst_cursor, true);

            (PeekL)(CntrRef, lazy_copy_elem, dst_elem_view, real_dst_cursor,
                    dst_elem);

            StoreRealCursor(dst_cursor, true);
        });

    table.peek_l.left_obj = F(  //
        PeekL, [](void* cntr, bool lazy_copy_elem,
                  lifecycle::DynamicStorageView<Elem(Cntr)>* dst_elem_view,
                  seq_cntr::CursorLimit* dst_cursor,
                  lifecycle::LeftObjView<Elem(Cntr)> dst_elem) {
            LoadRealCursor(dst_cursor, true);

            (PeekL)(CntrRef, lazy_copy_elem, dst_elem_view, real_dst_cursor,
                    dst_elem);

            StoreRealCursor(dst_cursor, true);
        });

    table.peek_l.right_obj = F(  //
        PeekL, [](void* cntr, bool lazy_copy_elem,
                  lifecycle::DynamicStorageView<Elem(Cntr)>* dst_elem_view,
                  seq_cntr::CursorLimit* dst_cursor,
                  lifecycle::RightObjView<Elem(Cntr)> dst_elem) {
            LoadRealCursor(dst_cursor, true);

            (PeekL)(CntrRef, lazy_copy_elem, dst_elem_view, real_dst_cursor,
                    dst_elem);

            StoreRealCursor(dst_cursor, true);
        });

    // -------------------------------------------------------------------------

    table.peek_r.constructing_mem_to_obj = F(  //
        PeekR, [](void* cntr, bool lazy_copy_elem,
                  lifecycle::DynamicStorageView<Elem(Cntr)>* dst_elem_view,
                  seq_cntr::CursorLimit* dst_cursor,
                  lifecycle::ConstructingMemToObjView<Elem(Cntr)> dst_elem) {
            LoadRealCursor(dst_cursor, true);

            (PeekR)(CntrRef, lazy_copy_elem, dst_elem_view, real_dst_cursor,
                    dst_elem);

            StoreRealCursor(dst_cursor, true);
        });

    table.peek_r.left_obj = F(  //
        PeekR, [](void* cntr, bool lazy_copy_elem,
                  lifecycle::DynamicStorageView<Elem(Cntr)>* dst_elem_view,
                  seq_cntr::CursorLimit* dst_cursor,
                  lifecycle::LeftObjView<Elem(Cntr)> dst_elem) {
            LoadRealCursor(dst_cursor, true);

            (PeekR)(CntrRef, lazy_copy_elem, dst_elem_view, real_dst_cursor,
                    dst_elem);

            StoreRealCursor(dst_cursor, true);
        });

    table.peek_r.right_obj = F(  //
        PeekR, [](void* cntr, bool lazy_copy_elem,
                  lifecycle::DynamicStorageView<Elem(Cntr)>* dst_elem_view,
                  seq_cntr::CursorLimit* dst_cursor,
                  lifecycle::RightObjView<Elem(Cntr)> dst_elem) {
            LoadRealCursor(dst_cursor, true);

            (PeekR)(CntrRef, lazy_copy_elem, dst_elem_view, real_dst_cursor,
                    dst_elem);

            StoreRealCursor(dst_cursor, true);
        });

    // -------------------------------------------------------------------------

    table.refer.constructing_mem_to_obj = F(  //
        Refer, [](void* cntr, size_t idx, bool lazy_copy_elem,
                  lifecycle::DynamicStorageView<Elem(Cntr)>* dst_elem_view,
                  seq_cntr::CursorLimit* dst_cursor,
                  lifecycle::ConstructingMemToObjView<Elem(Cntr)> dst_elem) {
            LoadRealCursor(dst_cursor, true);

            (Refer)(CntrRef, idx, lazy_copy_elem, dst_elem_view,
                    real_dst_cursor, dst_elem);

            StoreRealCursor(dst_cursor, true);
        });

    table.refer.left_obj = F(  //
        Refer, [](void* cntr, size_t idx, bool lazy_copy_elem,
                  lifecycle::DynamicStorageView<Elem(Cntr)>* dst_elem_view,
                  seq_cntr::CursorLimit* dst_cursor,
                  lifecycle::LeftObjView<Elem(Cntr)> dst_elem) {
            LoadRealCursor(dst_cursor, true);

            (Refer)(CntrRef, idx, lazy_copy_elem, dst_elem_view,
                    real_dst_cursor, dst_elem);

            StoreRealCursor(dst_cursor, true);
        });

    table.refer.right_obj = F(  //
        Refer, [](void* cntr, size_t idx, bool lazy_copy_elem,
                  lifecycle::DynamicStorageView<Elem(Cntr)>* dst_elem_view,
                  seq_cntr::CursorLimit* dst_cursor,
                  lifecycle::RightObjView<Elem(Cntr)> dst_elem) {
            LoadRealCursor(dst_cursor, true);

            (Refer)(CntrRef, idx, lazy_copy_elem, dst_elem_view,
                    real_dst_cursor, dst_elem);

            StoreRealCursor(dst_cursor, true);
        });

    // -------------------------------------------------------------------------

    table.derefer.constructing_mem_to_obj = F(  //
        Derefer,
        [](void* cntr, seq_cntr::CursorLimit* pos_cursor, bool lazy_copy_elem,
           lifecycle::DynamicStorageView<Elem(Cntr)>* dst_elem_view,
           lifecycle::ConstructingMemToObjView<Elem(Cntr)> dst_elem) {
            LoadRealCursor(pos_cursor, false);

            (Derefer)(CntrRef, real_pos_cursor, lazy_copy_elem, dst_elem_view,
                      dst_elem);

            StoreRealCursor(pos_cursor, false);
        });

    table.derefer.left_obj = F(  //
        Derefer,
        [](void* cntr, seq_cntr::CursorLimit* pos_cursor, bool lazy_copy_elem,
           lifecycle::DynamicStorageView<Elem(Cntr)>* dst_elem_view,
           lifecycle::LeftObjView<Elem(Cntr)> dst_elem) {
            LoadRealCursor(pos_cursor, false);

            (Derefer)(CntrRef, real_pos_cursor, lazy_copy_elem, dst_elem_view,
                      dst_elem);

            StoreRealCursor(pos_cursor, false);
        });

    table.derefer.right_obj = F(  //
        Derefer,
        [](void* cntr, seq_cntr::CursorLimit* pos_cursor, bool lazy_copy_elem,
           lifecycle::DynamicStorageView<Elem(Cntr)>* dst_elem_view,
           lifecycle::RightObjView<Elem(Cntr)> dst_elem) {
            LoadRealCursor(pos_cursor, false);

            (Derefer)(CntrRef, real_pos_cursor, lazy_copy_elem, dst_elem_view,
                      dst_elem);

            StoreRealCursor(pos_cursor, false);
        });

    // -------------------------------------------------------------------------

    table.read.basic =
        F(Read, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                   seq_endpoint::acceptor::BasicAcceptor acceptor,
                   seq_cntr::CursorLimit* dst_cursor) {
            LoadTwoRealCursor_PosDst(pos_cursor, dst_cursor);

            (Read)(CntrRef, real_pos_cursor, cnt, acceptor, real_dst_cursor);

            StoreTwoRealCursor_PosDst(pos_cursor, dst_cursor);
        });

    table.read.poly = F(  //
        Read, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                 poly_seq_endpoint::acceptor::Acceptor<Elem(Cntr)> acceptor,
                 seq_cntr::CursorLimit* dst_cursor) {
            LoadTwoRealCursor_PosDst(pos_cursor, dst_cursor);

            (Read)(CntrRef, real_pos_cursor, cnt, acceptor, real_dst_cursor);

            StoreTwoRealCursor_PosDst(pos_cursor, dst_cursor);
        });

    table.write.basic = F(  //
        Write, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                  seq_endpoint::provider::BasicProvider provider,
                  seq_cntr::CursorLimit* dst_cursor) {
            LoadTwoRealCursor_PosDst(pos_cursor, dst_cursor);

            (Write)(CntrRef, real_pos_cursor, cnt, provider, real_dst_cursor);

            StoreTwoRealCursor_PosDst(pos_cursor, dst_cursor);
        });

    table.write.poly = F(  //
        Write, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                  poly_seq_endpoint::provider::Provider<Elem(Cntr)> provider,
                  seq_cntr::CursorLimit* dst_cursor) {
            LoadTwoRealCursor_PosDst(pos_cursor, dst_cursor);

            (Write)(CntrRef, real_pos_cursor, cnt, provider, real_dst_cursor);

            StoreTwoRealCursor_PosDst(pos_cursor, dst_cursor);
        });

    table.read_write.poly = F(  //
        ReadWrite,
        [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
           poly_seq_endpoint::acceptor::Acceptor<Elem(Cntr)> acceptor,
           seq_cntr::CursorLimit* dst_cursor) {
            LoadTwoRealCursor_PosDst(pos_cursor, dst_cursor);

            (ReadWrite)(CntrRef, real_pos_cursor, cnt, acceptor,
                        real_dst_cursor);

            StoreTwoRealCursor_PosDst(pos_cursor, dst_cursor);
        });

    table.push_l.basic = F(  //
        PushL, [](void* cntr, size_t cnt,
                  seq_endpoint::provider::BasicProvider provider,
                  seq_cntr::CursorLimit* dst_beg_cursor,
                  seq_cntr::CursorLimit* dst_end_cursor) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                dst_beg_cursor == nullptr || dst_end_cursor == nullptr ||
                dst_beg_cursor != dst_end_cursor);

            LoadRealCursor(dst_beg_cursor, true);
            LoadRealCursor(dst_end_cursor, true);

            (PushL)(CntrRef, cnt, provider, real_dst_beg_cursor,
                    real_dst_end_cursor);

            StoreRealCursor(dst_end_cursor, true);
            StoreRealCursor(dst_beg_cursor, true);
        });

    table.push_l.poly = F(  //
        PushL, [](void* cntr, size_t cnt,
                  poly_seq_endpoint::provider::Provider<Elem(Cntr)> provider,
                  seq_cntr::CursorLimit* dst_beg_cursor,
                  seq_cntr::CursorLimit* dst_end_cursor) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                dst_beg_cursor == nullptr || dst_end_cursor == nullptr ||
                dst_beg_cursor != dst_end_cursor);

            LoadRealCursor(dst_beg_cursor, true);
            LoadRealCursor(dst_end_cursor, true);

            (PushL)(CntrRef, cnt, provider, real_dst_beg_cursor,
                    real_dst_end_cursor);

            StoreRealCursor(dst_end_cursor, true);
            StoreRealCursor(dst_beg_cursor, true);
        });

    table.push_r.basic = F(  //
        PushR, [](void* cntr, size_t cnt,
                  seq_endpoint::provider::BasicProvider provider,
                  seq_cntr::CursorLimit* dst_beg_cursor,
                  seq_cntr::CursorLimit* dst_end_cursor) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                dst_beg_cursor == nullptr || dst_end_cursor == nullptr ||
                dst_beg_cursor != dst_end_cursor);

            LoadRealCursor(dst_beg_cursor, true);
            LoadRealCursor(dst_end_cursor, true);

            (PushR)(CntrRef, cnt, provider, real_dst_beg_cursor,
                    real_dst_end_cursor);

            StoreRealCursor(dst_end_cursor, true);
            StoreRealCursor(dst_beg_cursor, true);
        });

    table.push_r.poly = F(  //
        PushR, [](void* cntr, size_t cnt,
                  poly_seq_endpoint::provider::Provider<Elem(Cntr)> provider,
                  seq_cntr::CursorLimit* dst_beg_cursor,
                  seq_cntr::CursorLimit* dst_end_cursor) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                dst_beg_cursor == nullptr || dst_end_cursor == nullptr ||
                dst_beg_cursor != dst_end_cursor);

            LoadRealCursor(dst_beg_cursor, true);
            LoadRealCursor(dst_end_cursor, true);

            (PushR)(CntrRef, cnt, provider, real_dst_beg_cursor,
                    real_dst_end_cursor);

            StoreRealCursor(dst_end_cursor, true);
            StoreRealCursor(dst_beg_cursor, true);
        });

    table.insert.basic = F(  //
        Insert, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                   seq_endpoint::provider::BasicProvider provider,
                   seq_cntr::CursorLimit* dst_cursor) {
            LoadTwoRealCursor_PosDst(pos_cursor, dst_cursor);

            (Insert)(CntrRef, real_pos_cursor, cnt, provider, real_dst_cursor);

            StoreTwoRealCursor_PosDst(pos_cursor, dst_cursor);
        });

    table.insert.poly = F(  //
        Insert, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                   poly_seq_endpoint::provider::Provider<Elem(Cntr)> provider,
                   seq_cntr::CursorLimit* dst_cursor) {
            LoadTwoRealCursor_PosDst(pos_cursor, dst_cursor);

            (Insert)(CntrRef, real_pos_cursor, cnt, provider, real_dst_cursor);

            StoreTwoRealCursor_PosDst(pos_cursor, dst_cursor);
        });

    table.pop_l.basic = F(  //
        PopL, [](void* cntr, size_t cnt,
                 seq_endpoint::acceptor::BasicAcceptor acceptor,
                 seq_cntr::CursorLimit* dst_cursor) {
            LoadRealCursor(dst_cursor, true);

            (PopL)(CntrRef, cnt, acceptor, real_dst_cursor);

            StoreRealCursor(dst_cursor, true);
        });

    table.pop_l.poly = F(  //
        PopL, [](void* cntr, size_t cnt,
                 poly_seq_endpoint::acceptor::Acceptor<Elem(Cntr)> acceptor,
                 seq_cntr::CursorLimit* dst_cursor) {
            LoadRealCursor(dst_cursor, true);

            (PopL)(CntrRef, cnt, acceptor, real_dst_cursor);

            StoreRealCursor(dst_cursor, true);
        });

    table.pop_r.basic = F(  //
        PopR, [](void* cntr, size_t cnt,
                 seq_endpoint::acceptor::BasicAcceptor acceptor,
                 seq_cntr::CursorLimit* dst_cursor) {
            LoadRealCursor(dst_cursor, true);

            (PopR)(CntrRef, cnt, acceptor, real_dst_cursor);

            StoreRealCursor(dst_cursor, true);
        });

    table.pop_r.poly = F(  //
        PopR, [](void* cntr, size_t cnt,
                 poly_seq_endpoint::acceptor::Acceptor<Elem(Cntr)> acceptor,
                 seq_cntr::CursorLimit* dst_cursor) {
            LoadRealCursor(dst_cursor, true);

            (PopR)(CntrRef, cnt, acceptor, real_dst_cursor);

            StoreRealCursor(dst_cursor, true);
        });

    table.erase.basic = F(  //
        Erase, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                  seq_endpoint::acceptor::BasicAcceptor acceptor) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

            LoadRealCursor(pos_cursor, false);

            (Erase)(CntrRef, real_pos_cursor, cnt, acceptor);

            StoreRealCursor(pos_cursor, false);
        });

    table.erase.poly = F(  //
        Erase, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                  poly_seq_endpoint::acceptor::Acceptor<Elem(Cntr)> acceptor) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

            LoadRealCursor(pos_cursor, false);

            (Erase)(CntrRef, real_pos_cursor, cnt, acceptor);

            StoreRealCursor(pos_cursor, false);
        });

    table.erase_all.basic =
        F(EraseAll,
          [](void* cntr, seq_endpoint::acceptor::BasicAcceptor acceptor) {
              (EraseAll)(CntrRef, acceptor);
          });

    table.erase_all.poly =
        F(EraseAll,
          [](void* cntr,
             poly_seq_endpoint::acceptor::Acceptor<Elem(Cntr)> acceptor) {
              (EraseAll)(CntrRef, acceptor);
          });

    table.copy_cursor = F(  //
        CopyCursor, [](void* cntr, seq_cntr::CursorLimit* src_cursor,
                       seq_cntr::CursorLimit* dst_cursor) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(src_cursor != nullptr);
            ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(src_cursor != dst_cursor);

            Cursor(Cntr) real_src_cursor{ *src_cursor };
            Cursor(Cntr) real_dst_cursor;

            (CopyCursor)(CntrRef, &real_src_cursor, &real_dst_cursor);

            *src_cursor = meta::Move(real_src_cursor);
            *dst_cursor = meta::Move(real_dst_cursor);
        });

    table.are_equal_cursor = F(  //
        AreEqualCursor, [](void* cntr, seq_cntr::CursorLimit* cursor_a,
                           seq_cntr::CursorLimit* cursor_b) {
            LoadTwoRealCursor_AB(cursor_a, cursor_b);

            decltype(auto) ret{ (AreEqualCursor)(CntrRef, real_cursor_a,
                                                 real_cursor_b) };

            StoreTwoRealCursor_AB(cursor_a, cursor_b);

            return ret;
        });

    table.compare_cursor = F(  //
        CompareCursor, [](void* cntr, seq_cntr::CursorLimit* cursor_a,
                          seq_cntr::CursorLimit* cursor_b) {
            LoadTwoRealCursor_AB(cursor_a, cursor_b);

            decltype(auto) ret{ (CompareCursor)(CntrRef, real_cursor_a,
                                                real_cursor_b) };

            StoreTwoRealCursor_AB(cursor_a, cursor_b);

            return ret;
        });

    table.get_cursor_dist = F(  //
        GetCursorDist, [](void* cntr, seq_cntr::CursorLimit* cursor_a,
                          seq_cntr::CursorLimit* cursor_b) {
            LoadTwoRealCursor_AB(cursor_a, cursor_b);

            decltype(auto) ret{ (GetCursorDist)(CntrRef, real_cursor_a,
                                                real_cursor_b) };

            StoreTwoRealCursor_AB(cursor_a, cursor_b);

            return ret;
        });

    table.get_cursor_idx = F(  //
        GetCursorIdx, [](void* cntr, seq_cntr::CursorLimit* cursor) {
            LoadRealCursor(cursor, false);

            decltype(auto) ret{ (GetCursorIdx)(CntrRef, real_cursor) };

            StoreRealCursor(cursor, false);

            return ret;
        });

    table.cursor_step_l = F(  //
        CursorStepL, [](void* cntr, seq_cntr::CursorLimit* cursor) {
            LoadRealCursor(cursor, false);

            (CursorStepL)(CntrRef, real_cursor);

            StoreRealCursor(cursor, false);
        });

    table.cursor_step_r = F(  //
        CursorStepR, [](void* cntr, seq_cntr::CursorLimit* cursor) {
            LoadRealCursor(cursor, false);

            (CursorStepR)(CntrRef, real_cursor);

            StoreRealCursor(cursor, false);
        });

    table.cursor_advance_l = F(  //
        CursorAdvanceL,
        [](void* cntr, seq_cntr::CursorLimit* cursor, size_t step) {
            LoadRealCursor(cursor, false);

            (CursorAdvanceL)(CntrRef, real_cursor, step);

            StoreRealCursor(cursor, false);
        });

    table.cursor_advance_r = F(  //
        CursorAdvanceR,
        [](void* cntr, seq_cntr::CursorLimit* cursor, size_t step) {
            LoadRealCursor(cursor, false);

            (CursorAdvanceR)(CntrRef, real_cursor, step);

            StoreRealCursor(cursor, false);
        });

    for (auto& custom_method : table.custom_methods) {
        custom_method = nullptr;
    }

#pragma pop_macro("F")
#pragma pop_macro("StoreTwoRealCursor_AB")
#pragma pop_macro("LoadTwoRealCursor_AB")
#pragma pop_macro("StoreTwoRealCursor_PosDst")
#pragma pop_macro("LoadTwoRealCursor_PosDst")
#pragma pop_macro("StoreRealCursor")
#pragma pop_macro("LoadRealCursor")
#pragma pop_macro("CntrRef")

    return table;
}

template <IsSeqCntr Cntr>
inline constexpr seq_cntr::VTable<Elem(Cntr)> vtable_{ (BuildVTable_<Cntr>)() };

}  // namespace seq_cntr::detail

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr::VTable<Elem(Cntr)> const& seq_cntr::GetVTable() {
    return detail::vtable_<Cntr>;
}

constexpr bool seq_cntr::op_check::CanRefer(size_t idx, size_t cnt,
                                            size_t elem_cnt) {
    return elem_cnt <= max_max_elem_cnt && idx + 1 < elem_cnt + 2 &&
           cnt <= elem_cnt - idx + 1;
}

constexpr bool seq_cntr::op_check::CanDerefer(size_t idx, size_t cnt,
                                              size_t elem_cnt) {
    return elem_cnt <= max_max_elem_cnt && idx <= elem_cnt &&
           cnt <= elem_cnt - idx;
}

constexpr bool seq_cntr::op_check::CanPushL(size_t cnt, size_t elem_cnt,
                                            size_t max_elem_cnt) {
    return elem_cnt <= max_elem_cnt && max_elem_cnt <= max_max_elem_cnt &&
           cnt <= max_elem_cnt - elem_cnt;
}

constexpr bool seq_cntr::op_check::CanPushR(size_t cnt, size_t elem_cnt,
                                            size_t max_elem_cnt) {
    return elem_cnt <= max_elem_cnt && max_elem_cnt <= max_max_elem_cnt &&
           cnt <= max_elem_cnt - elem_cnt;
}

constexpr bool seq_cntr::op_check::CanInsert(size_t idx, size_t cnt,
                                             size_t elem_cnt,
                                             size_t max_elem_cnt) {
    return elem_cnt <= max_elem_cnt && max_elem_cnt <= max_max_elem_cnt &&
           idx <= elem_cnt && elem_cnt <= max_elem_cnt &&
           cnt <= max_elem_cnt - elem_cnt;
}

constexpr bool seq_cntr::op_check::CanPopL(size_t cnt, size_t elem_cnt) {
    return cnt <= elem_cnt;
}

constexpr bool seq_cntr::op_check::CanPopR(size_t cnt, size_t elem_cnt) {
    return cnt <= elem_cnt;
}

constexpr bool seq_cntr::op_check::CanErase(size_t idx, size_t cnt,
                                            size_t elem_cnt) {
    return (CanDerefer)(idx, cnt, elem_cnt);
}

constexpr bool seq_cntr::op_check::CanStepL(size_t idx, size_t elem_cnt) {
    return (CanAdvanceL)(idx, 1, elem_cnt);
}

constexpr bool seq_cntr::op_check::CanStepR(size_t idx, size_t elem_cnt) {
    return (CanAdvanceR)(idx, 1, elem_cnt);
}

constexpr bool seq_cntr::op_check::CanAdvanceL(size_t idx, size_t step,
                                               size_t elem_cnt) {
    return elem_cnt <= max_max_elem_cnt && idx + 1 < elem_cnt + 2 &&
           step <= idx + 1;
}

constexpr bool seq_cntr::op_check::CanAdvanceR(size_t idx, size_t step,
                                               size_t elem_cnt) {
    return elem_cnt <= max_max_elem_cnt && idx + 1 < elem_cnt + 2 &&
           step <= elem_cnt - idx;
}

#pragma pop_macro("Elem")
#pragma pop_macro("Cursor")
#pragma pop_macro("TestCapability")

}  // namespace zeta::core
