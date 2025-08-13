#include <zeta/core/debugger.h>
#include <zeta/core/io.h>
#include <zeta/core/utils.h>

static unsigned char lower_chars[] = "0123456789abcdef";

static unsigned char upper_chars[] = { "0123456789ABCDEF" };

static void Pipe_WriteInt_(Zeta_Core_Pipe pipe, unsigned long long val,
                           unsigned char sign_char, unsigned base,
                           unsigned char const* chars, bool_t just_left,
                           size_t just_width, unsigned char just_char) {
    ZETA_Core_DebugAssert(1 < base);
    ZETA_Core_DebugAssert(base <= 16);

    ZETA_Core_DebugAssert(sign_char == '\0' || sign_char == '-' ||
                          sign_char == '+');

    unsigned char buffer[ZETA_Core_ullong_width];
    unsigned char* buffer_end = buffer + ZETA_Core_ullong_width;
    unsigned char* buffer_i = buffer_end;

    if (val == 0) {
        *(--buffer_i) = chars[0];
    } else {
        for (; 0 < val; val /= base) { *(--buffer_i) = chars[val % base]; }
    }

    if (sign_char != '\0') { *(buffer_i++) = sign_char; }

    size_t width = buffer_end - buffer_i;

    if (just_left) { ZETA_Core_Pipe_Write(pipe, width, buffer_i, 1); }

    if (width < just_width) {
        ZETA_Core_Pipe_Write(pipe, just_width - width, &just_char, 0);
    }

    if (!just_left) { ZETA_Core_Pipe_Write(pipe, width, buffer_i, 1); }
}

void Zeta_Core_Pipe_WriteUInt(Zeta_Core_Pipe pipe, unsigned long long val,
                              bool_t sign, unsigned long long base,
                              bool_t lower_case, bool_t just_left,
                              size_t just_width, unsigned char just_char) {
    Pipe_WriteInt_(pipe, val, sign ? '+' : '\0', base,
                   lower_case ? lower_chars : upper_chars, just_left,
                   just_width, just_char);
}

void Zeta_Core_Pipe_WriteSInt(Zeta_Core_Pipe pipe, long long val, bool_t sign,
                              unsigned long long base, bool_t lower_case,
                              bool_t just_left, size_t just_width,
                              unsigned char just_char) {
    Pipe_WriteInt_(pipe, val < 0 ? -(unsigned long long)val : val,
                   val < 0 ? '-' : (sign ? '+' : '\0'), base,
                   lower_case ? lower_chars : upper_chars, just_left,
                   just_width, just_char);
}

static size_t CountStrLength_(unsigned char const* str) {
    ZETA_Core_DebugAssert(str != NULL);

    size_t ret = 0;

    for (; *str != '\0'; ++str) { ++ret; }

    return ret;
}

void Zeta_Core_Pipe_WriteStr(Zeta_Core_Pipe pipe, unsigned char const* str,
                             bool_t just_left, size_t just_width,
                             unsigned char just_char) {
    ZETA_Core_DebugAssert(str != NULL);

    size_t str_length = CountStrLength_(str);

    if (just_left) { ZETA_Core_Pipe_Write(pipe, str_length, str, 1); }

    if (str_length < just_width) {
        ZETA_Core_Pipe_Write(pipe, just_width - str_length, &just_char, 0);
    }

    if (!just_left) { ZETA_Core_Pipe_Write(pipe, str_length, str, 1); }
}
