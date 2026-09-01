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

#pragma push_macro("TestCapability")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestCapability(cap_flag, cap_name)           \
    (((cap_flag) & (static_cast<capability::Flag>(1) \
                    << meta::ToUnderlying(capability::Kind::cap_name))) != 0)

#pragma push_macro("Cursor")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define Cursor meta::GetTypeWrapperType<decltype((GetCursorType<Cntr>)())>

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(cap_name, method_name, ...)                              \
    (CheckCapabilityFlags)(cntr);                                           \
                                                                            \
    if constexpr (!TestCapability((GetStaticEnabledCapabilityFlag<Cntr>)(), \
                                  cap_name)) {                              \
        ZETA_Core_StaticAssert(!TestCapability(                             \
            (GetStaticDisabledCapabilityFlag<Cntr>)(), cap_name));          \
                                                                            \
        ZETA_Core_DebugAssert(TestCapability(                               \
            (GetDynamicEnabledCapabilityFlag)(cntr), cap_name));            \
    }                                                                       \
                                                                            \
    return cntr.method_name(Tag{}, __VA_ARGS__);                            \
                                                                            \
    ZETA_Core_StaticAssert(true)

template <seq_cntr::IsSeqCntr Cntr>
constexpr void seq_cntr::CheckCapabilityFlags(Cntr& cntr) {
    constexpr capability::Flag static_enabled_capability_flag{
        Cntr::GetStaticEnabledCapabilityFlag(Tag{}, meta::TypeWrapper<Cntr>{})
    };

    constexpr capability::Flag static_disabled_capability_flag{
        Cntr::GetStaticDisabledCapabilityFlag(Tag{}, meta::TypeWrapper<Cntr>{})
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
constexpr decltype(auto) seq_cntr::GetReferedInstPtr(Cntr& cntr) {
    return cntr.GetReferedInstPtr(Tag{});
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetStaticEnabledCapabilityFlag() {
    return Cntr::GetStaticEnabledCapabilityFlag(Tag{},
                                                meta::TypeWrapper<Cntr>{});
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetStaticDisabledCapabilityFlag() {
    return Cntr::GetStaticDisabledCapabilityFlag(Tag{},
                                                 meta::TypeWrapper<Cntr>{});
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetDynamicEnabledCapabilityFlag(Cntr& cntr) {
    return cntr.GetDynamicEnabledCapabilityFlag(Tag{});
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetDynamicDisabledCapabilityFlag(
    Cntr& cntr) {
    return cntr.GetDynamicDisabledCapabilityFlag(Tag{});
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetCursorType() {
    return Cntr::GetCursorType(Tag{}, meta::TypeWrapper<Cntr>{});
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetCursorSize(Cntr& cntr) {
    CallMethod(GetCursorSize, GetCursorSize);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetElemSize(Cntr& cntr) {
    CallMethod(GetElemSize, GetElemSize);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetElemCnt(Cntr& cntr) {
    CallMethod(GetElemCnt, GetElemCnt);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetMaxElemCnt(Cntr& cntr) {
    CallMethod(GetMaxElemCnt, GetMaxElemCnt);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetLBCursor(Cntr& cntr, Cursor* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetRBCursor(Cntr& cntr, Cursor* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::PeekL(Cntr& cntr, bool lazy_copy_elem,
                                         ElemPtrView* dst_elem_ptr_view,
                                         Cursor* dst_cursor, void* dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::PeekR(Cntr& cntr, bool lazy_copy_elem,
                                         ElemPtrView* dst_elem_ptr_view,
                                         Cursor* dst_cursor, void* dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::Refer(Cntr& cntr, size_t idx,
                                         bool lazy_copy_elem,
                                         ElemPtrView* dst_elem_ptr_view,
                                         Cursor* dst_cursor, void* dst_elem) {
    CallMethod(Refer, Refer, idx, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::Derefer(Cntr& cntr, Cursor* pos_cursor,
                                           bool lazy_copy_elem,
                                           ElemPtrView* dst_elem_ptr_view,
                                           void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem_ptr_view,
               dst_elem);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsReader Reader>
constexpr decltype(auto) seq_cntr::Read(
    Cntr& cntr, Cursor* pos_cursor, size_t cnt,
    Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    CallMethod(Read, Read, pos_cursor, cnt, reader, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsWriter Writer>
constexpr decltype(auto) seq_cntr::Write(
    Cntr& cntr, Cursor* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    CallMethod(Write, Write, pos_cursor, cnt, writer, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsReaderWriter ReaderWriter>
constexpr decltype(auto) seq_cntr::ReadWrite(
    Cntr& cntr, Cursor* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    CallMethod(ReadWrite, ReadWrite, pos_cursor, cnt, reader_writer,
               dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsWriter Writer>
constexpr decltype(auto) seq_cntr::PushL(
    Cntr& cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    CallMethod(PushL, PushL, cnt, writer, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsWriter Writer>
constexpr decltype(auto) seq_cntr::PushR(
    Cntr& cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    CallMethod(PushR, PushR, cnt, writer, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsWriter Writer>
constexpr decltype(auto) seq_cntr::Insert(
    Cntr& cntr, Cursor* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    CallMethod(Insert, Insert, pos_cursor, cnt, writer, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsReader Reader>
constexpr decltype(auto) seq_cntr::PopL(
    Cntr& cntr, size_t cnt,
    Reader&& reader  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    CallMethod(PopL, PopL, cnt, reader);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsReader Reader>
constexpr decltype(auto) seq_cntr::PopR(
    Cntr& cntr, size_t cnt,
    Reader&& reader  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    CallMethod(PopR, PopR, cnt, reader);
}

template <seq_cntr::IsSeqCntr Cntr, seq_cntr::IsReader Reader>
constexpr decltype(auto) seq_cntr::Erase(
    Cntr& cntr, Cursor* pos_cursor, size_t cnt,
    Reader&& reader  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    CallMethod(Erase, Erase, pos_cursor, cnt, reader);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::EraseAll(Cntr& cntr) {
    CallMethod(EraseAll, EraseAll);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CopyCursor(Cntr& cntr, Cursor* src_cursor,
                                              Cursor* dst_cursor) {
    CallMethod(CopyCursor, CopyCursor, src_cursor, dst_cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::AreEqualCursor(Cntr& cntr, Cursor* cursor_a,
                                                  Cursor* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CompareCursor(Cntr& cntr, Cursor* cursor_a,
                                                 Cursor* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cursor_a, cursor_b);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetCursorDist(Cntr& cntr, Cursor* cursor_a,
                                                 Cursor* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::GetCursorIdx(Cntr& cntr, Cursor* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CursorStepL(Cntr& cntr, Cursor* cursor) {
    CallMethod(CursorStepL, CursorStepL, cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CursorStepR(Cntr& cntr, Cursor* cursor) {
    CallMethod(CursorStepR, CursorStepR, cursor);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CursorAdvanceL(Cntr& cntr, Cursor* cursor,
                                                  size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cursor, step);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr decltype(auto) seq_cntr::CursorAdvanceR(Cntr& cntr, Cursor* cursor,
                                                  size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")
#pragma pop_macro("Cursor")

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr::VTable seq_cntr::BuildVTableBasic() {
    using Cursor = meta::GetTypeWrapperType<decltype(GetCursorType<Cntr>())>;

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

        .get_elem_cnt = F(  //
            GetElemCnt,
            [](void* cntr) { return (GetElemCnt)(*static_cast<Cntr*>(cntr)); }),

        .get_max_elem_cnt = F(  //
            GetMaxElemCnt,
            [](void* cntr) {
                return (GetMaxElemCnt)(*static_cast<Cntr*>(cntr));
            }),

        .get_lb_cursor = F(  //
            GetLBCursor,
            [](void* cntr, void* dst_cursor) {
                return (GetLBCursor)(*static_cast<Cntr*>(cntr),
                                     static_cast<Cursor*>(dst_cursor));
            }),

        .get_rb_cursor = F(  //
            GetRBCursor,
            [](void* cntr, void* dst_cursor) {
                return (GetRBCursor)(*static_cast<Cntr*>(cntr),
                                     static_cast<Cursor*>(dst_cursor));
            }),

        .peek_l = F(  //
            PeekL,
            [](void* cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
               void* dst_cursor, void* dst_elem) {
                return (PeekL)(*static_cast<Cntr*>(cntr), lazy_copy_elem,
                               dst_elem_ptr_view,
                               static_cast<Cursor*>(dst_cursor), dst_elem);
            }),

        .peek_r = F(  //
            PeekR,
            [](void* cntr, bool lazy_copy_elem, ElemPtrView* dst_elem_ptr_view,
               void* dst_cursor, void* dst_elem) {
                return (PeekR)(*static_cast<Cntr*>(cntr), lazy_copy_elem,
                               dst_elem_ptr_view,
                               static_cast<Cursor*>(dst_cursor), dst_elem);
            }),

        .refer = F(  //
            Refer,
            [](void* cntr, size_t idx, bool lazy_copy_elem,
               ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
               void* dst_elem) {
                return (Refer)(*static_cast<Cntr*>(cntr), idx, lazy_copy_elem,
                               dst_elem_ptr_view,
                               static_cast<Cursor*>(dst_cursor), dst_elem);
            }),

        .derefer = F(  //
            Derefer,
            [](void* cntr, void* pos_cursor, bool lazy_copy_elem,
               ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
                return (Derefer)(*static_cast<Cntr*>(cntr),
                                 static_cast<Cursor*>(pos_cursor),
                                 lazy_copy_elem, dst_elem_ptr_view, dst_elem);
            }),

        .read{
            .empty = F(  //
                Read,
                [](void* cntr, void* pos_cursor, size_t cnt, EmptyReader reader,
                   void* dst_cursor) {
                    return (Read)(*static_cast<Cntr*>(cntr),
                                  static_cast<Cursor*>(pos_cursor), cnt, reader,
                                  static_cast<Cursor*>(dst_cursor));
                }),
            .lin_seq = F(  //
                Read,
                [](void* cntr, void* pos_cursor, size_t cnt,
                   LinSeqReader& reader, void* dst_cursor) {
                    return (Read)(*static_cast<Cntr*>(cntr),
                                  static_cast<Cursor*>(pos_cursor), cnt, reader,
                                  static_cast<Cursor*>(dst_cursor));
                }),
            .poly = F(  //
                Read,
                [](void* cntr, void* pos_cursor, size_t cnt, PolyReader reader,
                   void* dst_cursor) {
                    return (Read)(*static_cast<Cntr*>(cntr),
                                  static_cast<Cursor*>(pos_cursor), cnt, reader,
                                  static_cast<Cursor*>(dst_cursor));
                }),
        },

        .write{
            .empty = F(  //
                Write,
                [](void* cntr, void* pos_cursor, size_t cnt, EmptyWriter writer,
                   void* dst_cursor) {
                    return (Write)(*static_cast<Cntr*>(cntr),
                                   static_cast<Cursor*>(pos_cursor), cnt,
                                   writer, static_cast<Cursor*>(dst_cursor));
                }),
            .lin_seq = F(  //
                Write,
                [](void* cntr, void* pos_cursor, size_t cnt,
                   LinSeqWriter& writer, void* dst_cursor) {
                    return (Write)(*static_cast<Cntr*>(cntr),
                                   static_cast<Cursor*>(pos_cursor), cnt,
                                   writer, static_cast<Cursor*>(dst_cursor));
                }),
            .poly = F(  //
                Write,
                [](void* cntr, void* pos_cursor, size_t cnt, PolyWriter writer,
                   void* dst_cursor) {
                    return (Write)(*static_cast<Cntr*>(cntr),
                                   static_cast<Cursor*>(pos_cursor), cnt,
                                   writer, static_cast<Cursor*>(dst_cursor));
                }),
        },

        .read_write{
            .poly = F(  //
                ReadWrite,
                [](void* cntr, void* pos_cursor, size_t cnt,
                   PolyReaderWriter reader_writer, void* dst_cursor) {
                    return (ReadWrite)(*static_cast<Cntr*>(cntr),
                                       static_cast<Cursor*>(pos_cursor), cnt,
                                       reader_writer,
                                       static_cast<Cursor*>(dst_cursor));
                }),
        },

        .push_l{
            .empty = F(  //
                PushL,
                [](void* cntr, size_t cnt, EmptyWriter writer,
                   void* dst_cursor) {
                    return (PushL)(*static_cast<Cntr*>(cntr), cnt, writer,
                                   static_cast<Cursor*>(dst_cursor));
                }),
            .lin_seq = F(  //
                PushL,
                [](void* cntr, size_t cnt, LinSeqWriter& writer,
                   void* dst_cursor) {
                    return (PushL)(*static_cast<Cntr*>(cntr), cnt, writer,
                                   static_cast<Cursor*>(dst_cursor));
                }),
            .poly = F(  //
                PushL,
                [](void* cntr, size_t cnt, PolyWriter writer,
                   void* dst_cursor) {
                    return (PushL)(*static_cast<Cntr*>(cntr), cnt, writer,
                                   static_cast<Cursor*>(dst_cursor));
                }),
        },

        .push_r{
            .empty = F(  //
                PushR,
                [](void* cntr, size_t cnt, EmptyWriter writer,
                   void* dst_cursor) {
                    return (PushR)(*static_cast<Cntr*>(cntr), cnt, writer,
                                   static_cast<Cursor*>(dst_cursor));
                }),
            .lin_seq = F(  //
                PushR,
                [](void* cntr, size_t cnt, LinSeqWriter& writer,
                   void* dst_cursor) {
                    return (PushR)(*static_cast<Cntr*>(cntr), cnt, writer,
                                   static_cast<Cursor*>(dst_cursor));
                }),
            .poly = F(  //
                PushR,
                [](void* cntr, size_t cnt, PolyWriter writer,
                   void* dst_cursor) {
                    return (PushR)(*static_cast<Cntr*>(cntr), cnt, writer,
                                   static_cast<Cursor*>(dst_cursor));
                }),
        },

        .insert{
            .empty = F(  //
                Insert,
                [](void* cntr, void* pos_cursor, size_t cnt, EmptyWriter writer,
                   void* dst_cursor) {
                    return (Insert)(*static_cast<Cntr*>(cntr),
                                    static_cast<Cursor*>(pos_cursor), cnt,
                                    writer, static_cast<Cursor*>(dst_cursor));
                }),
            .lin_seq = F(  //
                Insert,
                [](void* cntr, void* pos_cursor, size_t cnt,
                   LinSeqWriter& writer, void* dst_cursor) {
                    return (Insert)(*static_cast<Cntr*>(cntr),
                                    static_cast<Cursor*>(pos_cursor), cnt,
                                    writer, static_cast<Cursor*>(dst_cursor));
                }),
            .poly = F(  //
                Insert,
                [](void* cntr, void* pos_cursor, size_t cnt, PolyWriter writer,
                   void* dst_cursor) {
                    return (Insert)(*static_cast<Cntr*>(cntr),
                                    static_cast<Cursor*>(pos_cursor), cnt,
                                    writer, static_cast<Cursor*>(dst_cursor));
                }),
        },

        .pop_l{
            .empty = F(  //
                PopL,
                [](void* cntr, size_t cnt, EmptyReader reader) {
                    return (PopL)(*static_cast<Cntr*>(cntr), cnt, reader);
                }),
            .lin_seq = F(  //
                PopL,
                [](void* cntr, size_t cnt, LinSeqReader& reader) {
                    return (PopL)(*static_cast<Cntr*>(cntr), cnt, reader);
                }),
            .poly = F(  //
                PopL,
                [](void* cntr, size_t cnt, PolyReader reader) {
                    return (PopL)(*static_cast<Cntr*>(cntr), cnt, reader);
                }),
        },

        .pop_r{
            .empty = F(  //
                PopR,
                [](void* cntr, size_t cnt, EmptyReader reader) {
                    return (PopR)(*static_cast<Cntr*>(cntr), cnt, reader);
                }),
            .lin_seq = F(  //
                PopR,
                [](void* cntr, size_t cnt, LinSeqReader& reader) {
                    return (PopR)(*static_cast<Cntr*>(cntr), cnt, reader);
                }),
            .poly = F(  //
                PopR,
                [](void* cntr, size_t cnt, PolyReader reader) {
                    return (PopR)(*static_cast<Cntr*>(cntr), cnt, reader);
                }),
        },

        .erase{
            .empty = F(  //
                Erase,
                [](void* cntr, void* pos_cursor, size_t cnt,
                   EmptyReader reader) {
                    return (Erase)(*static_cast<Cntr*>(cntr),
                                   static_cast<Cursor*>(pos_cursor), cnt,
                                   reader);
                }),
            .lin_seq = F(  //
                Erase,
                [](void* cntr, void* pos_cursor, size_t cnt,
                   LinSeqReader& reader) {
                    return (Erase)(*static_cast<Cntr*>(cntr),
                                   static_cast<Cursor*>(pos_cursor), cnt,
                                   reader);
                }),
            .poly = F(  //
                Erase,
                [](void* cntr, void* pos_cursor, size_t cnt,
                   PolyReader reader) {
                    return (Erase)(*static_cast<Cntr*>(cntr),
                                   static_cast<Cursor*>(pos_cursor), cnt,
                                   reader);
                }),
        },

        .erase_all = F(  //
            EraseAll,
            [](void* cntr) { return (EraseAll)(*static_cast<Cntr*>(cntr)); }),

        .copy_cursor = F(  //
            CopyCursor,
            [](void* cntr, void* src_cursor, void* dst_cursor) {
                return (CopyCursor)(*static_cast<Cntr*>(cntr),
                                    static_cast<Cursor*>(src_cursor),
                                    static_cast<Cursor*>(dst_cursor));
            }),

        .are_equal_cursor = F(  //
            AreEqualCursor,
            [](void* cntr, void* cursor_a, void* cursor_b) {
                return (AreEqualCursor)(*static_cast<Cntr*>(cntr),
                                        static_cast<Cursor*>(cursor_a),
                                        static_cast<Cursor*>(cursor_b));
            }),

        .compare_cursor = F(  //
            CompareCursor,
            [](void* cntr, void* cursor_a, void* cursor_b) {
                return (CompareCursor)(*static_cast<Cntr*>(cntr),
                                       static_cast<Cursor*>(cursor_a),
                                       static_cast<Cursor*>(cursor_b));
            }),

        .get_cursor_dist = F(  //
            GetCursorDist,
            [](void* cntr, void* cursor_a, void* cursor_b) {
                return (GetCursorDist)(*static_cast<Cntr*>(cntr),
                                       static_cast<Cursor*>(cursor_a),
                                       static_cast<Cursor*>(cursor_b));
            }),

        .get_cursor_idx = F(  //
            GetCursorIdx,
            [](void* cntr, void* cursor) {
                return (GetCursorIdx)(*static_cast<Cntr*>(cntr),
                                      static_cast<Cursor*>(cursor));
            }),

        .cursor_step_l = F(  //
            CursorStepL,
            [](void* cntr, void* cursor) {
                return (CursorStepL)(*static_cast<Cntr*>(cntr),
                                     static_cast<Cursor*>(cursor));
            }),

        .cursor_step_r = F(  //
            CursorStepR,
            [](void* cntr, void* cursor) {
                return (CursorStepR)(*static_cast<Cntr*>(cntr),
                                     static_cast<Cursor*>(cursor));
            }),

        .cursor_advance_l = F(  //
            CursorAdvanceL,
            [](void* cntr, void* cursor, size_t step) {
                return (CursorAdvanceL)(*static_cast<Cntr*>(cntr),
                                        static_cast<Cursor*>(cursor), step);
            }),

        .cursor_advance_r = F(  //
            CursorAdvanceR,
            [](void* cntr, void* cursor, size_t step) {
                return (CursorAdvanceR)(*static_cast<Cntr*>(cntr),
                                        static_cast<Cursor*>(cursor), step);
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

namespace seq_cntr::detail {

template <IsSeqCntr Cntr>
struct VTableHolder_ {
    static constexpr seq_cntr::VTable vtable{ BuildVTableImpl<Cntr>::Call() };
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
    return (CanAdvanceL)(idx, 1, elem_cnt);
}

constexpr bool seq_cntr::check_operation::CanStepR(size_t idx,
                                                   size_t elem_cnt) {
    return (CanAdvanceR)(idx, 1, elem_cnt);
}

constexpr bool seq_cntr::check_operation::CanAdvanceL(size_t idx, size_t step,
                                                      size_t elem_cnt) {
    return elem_cnt <= max_max_elem_cnt && idx + 1 < elem_cnt + 2 &&
           step <= idx + 1;
}

constexpr bool seq_cntr::check_operation::CanAdvanceR(size_t idx, size_t step,
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
                       .elem_stride = static_cast<ptrdiff_t>(elem_size),
                       .elem_cnt = cur_cnt,
                   },
                   &src_cursor);
            (Write)(dst_cntr, &dst_cursor, cur_cnt,
                    LinSeqWriter{
                        .data = buffer,
                        .elem_size = elem_size,
                        .elem_stride = static_cast<ptrdiff_t>(elem_size),
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
                   .elem_stride = static_cast<ptrdiff_t>(elem_size),
                   .elem_cnt = cur_cnt,
               },
               nullptr);

        (Write)(dst_cntr, &dst_cursor, cur_cnt,
                LinSeqWriter{
                    .data = buffer,
                    .elem_size = elem_size,
                    .elem_stride = static_cast<ptrdiff_t>(elem_size),
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
