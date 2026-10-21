#pragma once

#include <string>

namespace zeta::core::debug_utils::symbol {

struct CodeSymbolInfo {
    void const* addr;

    std::string file;
    size_t line;
    std::string func;
};

constexpr CodeSymbolInfo SymbolizeCode(void const* code_addr);

}  // namespace zeta::core::debug_utils::symbol
