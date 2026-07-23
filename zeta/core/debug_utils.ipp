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

#define ZETA_Core_ForcePrintCurPosToPipe(pipe)                    \
    zeta::core::debug_utils::PrintPos((pipe), __FILE__, __LINE__, \
                                      __PRETTY_FUNCTION__)        \
        << "\n\n";                                                \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_PrintCurPos \
    ZETA_Core_WhenEnPrint(ZETA_Core_ForcePrintCurPosToPipe(std::cout))

#define ZETA_Core_Debug_PrintCurPos \
    ZETA_Core_ForcePrintCurPosToPipe(zeta::core::debug_utils::debug_str_stream)

#define ZETA_Core_ForcePrintVarToPipe_(tmp_var, dst_pipe, var)           \
    ({                                                                   \
        decltype(auto) tmp_var{ (var) };                                 \
        zeta::core::debug_utils::PrintVar(dst_pipe, __FILE__, __LINE__,  \
                                          __PRETTY_FUNCTION__,           \
                                          ZETA_Core_ToStr(var), tmp_var) \
            << "\n\n";                                                   \
        if (ZETA_Core_ImmPrint) { dst_pipe.flush(); }                    \
        zeta::core::meta::Forward<decltype(tmp_var)>(tmp_var);           \
    })

#define ZETA_Core_ForcePrintVarToPipe(dst_pipe, var) \
    ZETA_Core_ForcePrintVarToPipe_(ZETA_Core_TmpName, dst_pipe, var)

#define ZETA_Core_ForcePrintVar(var) \
    ZETA_Core_ForcePrintVarToPipe(std::cout, var)

#define ZETA_Core_PrintVar(var) \
    ZETA_Core_WhenEnPrint(ZETA_Core_ForcePrintVar(var))

#define ZETA_Core_Debug_PrintVar(var)                        \
    ZETA_Core_WhenEnableDebug(ZETA_Core_ForcePrintVarToPipe( \
        zeta::core::debug_utils::debug_str_stream, var))

