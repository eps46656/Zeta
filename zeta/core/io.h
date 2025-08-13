#pragma once

#include <zeta/core/pipe.h>

ZETA_Core_ExternC_Beg;

void Zeta_Core_Pipe_WriteUInt(Zeta_Core_Pipe dst, unsigned long long val,
                              bool_t sign, unsigned long long base,
                              bool_t lower_case, bool_t just_left,
                              size_t just_width, unsigned char just_char);

void Zeta_Core_Pipe_WriteSInt(Zeta_Core_Pipe dst, long long val, bool_t sign,
                              unsigned long long base, bool_t lower_case,
                              bool_t just_left, size_t just_width,
                              unsigned char just_char);

void Zeta_Core_Pipe_WriteStr(Zeta_Core_Pipe dst, unsigned char const* str,
                             bool_t just_left, size_t just_width,
                             unsigned char just_char);

ZETA_Core_ExternC_End;
