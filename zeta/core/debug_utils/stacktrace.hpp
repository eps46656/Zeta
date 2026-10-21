#pragma once

#include <vector>

namespace zeta::core::debug_utils::stacktrace {

constexpr void PrintStackTrace(std::ostream& os, std::stacktrace const& st);

}
