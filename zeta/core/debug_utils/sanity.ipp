#pragma once

#if !defined(ZETA_Core_DebugUtils_Sanity_Enable)
#error "ZETA_Core_DebugUtils_Sanity_Enable must be defined"
#endif

#if ZETA_Core_DebugUtils_Sanity_Enable

#include <unordered_map>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/sanity.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core::debug_utils {

namespace sanity::detail {

std::unordered_map<void const*, SanityCheckFunc> sanity_check_func_map_;

inline size_t cur_sanity_check_id_{ 0 };

std::unordered_map<void const*,
                   std::pair<meta::UnderlyingType<SanityCheckScope>,
                             meta::UnderlyingType<SanityCheckScope>>>
    cur_sanity_check_scope_table_;

}  // namespace sanity::detail

constexpr size_t sanity::GetCurSanityCheckId() {
    return detail::cur_sanity_check_id_;
}

constexpr bool sanity::RegisterSanityCheckFunc(
    void const* obj, SanityCheckFunc sanity_check_func) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(sanity_check_func != nullptr);

    return detail::sanity_check_func_map_.insert({ obj, sanity_check_func })
        .second;
}

constexpr bool sanity::UnregisterSanityCheckFunc(void const* obj) {
    return detail::sanity_check_func_map_.erase(obj) != 0;
}

constexpr void sanity::SanityCheck(void const* obj, SanityCheckScope scope) {
    meta::UnderlyingType<SanityCheckScope> scope_val{ meta::ToUnderlying(
        scope) };

    bool sanity_check_triggered_here{
        detail::cur_sanity_check_scope_table_.empty()
    };

    if (sanity_check_triggered_here) { ++detail::cur_sanity_check_id_; }

    auto sanity_check_func_iter{ detail::sanity_check_func_map_.find(obj) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        sanity_check_func_iter != detail::sanity_check_func_map_.end());

    SanityCheckFunc sanity_check_func{ sanity_check_func_iter->second };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(sanity_check_func != nullptr);

    auto iter{ detail::cur_sanity_check_scope_table_.insert(
        { obj, { 0, 0 } }) };

    meta::UnderlyingType<SanityCheckScope> finished_scope{
        iter.first->second.first
    };

    meta::UnderlyingType<SanityCheckScope> checking_scope{
        iter.first->second.second
    };

    if (finished_scope < scope_val) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(finished_scope ==
                                                checking_scope);

        checking_scope = scope_val;

        sanity_check_func(obj, scope);

        (ExpandFinishedSanityCheckScope)(obj, scope);
    }

    if (sanity_check_triggered_here) {
        detail::cur_sanity_check_scope_table_.clear();
    }
}

constexpr void sanity::ExpandFinishedSanityCheckScope(void const* obj,
                                                      SanityCheckScope scope) {
    meta::UnderlyingType<SanityCheckScope> raw_scope{ meta::ToUnderlying(
        scope) };

    auto scope_iter{ detail::cur_sanity_check_scope_table_.find(obj) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        scope_iter != detail::cur_sanity_check_scope_table_.end());

    meta::UnderlyingType<SanityCheckScope> finished_scope{
        scope_iter->second.first
    };

    meta::UnderlyingType<SanityCheckScope> checking_scope{
        scope_iter->second.second
    };

    if (raw_scope <= finished_scope) { return; }

    finished_scope = raw_scope;

    if (raw_scope < checking_scope) { return; }

    checking_scope = raw_scope;
}

constexpr void sanity::DummySanityCheckFunc(void const*, SanityCheckScope) {}

}  // namespace zeta::core::debug_utils

#endif
