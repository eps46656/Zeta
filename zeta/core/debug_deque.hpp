#pragma once

#include <deque>
#include <zeta/core/debug_utils/revision.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/seq_cntr.hpp>

namespace zeta::core::debug_deque {

struct Cursor {
    size_t idx;

#if ZETA_Core_DebugEnable
    mutable debug_utils::revision::Stamp stamp;
#endif

    constexpr Cursor() = default;

    constexpr Cursor(seq_cntr::CursorLimit const& src_cursor);

    constexpr operator seq_cntr::CursorLimit(this Cursor const& self);

    constexpr Cursor& operator=(Cursor const& src_cursor) = default;

    constexpr Cursor& operator=(seq_cntr::CursorLimit const& src_cursor);
};

template <meta::IsNonConstObj Elem_>
struct Cntr {
    using Elem = Elem_;

    std::deque<Elem*>* deque;

#if ZETA_Core_DebugEnable
    debug_utils::revision::Stamp stamp;
#endif

    constexpr Cntr();

    constexpr ~Cntr();

    template <meta::IsAnySame<Cntr&, Cntr&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <meta::IsAnySame<Cntr const&, Cntr const&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <meta::IsAnySame<Cntr&, Cntr&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <meta::IsAnySame<Cntr const&, Cntr const&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        seq_cntr::Tag);

    static constexpr seq_cntr::capability::Flag
        GetDynamicDisabledCapabilityFlag(seq_cntr::Tag);

    constexpr void* GetReferedInstPtr(this Cntr const& self, seq_cntr::Tag);

    template <
        meta::IsAnySame<Cntr&, Cntr const&, Cntr&&, Cntr const&&> TargetCntr>
    static constexpr meta::TypeWrapper<Elem> GetElemType(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <
        meta::IsAnySame<Cntr&, Cntr const&, Cntr&&, Cntr const&&> TargetCntr>
    static constexpr meta::TypeWrapper<Cursor> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    constexpr size_t GetElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetMaxElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr void GetLBCursor(this Cntr const& self, seq_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& self, seq_cntr::Tag,
                               Cursor* dst_cursor);

    template <lifecycle::IsDstStorageView DstStorageView>
    constexpr void PeekL(this auto&& self, seq_cntr::Tag, bool lazy_copy_elem,
                         lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                         Cursor* dst_cursor, DstStorageView dst_elem);

    template <lifecycle::IsDstStorageView DstStorageView>
    constexpr void PeekR(this auto&& self, seq_cntr::Tag, bool lazy_copy_elem,
                         lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                         Cursor* dst_cursor, DstStorageView dst_elem);

    template <lifecycle::IsDstStorageView DstStorageView>
    constexpr void Refer(this auto&& self, seq_cntr::Tag, size_t idx,
                         bool lazy_copy_elem,
                         lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                         Cursor* dst_cursor, DstStorageView dst_elem);

    template <lifecycle::IsDstStorageView DstStorageView>
    constexpr void Derefer(this auto&& self, seq_cntr::Tag,
                           Cursor const* pos_cursor, bool lazy_copy_elem,
                           lifecycle::DynamicStorageView<Elem>* dst_elem_view,
                           DstStorageView dst_elem);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void Read(this auto&& self, seq_cntr::Tag,
                        Cursor const* pos_cursor, size_t cnt, Acceptor&& reader,
                        Cursor* dst_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void Write(this meta::IsNonConstWithoutRef auto&& self,
                         seq_cntr::Tag, Cursor const* pos_cursor, size_t cnt,
                         Provider&& writer, Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void ReadWrite(this meta::IsNonConstWithoutRef auto&& self,
                             seq_cntr::Tag, Cursor const* pos_cursor,
                             size_t cnt, Acceptor&& acceptor,
                             Cursor* dst_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void PushL(this meta::IsNonConstWithoutRef auto&& self,
                         seq_cntr::Tag, size_t cnt, Provider&& writer,
                         Cursor* dst_beg_cursor, Cursor* dst_end_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void PushR(this meta::IsNonConstWithoutRef auto&& self,
                         seq_cntr::Tag, size_t cnt, Provider&& writer,
                         Cursor* dst_beg_cursor, Cursor* dst_end_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void Insert(this meta::IsNonConstWithoutRef auto&& self,
                          seq_cntr::Tag, Cursor* pos_cursor, size_t cnt,
                          Provider&& writer, Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void PopL(this meta::IsNonConstWithoutRef auto&& self,
                        seq_cntr::Tag, size_t cnt, Acceptor&& reader,
                        Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void PopR(this meta::IsNonConstWithoutRef auto&& self,
                        seq_cntr::Tag, size_t cnt, Acceptor&& reader,
                        Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void Erase(this meta::IsNonConstWithoutRef auto&& self,
                         seq_cntr::Tag, Cursor* pos_cursor, size_t cnt,
                         Acceptor&& reader);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void EraseAll(this meta::IsNonConstWithoutRef auto&& self,
                            seq_cntr::Tag, Acceptor&& acceptor);

    constexpr void CopyCursor(this Cntr const& self, seq_cntr::Tag,
                              Cursor const* src_cursor, Cursor* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& self, seq_cntr::Tag,
                                  Cursor const* cursor_a,
                                  Cursor const* cursor_b);

    constexpr comparison::Ordering CompareCursor(this Cntr const& self,
                                                 seq_cntr::Tag,
                                                 Cursor const* cursor_a,
                                                 Cursor const* cursor_b);

    constexpr size_t GetCursorDist(this Cntr const& self, seq_cntr::Tag,
                                   Cursor const* cursor_a,
                                   Cursor const* cursor_b);

    constexpr size_t GetCursorIdx(this Cntr const& self, seq_cntr::Tag,
                                  Cursor const* cursor);

    constexpr void CursorStepL(this Cntr const& self, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorStepR(this Cntr const& self, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorAdvanceL(this Cntr const& self, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);

    constexpr void CursorAdvanceR(this Cntr const& self, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);
};

}  // namespace zeta::core::debug_deque
