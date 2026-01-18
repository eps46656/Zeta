#pragma once

#include <cstdlib>
#include <zeta/core/debug_deque.hpp>
#include <zeta/core/debug_deque.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>

namespace zeta::core_test::debug_deque_utils {

using SeqCntrRef = core::seq_cntr::Ref<core::value_wrapper::FalseType>;
namespace DebugDequeNS = core::debug_deque;
namespace DebugDequeOps = DebugDequeNS::ops;
using DebugDeque = DebugDequeNS::Cntr;

struct Pack {
    DebugDeque debug_deque;
};

template <typename Elem>
SeqCntrRef Create();

void Destroy(void* dd);

void Sanitize(void const* dd);

template <typename Elem>
SeqCntrRef Create() {
    Pack* pack{ new Pack{} };

    auto dd{ static_cast<DebugDeque*>(&pack->debug_deque) };

    dd->width = sizeof(Elem);

    DebugDequeOps::Init(dd);

    ZETA_Core_PrintVar(pack->debug_deque.deque);

    SeqCntrRef seq_cntr_ref{ zeta::core::seq_cntr::MakeRef(
        core::debug_deque::SeqCntrOperator{}, dd) };

    seq_cntr_utils::AddSanitizeFunc(dd, Sanitize);

    seq_cntr_utils::AddDestroyFunc(dd, Destroy);

    return seq_cntr_ref;
}

inline void Destroy(void* dd_) {
    DebugDeque* dd{ static_cast<DebugDeque*>(dd_) };

    if (dd_ == nullptr) { return; }

    Pack* pack{ ZETA_Core_MemberToStruct(Pack, debug_deque, dd) };

    DebugDequeOps::Deinit(dd);

    delete pack;
}

inline void Sanitize(void const* dd_) {
    DebugDeque const* dd{ static_cast<DebugDeque const*>(dd_) };

    if (dd == nullptr) { return; }
}

}  // namespace zeta::core_test::debug_deque_utils
