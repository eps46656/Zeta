#pragma once

#include <stdlib.h>

#include <__msvc_ostream.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <xstring>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

#define ZETA_Core_PrintVar(var)                                      \
    zeta::core::debug_utils::PrintVar(std::cout, __FILE__, __LINE__, \
                                      __PRETTY_FUNCTION__,           \
                                      ZETA_Core_ToStr(var), (var))   \
        << "\n\n";                                                   \
    if (ZETA_Core_ImmPrint) { std::cout.flush(); }                   \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_Debug_PrintVar(var)                                  \
    zeta::core::debug_utils::PrintVar(                                 \
        zeta::core::debug_utils::debug_str_stream, __FILE__, __LINE__, \
        __PRETTY_FUNCTION__, ZETA_Core_ToStr(var), (var))              \
        << "\n\n";                                                     \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_DebugAssert_(tmp_cond, cond)                                 \
    {                                                                          \
        auto tmp_cond{ cond };                                                 \
                                                                               \
        if (tmp_cond) {                                                        \
        } else {                                                               \
            ZETA_Core_PrintVar("Debug Assert !!!");                            \
                                                                               \
            zeta::core::debug_utils::PrintVar(std::cout, __FILE__, __LINE__,   \
                                              __PRETTY_FUNCTION__,             \
                                              ZETA_Core_ToStr(cond), tmp_cond) \
                << '\n';                                                       \
                                                                               \
            zeta::core::debug_utils::PrintDebugStrStream();                    \
                                                                               \
            ZETA_Core_PrintStackTrace;                                         \
                                                                               \
            std::cout.flush();                                                 \
                                                                               \
            exit(1);                                                           \
        }                                                                      \
    }

#define ZETA_Core_DebugAssert(...) \
    ZETA_Core_WhenEnableDebug(     \
        ZETA_Core_DebugAssert_(ZETA_Core_TmpName, (__VA_ARGS__)))

#pragma push_macro("Format")
#define Format(left_right, width) \
    left_right << std::setfill(' ') << std::setw(width)

namespace zeta::core::debug_utils {

template <typename T>
constexpr std::string GetTypeStr() {
    std::string func_name{ __PRETTY_FUNCTION__ };

    size_t l_size{ func_name.find("T = ") + 4 };
    size_t r_size{ 1 };

    return func_name.substr(l_size, func_name.size() - r_size - l_size);
}

template <>
struct VarPrinter<bool> {
    static std::ostream& Print(std::ostream& os, bool value) {
        return os << Format(std::right, dec_width)
                  << (value ? "true" : "false");
    }
};

template <typename T>
struct VarPrinter<
    T, meta::EnableIf<(integral::IsIntegral<T> || meta::IsPointer<T>), void>> {
    static std::ostream& Print(std::ostream& os, T const& value) {
        auto proc_value{ [=]() {
            if constexpr (meta::IsPointer<T>) {
                return reinterpret_cast<uintptr_t>(value);
            } else {
                return value;
            }
        }() };

        os << Format(std::right, dec_width) << std::dec << proc_value << 'd'
           << space_str << Format(std::right, hex_width) << std::hex
           << std::uppercase << proc_value << 'h';

        if constexpr (meta::IsAnyOf<T, char*, char const*>) {
            os << space_str << "\"" << value << "\"";
        }

        return os;
    }
};

template <>
struct VarPrinter<double> {
    static std::ostream& Print(std::ostream& os, double value) {
        return os << space_str << "\"" << value << "\"";
    }
};

template <>
struct VarPrinter<char[]> {
    static std::ostream& Print(std::ostream& os, char const* value) {
        return os << space_str << "\"" << value << "\"";
    }
};

template <>
struct VarPrinter<char const[]> : public VarPrinter<char[]> {};

template <size_t N>
struct VarPrinter<char[N]> : public VarPrinter<char[]> {};

template <size_t N>
struct VarPrinter<char const[N]> : public VarPrinter<char[]> {};

template <>
struct VarPrinter<std::string> {
    static std::ostream& Print(std::ostream& os, std::string const& str) {
        return VarPrinter<char const*>::Print(os, str.c_str());
    }
};

template <>
struct VarPrinter<std::string const> : public VarPrinter<std::string> {};

inline std::ostream& PrintPos(std::ostream& os, char const* file, int line,
                              char const* func) {
    return os <<                                                           //
           "\033[36m" << Format(std::right, file_width) << file << ':' <<  //
           Format(std::left, line_width) << std::dec << line << "\033[0m"
              << space_str <<  //
           Format(std::left, func_width) << func;
}

template <typename T>
std::ostream& PrintVar(std::ostream& os, char const* file, int line,
                       char const* func, char const* var_name, T const& var) {
    PrintPos(os, file, line, func) << space_str <<                       //
        Format(std::right, var_name_width) << "\033[32m" << var_name <<  //
        " = " <<                                                         //
        Format(std::right, type_str_width) << GetTypeStr<T>()
                                   << space_str.c_str();

    return VarPrinter<T>::Print(os, var) << "\033[0m";
}

inline void PrintDebugStrStream() {
    std::cout << debug_str_stream.rdbuf();
    std::cout.flush();
}

inline void ClearDebugStrStream() {
    debug_str_stream.str("");
    debug_str_stream.clear();
}

template <typename Iterator>
Iterator FindEnclosedBlock(Iterator iter) {
    char first_c{ *iter };

    switch (first_c) {
    case '\'':
    case '\"':
    case '(':
    case '[':
    case '{':
    default: ZETA_Core_DebugAssert(false);
    }

    std::string stack;
    stack.push_back(first_c);

    while (!stack.empty()) {
        char c{ *(++iter) };

        if (stack.back() == '\'') {
            if (c == '\'') { stack.pop_back(); }
            continue;
        }

        if (stack.back() == '\"') {
            if (c == '\"') { stack.pop_back(); }
            continue;
        }

        switch (c) {
        case '\'':
        case '\"':
        case '(':
        case '[':
        case '{': stack.push_back(c); break;
        case ')':
            ZETA_Core_DebugAssert(stack.back() == '(');
            stack.pop_back();
            break;
        case ']':
            ZETA_Core_DebugAssert(stack.back() == '[');
            stack.pop_back();
            break;
        case '}':
            ZETA_Core_DebugAssert(stack.back() == '{');
            stack.pop_back();
            break;
        }
    }

    return iter;
}

}  // namespace zeta::core::debug_utils

#pragma pop_macro("Format")