#define ZETA_Core_DebugAssert_(tmp_cond, cond)                                 \
    {                                                                          \
        auto tmp_cond{ cond };                                                 \
                                                                               \
        if (tmp_cond) {                                                        \
        } else {                                                               \
            ZETA_Core_ForcePrintVar("Debug Assert !!!");                       \
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
            if (ZETA_Core_ImmPrint) { std::cout.flush(); }                     \
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

namespace zeta::core {

namespace debug_utils::detail {

template <unsigned Numeral, typename UnsignedIntegral>
std::string IntegralToStr_(UnsignedIntegral value) {
    ZETA_Core_StaticAssert(integral::IsIntegral<UnsignedIntegral>);

    ZETA_Core_StaticAssert(Numeral == 2 || Numeral == 8 || Numeral == 10 ||
                           Numeral == 16);

    constexpr char c[]{ "0123456789ABCDEF" };

    unsigned sep_size;

    switch (Numeral) {
    case 2: sep_size = 4; break;
    case 8: sep_size = 3; break;
    case 10: sep_size = 3; break;
    case 16: sep_size = 2; break;
    }

    std::string str;

    for (; 0 < value; value /= Numeral) {
        if (str.size() % (sep_size + 1) == sep_size) { str.push_back('\''); }

        str.push_back(c[value % Numeral]);
    }

    if (str.empty()) { str.push_back('0'); }

    std::reverse(str.begin(), str.end());

    return str;
}

}  // namespace debug_utils::detail

template <typename T>
constexpr std::string debug_utils::GetTypeStr() {
    std::string func_name{ __PRETTY_FUNCTION__ };

    size_t l_size{ func_name.find("T = ") + 4 };
    size_t r_size{ 1 };

    return func_name.substr(l_size, func_name.size() - r_size - l_size);
}

template <>
struct debug_utils::VarPrinter<bool> {
    static std::ostream& Print(std::ostream& os, bool value) {
        return os << Format(std::right, dec_width)
                  << (value ? "true" : "false");
    }
};

template <typename T>
struct debug_utils::VarPrinter<
    T, meta::EnableIf<(integral::IsIntegral<T> || meta::IsPointer<T>), void>> {
    static std::ostream& Print(std::ostream& os, T const& value) {
        auto proc_value{ [=]() {
            if constexpr (meta::IsPointer<T>) {
                return reinterpret_cast<uintptr_t>(value);
            } else if constexpr (integral::IsSignedIntegral<T> &&
                                 integral::WidthOf<T> <
                                     integral::WidthOf<int>) {
                return static_cast<int>(value);
            } else if constexpr (integral::IsUnsignedIntegral<T> &&
                                 integral::WidthOf<T> <
                                     integral::WidthOf<unsigned>) {
                return static_cast<unsigned>(value);
            } else {
                return value;
            }
        }() };

        bool is_signed{ integral::IsSignedIntegral<T> };

        using UnProcValue = integral::MakeUnsignedOf<decltype(proc_value)>;

        UnProcValue un_proc_value;

        if (is_signed) {
            un_proc_value = static_cast<UnProcValue>(
                proc_value < 0 ? -proc_value : proc_value);
        } else {
            un_proc_value = static_cast<UnProcValue>(proc_value);
        }

        os << Format(std::right, dec_width)
           << (is_signed ? proc_value < 0 ? '-' : '+' : ' ')
           << detail::IntegralToStr_<10>(un_proc_value) << "d";

        os << space_str;

        os << Format(std::right, dec_width)
           << (is_signed ? proc_value < 0 ? '-' : '+' : ' ')
           << detail::IntegralToStr_<16>(un_proc_value) << "h";

        os << Format(std::right, dec_width)
           << (is_signed ? proc_value < 0 ? '-' : '+' : ' ')
           << detail::IntegralToStr_<2>(un_proc_value) << "b";

        if constexpr (meta::IsAnySame<T, char*, char const*>) {
            os << space_str << "\"" << value << "\"";
        }

        return os;
    }
};

template <>
struct debug_utils::VarPrinter<double> {
    static std::ostream& Print(std::ostream& os, double value) {
        return os << space_str << "\"" << value << "\"";
    }
};

template <>
struct debug_utils::VarPrinter<char[]> {
    static std::ostream& Print(std::ostream& os, char const* value) {
        return os << space_str << "\"" << value << "\"";
    }
};

template <>
struct debug_utils::VarPrinter<char const[]> : public VarPrinter<char[]> {};

template <size_t N>
struct debug_utils::VarPrinter<char[N]> : public VarPrinter<char[]> {};

template <size_t N>
struct debug_utils::VarPrinter<char const[N]> : public VarPrinter<char[]> {};

template <>
struct debug_utils::VarPrinter<std::string> {
    static std::ostream& Print(std::ostream& os, std::string const& str) {
        return VarPrinter<char const*>::Print(os, str.c_str());
    }
};

template <>
struct debug_utils::VarPrinter<std::string const>
    : public VarPrinter<std::string> {};

inline std::ostream& debug_utils::PrintPos(std::ostream& os, char const* file,
                                           int line, char const* func) {
    return os <<                                                           //
           "\033[36m" << Format(std::right, file_width) << file << ':' <<  //
           Format(std::left, line_width) << std::dec << line << "\033[0m"
              << space_str <<  //
           Format(std::left, func_width) << func;
}

template <typename T>
std::ostream& debug_utils::PrintVar(std::ostream& os, char const* file,
                                    int line, char const* func,
                                    char const* var_name, T const& var) {
    PrintPos(os, file, line, func) << space_str <<                       //
        Format(std::right, var_name_width) << "\033[32m" << var_name <<  //
        " = " <<                                                         //
        Format(std::right, type_str_width) << GetTypeStr<T>()
                                   << space_str.c_str();

    return VarPrinter<T>::Print(os, var) << "\033[0m";
}

inline void debug_utils::PrintDebugStrStream() {
    std::cout << debug_str_stream.str();
    std::cout.flush();
}

inline void debug_utils::ClearDebugStrStream() {
    debug_str_stream.str("");
    debug_str_stream.clear();
}

/*
template <typename Iterator>
Iterator debug_utils::FindEnclosedBlock(Iterator iter) {
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
*/

}  // namespace zeta::core

#pragma pop_macro("Format")
