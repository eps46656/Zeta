#pragma once

#include <zeta/core/debug_utils/stacktrace.hpp>

namespace zeta::core::debug_utils {

constexpr void stacktrace::PrintStackTrace(std::ostream& os,
                                           std::stacktrace const& st) {
    size_t depth{ st.size() };

    for (size_t i{ 0 }; i < depth; ++i) {
        std::stacktrace_entry const& frame{ st[i] };

        //
    }
}

}  // namespace zeta::core::debug_utils
