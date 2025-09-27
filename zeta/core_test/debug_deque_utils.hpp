#pragma once

#include <cstdlib>
#include <zeta/core/debug_deque.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>

namespace zeta::core_test::debug_deque_utils {

using DebugDeque = core::DebugDeque;

struct Pack {
    DebugDeque debug_deque;
};

template <typename Elem>
core::SeqCntr Create();

void Destroy(core::SeqCntr seq_cntr);

void Sanitize(core::SeqCntr seq_cntr);

template <typename Elem>
core::SeqCntr Create() {
    Pack* pack{ new Pack{} };

    pack->debug_deque.width = sizeof(Elem);

    DebugDeque::Init(&pack->debug_deque);

    ZETA_Core_PrintVar(pack->debug_deque.deque);

    seq_cntr_utils::AddSanitizeFunc(&core::SeqCntr::MakeVTable<DebugDeque>(),
                                    Sanitize);

    seq_cntr_utils::AddDestroyFunc(&core::SeqCntr::MakeVTable<DebugDeque>(),
                                   Destroy);

    return DebugDeque::ToSeqCntr(&pack->debug_deque);
}

inline void Destroy(core::SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(seq_cntr.vtable ==
                          &core::SeqCntr::MakeVTable<DebugDeque>());
    if (seq_cntr.inst == NULL) { return; }

    ZETA_Core_DebugAssert(seq_cntr.vtable ==
                          &core::SeqCntr::MakeVTable<DebugDeque>());
    if (seq_cntr.inst == NULL) { return; }

    Pack* pack{ ZETA_Core_MemberToStruct(Pack, debug_deque, seq_cntr.inst) };

    DebugDeque::Deinit(seq_cntr.inst);

    delete pack;
}

inline void Sanitize(core::SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(seq_cntr.vtable ==
                          &core::SeqCntr::MakeVTable<DebugDeque>());
    if (seq_cntr.inst == NULL) { return; }
}

}  // namespace zeta::core_test::debug_deque_utils
