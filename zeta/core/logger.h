#pragma once

#include <stdio.h>
#include <zeta/core/io.h>

ZETA_Core_ExternC_Beg;

#define ZETA_Core_LogPos_filename_just_width (48)
#define ZETA_Core_LogPos_line_num_just_width (4)
#define ZETA_Core_LogPos_funcname_just_width (16)
#define ZETA_Core_LogVar_varname_just_width (16)
#define ZETA_Core_LogVar_varval_just_width (24)

#define ZETA_Core_LogPos_(pipe_tmp, filename_tmp, line_num_tmp, fucname_tmp,  \
                          pipe, filename, line_num, funcname, new_line)       \
    {                                                                         \
        Zeta_Core_Pipe pipe_tmp = (pipe);                                     \
        unsigned char const filename_tmp[] = (filename);                      \
        size_t line_num_tmp = (line_num);                                     \
        unsigned char* const funcname_tmp = (unsigned char* const)(funcname); \
                                                                              \
        Zeta_Core_Pipe_WriteStr(pipe_tmp, filename_tmp, ZETA_Core_io_r_just,  \
                                ZETA_Core_LogPos_filename_just_width, ' ');   \
                                                                              \
        ZETA_Core_Pipe_Write(pipe_tmp, 1, ":", 1);                            \
                                                                              \
        Zeta_Core_Pipe_WriteUInt(pipe_tmp, line_num_tmp, FALSE, 10,           \
                                 ZETA_Core_io_l_just,                         \
                                 ZETA_Core_LogPos_line_num_just_width, ' ');  \
                                                                              \
        ZETA_Core_Pipe_Write(pipe_tmp, 1, "\t", 1);                           \
                                                                              \
        Zeta_Core_Pipe_WriteStr(pipe_tmp, funcname_tmp, ZETA_Core_io_l_just,  \
                                ZETA_Core_LogPos_funcname_just_width, ' ');   \
                                                                              \
        if (new_line) { ZETA_Core_Pipe_Write(pipe_tmp, 1, "\n", 1); }         \
    }                                                                         \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_LogPos(pipe, filename, line_num, funcname, new_line)         \
    ZETA_Core_LogPos_(ZETA_Core_TmpName, ZETA_Core_TmpName, ZETA_Core_TmpName, \
                      ZETA_Core_TmpName, (pipe), (filename), (line_num),       \
                      (funcname), (new_line))

#define ZETA_Core_LogCurPos(pipe) \
    ZETA_Core_LogPos((pipe), __FILE__, __LINE__, __PRETTY_FUNCTION__, TRUE)

#define ZETA_Core_LogVar_(pipe_tmp, varname_tmp, varval_tmp, pipe, var)       \
    {                                                                         \
        Zeta_Core_Pipe pipe_tmp = (pipe);                                     \
        unsigned char varname_tmp[] = ZETA_Core_ToStr(var);                   \
        ZETA_Core_AutoVar(varval_tmp, var);                                   \
                                                                              \
        ZETA_Core_LogPos(pipe_tmp, __FILE__, __LINE__, __PRETTY_FUNCTION__,   \
                         FALSE);                                              \
                                                                              \
        ZETA_Core_Pipe_Write(pipe_tmp, 1, "\t", 1);                           \
                                                                              \
        Zeta_Core_Pipe_WriteStr(pipe_tmp, varname_tmp, ZETA_Core_io_r_just,   \
                                ZETA_Core_LogVar_varname_just_width, ' ');    \
                                                                              \
        ZETA_Core_Pipe_Write(pipe_tmp, 3, " = ", 1);                          \
                                                                              \
        _Generic((var),                                                       \
            bool_t: Zeta_Core_Logger_LogBool_(pipe_tmp, &varval_tmp),         \
                                                                              \
            char: Zeta_Core_Logger_LogChar_(pipe_tmp, &varval_tmp),           \
            unsigned char: Zeta_Core_Logger_LogUChar_(pipe_tmp, &varval_tmp), \
            signed char: Zeta_Core_Logger_LogSChar_(pipe_tmp, &varval_tmp),   \
                                                                              \
            unsigned short: Zeta_Core_Logger_LogUShort_(pipe_tmp,             \
                                                        &varval_tmp),         \
            short: Zeta_Core_Logger_LogSShort_(pipe_tmp, &varval_tmp),        \
                                                                              \
            unsigned: Zeta_Core_Logger_LogUInt_(pipe_tmp, &varval_tmp),       \
            int: Zeta_Core_Logger_LogSInt_(pipe_tmp, &varval_tmp),            \
                                                                              \
            unsigned long: Zeta_Core_Logger_LogULong_(pipe_tmp, &varval_tmp), \
            long: Zeta_Core_Logger_LogSLong_(pipe_tmp, &varval_tmp),          \
                                                                              \
            unsigned long long: Zeta_Core_Logger_LogULLong_(pipe_tmp,         \
                                                            &varval_tmp),     \
            long long: Zeta_Core_Logger_LogSLLong_(pipe_tmp, &varval_tmp),    \
                                                                              \
            void*: Zeta_Core_Logger_LogPtr_(pipe_tmp, &varval_tmp),           \
            void const*: Zeta_Core_Logger_LogPtr_(pipe_tmp, &varval_tmp),     \
                                                                              \
            unsigned char*: Zeta_Core_Logger_LogStr_(pipe_tmp, &varval_tmp),  \
            unsigned char const*: Zeta_Core_Logger_LogStr_(pipe_tmp,          \
                                                           &varval_tmp));     \
                                                                              \
        ZETA_Core_Pipe_Write(pipe_tmp, Write, 1, "\n", 1);                    \
    }                                                                         \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_LogVar(pipe, var)                                            \
    ZETA_Core_LogVar_(ZETA_Core_TmpName, ZETA_Core_TmpName, ZETA_Core_TmpName, \
                      (pipe), (var))

void Zeta_Core_Logger_LogBool_(Zeta_Core_Pipe pipe, void const* ptr);

void Zeta_Core_Logger_LogChar_(Zeta_Core_Pipe pipe, void const* ptr);
void Zeta_Core_Logger_LogUChar_(Zeta_Core_Pipe pipe, void const* ptr);
void Zeta_Core_Logger_LogSChar_(Zeta_Core_Pipe pipe, void const* ptr);

void Zeta_Core_Logger_LogUShort_(Zeta_Core_Pipe pipe, void const* ptr);
void Zeta_Core_Logger_LogSShort_(Zeta_Core_Pipe pipe, void const* ptr);

void Zeta_Core_Logger_LogUInt_(Zeta_Core_Pipe pipe, void const* ptr);
void Zeta_Core_Logger_LogSInt_(Zeta_Core_Pipe pipe, void const* ptr);

void Zeta_Core_Logger_LogULong_(Zeta_Core_Pipe pipe, void const* ptr);
void Zeta_Core_Logger_LogSLong_(Zeta_Core_Pipe pipe, void const* ptr);

void Zeta_Core_Logger_LogULLong_(Zeta_Core_Pipe pipe, void const* ptr);
void Zeta_Core_Logger_LogSLLong_(Zeta_Core_Pipe pipe, void const* ptr);

void Zeta_Core_Logger_LogPtr_(Zeta_Core_Pipe pipe, void const* ptr);

void Zeta_Core_Logger_LogStr_(Zeta_Core_Pipe pipe, void const* ptr);

ZETA_Core_ExternC_End;
