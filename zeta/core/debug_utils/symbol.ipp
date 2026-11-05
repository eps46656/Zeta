#pragma once

#include <sanitizer/common_interface_defs.h>

#include <zeta/core/debug_utils/lru.ipp>
#include <zeta/core/debug_utils/symbol.hpp>

namespace zeta::core::debug_utils {

namespace symbol::detail {

inline lru::CacheManager<void const*, CodeSymbolInfo> code_symbol_cache_server{
    1024 * 4
};

inline char symbolize_buffer[1024];

}  // namespace symbol::detail

constexpr symbol::CodeSymbolInfo symbol::SymbolizeCode(void const* code_addr) {
    auto cache_iter{ detail::code_symbol_cache_server.Find(code_addr) };

    if (cache_iter != detail::code_symbol_cache_server.End()) {
        return cache_iter->second;
    }

    CodeSymbolInfo code_sym_info{
        .addr = code_addr,
        .file = {},
        .line = 0,
        .func = {},
    };

    char* buffer_end{ detail::symbolize_buffer +
                      sizeof(detail::symbolize_buffer) };

    char* iter{ buffer_end };
    char* end{ buffer_end };

    for (bool fetched{ false };;) {
        if (iter == end) {
            __sanitizer_symbolize_pc(const_cast<void*>(code_addr),
                                     "%s\n%l\n%f\n", detail::symbolize_buffer,
                                     sizeof(detail::symbolize_buffer));

            iter    = detail::symbolize_buffer;
            end     = buffer_end;
            fetched = true;
        }

        for (; iter != end; ++iter) {
            if (*iter == '\0') {
                end = iter;
                break;
            }

            if (*iter == '\n') { break; }

            code_sym_info.file.push_back(*iter);
        }

        if (iter != end) {
            ++iter;
            break;
        }

        if (fetched) {
            if (3 <= code_sym_info.file.size()) {
                char* c{ &code_sym_info.file[code_sym_info.file.size() - 3] };

                c[0] = '.';
                c[1] = '.';
                c[2] = '.';
            }

            break;
        }
    }

    for (bool fetched{ false };;) {
        if (iter == end) {
            __sanitizer_symbolize_pc(const_cast<void*>(code_addr), "%l\n%f\n",
                                     detail::symbolize_buffer,
                                     sizeof(detail::symbolize_buffer));

            iter    = detail::symbolize_buffer;
            end     = buffer_end;
            fetched = true;
        }

        char* cur_iter{ iter };

        for (; cur_iter != end; ++cur_iter) {
            if (*cur_iter == '\0') {
                end = cur_iter;
                break;
            }

            if (*cur_iter == '\n') { break; }
        }

        if (cur_iter != end) {
            *cur_iter = '\0';

            code_sym_info.line = std::strtoull(iter, nullptr, 10);

            iter = cur_iter + 1;

            break;
        }

        if (fetched) {
            code_sym_info.line = 0xFF'FF'FF'FF;
            break;
        }

        iter = end;
    }

    for (bool fetched{ false };;) {
        if (iter == end) {
            __sanitizer_symbolize_pc(const_cast<void*>(code_addr), "%f\n",
                                     detail::symbolize_buffer,
                                     sizeof(detail::symbolize_buffer));

            iter    = detail::symbolize_buffer;
            end     = buffer_end;
            fetched = true;
        }

        for (; iter != end; ++iter) {
            if (*iter == '\0') {
                end = iter;
                break;
            }

            if (*iter == '\n') { break; }

            code_sym_info.func.push_back(*iter);
        }

        if (iter != end) {
            ++iter;
            break;
        }

        if (fetched) {
            if (3 <= code_sym_info.func.size()) {
                char* c{ &code_sym_info.func[code_sym_info.func.size() - 3] };

                c[0] = '.';
                c[1] = '.';
                c[2] = '.';
            }

            break;
        }
    }

    detail::code_symbol_cache_server.Set(code_addr, code_sym_info);

    return code_sym_info;
}

}  // namespace zeta::core::debug_utils
