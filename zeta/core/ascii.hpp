#pragma once

namespace zeta::core::ascii {

// clang-format off

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ZETA_Core_ascii_Char_XMacro(func, sep)                                 \
    func(null,             0) sep                                              \
    func(bell,             7) sep                                              \
    func(backspace,        8) sep                                              \
    func(horizontal_tab,   9) sep                                              \
    func(line_feed,       10) sep                                              \
    func(vertical_tab,    11) sep                                              \
    func(form_feed,       12) sep                                              \
    func(carriage_return, 13) sep                                              \
                                                                               \
    func(space,        32) sep                                                 \
    func(exclamation,  33) sep                                                 \
    func(double_quote, 34) sep                                                 \
    func(number,       35) sep                                                 \
    func(dollar,       36) sep                                                 \
    func(percent,      37) sep                                                 \
    func(ampersand,    38) sep                                                 \
    func(single_quote, 39) sep                                                 \
    func(paren_l,      40) sep                                                 \
    func(paren_r,      41) sep                                                 \
    func(asterisk,     42) sep                                                 \
    func(plus,         43) sep                                                 \
    func(comma,        44) sep                                                 \
    func(minus,        45) sep                                                 \
    func(point,        46) sep                                                 \
    func(slash,        47) sep                                                 \
                                                                               \
    func(num_0, 48) sep                                                        \
    func(num_1, 49) sep                                                        \
    func(num_2, 50) sep                                                        \
    func(num_3, 51) sep                                                        \
    func(num_4, 52) sep                                                        \
    func(num_5, 53) sep                                                        \
    func(num_6, 54) sep                                                        \
    func(num_7, 55) sep                                                        \
    func(num_8, 56) sep                                                        \
    func(num_9, 57) sep                                                        \
                                                                               \
    func(colon,     58) sep                                                    \
    func(semicolon, 59) sep                                                    \
    func(angle_l,   60) sep                                                    \
    func(equal,     61) sep                                                    \
    func(angle_r,   62) sep                                                    \
    func(question,  63) sep                                                    \
    func(at,        64) sep                                                    \
                                                                               \
    func(A, 65) sep                                                           \
    func(B, 66) sep                                                           \
    func(C, 67) sep                                                           \
    func(D, 68) sep                                                           \
    func(E, 69) sep                                                           \
    func(F, 70) sep                                                           \
    func(G, 71) sep                                                           \
    func(H, 72) sep                                                           \
    func(I, 73) sep                                                           \
    func(J, 74) sep                                                           \
    func(K, 75) sep                                                           \
    func(L, 76) sep                                                           \
    func(M, 77) sep                                                           \
    func(N, 78) sep                                                           \
    func(O, 79) sep                                                           \
    func(P, 80) sep                                                           \
    func(Q, 81) sep                                                           \
    func(R, 82) sep                                                           \
    func(S, 83) sep                                                           \
    func(T, 84) sep                                                           \
    func(U, 85) sep                                                           \
    func(V, 86) sep                                                           \
    func(W, 87) sep                                                           \
    func(X, 88) sep                                                           \
    func(Y, 89) sep                                                           \
    func(Z, 90) sep                                                           \
                                                                               \
    func(bracket_l,  91) sep                                                   \
    func(backslash,  92) sep                                                   \
    func(bracket_r,  93) sep                                                   \
    func(caret,      94) sep                                                   \
    func(underscore, 95) sep                                                   \
    func(grave,      96) sep                                                   \
                                                                               \
    func(a,  97) sep                                                           \
    func(b,  98) sep                                                           \
    func(c,  99) sep                                                           \
    func(d, 100) sep                                                           \
    func(e, 101) sep                                                           \
    func(f, 102) sep                                                           \
    func(g, 103) sep                                                           \
    func(h, 104) sep                                                           \
    func(i, 105) sep                                                           \
    func(j, 106) sep                                                           \
    func(k, 107) sep                                                           \
    func(l, 108) sep                                                           \
    func(m, 109) sep                                                           \
    func(n, 110) sep                                                           \
    func(o, 111) sep                                                           \
    func(p, 112) sep                                                           \
    func(q, 113) sep                                                           \
    func(r, 114) sep                                                           \
    func(s, 115) sep                                                           \
    func(t, 116) sep                                                           \
    func(u, 117) sep                                                           \
    func(v, 118) sep                                                           \
    func(w, 119) sep                                                           \
    func(x, 120) sep                                                           \
    func(y, 121) sep                                                           \
    func(z, 122) sep                                                           \
                                                                               \
    func(brace_l,      123) sep                                                \
    func(vertical_bar, 124) sep                                                \
    func(brace_r,      125) sep                                                \
    func(tilde,        126) sep                                                \
                                                                               \
    func(empty,        255) sep

// clang-format on

enum struct CharEnum : unsigned char {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, code) name = code,

    ZETA_Core_ascii_Char_XMacro(F, )

#pragma pop_macro("F")
};

struct CharCodeTable {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, code) static constexpr unsigned char name{ code };

    ZETA_Core_ascii_Char_XMacro(F, )

#pragma pop_macro("F")
};

constexpr CharEnum CodeToChar(unsigned char code);

}  // namespace zeta::core::ascii
