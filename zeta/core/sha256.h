#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

typedef u32_t Zeta_Core_SHA256Hasher_word_t;

ZETA_Core_DeclareStruct(Zeta_Core_SHA256Hasher);

/**
 * @brief The sha256 hasher.
 */
struct Zeta_Core_SHA256Hasher {
    /**
     * The intermediate state of sha256.
     */
    Zeta_Core_SHA256Hasher_word_t hs[8];

    /**
     * The size(byte) of last chunk.
     */
    size_t size;

    /**
     * The last chunk of inputted data.
     */
    byte_t last_chunk[64];
};

/**
 * @brief Get hash result of sha256
 *
 * @param dst The destination of hash result.
 * @param data The target data.
 * @param data_end The end of the target data.
 */
void Zeta_Core_SHA256_Hash(byte_t* dst, byte_t const* src, size_t cnt);

/**
 * @brief Initialize the hasher
 *
 * @param hasher The target hasher.
 */
void Zeta_Core_SHA256Hasher_Init(void* hasher);

void Zeta_Core_SHA256Hasher_Reset(void* hasher);

/**
 * @brief Get the size(byte) of inputted data.
 *
 * @param hasher The target hasher.
 *
 * @return The size(byte) of inputted data.
 */
size_t Zeta_Core_SHA256Hasher_GetSize(void* hasher);

/**
 * @brief Get the size(byte) of result. Always return 32.
 *
 * @param hasher The target hasher.
 *
 * @return The size(byte) of result. Always return 32.
 */
size_t Zeta_Core_SHA256Hasher_GetResultSize(void* hasher);

/**
 * @brief Get the size(byte) of result. Always return 32.
 *
 * @param hasher The target hasher.
 * @param dst The destination of result.
 */
void Zeta_Core_SHA256Hasher_GetResult(void* hasher, byte_t* dst);

/**
 * @brief Append new data after previous inputted data to hash.
 *
 * @param hasher The target hasher.
 * @param data The target data.
 * @param data_end The end of the target data.
 */
void Zeta_Core_SHA256Hasher_Rotate(void* hasher, byte_t const* data,
                                   size_t cnt);

ZETA_Core_ExternC_End;
