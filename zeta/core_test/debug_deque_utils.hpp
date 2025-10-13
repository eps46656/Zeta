#pragma once

#include <cstdlib>
#include <zeta/core/debug_deque.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>

namespace zeta::core_test::debug_deque_utils {

using SeqCntrRef = core::seq_cntr::SeqCntrRef;
using DebugDeque = core::DebugDeque;

struct Pack {
    DebugDeque debug_deque;
};

template <typename Elem>
SeqCntrRef Create();

void Destroy(SeqCntrRef seq_cntr_ref);

void Sanitize(SeqCntrRef seq_cntr_ref);

template <typename Elem>
SeqCntrRef Create() {
    Pack* pack{ new Pack{} };

    pack->debug_deque.width = sizeof(Elem);

    DebugDeque::Init(&pack->debug_deque);

    ZETA_Core_PrintVar(pack->debug_deque.deque);

    SeqCntrRef seq_cntr_ref{ DebugDeque::GetSeqCntrRef(&pack->debug_deque) };

    seq_cntr_utils::AddSanitizeFunc(&pack->debug_deque, Sanitize);

    seq_cntr_utils::AddDestroyFunc(&pack->debug_deque, Destroy);

    return seq_cntr_ref;
}

inline void Destroy(SeqCntrRef seq_cntr_ref) {
    if (seq_cntr_ref.inst == nullptr) { return; }

    Pack* pack{ ZETA_Core_MemberToStruct(Pack, debug_deque,
                                         seq_cntr_ref.inst) };

    DebugDeque::Deinit(seq_cntr_ref.inst);

    delete pack;
}

inline void Sanitize(SeqCntrRef seq_cntr_ref) {
    if (seq_cntr_ref.inst == nullptr) { return; }
}

}  // namespace zeta::core_test::debug_deque_utils
