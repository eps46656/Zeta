#pragma once

#include <zeta/core/poly_seq_endpoint.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/type_identity.hpp>

namespace zeta::core::poly_seq_cntr {

template <meta::IsObjectType Elem>
struct Cntr {
    type_identity::TypeHashCode elem_type_hash_code;
    type_identity::TypeHashCode cursor_type_hash_code;

    size_t max_elem_cnt;

    seq_cntr::capability::Flag dynamic_enabled_capability_flag;
    seq_cntr::capability::Flag dynamic_disabled_capability_flag;

    seq_cntr::VTable<Elem> const* vtable;

    void* target_cntr;

    constexpr Cntr();

    constexpr Cntr(Cntr&) = default;

    constexpr Cntr(Cntr const&) = default;

    constexpr Cntr(Cntr&&) = default;

    template <seq_cntr::IsSeqCntrWithRightReference TargetCntr>
    constexpr Cntr(TargetCntr&& target_cntr);

    constexpr Cntr& operator=(Cntr const&) = default;

    template <seq_cntr::IsSeqCntrWithRightReference TargetCntr>
    constexpr void Set(this Cntr& self, TargetCntr&& target_cntr)
        requires meta::IsSame<
            Elem, meta::GetTypeWrapperType<decltype(seq_cntr::GetElemType(
                      meta::TypeWrapper<TargetCntr&&>{}))>>;

    template <
        meta::IsAnySame<Cntr&, Cntr const&, Cntr&&, Cntr const&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <meta::IsAnySame<Cntr&, Cntr&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <meta::IsAnySame<Cntr const&, Cntr const&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        this Cntr const& self, seq_cntr::Tag);

    constexpr seq_cntr::capability::Flag GetDynamicDisabledCapabilityFlag(
        this Cntr const& self, seq_cntr::Tag);

    constexpr void* GetReferedInstPtr(this Cntr const& self, seq_cntr::Tag);

    template <
        meta::IsAnySame<Cntr&, Cntr const&, Cntr&&, Cntr const&&> TargetCntr>
    static constexpr meta::TypeWrapper<Elem> GetElemType(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <
        meta::IsAnySame<Cntr&, Cntr const&, Cntr&&, Cntr const&&> TargetCntr>
    static constexpr meta::TypeWrapper<seq_cntr::CursorLimit> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    constexpr size_t GetSride(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetOffset(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetMaxElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr void GetLBCursor(this Cntr const& self, seq_cntr::Tag,
                               seq_cntr::CursorLimit* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& self, seq_cntr::Tag,
                               seq_cntr::CursorLimit* dst_cursor);

    constexpr void PeekL(
        this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* dst_cursor,
        resolving::Packet<meta::RemoveConst<Elem>, meta::RemoveConst<Elem>>*
            resolving_packet);

    constexpr void PeekR(
        this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* dst_cursor,
        resolving::Packet<meta::RemoveConst<Elem>, meta::RemoveConst<Elem>>*
            resolving_packet);

    constexpr void Refer(
        this Cntr const& self, seq_cntr::Tag, size_t idx,
        seq_cntr::CursorLimit* dst_cursor,
        resolving::Packet<meta::RemoveConst<Elem>, meta::RemoveConst<Elem>>*
            resolving_packet);

    constexpr void Derefer(
        this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* pos_cursor,
        resolving::Packet<meta::RemoveConst<Elem>, meta::RemoveConst<Elem>>*
            resolving_packet);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void Read(this Cntr const& self, seq_cntr::Tag,
                        seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                        Acceptor&& acceptor, seq_cntr::CursorLimit* dst_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void Write(this Cntr const& self, seq_cntr::Tag,
                         seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                         Provider&& writer, seq_cntr::CursorLimit* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void ReadWrite(this Cntr const& self, seq_cntr::Tag,
                             seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                             Acceptor&& acceptor,
                             seq_cntr::CursorLimit* dst_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void PushL(this Cntr const& self, seq_cntr::Tag, size_t cnt,
                         Provider&& writer,
                         seq_cntr::CursorLimit* dst_beg_cursor,
                         seq_cntr::CursorLimit* dst_end_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void PushR(this Cntr const& self, seq_cntr::Tag, size_t cnt,
                         Provider&& writer,
                         seq_cntr::CursorLimit* dst_beg_cursor,
                         seq_cntr::CursorLimit* dst_end_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void Insert(this Cntr const& self, seq_cntr::Tag,
                          seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                          Provider&& writer, seq_cntr::CursorLimit* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void PopL(this Cntr const& self, seq_cntr::Tag, size_t cnt,
                        Acceptor&& acceptor, seq_cntr::CursorLimit* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void PopR(this Cntr const& self, seq_cntr::Tag, size_t cnt,
                        Acceptor&& acceptor, seq_cntr::CursorLimit* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void Erase(this Cntr const& self, seq_cntr::Tag,
                         seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                         Acceptor&& acceptor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void EraseAll(this Cntr const& self, seq_cntr::Tag,
                            Acceptor&& acceptor);

    constexpr void CopyCursor(this Cntr const& self, seq_cntr::Tag,
                              seq_cntr::CursorLimit* src_cursor,
                              seq_cntr::CursorLimit* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& self, seq_cntr::Tag,
                                  seq_cntr::CursorLimit* cursor_a,
                                  seq_cntr::CursorLimit* cursor_b);

    constexpr comparison::Ordering CompareCursor(
        this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* cursor_a,
        seq_cntr::CursorLimit* cursor_b);

    constexpr size_t GetCursorDist(this Cntr const& self, seq_cntr::Tag,
                                   seq_cntr::CursorLimit* cursor_a,
                                   seq_cntr::CursorLimit* cursor_b);

    constexpr size_t GetCursorIdx(this Cntr const& self, seq_cntr::Tag,
                                  seq_cntr::CursorLimit* cursor);

    constexpr void CursorStepL(this Cntr const& self, seq_cntr::Tag,
                               seq_cntr::CursorLimit* cursor);

    constexpr void CursorStepR(this Cntr const& self, seq_cntr::Tag,
                               seq_cntr::CursorLimit* cursor);

    constexpr void CursorAdvanceL(this Cntr const& self, seq_cntr::Tag,
                                  seq_cntr::CursorLimit* cursor, size_t step);

    constexpr void CursorAdvanceR(this Cntr const& self, seq_cntr::Tag,
                                  seq_cntr::CursorLimit* cursor, size_t step);

    constexpr void Check(this Cntr const& self);
};

}  // namespace zeta::core::poly_seq_cntr
