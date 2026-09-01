#pragma once

#include <cstdlib>
#include <zeta/core/debug_deque.hpp>
#include <zeta/core/debug_deque.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>

namespace zeta::core_test::debug_deque_utils {

using PolySeqCntr = core::poly_seq_cntr::Cntr;
namespace DebugDequeNS = core::debug_deque;
using DebugDeque = DebugDequeNS::Cntr;

struct Pack {
    DebugDeque debug_deque;
};

template <typename Elem>
constexpr PolySeqCntr Create();

constexpr void Destroy(void* dd);

constexpr void Sanitize(void const* dd);

template <typename Elem>
constexpr PolySeqCntr Create() {
    Pack* pack{ new Pack{
        .debug_deque{ sizeof(Elem) },
    } };

    auto* dd{ &pack->debug_deque };

    ZETA_Core_StaticAssert(core::seq_cntr::IsSeqCntr<DebugDeque>);

    seq_cntr_utils::AddSanitizeFunc(dd, Sanitize);

    seq_cntr_utils::AddDestroyFunc(dd, Destroy);

    return *dd;
}

constexpr void Destroy(void* dd_) {
    DebugDeque* dd{ static_cast<DebugDeque*>(dd_) };

    if (dd_ == nullptr) { return; }

    Pack* pack{ ZETA_Core_MemberToStruct(Pack, debug_deque, dd) };

    delete pack;
}

constexpr void Sanitize(void const* dd_) {
    DebugDeque const* dd{ static_cast<DebugDeque const*>(dd_) };

    if (dd == nullptr) { return; }
}

}  // namespace zeta::core_test::debug_deque_utils
