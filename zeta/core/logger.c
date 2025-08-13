#include <zeta/core/debugger.h>
#include <zeta/core/logger.h>
#include <zeta/core/pipe.h>
#include <zeta/core/utils.h>

void Zeta_Core_Logger_LogUDec_(Zeta_Core_Pipe pipe, void* ptr_) {
    void* ptr = *((void**)ptr_);

    ZETA_Core_Pipe_Write(pipe, 2, "0x", 1);

    Zeta_Core_Pipe_WriteUInt(      //
        pipe,                      // pipe
        ZETA_Core_PtrToAddr(ptr),  // val
        FALSE,                     // sign
        16,                        // base
        FALSE,                     // lower_case
        FALSE,                     // just_left
        ZETA_Core_UnsafeCeilIntDiv(ZETA_Core_WidthOf(uintptr_t),
                                   4),  // just_width
        '0'                             // just_char
    );
}

void Zeta_Core_Logger_LogBool_(Zeta_Core_Pipe pipe, void const* ptr) {
    Zeta_Core_Pipe_WriteStr(
        pipe, (unsigned char const*)(*((bool_t const*)ptr) ? "True " : "False"),
        TRUE, 0, ' ');
}

void Zeta_Core_Logger_LogChar_(Zeta_Core_Pipe pipe, void const* ptr) {
    Zeta_Core_Pipe_WriteSInt(                //
        pipe,                                // pipe
        *((char const*)ptr),                 // val
        FALSE,                               // sign
        10,                                  // base
        FALSE,                               // lower_case
        FALSE,                               // just_left
        ZETA_Core_LogVar_varval_just_width,  // just_width
        ' '                                  // just_char
    );
}

void Zeta_Core_Logger_LogUChar_(Zeta_Core_Pipe pipe, void const* ptr) {
    Zeta_Core_Pipe_WriteUInt(                //
        pipe,                                // pipe
        *((unsigned char const*)ptr),        // val
        FALSE,                               // sign
        10,                                  // base
        FALSE,                               // lower_case
        FALSE,                               // just_left
        ZETA_Core_LogVar_varval_just_width,  // just_width
        ' '                                  // just_char
    );
}

void Zeta_Core_Logger_LogSChar_(Zeta_Core_Pipe pipe, void const* ptr) {
    Zeta_Core_Pipe_WriteSInt(                //
        pipe,                                // pipe
        *((signed char const*)ptr),          // val
        FALSE,                               // sign
        10,                                  // base
        FALSE,                               // lower_case
        FALSE,                               // just_left
        ZETA_Core_LogVar_varval_just_width,  // just_width
        ' '                                  // just_char
    );
}

void Zeta_Core_Logger_LogUShort_(Zeta_Core_Pipe pipe, void const* ptr) {
    Zeta_Core_Pipe_WriteUInt(                //
        pipe,                                // pipe
        *((unsigned short const*)ptr),       // val
        FALSE,                               // sign
        10,                                  // base
        FALSE,                               // lower_case
        FALSE,                               // just_left
        ZETA_Core_LogVar_varval_just_width,  // just_width
        ' '                                  // just_char
    );
}

void Zeta_Core_Logger_LogSShort_(Zeta_Core_Pipe pipe, void const* ptr) {
    Zeta_Core_Pipe_WriteSInt(                //
        pipe,                                // pipe
        *((short const*)ptr),                // val
        FALSE,                               // sign
        10,                                  // base
        FALSE,                               // lower_case
        FALSE,                               // just_left
        ZETA_Core_LogVar_varval_just_width,  // just_width
        ' '                                  // just_char
    );
}

void Zeta_Core_Logger_LogUInt_(Zeta_Core_Pipe pipe, void const* ptr) {
    Zeta_Core_Pipe_WriteUInt(                //
        pipe,                                // pipe
        *((unsigned const*)ptr),             // val
        FALSE,                               // sign
        10,                                  // base
        FALSE,                               // lower_case
        FALSE,                               // just_left
        ZETA_Core_LogVar_varval_just_width,  // just_width
        ' '                                  // just_char
    );
}

void Zeta_Core_Logger_LogSInt_(Zeta_Core_Pipe pipe, void const* ptr) {
    Zeta_Core_Pipe_WriteSInt(                //
        pipe,                                // pipe
        *((int const*)ptr),                  // val
        FALSE,                               // sign
        10,                                  // base
        FALSE,                               // lower_case
        FALSE,                               // just_left
        ZETA_Core_LogVar_varval_just_width,  // just_width
        ' '                                  // just_char
    );
}

void Zeta_Core_Logger_LogULong_(Zeta_Core_Pipe pipe, void const* ptr) {
    Zeta_Core_Pipe_WriteUInt(                //
        pipe,                                // pipe
        *((unsigned long const*)ptr),        // val
        FALSE,                               // sign
        10,                                  // base
        FALSE,                               // lower_case
        FALSE,                               // just_left
        ZETA_Core_LogVar_varval_just_width,  // just_width
        ' '                                  // just_char
    );
}

void Zeta_Core_Logger_LogSLong_(Zeta_Core_Pipe pipe, void const* ptr) {
    Zeta_Core_Pipe_WriteSInt(                //
        pipe,                                // pipe
        *((long const*)ptr),                 // val
        FALSE,                               // sign
        10,                                  // base
        FALSE,                               // lower_case
        FALSE,                               // just_left
        ZETA_Core_LogVar_varval_just_width,  // just_width
        ' '                                  // just_char
    );
}

void Zeta_Core_Logger_LogULLong_(Zeta_Core_Pipe pipe, void const* ptr) {
    Zeta_Core_Pipe_WriteUInt(                //
        pipe,                                // pipe
        *((unsigned long long const*)ptr),   // val
        FALSE,                               // sign
        10,                                  // base
        FALSE,                               // lower_case
        FALSE,                               // just_left
        ZETA_Core_LogVar_varval_just_width,  // just_width
        ' '                                  // just_char
    );
}

void Zeta_Core_Logger_LogSLLong_(Zeta_Core_Pipe pipe, void const* ptr) {
    Zeta_Core_Pipe_WriteSInt(                //
        pipe,                                // pipe
        *((long long const*)ptr),            // val
        FALSE,                               // sign
        10,                                  // base
        FALSE,                               // lower_case
        FALSE,                               // just_left
        ZETA_Core_LogVar_varval_just_width,  // just_width
        ' '                                  // just_char
    );
}

void Zeta_Core_Logger_LogPtr_(Zeta_Core_Pipe pipe, void const* ptr) {
    ZETA_Core_Pipe_Write(pipe, 2, "0x", 1);

    Zeta_Core_Pipe_WriteUInt(                       //
        pipe,                                       // pipe
        ZETA_Core_PtrToAddr(*((void* const*)ptr)),  // val
        FALSE,                                      // sign
        16,                                         // base
        FALSE,                                      // lower_case
        FALSE,                                      // just_left
        ZETA_Core_LogVar_varval_just_width - 2,     // just_width
        '0'                                         // just_char
    );
}

void Zeta_Core_Logger_LogStr_(Zeta_Core_Pipe pipe, void const* ptr) {
    Zeta_Core_Pipe_WriteStr(pipe, *(unsigned char const**)ptr, TRUE, 0, ' ');
}
