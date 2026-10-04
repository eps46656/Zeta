#pragma once

#include <zeta/core/debug_utils/sanity.hpp>
#include <zeta/core/poly_seq_endpoint.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/type_identity.hpp>

namespace zeta::core::poly_seq_cntr {

template <meta::IsContainerElem Elem>
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

    template <seq_cntr::IsSeqCntr TargetCntr>
    constexpr Cntr(TargetCntr& target_cntr);

    constexpr ~Cntr();

    constexpr Cntr& operator=(Cntr const&) = default;

    template <seq_cntr::IsSeqCntr TargetCntr>
    constexpr void Set(this Cntr& self, TargetCntr& target_cntr)
        requires meta::IsSame<
            Elem, meta::GetTypeWrapperType<
                      decltype(seq_cntr::GetElemType<TargetCntr>())>>;

    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        this Cntr& self, seq_cntr::Tag);

    constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        this Cntr const& self, seq_cntr::Tag);

    constexpr seq_cntr::capability::Flag GetDynamicDisabledCapabilityFlag(
        this Cntr& self, seq_cntr::Tag);

    constexpr seq_cntr::capability::Flag GetDynamicDisabledCapabilityFlag(
        this Cntr const& self, seq_cntr::Tag);

    constexpr void* GetReferedInstPtr(this Cntr const& self, seq_cntr::Tag);

    static constexpr meta::TypeWrapper<Elem> GetElemType(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr meta::TypeWrapper<seq_cntr::CursorLimit> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr meta::TypeWrapper<seq_cntr::CursorLimit> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    constexpr size_t GetSride(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetOffset(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetMaxElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr void GetLBCursor(this Cntr const& self, seq_cntr::Tag,
                               seq_cntr::CursorLimit* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& self, seq_cntr::Tag,
                               seq_cntr::CursorLimit* dst_cursor);

    constexpr void PeekL(this Cntr const& self, seq_cntr::Tag,
                         bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         seq_cntr::CursorLimit* dst_cursor,
                         lifecycle::DataLifeState dst_elem_life_state,
                         Elem* dst_elem);

    constexpr void PeekR(this Cntr const& self, seq_cntr::Tag,
                         bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         seq_cntr::CursorLimit* dst_cursor,
                         lifecycle::DataLifeState dst_elem_life_state,
                         Elem* dst_elem);

    constexpr void Refer(this Cntr const& self, seq_cntr::Tag, size_t idx,
                         bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         seq_cntr::CursorLimit* dst_cursor,
                         lifecycle::DataLifeState dst_elem_life_state,
                         Elem* dst_elem);

    constexpr void Derefer(this Cntr const& self, seq_cntr::Tag,
                           seq_cntr::CursorLimit* pos_cursor,
                           bool lazy_copy_elem,
                           seq_cntr::ElemPtrView* dst_elem_ptr_view,
                           lifecycle::DataLifeState dst_elem_life_state,
                           Elem* dst_elem);

    template <typename Acceptor>
    constexpr void Read(this Cntr const& self, seq_cntr::Tag,
                        seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                        Acceptor&& acceptor, seq_cntr::CursorLimit* dst_cursor);

    template <typename Provider>
    constexpr void Write(this Cntr& self, seq_cntr::Tag,
                         seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                         Provider&& writer, seq_cntr::CursorLimit* dst_cursor);

    template <typename Acceptor>
    constexpr void ReadWrite(this Cntr& self, seq_cntr::Tag,
                             seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                             Acceptor&& acceptor,
                             seq_cntr::CursorLimit* dst_cursor);

    template <typename Provider>
    constexpr void PushL(this Cntr& self, seq_cntr::Tag, size_t cnt,
                         Provider&& writer,
                         seq_cntr::CursorLimit* dst_beg_cursor,
                         seq_cntr::CursorLimit* dst_end_cursor);

    template <typename Provider>
    constexpr void PushR(this Cntr& self, seq_cntr::Tag, size_t cnt,
                         Provider&& writer,
                         seq_cntr::CursorLimit* dst_beg_cursor,
                         seq_cntr::CursorLimit* dst_end_cursor);

    template <typename Provider>
    constexpr void Insert(this Cntr& self, seq_cntr::Tag,
                          seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                          Provider&& writer, seq_cntr::CursorLimit* dst_cursor);

    template <typename Acceptor>
    constexpr void PopL(this Cntr& self, seq_cntr::Tag, size_t cnt,
                        Acceptor&& acceptor, seq_cntr::CursorLimit* dst_cursor);

    template <typename Acceptor>
    constexpr void PopR(this Cntr& self, seq_cntr::Tag, size_t cnt,
                        Acceptor&& acceptor, seq_cntr::CursorLimit* dst_cursor);

    template <typename Acceptor>
    constexpr void Erase(this Cntr& self, seq_cntr::Tag,
                         seq_cntr::CursorLimit* pos_cursor, size_t cnt,
                         Acceptor&& acceptor);

    template <typename Acceptor>
    constexpr void EraseAll(this Cntr& self, seq_cntr::Tag,
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

    static constexpr void SanityCheck(
        void const* self, debug_utils::sanity::SanityCheckScope scope);
};

}  // namespace zeta::core::poly_seq_cntr
