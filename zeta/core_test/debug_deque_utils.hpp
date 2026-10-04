#pragma once

#include <zeta/core/debug_deque.hpp>
#include <zeta/core/debug_deque.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>

namespace zeta::core_test::debug_deque_utils {

template <typename Elem>
using DebugDeque = core::debug_deque::Cntr<Elem>;

template <typename Elem>
struct Pack {
    DebugDeque<Elem> debug_deque;
};

template <typename Elem>
constexpr core::poly_seq_cntr::Cntr<Elem> Create() {
    Pack<Elem>* pack{ new Pack<Elem>{} };

    auto* dd{ &pack->debug_deque };

    static_assert(core::seq_cntr::IsSeqCntr<DebugDeque<Elem>>);

    seq_cntr_utils::AddSanitizeFunc(dd, [](void const* dd_) {
        DebugDeque<Elem> const* dd{ static_cast<DebugDeque<Elem> const*>(dd_) };

        if (dd == nullptr) { return; }
    });

    seq_cntr_utils::AddDestroyFunc(dd, [](void* dd_) {
        DebugDeque<Elem>* dd{ static_cast<DebugDeque<Elem>*>(dd_) };

        if (dd_ == nullptr) { return; }

        Pack<Elem>* pack{ ZETA_Core_MemberToStruct(Pack<Elem>, debug_deque,
                                                   dd) };

        delete pack;
    });

    return *dd;
}

}  // namespace zeta::core_test::debug_deque_utils
