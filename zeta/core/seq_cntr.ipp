/**
@file zeta/core/seq_cntr.ipp
*/

#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/lin_seq_endpoint.ipp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

#pragma push_macro("TestCapability")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestCapability(cap_flag, cap_name)           \
    (((cap_flag) & (static_cast<capability::Flag>(1) \
                    << meta::ToUnderlying(capability::Kind::cap_name))) != 0)

#pragma push_macro("Elem")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define Elem(CntrType)                                                 \
    zeta::core::meta::GetTypeWrapperType<                              \
        decltype(zeta::core::meta::RemoveCVRef<CntrType>::GetElemType( \
            zeta::core::seq_cntr::Tag{},                               \
            zeta::core::meta::TypeWrapper<meta::RemoveRef<CntrType>>{}))>

#pragma push_macro("Cursor")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define Cursor(CntrType)                                                 \
    zeta::core::meta::GetTypeWrapperType<                                \
        decltype(zeta::core::meta::RemoveCVRef<CntrType>::GetCursorType( \
            zeta::core::seq_cntr::Tag{},                                 \
            zeta::core::meta::TypeWrapper<meta::RemoveRef<CntrType>>{}))>

constexpr bool seq_cntr::capability::CheckFlags(
    Flag static_enabled_capability_flag, Flag static_disabled_capability_flag) {
    Flag capability_flags[]{ static_enabled_capability_flag,
                             static_disabled_capability_flag };

    for (Flag capability_flag : capability_flags) {
        if ((capability_flag & empty_capability_flag) !=
            empty_capability_flag) {
            return false;
        }

        if ((capability_flag | full_capability_flag) != full_capability_flag) {
            return false;
        }
    }

    if ((static_enabled_capability_flag & static_disabled_capability_flag) !=
        empty_capability_flag) {
        return false;
    }

    return true;
}

constexpr bool seq_cntr::capability::CheckFlags(
    Flag static_enabled_capability_flag, Flag static_disabled_capability_flag,
    Flag dynamic_enabled_capability_flag,
    Flag dynamic_disabled_capability_flag) {
    Flag capability_flags[]{ static_enabled_capability_flag,
                             static_disabled_capability_flag,
                             dynamic_enabled_capability_flag,
                             dynamic_disabled_capability_flag };

    for (Flag capability_flag : capability_flags) {
        if ((capability_flag & empty_capability_flag) !=
            empty_capability_flag) {
            return false;
        }

        if ((capability_flag | full_capability_flag) != full_capability_flag) {
            return false;
        }
    }

    for (int i{ 0 }; i < 4; ++i) {
        for (int j{ i + 1 }; j < 4; ++j) {
            if ((capability_flags[i] & capability_flags[j]) !=
                empty_capability_flag) {
                return false;
            }
        }
    }

    if ((static_enabled_capability_flag | static_disabled_capability_flag |
         dynamic_enabled_capability_flag | dynamic_disabled_capability_flag) !=
        full_capability_flag) {
        return false;
    }

    return true;
}

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(cap_name, method_name, ...)                              \
    (CheckCapabilityFlags)(meta::Forward<Cntr>(cntr));                      \
                                                                            \
    if constexpr (!TestCapability((GetStaticEnabledCapabilityFlag<Cntr>)(), \
                                  cap_name)) {                              \
        static_assert(!TestCapability(                                      \
            (GetStaticDisabledCapabilityFlag<Cntr>)(), cap_name));          \
                                                                            \
        ZETA_Core_DebugUtils_Diag_PromiseAssert(TestCapability(             \
            (GetDynamicEnabledCapabilityFlag)(meta::Forward<Cntr>(cntr)),   \
            cap_name));                                                     \
    }                                                                       \
                                                                            \
    return meta::Forward<Cntr>(cntr).method_name(Tag{}, __VA_ARGS__);       \
                                                                            \
    static_assert(true)

