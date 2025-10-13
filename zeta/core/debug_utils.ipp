#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>

#pragma push_macro("Format")

#define Format(left_right, width) \
    left_right << std::setfill(' ') << std::setw(width)

namespace zeta::core::debug_utils {

template <typename T>
std::string GetTypeStr() {
    std::string func_name{ __PRETTY_FUNCTION__ };

    size_t l_size{ func_name.find("T = ") + 4 };
    size_t r_size{ 1 };

    return func_name.substr(l_size, func_name.size() - r_size - l_size);
}

template <typename T>
struct PrintVarCore<T, EnableIf<(IsIntegral<T> || IsPointer<T>), void>> {
    static constexpr auto PreProcess_(T const& value) {
        if constexpr (IsPointer<T>) {
            return reinterpret_cast<uintptr_t>(value);
        } else if constexpr (IsUnsignedIntegral<T>) {
            return static_cast<unsigned long long>(value);
        } else if constexpr (IsSignedIntegral<T>) {
            return static_cast<long long>(value);
        }
    }

    std::ostream& operator()(std::ostream& os, T const& value) const {
        if constexpr (IsAnyOf<T, bool>) {
            os << Format(std::right, dec_width) << (value ? "true" : "false");
        } else {
            auto proc_value{ PreProcess_(value) };

            os << Format(std::right, dec_width) << std::dec << proc_value << 'd'
               << space_str << Format(std::right, hex_width) << std::hex
               << std::uppercase << proc_value << 'h';
        }

        if constexpr (IsAnyOf<T, char*, char const*>) {
            os << space_str << "\"" << value << "\"";
        }

        return os;
    }
};

template <>
struct PrintVarCore<char[]> {
    std::ostream& operator()(std::ostream& os, char const* value) const {
        os << space_str << "\"" << value << "\"";
        return os;
    }
};

template <>
struct PrintVarCore<char const[]> {
    std::ostream& operator()(std::ostream& os, char const* value) const {
        os << space_str << "\"" << value << "\"";
        return os;
    }
};

template <size_t N>
struct PrintVarCore<char[N]> {
    std::ostream& operator()(std::ostream& os, char const* value) const {
        os << space_str << "\"" << value << "\"";
        return os;
    }
};

template <size_t N>
struct PrintVarCore<char const[N]> {
    std::ostream& operator()(std::ostream& os, char const* value) const {
        os << space_str << "\"" << value << "\"";
        return os;
    }
};

// -----------------------------------------------------------------------------

template <typename T>
PrintVarCore<T> const& GetPrintVarCore() {
    static PrintVarCore<T> const print_var_core;
    return print_var_core;
}

// -----------------------------------------------------------------------------

inline std::ostream& PrintPos(std::ostream& os, char const* file, int line,
                              char const* func) {
    return os <<                                                              //
           Format(std::right, file_width) << file << ':' <<                   //
           Format(std::left, line_width) << std::dec << line << space_str <<  //
           Format(std::left, func_width) << func;
}

template <typename T>
std::ostream& PrintVar(std::ostream& os, char const* file, int line,
                       char const* func, char const* var_name, T const& var) {
    PrintPos(os, file, line, func) << space_str <<         //
        Format(std::right, var_name_width) << var_name <<  //
        " = " <<                                           //
        Format(std::right, type_str_width) << GetTypeStr<T>()
                                   << space_str.c_str();

    PrintVarCore<T>{}(os, var);

    return os;
}

inline void PrintDebugStrStream() {
    std::cout << debug_str_stream.rdbuf();
    std::cout.flush();
}

inline void ClearDebugStrStream() {
    debug_str_stream.str("");
    debug_str_stream.clear();
}

}  // namespace zeta::core::debug_utils

#pragma pop_macro("Format")
