/**
@file zeta/core/seq_cntr.ipp
*/

#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/lin_seq_elem_stream.ipp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

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

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetReferedInstPtr(
    Cntr& cntr) {
    return cntr.GetReferedInstPtr();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto) seq_cntr::MemberFuncCntrTraitsAdapter<
    Cntr, Cursor>::GetStaticEnabledCapabilityFlag() {
    return Cntr::GetStaticEnabledCapabilityFlag();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto) seq_cntr::MemberFuncCntrTraitsAdapter<
    Cntr, Cursor>::GetStaticDisabledCapabilityFlag() {
    return Cntr::GetStaticDisabledCapabilityFlag();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto) seq_cntr::MemberFuncCntrTraitsAdapter<
    Cntr, Cursor>::GetDynamicEnabledCapabilityFlag(Cntr& cntr) {
    return cntr.GetDynamicEnabledCapabilityFlag();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto) seq_cntr::MemberFuncCntrTraitsAdapter<
    Cntr, Cursor>::GetDynamicDisabledCapabilityFlag(Cntr& cntr) {
    return cntr.GetDynamicDisabledCapabilityFlag();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetCursorSize(Cntr& cntr) {
    return cntr.GetCursorSize();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetElemSize(Cntr& cntr) {
    return cntr.GetElemSize();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetElemCnt(Cntr& cntr) {
    return cntr.GetElemCnt();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetMaxElemCnt(Cntr& cntr) {
    return cntr.GetMaxElemCnt();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetLBCursor(
    Cntr& cntr, void* dst_cursor) {
    return cntr.GetLBCursor(static_cast<Cursor*>(dst_cursor));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetRBCursor(
    Cntr& cntr, void* dst_cursor) {
    return cntr.GetRBCursor(static_cast<Cursor*>(dst_cursor));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::PeekL(
    Cntr& cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
    void* dst_cursor, void* dst_elem) {
    return cntr.PeekL(lazy_copy_elem, dst_elem_ptr_view,
                      static_cast<Cursor*>(dst_cursor), dst_elem);
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::PeekR(
    Cntr& cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
    void* dst_cursor, void* dst_elem) {
    return cntr.PeekR(lazy_copy_elem, dst_elem_ptr_view,
                      static_cast<Cursor*>(dst_cursor), dst_elem);
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::Refer(
    Cntr& cntr, size_t idx, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
    void* dst_cursor, void* dst_elem) {
    return cntr.Refer(idx, lazy_copy_elem, dst_elem_ptr_view,
                      static_cast<Cursor*>(dst_cursor), dst_elem);
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::Derefer(
    Cntr& cntr, void* pos_cursor, bool lazy_copy_elem,
    ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
    return cntr.Derefer(static_cast<Cursor*>(pos_cursor), lazy_copy_elem,
                        dst_elem_ptr_view, dst_elem);
}

template <typename Cntr, typename Cursor>
template <seq_cntr::IsReader Reader>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::Read(Cntr& cntr,
                                                          void* pos_cursor,
                                                          size_t cnt,
                                                          Reader&& reader,
                                                          void* dst_cursor) {
    return cntr.Read(static_cast<Cursor*>(pos_cursor), cnt, reader,
                     static_cast<Cursor*>(dst_cursor));
}

template <typename Cntr, typename Cursor>
template <seq_cntr::IsWriter Writer>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::Write(Cntr& cntr,
                                                           void* pos_cursor,
                                                           size_t cnt,
                                                           Writer&& writer,
                                                           void* dst_cursor) {
    return cntr.Write(static_cast<Cursor*>(pos_cursor), cnt, writer,
                      static_cast<Cursor*>(dst_cursor));
}

template <typename Cntr, typename Cursor>
template <seq_cntr::IsReaderWriter ReaderWriter>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::ReadWrite(
    Cntr& cntr, void* pos_cursor, size_t cnt, ReaderWriter&& reader_writer,
    void* dst_cursor) {
    return cntr.ReadWrite(static_cast<Cursor*>(pos_cursor), cnt, reader_writer,
                          static_cast<Cursor*>(dst_cursor));
}

template <typename Cntr, typename Cursor>
template <seq_cntr::IsWriter Writer>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::PushL(Cntr& cntr,
                                                           size_t cnt,
                                                           Writer&& writer,
                                                           void* dst_cursor) {
    return cntr.PushL(cnt, writer, static_cast<Cursor*>(dst_cursor));
}

template <typename Cntr, typename Cursor>
template <seq_cntr::IsWriter Writer>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::PushR(Cntr& cntr,
                                                           size_t cnt,
                                                           Writer&& writer,
                                                           void* dst_cursor) {
    return cntr.PushR(cnt, writer, static_cast<Cursor*>(dst_cursor));
}

template <typename Cntr, typename Cursor>
template <seq_cntr::IsWriter Writer>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::Insert(Cntr& cntr,
                                                            void* pos_cursor,
                                                            size_t cnt,
                                                            Writer&& writer,
                                                            void* dst_cursor) {
    return cntr.Insert(static_cast<Cursor*>(pos_cursor), cnt, writer,
                       static_cast<Cursor*>(dst_cursor));
}

template <typename Cntr, typename Cursor>
template <seq_cntr::IsReader Reader>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::PopL(Cntr& cntr,
                                                          size_t cnt,
                                                          Reader&& reader) {
    return cntr.PopL(cnt, reader);
}

template <typename Cntr, typename Cursor>
template <seq_cntr::IsReader Reader>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::PopR(Cntr& cntr,
                                                          size_t cnt,
                                                          Reader&& reader) {
    return cntr.PopR(cnt, reader);
}

template <typename Cntr, typename Cursor>
template <seq_cntr::IsReader Reader>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::Erase(Cntr& cntr,
                                                           void* pos_cursor,
                                                           size_t cnt,
                                                           Reader&& reader) {
    return cntr.Erase(static_cast<Cursor*>(pos_cursor), cnt, reader);
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::EraseAll(Cntr& cntr) {
    return cntr.EraseAll();
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::CopyCursor(
    Cntr& cntr, void* src_cursor, void* dst_cursor) {
    return cntr.CopyCursor(static_cast<Cursor*>(src_cursor),
                           static_cast<Cursor*>(dst_cursor));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::AreEqualCursor(
    Cntr& cntr, void* cursor_a, void* cursor_b) {
    return cntr.AreEqualCursor(static_cast<Cursor*>(cursor_a),
                               static_cast<Cursor*>(cursor_b));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::CompareCursor(
    Cntr& cntr, void* cursor_a, void* cursor_b) {
    return cntr.CompareCursor(static_cast<Cursor*>(cursor_a),
                              static_cast<Cursor*>(cursor_b));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetCursorDist(
    Cntr& cntr, void* cursor_a, void* cursor_b) {
    return cntr.GetCursorDist(static_cast<Cursor*>(cursor_a),
                              static_cast<Cursor*>(cursor_b));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::GetCursorIdx(
    Cntr& cntr, void* cursor) {
    return cntr.GetCursorIdx(static_cast<Cursor*>(cursor));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::CursorStepL(Cntr& cntr,
                                                                 void* cursor) {
    return cntr.CursorStepL(static_cast<Cursor*>(cursor));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::CursorStepR(Cntr& cntr,
                                                                 void* cursor) {
    return cntr.CursorStepR(static_cast<Cursor*>(cursor));
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::CursorAdvanceL(
    Cntr& cntr, void* cursor, size_t step) {
    return cntr.CursorAdvanceL(static_cast<Cursor*>(cursor), step);
}

template <typename Cntr, typename Cursor>
constexpr decltype(auto)
seq_cntr::MemberFuncCntrTraitsAdapter<Cntr, Cursor>::CursorAdvanceR(
    Cntr& cntr, void* cursor, size_t step) {
    return cntr.CursorAdvanceR(static_cast<Cursor*>(cursor), step);
}

#pragma push_macro("TestCapability")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestCapability(cap_flag, cap_name)           \
    (((cap_flag) & (static_cast<capability::Flag>(1) \
                    << meta::ToUnderlying(capability::Kind::cap_name))) != 0)

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(cap_name, method_name, ...)                              \
    if constexpr (!TestCapability((GetStaticEnabledCapabilityFlag<Cntr>)(), \
                                  cap_name)) {                              \
        ZETA_Core_StaticAssert(!TestCapability(                             \
            (GetStaticDisabledCapabilityFlag<Cntr>)(), cap_name));          \
                                                                            \
        ZETA_Core_DebugAssert(TestCapability(                               \
            (GetDynamicEnabledCapabilityFlag)(cntr), cap_name));            \
    }                                                                       \
                                                                            \
    return CntrTraits<Cntr>::method_name(__VA_ARGS__);                      \
                                                                            \
    ZETA_Core_StaticAssert(true)

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetReferedInstPtr(Cntr& cntr) {
    return CntrTraits<Cntr>::GetReferedInstPtr(cntr);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetStaticEnabledCapabilityFlag() {
    constexpr capability::Flag static_enabled_capability_flag{
        CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag()
    };

    constexpr capability::Flag static_disabled_capability_flag{
        CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag()
    };

    capability::CheckFlags(static_enabled_capability_flag,
                           static_disabled_capability_flag);

    return static_enabled_capability_flag;
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetStaticDisabledCapabilityFlag() {
    constexpr capability::Flag static_enabled_capability_flag{
        CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag()
    };

    constexpr capability::Flag static_disabled_capability_flag{
        CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag()
    };

    capability::CheckFlags(static_enabled_capability_flag,
                           static_disabled_capability_flag);

    return static_disabled_capability_flag;
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetDynamicEnabledCapabilityFlag(Cntr& cntr) {
    constexpr capability::Flag static_enabled_capability_flag{
        CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag()
    };

    constexpr capability::Flag static_disabled_capability_flag{
        CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag()
    };

    capability::CheckFlags(static_enabled_capability_flag,
                           static_disabled_capability_flag);

    capability::Flag dynamic_enabled_capability_flag{
        CntrTraits<Cntr>::GetDynamicEnabledCapabilityFlag(cntr)
    };

    capability::Flag dynamic_disabled_capability_flag{
        CntrTraits<Cntr>::GetDynamicDisabledCapabilityFlag(cntr)
    };

    capability::CheckFlags(
        static_enabled_capability_flag, static_disabled_capability_flag,
        dynamic_enabled_capability_flag, dynamic_disabled_capability_flag);

    return dynamic_enabled_capability_flag;
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetDynamicDisabledCapabilityFlag(
    Cntr& cntr) {
    constexpr capability::Flag static_enabled_capability_flag{
        CntrTraits<Cntr>::GetStaticEnabledCapabilityFlag()
    };

    constexpr capability::Flag static_disabled_capability_flag{
        CntrTraits<Cntr>::GetStaticDisabledCapabilityFlag()
    };

    capability::CheckFlags(static_enabled_capability_flag,
                           static_disabled_capability_flag);

    capability::Flag dynamic_enabled_capability_flag{
        CntrTraits<Cntr>::GetDynamicEnabledCapabilityFlag(cntr)
    };

    capability::Flag dynamic_disabled_capability_flag{
        CntrTraits<Cntr>::GetDynamicDisabledCapabilityFlag(cntr)
    };

    capability::CheckFlags(
        static_enabled_capability_flag, static_disabled_capability_flag,
        dynamic_enabled_capability_flag, dynamic_disabled_capability_flag);

    return dynamic_enabled_capability_flag;
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetCursorSize(Cntr& cntr) {
    CallMethod(GetCursorSize, GetCursorSize, cntr);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetElemSize(Cntr& cntr) {
    CallMethod(GetElemSize, GetElemSize, cntr);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetElemCnt(Cntr& cntr) {
    CallMethod(GetElemCnt, GetElemCnt, cntr);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetMaxElemCnt(Cntr& cntr) {
    CallMethod(GetMaxElemCnt, GetMaxElemCnt, cntr);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetLBCursor(Cntr& cntr, void* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, cntr, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetRBCursor(Cntr& cntr, void* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, cntr, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::PeekL(Cntr& cntr, bool lazy_copy_elem,
                                         ElemPtrView* dst_elem_ptr_view,
                                         void* dst_cursor, void* dst_elem) {
    CallMethod(PeekL, PeekL, cntr, lazy_copy_elem, dst_elem_ptr_view,
               dst_cursor, dst_elem);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::PeekR(Cntr& cntr, bool lazy_copy_elem,
                                         ElemPtrView* dst_elem_ptr_view,
                                         void* dst_cursor, void* dst_elem) {
    CallMethod(PeekR, PeekR, cntr, lazy_copy_elem, dst_elem_ptr_view,
               dst_cursor, dst_elem);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::Refer(Cntr& cntr, size_t idx,
                                         bool lazy_copy_elem,
                                         ElemPtrView* dst_elem_ptr_view,
                                         void* dst_cursor, void* dst_elem) {
    CallMethod(Refer, Refer, cntr, idx, lazy_copy_elem, dst_elem_ptr_view,
               dst_cursor, dst_elem);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::Derefer(Cntr& cntr, void* pos_cursor,
                                           bool lazy_copy_elem,
                                           ElemPtrView* dst_elem_ptr_view,
                                           void* dst_elem) {
    CallMethod(Derefer, Derefer, cntr, pos_cursor, lazy_copy_elem,
               dst_elem_ptr_view, dst_elem);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsReader Reader>
constexpr decltype(auto) seq_cntr::Read(
    Cntr& cntr, void* pos_cursor, size_t cnt,
    Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(Read, Read, cntr, pos_cursor, cnt, reader, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsWriter Writer>
constexpr decltype(auto) seq_cntr::Write(
    Cntr& cntr, void* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(Write, Write, cntr, pos_cursor, cnt, writer, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsReaderWriter ReaderWriter>
constexpr decltype(auto) seq_cntr::ReadWrite(
    Cntr& cntr, void* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(ReadWrite, ReadWrite, cntr, pos_cursor, cnt, reader_writer,
               dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsWriter Writer>
constexpr decltype(auto) seq_cntr::PushL(
    Cntr& cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(PushL, PushL, cntr, cnt, writer, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsWriter Writer>
constexpr decltype(auto) seq_cntr::PushR(
    Cntr& cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(PushR, PushR, cntr, cnt, writer, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsWriter Writer>
constexpr decltype(auto) seq_cntr::Insert(
    Cntr& cntr, void* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(Insert, Insert, cntr, pos_cursor, cnt, writer, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsReader Reader>
constexpr decltype(auto) seq_cntr::PopL(
    Cntr& cntr, size_t cnt,
    Reader&& reader  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    CallMethod(PopL, PopL, cntr, cnt, reader);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsReader Reader>
constexpr decltype(auto) seq_cntr::PopR(
    Cntr& cntr, size_t cnt,
    Reader&& reader  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    CallMethod(PopR, PopR, cntr, cnt, reader);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsReader Reader>
constexpr decltype(auto) seq_cntr::Erase(
    Cntr& cntr, void* pos_cursor, size_t cnt,
    Reader&& reader  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    CallMethod(Erase, Erase, cntr, pos_cursor, cnt, reader);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::EraseAll(Cntr& cntr) {
    CallMethod(EraseAll, EraseAll, cntr);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CopyCursor(Cntr& cntr, void* src_cursor,
                                              void* dst_cursor) {
    CallMethod(CopyCursor, CopyCursor, cntr, src_cursor, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::AreEqualCursor(Cntr& cntr, void* cursor_a,
                                                  void* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cntr, cursor_a, cursor_b);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CompareCursor(Cntr& cntr, void* cursor_a,
                                                 void* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cntr, cursor_a, cursor_b);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetCursorDist(Cntr& cntr, void* cursor_a,
                                                 void* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cntr, cursor_a, cursor_b);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetCursorIdx(Cntr& cntr, void* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cntr, cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CursorStepL(Cntr& cntr, void* cursor) {
    CallMethod(CursorStepL, CursorStepL, cntr, cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CursorStepR(Cntr& cntr, void* cursor) {
    CallMethod(CursorStepR, CursorStepR, cntr, cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CursorAdvanceL(Cntr& cntr, void* cursor,
                                                  size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cntr, cursor, step);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CursorAdvanceR(Cntr& cntr, void* cursor,
                                                  size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cntr, cursor, step);
}

#pragma pop_macro("CallMethod")

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr::VTable seq_cntr::BuildVTableBasic() {
    constexpr capability::Flag static_disabled_capability_flag{ (
        GetStaticDisabledCapabilityFlag<Cntr>)() };

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

    constexpr VTable table{
        .custom_tags{ 0 },

        .GetElemCnt = F(
            GetElemCnt,
            [](void* cntr) { return (GetElemCnt)(*static_cast<Cntr*>(cntr)); }),

        .GetMaxElemCnt =
            F(GetMaxElemCnt,
              [](void* cntr) {
                  return (GetMaxElemCnt)(*static_cast<Cntr*>(cntr));
              }),

        .GetLBCursor = F(GetLBCursor,
                         [](void* cntr, void* dst_cursor) {
                             return (GetLBCursor)(*static_cast<Cntr*>(cntr),
                                                  dst_cursor);
                         }),

        .GetRBCursor = F(GetRBCursor,
                         [](void* cntr, void* dst_cursor) {
                             return (GetRBCursor)(*static_cast<Cntr*>(cntr),
                                                  dst_cursor);
                         }),

        .PeekL = F(
            PeekL,
            [](void* cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
               void* dst_cursor, void* dst_elem) {
                return (PeekL)(*static_cast<Cntr*>(cntr), lazy_copy_elem,
                               dst_elem_ptr_view, dst_cursor, dst_elem);
            }),

        .PeekR = F(
            PeekR,
            [](void* cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
               void* dst_cursor, void* dst_elem) {
                return (PeekR)(*static_cast<Cntr*>(cntr), lazy_copy_elem,
                               dst_elem_ptr_view, dst_cursor, dst_elem);
            }),

        .Refer = F(Refer,
                   [](void* cntr, size_t idx, bool lazy_copy_elem,
                      ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
                      void* dst_elem) {
                       return (Refer)(*static_cast<Cntr*>(cntr), idx,
                                      lazy_copy_elem, dst_elem_ptr_view,
                                      dst_cursor, dst_elem);
                   }),

        .Derefer = F(Derefer,
                     [](void* cntr, void* pos_cursor, bool lazy_copy_elem,
                        ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
                         return (Derefer)(*static_cast<Cntr*>(cntr), pos_cursor,
                                          lazy_copy_elem, dst_elem_ptr_view,
                                          dst_elem);
                     }),

        .Read_EmptyReader = F(Read,
                              [](void* cntr, void* pos_cursor, size_t cnt,
                                 EmptyReader reader, void* dst_cursor) {
                                  return (Read)(*static_cast<Cntr*>(cntr),
                                                pos_cursor, cnt, reader,
                                                dst_cursor);
                              }),

        .Read_LinSeqReader = F(Read,
                               [](void* cntr, void* pos_cursor, size_t cnt,
                                  LinSeqReader& reader, void* dst_cursor) {
                                   return (Read)(*static_cast<Cntr*>(cntr),
                                                 pos_cursor, cnt, reader,
                                                 dst_cursor);
                               }),

        .Read_FnReader = F(Read,
                           [](void* cntr, void* pos_cursor, size_t cnt,
                              FnReader reader, void* dst_cursor) {
                               return (Read)(*static_cast<Cntr*>(cntr),
                                             pos_cursor, cnt, reader,
                                             dst_cursor);
                           }),

        .Write_EmptyWriter = F(Write,
                               [](void* cntr, void* pos_cursor, size_t cnt,
                                  EmptyWriter writer, void* dst_cursor) {
                                   return (Write)(*static_cast<Cntr*>(cntr),
                                                  pos_cursor, cnt, writer,
                                                  dst_cursor);
                               }),

        .Write_LinSeqWriter = F(Write,
                                [](void* cntr, void* pos_cursor, size_t cnt,
                                   LinSeqWriter& writer, void* dst_cursor) {
                                    return (Write)(*static_cast<Cntr*>(cntr),
                                                   pos_cursor, cnt, writer,
                                                   dst_cursor);
                                }),

        .Write_FnWriter = F(Write,
                            [](void* cntr, void* pos_cursor, size_t cnt,
                               FnWriter writer, void* dst_cursor) {
                                return (Write)(*static_cast<Cntr*>(cntr),
                                               pos_cursor, cnt, writer,
                                               dst_cursor);
                            }),

        .ReadWrite_FnReaderWriter =
            F(ReadWrite,
              [](void* cntr, void* pos_cursor, size_t cnt,
                 FnReaderWriter reader_writer, void* dst_cursor) {
                  return (ReadWrite)(*static_cast<Cntr*>(cntr), pos_cursor, cnt,
                                     reader_writer, dst_cursor);
              }),

        .PushL_EmptyWriter =
            F(PushL,
              [](void* cntr, size_t cnt, EmptyWriter writer, void* dst_cursor) {
                  return (PushL)(*static_cast<Cntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .PushL_LinSeqWriter = F(
            PushL,
            [](void* cntr, size_t cnt, LinSeqWriter& writer, void* dst_cursor) {
                return (PushL)(*static_cast<Cntr*>(cntr), cnt, writer,
                               dst_cursor);
            }),

        .PushL_FnWriter =
            F(PushL,
              [](void* cntr, size_t cnt, FnWriter writer, void* dst_cursor) {
                  return (PushL)(*static_cast<Cntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .PushR_EmptyWriter =
            F(PushR,
              [](void* cntr, size_t cnt, EmptyWriter writer, void* dst_cursor) {
                  return (PushR)(*static_cast<Cntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .PushR_LinSeqWriter = F(
            PushR,
            [](void* cntr, size_t cnt, LinSeqWriter& writer, void* dst_cursor) {
                return (PushR)(*static_cast<Cntr*>(cntr), cnt, writer,
                               dst_cursor);
            }),

        .PushR_FnWriter =
            F(PushR,
              [](void* cntr, size_t cnt, FnWriter writer, void* dst_cursor) {
                  return (PushR)(*static_cast<Cntr*>(cntr), cnt, writer,
                                 dst_cursor);
              }),

        .Insert_EmptyWriter = F(Insert,
                                [](void* cntr, void* pos_cursor, size_t cnt,
                                   EmptyWriter writer, void* dst_cursor) {
                                    return (Insert)(*static_cast<Cntr*>(cntr),
                                                    pos_cursor, cnt, writer,
                                                    dst_cursor);
                                }),

        .Insert_LinSeqWriter = F(Insert,
                                 [](void* cntr, void* pos_cursor, size_t cnt,
                                    LinSeqWriter& writer, void* dst_cursor) {
                                     return (Insert)(*static_cast<Cntr*>(cntr),
                                                     pos_cursor, cnt, writer,
                                                     dst_cursor);
                                 }),

        .Insert_FnWriter = F(Insert,
                             [](void* cntr, void* pos_cursor, size_t cnt,
                                FnWriter writer, void* dst_cursor) {
                                 return (Insert)(*static_cast<Cntr*>(cntr),
                                                 pos_cursor, cnt, writer,
                                                 dst_cursor);
                             }),

        .PopL_EmptyReader = F(PopL,
                              [](void* cntr, size_t cnt, EmptyReader reader) {
                                  return (PopL)(*static_cast<Cntr*>(cntr), cnt,
                                                reader);
                              }),

        .PopL_LinSeqReader =
            F(PopL,
              [](void* cntr, size_t cnt, LinSeqReader& reader) {
                  return (PopL)(*static_cast<Cntr*>(cntr), cnt, reader);
              }),

        .PopL_FnReader = F(PopL,
                           [](void* cntr, size_t cnt, FnReader reader) {
                               return (PopL)(*static_cast<Cntr*>(cntr), cnt,
                                             reader);
                           }),

        .PopR_EmptyReader = F(PopR,
                              [](void* cntr, size_t cnt, EmptyReader reader) {
                                  return (PopR)(*static_cast<Cntr*>(cntr), cnt,
                                                reader);
                              }),

        .PopR_LinSeqReader =
            F(PopR,
              [](void* cntr, size_t cnt, LinSeqReader& reader) {
                  return (PopR)(*static_cast<Cntr*>(cntr), cnt, reader);
              }),

        .PopR_FnReader = F(PopR,
                           [](void* cntr, size_t cnt, FnReader reader) {
                               return (PopR)(*static_cast<Cntr*>(cntr), cnt,
                                             reader);
                           }),
        .Erase_EmptyReader =
            F(Erase,
              [](void* cntr, void* pos_cursor, size_t cnt, EmptyReader reader) {
                  return (Erase)(*static_cast<Cntr*>(cntr), pos_cursor, cnt,
                                 reader);
              }),

        .Erase_LinSeqReader = F(
            Erase,
            [](void* cntr, void* pos_cursor, size_t cnt, LinSeqReader& reader) {
                return (Erase)(*static_cast<Cntr*>(cntr), pos_cursor, cnt,
                               reader);
            }),

        .Erase_FnReader =
            F(Erase,
              [](void* cntr, void* pos_cursor, size_t cnt, FnReader reader) {
                  return (Erase)(*static_cast<Cntr*>(cntr), pos_cursor, cnt,
                                 reader);
              }),

        .EraseAll =
            F(EraseAll,
              [](void* cntr) { return (EraseAll)(*static_cast<Cntr*>(cntr)); }),

        .CopyCursor = F(CopyCursor,
                        [](void* cntr, void* src_cursor, void* dst_cursor) {
                            return (CopyCursor)(*static_cast<Cntr*>(cntr),
                                                src_cursor, dst_cursor);
                        }),

        .AreEqualCursor =
            F(AreEqualCursor,
              [](void* cntr, void* cursor_a, void* cursor_b) {
                  return (AreEqualCursor)(*static_cast<Cntr*>(cntr), cursor_a,
                                          cursor_b);
              }),

        .CompareCursor = F(CompareCursor,
                           [](void* cntr, void* cursor_a, void* cursor_b) {
                               return (CompareCursor)(*static_cast<Cntr*>(cntr),
                                                      cursor_a, cursor_b);
                           }),

        .GetCursorDist = F(GetCursorDist,
                           [](void* cntr, void* cursor_a, void* cursor_b) {
                               return (GetCursorDist)(*static_cast<Cntr*>(cntr),
                                                      cursor_a, cursor_b);
                           }),

        .GetCursorIdx = F(GetCursorIdx,
                          [](void* cntr, void* cursor) {
                              return (GetCursorIdx)(*static_cast<Cntr*>(cntr),
                                                    cursor);
                          }),

        .CursorStepL = F(CursorStepL,
                         [](void* cntr, void* cursor) {
                             return (CursorStepL)(*static_cast<Cntr*>(cntr),
                                                  cursor);
                         }),

        .CursorStepR = F(CursorStepR,
                         [](void* cntr, void* cursor) {
                             return (CursorStepR)(*static_cast<Cntr*>(cntr),
                                                  cursor);
                         }),

        .CursorAdvanceL =
            F(CursorAdvanceL,
              [](void* cntr, void* cursor, size_t step) {
                  return (CursorAdvanceL)(*static_cast<Cntr*>(cntr), cursor,
                                          step);
              }),

        .CursorAdvanceR =
            F(CursorAdvanceR,
              [](void* cntr, void* cursor, size_t step) {
                  return (CursorAdvanceR)(*static_cast<Cntr*>(cntr), cursor,
                                          step);
              }),

        .CustomMethods{ nullptr },
    };

#pragma pop_macro("F")

    return table;
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr::VTable seq_cntr::BuildVTableImpl<Cntr>::Call() {
    return (BuildVTableBasic<Cntr>)();
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr::VTable seq_cntr::BuildVTable() {
    return BuildVTableImpl<Cntr>::Call();
};

namespace seq_cntr::detail {

template <IsSeqCntr Cntr>
struct VTableHolder_ {
    static constexpr seq_cntr::VTable vtable{ (BuildVTable<Cntr>)() };
};

}  // namespace seq_cntr::detail

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr::VTable const& seq_cntr::GetVTable() {
    return detail::VTableHolder_<Cntr>::vtable;
}

constexpr bool seq_cntr::check_operation::CanRefer(size_t idx, size_t cnt,
                                                   size_t elem_cnt) {
    return elem_cnt <= max_max_elem_cnt && idx + 1 < elem_cnt + 2 &&
           cnt <= elem_cnt - idx + 1;
}

constexpr bool seq_cntr::check_operation::CanDerefer(size_t idx, size_t cnt,
                                                     size_t elem_cnt) {
    return elem_cnt <= max_max_elem_cnt && idx <= elem_cnt &&
           cnt <= elem_cnt - idx;
}

constexpr bool seq_cntr::check_operation::CanPushL(size_t cnt, size_t elem_cnt,
                                                   size_t max_elem_cnt) {
    return elem_cnt <= max_elem_cnt && max_elem_cnt <= max_max_elem_cnt &&
           cnt <= max_elem_cnt - elem_cnt;
}

constexpr bool seq_cntr::check_operation::CanPushR(size_t cnt, size_t elem_cnt,
                                                   size_t max_elem_cnt) {
    return elem_cnt <= max_elem_cnt && max_elem_cnt <= max_max_elem_cnt &&
           cnt <= max_elem_cnt - elem_cnt;
}

constexpr bool seq_cntr::check_operation::CanInsert(size_t idx, size_t cnt,
                                                    size_t elem_cnt,
                                                    size_t max_elem_cnt) {
    return elem_cnt <= max_elem_cnt && max_elem_cnt <= max_max_elem_cnt &&
           idx <= elem_cnt && elem_cnt <= max_elem_cnt &&
           cnt <= max_elem_cnt - elem_cnt;
}

constexpr bool seq_cntr::check_operation::CanPopL(size_t cnt, size_t elem_cnt) {
    return cnt <= elem_cnt;
}

constexpr bool seq_cntr::check_operation::CanPopR(size_t cnt, size_t elem_cnt) {
    return cnt <= elem_cnt;
}

constexpr bool seq_cntr::check_operation::CanErase(size_t idx, size_t cnt,
                                                   size_t elem_cnt) {
    return (CanDerefer)(idx, cnt, elem_cnt);
}

constexpr bool seq_cntr::check_operation::CanStepL(size_t idx,
                                                   size_t elem_cnt) {
    return (CanAdvancableL)(idx, 1, elem_cnt);
}

constexpr bool seq_cntr::check_operation::CanStepR(size_t idx,
                                                   size_t elem_cnt) {
    return (CanAdvancableR)(idx, 1, elem_cnt);
}

constexpr bool seq_cntr::check_operation::CanAdvancableL(size_t idx,
                                                         size_t step,
                                                         size_t elem_cnt) {
    return elem_cnt <= max_max_elem_cnt && idx + 1 < elem_cnt + 2 &&
           step <= idx + 1;
}

constexpr bool seq_cntr::check_operation::CanAdvancableR(size_t idx,
                                                         size_t step,
                                                         size_t elem_cnt) {
    return elem_cnt <= max_max_elem_cnt && idx + 1 < elem_cnt + 2 &&
           step <= elem_cnt - idx;
}

template <seq_cntr::IsSeqCntr DstCntr, seq_cntr::IsSeqCntr SrcCntr>
void seq_cntr::RangeAssign(DstCntr& dst_cntr, SrcCntr& src_cntr, size_t dst_beg,
                           size_t src_beg, size_t cnt) {
    ZETA_Core_DebugAssert((GetElemSize)(dst_cntr) == (GetElemSize)(src_cntr));

    size_t elem_size{ (GetElemSize)(dst_cntr) };

    size_t buffer_capacity{ comparison_utils::BasicMax(
        1U, sizeof(void*) * 1024 / elem_size) };

    size_t dst_elem_cnt{ (GetElemCnt)(dst_cntr) };
    size_t src_elem_cnt{ (GetElemCnt)(src_cntr) };

    ZETA_Core_DebugAssert(
        check_operation::CanDerefer(dst_beg, cnt, dst_elem_cnt));
    ZETA_Core_DebugAssert(
        check_operation::CanDerefer(src_beg, cnt, src_elem_cnt));

    void const* real_dst_cntr{ (GetReferedInstPtr)(dst_cntr) };
    void const* real_src_cntr{ (GetReferedInstPtr)(src_cntr) };

    if (cnt == 0 || real_dst_cntr == real_src_cntr) { return; }

    size_t dst_end{ dst_beg + cnt };
    size_t src_end{ src_beg + cnt };

    void* buffer{ __builtin_alloca_with_align(
        elem_size * buffer_capacity, __CHAR_BIT__ * alignof(max_align_t)) };

    CursorLimit dst_cursor;
    CursorLimit src_cursor;

    if (real_dst_cntr != real_src_cntr || dst_beg <= src_beg ||
        src_end <= dst_beg) {
        (Refer)(dst_cntr, dst_beg, true, nullptr, &dst_cursor, nullptr);
        (Refer)(src_cntr, src_beg, true, nullptr, &src_cursor, nullptr);

        while (0 < cnt) {
            size_t cur_cnt{ comparison_utils::BasicMin(buffer_capacity, cnt) };

            (Read)(src_cntr, &src_cursor, cur_cnt,
                   LinSeqReader{
                       .data = buffer,
                       .elem_size = elem_size,
                       .elem_stride = elem_size,
                       .elem_cnt = cur_cnt,
                   },
                   &src_cursor);
            (Write)(dst_cntr, &dst_cursor, cur_cnt,
                    LinSeqWriter{
                        .data = buffer,
                        .elem_size = elem_size,
                        .elem_stride = elem_size,
                        .elem_cnt = cur_cnt,
                    },
                    &dst_cursor);

            cnt -= cur_cnt;
        }

        return;
    }

    (Refer)(dst_cntr, dst_end, true, nullptr, &dst_cursor, nullptr);
    (Refer)(src_cntr, src_end, true, nullptr, &src_cursor, nullptr);

    while (0 < cnt) {
        size_t cur_cnt{ comparison_utils::BasicMin(buffer_capacity, cnt) };

        (CursorAdvanceL)(src_cntr, &src_cursor, cur_cnt);
        (CursorAdvanceL)(dst_cntr, &dst_cursor, cur_cnt);

        (Read)(src_cntr, &src_cursor, cur_cnt,
               LinSeqReader{
                   .data = buffer,
                   .elem_size = elem_size,
                   .elem_stride = elem_size,
                   .elem_cnt = cur_cnt,
               },
               nullptr);

        (Write)(dst_cntr, &dst_cursor, cur_cnt,
                LinSeqWriter{
                    .data = buffer,
                    .elem_size = elem_size,
                    .elem_stride = elem_size,
                    .elem_cnt = cur_cnt,
                },
                nullptr);

        cnt -= cur_cnt;
    }
}

template <seq_cntr::IsSeqCntr DstCntr, seq_cntr::IsSeqCntr SrcCntr>
void seq_cntr::Assign(DstCntr& dst_cntr, SrcCntr& src_cntr) {
    size_t dst_size{ (GetElemCnt)(dst_cntr) };
    size_t src_size{ (GetElemCnt)(src_cntr) };

    constexpr capability::Flag static_enabled_capability_flag{ (
        GetStaticEnabledCapabilityFlag<DstCntr>)() };

    constexpr capability::Flag static_disabled_capability_flag{ (
        GetStaticDisabledCapabilityFlag<DstCntr>)() };

    capability::Flag dynamic_enabled_capability_flag{ (
        GetDynamicEnabledCapabilityFlag)(dst_cntr) };

    constexpr bool static_enabled_push_l{ TestCapability(
        static_enabled_capability_flag, PushL) };

    constexpr bool static_disabled_push_l{ TestCapability(
        static_disabled_capability_flag, PushL) };

    bool dynamic_enabled_push_l{ TestCapability(dynamic_enabled_capability_flag,
                                                PushL) };

    constexpr bool static_enabled_push_r{ TestCapability(
        static_enabled_capability_flag, PushR) };

    constexpr bool static_disabled_push_r{ TestCapability(
        static_disabled_capability_flag, PushR) };

    bool dynamic_enabled_push_r{ TestCapability(dynamic_enabled_capability_flag,
                                                PushR) };

    constexpr bool static_enabled_pop_l{ TestCapability(
        static_enabled_capability_flag, PopL) };

    constexpr bool static_disabled_pop_l{ TestCapability(
        static_disabled_capability_flag, PopL) };

    bool dynamic_enabled_pop_l{ TestCapability(dynamic_enabled_capability_flag,
                                               PopL) };

    constexpr bool static_enabled_pop_r{ TestCapability(
        static_enabled_capability_flag, PopR) };

    constexpr bool static_disabled_pop_r{ TestCapability(
        static_disabled_capability_flag, PopR) };

    bool dynamic_enabled_pop_r{ TestCapability(dynamic_enabled_capability_flag,
                                               PopR) };
    /*

    se sd de dd
    se R  L  L  L
    sd R  X  R  X
    de R  L  R  L
    dd R  X  R  X

    */

#pragma push_macro("FPushL")
#define FPushL (PushL)(dst_cntr, src_size - dst_size, EmptyWriter{}, nullptr);

#pragma push_macro("FPushR")
#define FPushR (PushR)(dst_cntr, src_size - dst_size, EmptyWriter{}, nullptr);

#pragma push_macro("FPopL")
#define FPopL (PopL)(dst_cntr, dst_size - src_size, EmptyReader{});

#pragma push_macro("FPopR")
#define FPopR (PopR)(dst_cntr, dst_size - src_size, EmptyReader{});

    // NOLINTBEGIN(bugprone-branch-clone)
    if (dst_size < src_size) {
        if constexpr (static_enabled_push_r) {
            FPushR;
        } else if constexpr (static_enabled_push_l) {
            FPushL;
        } else if constexpr (static_disabled_push_r) {
            ZETA_Core_DebugAssert(dynamic_enabled_push_l);
            FPushL;
        } else if constexpr (static_disabled_push_l) {
            ZETA_Core_DebugAssert(dynamic_enabled_push_r);
            FPushR;
        } else if (dynamic_enabled_push_r) {
            FPushR;
        } else {
            ZETA_Core_DebugAssert(dynamic_enabled_push_l);
            FPushL;
        }
    } else if (src_size < dst_size) {
        if constexpr (static_enabled_pop_r) {
            FPopR;
        } else if constexpr (static_enabled_pop_l) {
            FPopL;
        } else if constexpr (static_disabled_pop_r) {
            ZETA_Core_DebugAssert(dynamic_enabled_pop_l);
            FPopL;
        } else if constexpr (static_disabled_pop_l) {
            ZETA_Core_DebugAssert(dynamic_enabled_pop_r);
            FPopR;
        } else if (dynamic_enabled_pop_r) {
            FPopR;
        } else {
            ZETA_Core_DebugAssert(dynamic_enabled_pop_l);
            FPopL;
        }
    }
    // NOLINTEND(bugprone-branch-clone)

#pragma pop_macro("FPopR")
#pragma pop_macro("FPopL")
#pragma pop_macro("FPushR")
#pragma pop_macro("FPushL")

    (RangeAssign)(dst_cntr, src_cntr, 0, 0, src_size);
}

#pragma pop_macro("TestCapability")

}  // namespace zeta::core