template <seq_cntr::IsSeqCntr Cntr>
constexpr void seq_cntr::CheckCapabilityFlags(Cntr&& cntr) {
    constexpr capability::Flag static_enabled_capability_flag{
        meta::RemoveCVRef<Cntr>::GetStaticEnabledCapabilityFlag(
            Tag{}, meta::TypeWrapper<meta::RemoveRef<Cntr>>{})
    };

    constexpr capability::Flag static_disabled_capability_flag{
        meta::RemoveCVRef<Cntr>::GetStaticDisabledCapabilityFlag(
            Tag{}, meta::TypeWrapper<meta::RemoveRef<Cntr>>{})
    };

    capability::CheckFlags(static_enabled_capability_flag,
                           static_disabled_capability_flag);

    capability::Flag dynamic_enabled_capability_flag{
        cntr.GetDynamicEnabledCapabilityFlag(Tag{})
    };

    capability::Flag dynamic_disabled_capability_flag{
        cntr.GetDynamicDisabledCapabilityFlag(Tag{})
    };

    capability::CheckFlags(
        static_enabled_capability_flag, static_disabled_capability_flag,
        dynamic_enabled_capability_flag, dynamic_disabled_capability_flag);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr void* seq_cntr::GetReferedInstPtr(Cntr&& cntr) {
    return cntr.GetReferedInstPtr(Tag{});
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr::capability::Flag
seq_cntr::GetStaticEnabledCapabilityFlag() {
    return meta::RemoveCVRef<Cntr>::GetStaticEnabledCapabilityFlag(
        Tag{}, meta::TypeWrapper<meta::RemoveRef<Cntr>>{});
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr::capability::Flag
seq_cntr::GetStaticDisabledCapabilityFlag() {
    return meta::RemoveCVRef<Cntr>::GetStaticDisabledCapabilityFlag(
        Tag{}, meta::TypeWrapper<meta::RemoveRef<Cntr>>{});
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr::capability::Flag seq_cntr::GetDynamicEnabledCapabilityFlag(
    Cntr&& cntr) {
    return cntr.GetDynamicEnabledCapabilityFlag(Tag{});
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr::capability::Flag seq_cntr::GetDynamicDisabledCapabilityFlag(
    Cntr&& cntr) {
    return cntr.GetDynamicDisabledCapabilityFlag(Tag{});
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr meta::TypeWrapper<Elem(Cntr)> seq_cntr::GetElemType() {
    return meta::RemoveCVRef<Cntr>::GetElemType(
        Tag{}, meta::TypeWrapper<meta::RemoveRef<Cntr>>{});
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr meta::TypeWrapper<Cursor(Cntr)> seq_cntr::GetCursorType() {
    return meta::RemoveCVRef<Cntr>::GetCursorType(
        Tag{}, meta::TypeWrapper<meta::RemoveRef<Cntr>>{});
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr size_t seq_cntr::GetElemCnt(Cntr&& cntr) {
    CallMethod(GetElemCnt, GetElemCnt);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr size_t seq_cntr::GetMaxElemCnt(Cntr&& cntr) {
    CallMethod(GetMaxElemCnt, GetMaxElemCnt);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetLBCursor(Cntr&& cntr,
                                               Cursor(Cntr) * dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetRBCursor(Cntr&& cntr,
                                               Cursor(Cntr) * dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::PeekL(
    Cntr&& cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
    Cursor(Cntr) * dst_cursor, lifecycle::DataLifeState dst_elem_life_state,
    Elem(Cntr) * dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem_life_state, dst_elem);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::PeekR(
    Cntr&& cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
    Cursor(Cntr) * dst_cursor, lifecycle::DataLifeState dst_elem_life_state,
    Elem(Cntr) * dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem_life_state, dst_elem);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::Refer(
    Cntr&& cntr, size_t idx, bool lazy_copy_elem,
    ElemPtrView* dst_elem_ptr_view, Cursor(Cntr) * dst_cursor,
    lifecycle::DataLifeState dst_elem_life_state, Elem(Cntr) * dst_elem) {
    CallMethod(Refer, Refer, idx, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem_life_state, dst_elem);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::Derefer(
    Cntr&& cntr, Cursor(Cntr) * pos_cursor, bool lazy_copy_elem,
    ElemPtrView* dst_elem_ptr_view,
    lifecycle::DataLifeState dst_elem_life_state, Elem(Cntr) * dst_elem) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem_ptr_view,
               dst_elem_life_state, dst_elem);
}

template <seq_cntr::IsSeqCntr Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr)> acceptor>
constexpr decltype(auto) seq_cntr::Read(
    Cntr&& cntr, Cursor(Cntr) * pos_cursor, size_t cnt,
    acceptor&& acceptor,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr) * dst_cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

    CallMethod(Read, Read, pos_cursor, cnt, acceptor, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr,
          seq_endpoint::provider::IsProvider<Elem(Cntr)> Provider>
constexpr decltype(auto) seq_cntr::Write(
    Cntr&& cntr, Cursor(Cntr) * pos_cursor, size_t cnt,
    Provider&& provider,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr) * dst_cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

    CallMethod(Write, Write, pos_cursor, cnt, provider, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr)> Acceptor>
constexpr decltype(auto) seq_cntr::ReadWrite(
    Cntr&& cntr, Cursor(Cntr) * pos_cursor, size_t cnt,
    Acceptor&& acceptor,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr) * dst_cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

    CallMethod(ReadWrite, ReadWrite, pos_cursor, cnt, acceptor, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr,
          seq_endpoint::provider::IsProvider<Elem(Cntr)> Provider>
constexpr decltype(auto) seq_cntr::PushL(
    Cntr&& cntr, size_t cnt,
    Provider&& provider,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr) * dst_beg_cursor, Cursor(Cntr) * dst_end_cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_beg_cursor == nullptr ||
                                            dst_end_cursor == nullptr ||
                                            dst_beg_cursor != dst_end_cursor);

    CallMethod(PushL, PushL, cnt, provider, dst_beg_cursor, dst_end_cursor);
}

template <seq_cntr::IsSeqCntr Cntr,
          seq_endpoint::provider::IsProvider<Elem(Cntr)> Provider>
constexpr decltype(auto) seq_cntr::PushR(
    Cntr&& cntr, size_t cnt,
    Provider&& provider,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr) * dst_beg_cursor, Cursor(Cntr) * dst_end_cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_beg_cursor == nullptr ||
                                            dst_end_cursor == nullptr ||
                                            dst_beg_cursor != dst_end_cursor);

    CallMethod(PushR, PushR, cnt, provider, dst_beg_cursor, dst_end_cursor);
}

template <seq_cntr::IsSeqCntr Cntr,
          seq_endpoint::provider::IsProvider<Elem(Cntr)> Provider>
constexpr decltype(auto) seq_cntr::Insert(
    Cntr&& cntr, Cursor(Cntr) * pos_cursor, size_t cnt,
    Provider&& provider,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr) * dst_cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

    CallMethod(Insert, Insert, pos_cursor, cnt, provider, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr)> acceptor>
constexpr decltype(auto) seq_cntr::PopL(
    Cntr&& cntr, size_t cnt,
    acceptor&& acceptor,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr) * dst_cursor) {
    CallMethod(PopL, PopL, cnt, acceptor, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr)> acceptor>
constexpr decltype(auto) seq_cntr::PopR(
    Cntr&& cntr, size_t cnt,
    acceptor&& acceptor,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor(Cntr) * dst_cursor) {
    CallMethod(PopR, PopR, cnt, acceptor, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr)> acceptor>
constexpr decltype(auto) seq_cntr::Erase(
    Cntr&& cntr, Cursor(Cntr) * pos_cursor, size_t cnt,
    acceptor&& acceptor  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

    CallMethod(Erase, Erase, pos_cursor, cnt, acceptor);
}

template <seq_cntr::IsSeqCntr Cntr,
          seq_endpoint::acceptor::IsAcceptor<Elem(Cntr)> Acceptor>
constexpr decltype(auto) seq_cntr::EraseAll(Cntr&& cntr, Acceptor&& acceptor) {
    CallMethod(EraseAll, EraseAll, acceptor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CopyCursor(Cntr&& cntr,
                                              Cursor(Cntr) * src_cursor,
                                              Cursor(Cntr) * dst_cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(src_cursor != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr);

    CallMethod(CopyCursor, CopyCursor, src_cursor, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr bool seq_cntr::AreEqualCursor(Cntr&& cntr, Cursor(Cntr) * cursor_a,
                                        Cursor(Cntr) * cursor_b) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_a != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_b != nullptr);

    CallMethod(AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr comparison::Ordering seq_cntr::CompareCursor(
    Cntr&& cntr, Cursor(Cntr) * cursor_a, Cursor(Cntr) * cursor_b) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_a != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_b != nullptr);

    CallMethod(CompareCursor, CompareCursor, cursor_a, cursor_b);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr size_t seq_cntr::GetCursorDist(Cntr&& cntr, Cursor(Cntr) * cursor_a,
                                         Cursor(Cntr) * cursor_b) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_a != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor_b != nullptr);

    CallMethod(GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr size_t seq_cntr::GetCursorIdx(Cntr&& cntr, Cursor(Cntr) * cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

    CallMethod(GetCursorIdx, GetCursorIdx, cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CursorStepL(Cntr&& cntr,
                                               Cursor(Cntr) * cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

    CallMethod(CursorStepL, CursorStepL, cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CursorStepR(Cntr&& cntr,
                                               Cursor(Cntr) * cursor) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

    CallMethod(CursorStepR, CursorStepR, cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CursorAdvanceL(Cntr&& cntr,
                                                  Cursor(Cntr) * cursor,
                                                  size_t step) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

    CallMethod(CursorAdvanceL, CursorAdvanceL, cursor, step);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CursorAdvanceR(Cntr&& cntr,
                                                  Cursor(Cntr) * cursor,
                                                  size_t step) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

    CallMethod(CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr::VTable<Elem(Cntr)> seq_cntr::BuildVTableBasic() {
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
#define F(capability, method)                                         \
    []() constexpr {                                                  \
        if constexpr (TestCapability(static_disabled_capability_flag, \
                                     capability)) {                   \
            return nullptr;                                           \
        } else {                                                      \
            return method;                                            \
        }                                                             \
    }()

    constexpr capability::Flag static_disabled_capability_flag{ (
        GetStaticDisabledCapabilityFlag<Cntr>)() };

    VTable<Elem(Cntr)> table;

    for (auto& custom_tag : table.custom_tags) { custom_tag = 0; }

    table.get_elem_cnt = F(  //
        GetElemCnt,
        [](void* cntr) { return (GetElemCnt)(*static_cast<Cntr*>(cntr)); });

    table.get_max_elem_cnt = F(  //
        GetMaxElemCnt,
        [](void* cntr) { return (GetMaxElemCnt)(*static_cast<Cntr*>(cntr)); });

    table.get_lb_cursor = F(  //
        GetLBCursor, [](void* cntr, seq_cntr::CursorLimit* dst_cursor) {
            LoadRealCursor(dst_cursor, true);

            (GetLBCursor)(*static_cast<Cntr*>(cntr), real_dst_cursor);

            StoreRealCursor(dst_cursor, true);
        });

    table.get_rb_cursor = F(  //
        GetRBCursor, [](void* cntr, seq_cntr::CursorLimit* dst_cursor) {
            LoadRealCursor(dst_cursor, true);

            (GetRBCursor)(*static_cast<Cntr*>(cntr), real_dst_cursor);

            StoreRealCursor(dst_cursor, true);
        });

    table.peek_l = F(  //
        PeekL,
        [](void* cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
           seq_cntr::CursorLimit* dst_cursor,
           lifecycle::DataLifeState dst_elem_life_state, void* dst_elem) {
            LoadRealCursor(dst_cursor, true);

            (PeekL)(*static_cast<Cntr*>(cntr), lazy_copy_elem,
                    dst_elem_ptr_view, real_dst_cursor, dst_elem_life_state,
                    static_cast<Elem(Cntr)*>(dst_elem));

            StoreRealCursor(dst_cursor, true);
        });

    table.peek_r = F(  //
        PeekR,
        [](void* cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
           seq_cntr::CursorLimit* dst_cursor,
           lifecycle::DataLifeState dst_elem_life_state, void* dst_elem) {
            LoadRealCursor(dst_cursor, true);

            (PeekR)(*static_cast<Cntr*>(cntr), lazy_copy_elem,
                    dst_elem_ptr_view, real_dst_cursor, dst_elem_life_state,
                    static_cast<Elem(Cntr)*>(dst_elem));

            StoreRealCursor(dst_cursor, true);
        });

    table.refer = F(  //
        Refer,
        [](void* cntr, size_t idx, bool lazy_copy_elem,
           ElemPtrView* dst_elem_ptr_view, seq_cntr::CursorLimit* dst_cursor,
           lifecycle::DataLifeState dst_elem_life_state, void* dst_elem) {
            LoadRealCursor(dst_cursor, true);

            (Refer)(*static_cast<Cntr*>(cntr), idx, lazy_copy_elem,
                    dst_elem_ptr_view, real_dst_cursor, dst_elem_life_state,
                    static_cast<Elem(Cntr)*>(dst_elem));

            StoreRealCursor(dst_cursor, true);
        });

    table.derefer = F(  //
        Derefer,
        [](void* cntr, seq_cntr::CursorLimit* pos_cursor, bool lazy_copy_elem,
           ElemPtrView* dst_elem_ptr_view,
           lifecycle::DataLifeState dst_elem_life_state, void* dst_elem) {
            LoadRealCursor(pos_cursor, false);

            (Derefer)(*static_cast<Cntr*>(cntr), real_pos_cursor,
                      lazy_copy_elem, dst_elem_ptr_view, dst_elem_life_state,
                      static_cast<Elem(Cntr)*>(dst_elem));

            StoreRealCursor(pos_cursor, false);
        });

    table.read.basic =
        F(Read, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                   seq_endpoint::acceptor::BasicAcceptor acceptor,
                   seq_cntr::CursorLimit* dst_cursor) {
            LoadTwoRealCursor_PosDst(pos_cursor, dst_cursor);

            (Read)(*static_cast<Cntr*>(cntr), real_pos_cursor, cnt, acceptor,
                   real_dst_cursor);

            StoreTwoRealCursor_PosDst(pos_cursor, dst_cursor);
        });

    table.read.poly = F(  //
        Read, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                 poly_seq_endpoint::acceptor::Acceptor<Elem(Cntr)> acceptor,
                 seq_cntr::CursorLimit* dst_cursor) {
            LoadTwoRealCursor_PosDst(pos_cursor, dst_cursor);

            (Read)(*static_cast<Cntr*>(cntr), real_pos_cursor, cnt, acceptor,
                   real_dst_cursor);

            StoreTwoRealCursor_PosDst(pos_cursor, dst_cursor);
        });

    table.write.basic = F(  //
        Write, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                  seq_endpoint::provider::BasicProvider provider,
                  seq_cntr::CursorLimit* dst_cursor) {
            LoadTwoRealCursor_PosDst(pos_cursor, dst_cursor);

            (Write)(*static_cast<Cntr*>(cntr), real_pos_cursor, cnt, provider,
                    real_dst_cursor);

            StoreTwoRealCursor_PosDst(pos_cursor, dst_cursor);
        });

    table.write.poly = F(  //
        Write, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                  poly_seq_endpoint::provider::Provider<Elem(Cntr)> provider,
                  seq_cntr::CursorLimit* dst_cursor) {
            LoadTwoRealCursor_PosDst(pos_cursor, dst_cursor);

            (Write)(*static_cast<Cntr*>(cntr), real_pos_cursor, cnt, provider,
                    real_dst_cursor);

            StoreTwoRealCursor_PosDst(pos_cursor, dst_cursor);
        });

    table.read_write.poly = F(  //
        ReadWrite,
        [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
           poly_seq_endpoint::acceptor::Acceptor<Elem(Cntr)> acceptor,
           seq_cntr::CursorLimit* dst_cursor) {
            LoadTwoRealCursor_PosDst(pos_cursor, dst_cursor);

            (ReadWrite)(*static_cast<Cntr*>(cntr), real_pos_cursor, cnt,
                        acceptor, real_dst_cursor);

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

            (PushL)(*static_cast<Cntr*>(cntr), cnt, provider,
                    real_dst_beg_cursor, real_dst_end_cursor);

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

            (PushL)(*static_cast<Cntr*>(cntr), cnt, provider,
                    real_dst_beg_cursor, real_dst_end_cursor);

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

            (PushR)(*static_cast<Cntr*>(cntr), cnt, provider,
                    real_dst_beg_cursor, real_dst_end_cursor);

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

            (PushR)(*static_cast<Cntr*>(cntr), cnt, provider,
                    real_dst_beg_cursor, real_dst_end_cursor);

            StoreRealCursor(dst_end_cursor, true);
            StoreRealCursor(dst_beg_cursor, true);
        });

    table.insert.basic = F(  //
        Insert, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                   seq_endpoint::provider::BasicProvider provider,
                   seq_cntr::CursorLimit* dst_cursor) {
            LoadTwoRealCursor_PosDst(pos_cursor, dst_cursor);

            (Insert)(*static_cast<Cntr*>(cntr), real_pos_cursor, cnt, provider,
                     real_dst_cursor);

            StoreTwoRealCursor_PosDst(pos_cursor, dst_cursor);
        });

    table.insert.poly = F(  //
        Insert, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                   poly_seq_endpoint::provider::Provider<Elem(Cntr)> provider,
                   seq_cntr::CursorLimit* dst_cursor) {
            LoadTwoRealCursor_PosDst(pos_cursor, dst_cursor);

            (Insert)(*static_cast<Cntr*>(cntr), real_pos_cursor, cnt, provider,
                     real_dst_cursor);

            StoreTwoRealCursor_PosDst(pos_cursor, dst_cursor);
        });

    table.pop_l.basic = F(  //
        PopL, [](void* cntr, size_t cnt,
                 seq_endpoint::acceptor::BasicAcceptor acceptor,
                 seq_cntr::CursorLimit* dst_cursor) {
            LoadRealCursor(dst_cursor, true);

            (PopL)(*static_cast<Cntr*>(cntr), cnt, acceptor, real_dst_cursor);

            StoreRealCursor(dst_cursor, true);
        });

    table.pop_l.poly = F(  //
        PopL, [](void* cntr, size_t cnt,
                 poly_seq_endpoint::acceptor::Acceptor<Elem(Cntr)> acceptor,
                 seq_cntr::CursorLimit* dst_cursor) {
            LoadRealCursor(dst_cursor, true);

            (PopL)(*static_cast<Cntr*>(cntr), cnt, acceptor, real_dst_cursor);

            StoreRealCursor(dst_cursor, true);
        });

    table.pop_r.basic = F(  //
        PopR, [](void* cntr, size_t cnt,
                 seq_endpoint::acceptor::BasicAcceptor acceptor,
                 seq_cntr::CursorLimit* dst_cursor) {
            LoadRealCursor(dst_cursor, true);

            (PopR)(*static_cast<Cntr*>(cntr), cnt, acceptor, real_dst_cursor);

            StoreRealCursor(dst_cursor, true);
        });

    table.pop_r.poly = F(  //
        PopR, [](void* cntr, size_t cnt,
                 poly_seq_endpoint::acceptor::Acceptor<Elem(Cntr)> acceptor,
                 seq_cntr::CursorLimit* dst_cursor) {
            LoadRealCursor(dst_cursor, true);

            (PopR)(*static_cast<Cntr*>(cntr), cnt, acceptor, real_dst_cursor);

            StoreRealCursor(dst_cursor, true);
        });

    table.erase.basic = F(  //
        Erase, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                  seq_endpoint::acceptor::BasicAcceptor acceptor) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

            LoadRealCursor(pos_cursor, false);

            (Erase)(*static_cast<Cntr*>(cntr), real_pos_cursor, cnt, acceptor);

            StoreRealCursor(pos_cursor, false);
        });

    table.erase.poly = F(  //
        Erase, [](void* cntr, seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                  poly_seq_endpoint::acceptor::Acceptor<Elem(Cntr)> acceptor) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(pos_cursor != nullptr);

            LoadRealCursor(pos_cursor, false);

            (Erase)(*static_cast<Cntr*>(cntr), real_pos_cursor, cnt, acceptor);

            StoreRealCursor(pos_cursor, false);
        });

    table.erase_all.basic =
        F(EraseAll,
          [](void* cntr, seq_endpoint::acceptor::BasicAcceptor acceptor) {
              (EraseAll)(*static_cast<Cntr*>(cntr), acceptor);
          });

    table.erase_all.poly =
        F(EraseAll,
          [](void* cntr,
             poly_seq_endpoint::acceptor::Acceptor<Elem(Cntr)> acceptor) {
              (EraseAll)(*static_cast<Cntr*>(cntr), acceptor);
          });

    table.copy_cursor = F(  //
        CopyCursor, [](void* cntr, seq_cntr::CursorLimit* src_cursor,
                       seq_cntr::CursorLimit* dst_cursor) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(src_cursor != nullptr);
            ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(src_cursor != dst_cursor);

            Cursor(Cntr) real_src_cursor{ *src_cursor };
            Cursor(Cntr) real_dst_cursor;

            (CopyCursor)(*static_cast<Cntr*>(cntr), &real_src_cursor,
                         &real_dst_cursor);

            *src_cursor = meta::Move(real_src_cursor);
            *dst_cursor = meta::Move(real_dst_cursor);
        });

    table.are_equal_cursor = F(  //
        AreEqualCursor, [](void* cntr, seq_cntr::CursorLimit* cursor_a,
                           seq_cntr::CursorLimit* cursor_b) {
            LoadTwoRealCursor_AB(cursor_a, cursor_b);

            decltype(auto) ret{ (AreEqualCursor)(*static_cast<Cntr*>(cntr),
                                                 real_cursor_a,
                                                 real_cursor_b) };

            StoreTwoRealCursor_AB(cursor_a, cursor_b);

            return ret;
        });

    table.compare_cursor = F(  //
        CompareCursor, [](void* cntr, seq_cntr::CursorLimit* cursor_a,
                          seq_cntr::CursorLimit* cursor_b) {
            LoadTwoRealCursor_AB(cursor_a, cursor_b);

            decltype(auto) ret{ (CompareCursor)(*static_cast<Cntr*>(cntr),
                                                real_cursor_a, real_cursor_b) };

            StoreTwoRealCursor_AB(cursor_a, cursor_b);

            return ret;
        });

    table.get_cursor_dist = F(  //
        GetCursorDist, [](void* cntr, seq_cntr::CursorLimit* cursor_a,
                          seq_cntr::CursorLimit* cursor_b) {
            LoadTwoRealCursor_AB(cursor_a, cursor_b);

            decltype(auto) ret{ (GetCursorDist)(*static_cast<Cntr*>(cntr),
                                                real_cursor_a, real_cursor_b) };

            StoreTwoRealCursor_AB(cursor_a, cursor_b);

            return ret;
        });

    table.get_cursor_idx = F(  //
        GetCursorIdx, [](void* cntr, seq_cntr::CursorLimit* cursor) {
            LoadRealCursor(cursor, false);

            decltype(auto) ret{ (GetCursorIdx)(*static_cast<Cntr*>(cntr),
                                               real_cursor) };

            StoreRealCursor(cursor, false);

            return ret;
        });

    table.cursor_step_l = F(  //
        CursorStepL, [](void* cntr, seq_cntr::CursorLimit* cursor) {
            LoadRealCursor(cursor, false);

            (CursorStepL)(*static_cast<Cntr*>(cntr), real_cursor);

            StoreRealCursor(cursor, false);
        });

    table.cursor_step_r = F(  //
        CursorStepR, [](void* cntr, seq_cntr::CursorLimit* cursor) {
            LoadRealCursor(cursor, false);

            (CursorStepR)(*static_cast<Cntr*>(cntr), real_cursor);

            StoreRealCursor(cursor, false);
        });

    table.cursor_advance_l = F(  //
        CursorAdvanceL,
        [](void* cntr, seq_cntr::CursorLimit* cursor, size_t step) {
            LoadRealCursor(cursor, false);

            (CursorAdvanceL)(*static_cast<Cntr*>(cntr), real_cursor, step);

            StoreRealCursor(cursor, false);
        });

    table.cursor_advance_r = F(  //
        CursorAdvanceR,
        [](void* cntr, seq_cntr::CursorLimit* cursor, size_t step) {
            LoadRealCursor(cursor, false);

            (CursorAdvanceR)(*static_cast<Cntr*>(cntr), real_cursor, step);

            StoreRealCursor(cursor, false);
        });

    for (auto& custom_method : table.custom_methods) {
        custom_method = nullptr;
    }

#pragma pop_macro("StoreTwoRealCursor_AB")
#pragma pop_macro("LoadTwoRealCursor_AB")
#pragma pop_macro("StoreTwoRealCursor_PosDst")
#pragma pop_macro("LoadTwoRealCursor_PosDst")
#pragma pop_macro("StoreRealCursor")
#pragma pop_macro("LoadRealCursor")
#pragma pop_macro("F")

    return table;
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr::VTable<Elem(Cntr)> seq_cntr::BuildVTableImpl<Cntr>::Call() {
    return (BuildVTableBasic<Cntr>)();
}

namespace seq_cntr::detail {

template <IsSeqCntr Cntr>
struct VTableHolder_ {
    static constexpr seq_cntr::VTable<Elem(Cntr)> vtable{
        BuildVTableImpl<Cntr>::Call()
    };
};

}  // namespace seq_cntr::detail

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr::VTable<Elem(Cntr)> const& seq_cntr::GetVTable() {
    return detail::VTableHolder_<Cntr>::vtable;
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
