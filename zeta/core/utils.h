#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

#define ZETA_Core_IsPower2(x) (__builtin_popcountll(x) == 1)

#define ZETA_Core_FloorLog2_(tmp_x, x)                       \
    ({                                                       \
        ZETA_Core_AutoVar(tmp_x, x);                         \
        ZETA_Core_DebugAssert(0 < tmp_x);                    \
        ZETA_Core_ullong_width - 1 - __builtin_clzll(tmp_x); \
    })

#define ZETA_Core_FloorLog2(x) ZETA_Core_FloorLog2_(ZETA_Core_TmpName, (x))

#define ZETA_CeilLog2_(tmp_x, x)                             \
    ({                                                       \
        ZETA_Core_AutoVar(tmp_x, x);                         \
        ZETA_Core_DebugAssert(0 <= tmp_x);                   \
        tmp_x <= 1 ? 0 : ZETA_Core_FloorLog2(tmp_x - 1) + 1; \
    })

#define ZETA_CeilLog2(x) ZETA_CeilLog2_(ZETA_Core_TmpName, (x))

#define ZETA_Core_GetMinOf2_(tmp_x, tmp_y, x, y) \
    ({                                           \
        ZETA_Core_AutoVar(tmp_x, x);             \
        ZETA_Core_AutoVar(tmp_y, y);             \
        tmp_x <= tmp_y ? tmp_x : tmp_y;          \
    })

#define ZETA_Core_GetMinOf2(x, y) \
    ZETA_Core_GetMinOf2_(ZETA_Core_TmpName, ZETA_Core_TmpName, (x), (y))

#define ZETA_Core_GetMinOf(x, ...) \
    ZETA_Core_ForEach(ZETA_Core_GetMinOf2, x, __VA_ARGS__)

#define ZETA_Core_GetMaxOf2_(tmp_x, tmp_y, x, y) \
    ({                                           \
        ZETA_Core_AutoVar(tmp_x, x);             \
        ZETA_Core_AutoVar(tmp_y, y);             \
        tmp_x < tmp_y ? tmp_y : tmp_x;           \
    })

#define ZETA_Core_GetMaxOf2(x, y) \
    ZETA_Core_GetMaxOf2_(ZETA_Core_TmpName, ZETA_Core_TmpName, (x), (y))

#define ZETA_Core_GetMaxOf(x, ...) \
    ZETA_Core_ForEach(ZETA_Core_GetMaxOf2, x, __VA_ARGS__)

#define ZETA_Core_ThreeWayCompare_(tmp_x, tmp_y, x, y) \
    ({                                                 \
        ZETA_Core_AutoVar(tmp_x, x);                   \
        ZETA_Core_AutoVar(tmp_y, y);                   \
        (tmp_y < tmp_x) - (tmp_x < tmp_y);             \
    })

#define ZETA_Core_ThreeWayCompare(x, y) \
    ZETA_Core_ThreeWayCompare_(ZETA_Core_TmpName, ZETA_Core_TmpName, (x), (y))

#define ZETA_Core_Swap_(tmp_x, tmp_y, tmp, x, y) \
    {                                            \
        ZETA_Core_AutoVar(tmp_x, &(x));          \
        ZETA_Core_AutoVar(tmp_y, &(y));          \
        ZETA_Core_AutoVar(tmp, *tmp_x);          \
        *tmp_x = *tmp_y;                         \
        *tmp_y = tmp;                            \
    }                                            \
    ZETA_Core_StaticAssert(TRUE)

#define ZETA_Core_Swap(x, y)                                                 \
    ZETA_Core_Swap_(ZETA_Core_TmpName, ZETA_Core_TmpName, ZETA_Core_TmpName, \
                    (x), (y))

#define ZETA_Core_ModAddInv_(tmp_y, x, y)      \
    ({                                         \
        ZETA_Core_AutoVar(tmp_y, (y));         \
        tmp_y - 1 - ((x) + tmp_y - 1) % tmp_y; \
    })

#define ZETA_Core_ModAddInv(x, y) \
    ZETA_Core_ModAddInv_(ZETA_Core_TmpName, (x), (y))

#define ZETA_Core_UnsafeCeilIntDiv_(tmp_x, tmp_y, x, y) \
    ({                                                  \
        ZETA_Core_AutoVar(tmp_x, x);                    \
        ZETA_Core_AutoVar(tmp_y, y);                    \
        (tmp_x + tmp_y - 1) / tmp_y;                    \
    })

#define ZETA_Core_UnsafeCeilIntDiv(x, y) \
    ZETA_Core_UnsafeCeilIntDiv_(ZETA_Core_TmpName, ZETA_Core_TmpName, (x), (y))

#define ZETA_Core_CeilIntDiv_(tmp_x, tmp_y, x, y) \
    ({                                            \
        ZETA_Core_AutoVar(tmp_x, x);              \
        ZETA_Core_AutoVar(tmp_y, y);              \
        tmp_x == 0 ? 0 : (tmp_x - 1) / tmp_y + 1; \
    })

#define ZETA_Core_CeilIntDiv(x, y) \
    ZETA_Core_CeilIntDiv_(ZETA_Core_TmpName, ZETA_Core_TmpName, (x), (y))

#define ZETA_Core_RoundIntDiv_(tmp_x, tmp_y, x, y) \
    ({                                             \
        ZETA_Core_AutoVar(tmp_x, x);               \
        ZETA_Core_AutoVar(tmp_y, y);               \
        (tmp_x + tmp_y / 2) / tmp_y;               \
    })

#define ZETA_Core_RoundIntDiv(x, y) \
    ZETA_Core_RoundIntDiv_(ZETA_Core_TmpName, ZETA_Core_TmpName, (x), (y))

#define ZETA_Core_IntRoundDown_(tmp_x, x, y) \
    ({                                       \
        ZETA_Core_AutoVar(tmp_x, (x));       \
        tmp_x - tmp_x % (y);                 \
    })

#define ZETA_Core_IntRoundDown(x, y) \
    ZETA_Core_IntRoundDown_(ZETA_Core_TmpName, (x), (y))

#define ZETA_Core_IntRoundUp_(tmp_x, tmp_y, x, y)                 \
    ({                                                            \
        ZETA_Core_AutoVar(tmp_x, (x));                            \
        ZETA_Core_AutoVar(tmp_y, (y));                            \
        ZETA_Core_DebugAssert(0 < tmp_y);                         \
        tmp_x == 0 ? 0 : tmp_x - 1 - (tmp_x - 1) % tmp_y + tmp_y; \
    })

#define ZETA_Core_IntRoundUp(x, y) \
    ZETA_Core_IntRoundUp_(ZETA_Core_TmpName, ZETA_Core_TmpName, (x), (y))

#define ZETA_Core_FixedPoint_BaseOrder (ZETA_Core_ullong_width * 3 / 8)

#define ZETA_Core_FixedPoint_Base (1ULL << ZETA_Core_FixedPoint_BaseOrder)

#define ZETA_Core_AreOverlapped(a_beg, a_end, b_beg, b_end) \
    (((b_beg) < (a_end)) && ((a_beg) < (b_end)))

void Zeta_Core_HelloWorld();

int Zeta_Core_MemCompare(void const* a, void const* b, size_t size);

/**
 * @brief Copy byte array from dst to src.
 *
 * @param dst The pointer points to destination byte array.
 * @param src The pointer points to source byte array.
 * @param size The number of bytes to be copied.
 */
void Zeta_Core_MemCopy(void* dst, void const* src, size_t size);

/**
 * @brief Copy byte array from dst to src.
 *
 * @param dst The pointer points to destination byte array.
 * @param src The pointer points to source byte array.
 * @param size The number of bytes to be moved.
 */
void Zeta_Core_MemMove(void* dst, void const* src, size_t size);

/**
 * @brief Swap byte array between dst and src
 *
 * @param x The pointer points to the first byte array.
 * @param y The pointer points to the second byte array.
 * @param size The number of bytes to be swapped.
 */
void Zeta_Core_MemSwap(void* x, void* y, size_t size);

/**
 * @brief Rotate the bytes in [beg, end) with mid pivot.
 *
 * @details Let #lv and #rv denote the value of [beg, mid) and [mid, end). The
 * initial value of [beg, end) should be #lv + #rv. After rotation, the value of
 * [beg, end) will become #rv + #lv.
 *
 * @param beg The beginning of byte array.
 * @param mid The pivot of rotation in byte array.
 * @param end The end of byte array.
 *
 * @return The new pivot after rotation.
 */
void* Zeta_Core_MemRotate(void* data, size_t l_size, size_t r_size);

void Zeta_Core_MemReverse(void* data, size_t stride, size_t size);

unsigned long long Zeta_Core_MemHash(void const* data, size_t size,
                                     unsigned long long salt);

int Zeta_Core_ElemCompare(void const* a, void const* b, size_t width,
                          size_t a_stride, size_t b_stride, size_t size);

void Zeta_Core_ElemCopy(void* dst, void const* src, size_t width,
                        size_t dst_stride, size_t src_stride, size_t size);

void Zeta_Core_ElemMove(void* dst, void const* src, size_t width,
                        size_t dst_stride, size_t src_stride, size_t size);

void* Zeta_Core_ElemRotate(void* data, size_t width, size_t stride,
                           size_t l_size, size_t r_size);

// -----------------------------------------------------------------------------

#define ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, len) \
    case (len): tmp_ret = tmp_ret * 256 + tmp_src[(len) - 1]

#define ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, len) \
    case (len): tmp_ret = tmp_ret * 256 + tmp_src[-(len)]

#define ZETA_Core_BytesToUInt_(tmp_ret, tmp_src, tmp_len, tmp_endian,   \
                               val_type, src, len, endian)              \
    ({                                                                  \
        ZETA_Core_StaticAssert(ZETA_Core_WidthOf(val_type) <= 8 * 16);  \
                                                                        \
        ZETA_Core_AutoVar(tmp_src, (src));                              \
        ZETA_Core_DebugAssert(tmp_src != NULL);                         \
                                                                        \
        ZETA_Core_AutoVar(tmp_len, (len));                              \
        ZETA_Core_DebugAssert(0 <= tmp_len);                            \
        ZETA_Core_DebugAssert(tmp_len <= 8);                            \
                                                                        \
        ZETA_Core_AutoVar(tmp_endian, (endian));                        \
        ZETA_Core_DebugAssert(tmp_endian == ZETA_Core_LittleEndian ||   \
                              tmp_endian == ZETA_Core_BigEndian);       \
                                                                        \
        val_type tmp_ret = 0;                                           \
                                                                        \
        if (tmp_endian == ZETA_Core_LittleEndian) {                     \
            switch (tmp_len) {                                          \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x10); \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x0f); \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x0e); \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x0d); \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x0c); \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x0b); \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x0a); \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x09); \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x08); \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x07); \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x06); \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x05); \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x04); \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x03); \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x02); \
                ZETA_Core_BytesToUInt_LE_case_(tmp_ret, tmp_src, 0x01); \
                case 0x0:                                               \
            }                                                           \
        } else {                                                        \
            tmp_src += tmp_len;                                         \
                                                                        \
            switch (tmp_len) {                                          \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x10); \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x0f); \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x0e); \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x0d); \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x0c); \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x0b); \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x0a); \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x09); \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x08); \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x07); \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x06); \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x05); \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x04); \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x03); \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x02); \
                ZETA_Core_BytesToUInt_BE_case_(tmp_ret, tmp_src, 0x01); \
                case 0x0:                                               \
            }                                                           \
        }                                                               \
                                                                        \
        tmp_ret;                                                        \
    })

#define ZETA_Core_BytesToUInt(val_type, src, len, endian)                  \
    ZETA_Core_BytesToUInt_(ZETA_Core_TmpName, ZETA_Core_TmpName,           \
                           ZETA_Core_TmpName, ZETA_Core_TmpName, val_type, \
                           (src), (len), (endian))

// -----------------------------------------------------------------------------

#define ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, len) \
    case (len): tmp_dst[-(len)] = tmp_val % 256; tmp_val /= 256

#define ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, len) \
    case (len): tmp_dst[(len) - 1] = tmp_val % 256; tmp_val /= 256

#define ZETA_Core_UIntToBytes_(tmp_ret, tmp_src, tmp_len, tmp_endian, val, \
                               dst, len, endian)                           \
    ({                                                                     \
        ZETA_Core_AutoVar(tmp_val, (val));                                 \
                                                                           \
        ZETA_Core_AutoVar(tmp_dst, (dst));                                 \
        ZETA_Core_DebugAssert(tmp_dst != NULL);                            \
                                                                           \
        ZETA_Core_AutoVar(tmp_len, (len));                                 \
        ZETA_Core_DebugAssert(0 <= tmp_len);                               \
        ZETA_Core_DebugAssert(tmp_len <= 16);                              \
                                                                           \
        ZETA_Core_AutoVar(tmp_endian, (endian));                           \
        ZETA_Core_DebugAssert(tmp_endian == ZETA_Core_LittleEndian ||      \
                              tmp_endian == ZETA_Core_BigEndian);          \
                                                                           \
        if (tmp_endian == ZETA_Core_LittleEndian) {                        \
            tmp_dst += tmp_len;                                            \
                                                                           \
            switch (tmp_len) {                                             \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x10);    \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x0f);    \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x0e);    \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x0d);    \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x0c);    \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x0b);    \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x0a);    \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x09);    \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x08);    \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x07);    \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x06);    \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x05);    \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x04);    \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x03);    \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x02);    \
                ZETA_Core_UIntToBytes_LE_case_(tmp_val, tmp_dst, 0x01);    \
                case 0x0:                                                  \
            }                                                              \
        } else {                                                           \
            switch (tmp_len) {                                             \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x10);    \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x0f);    \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x0e);    \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x0d);    \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x0c);    \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x0b);    \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x0a);    \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x09);    \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x08);    \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x07);    \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x06);    \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x05);    \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x04);    \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x03);    \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x02);    \
                ZETA_Core_UIntToBytes_BE_case_(tmp_val, tmp_dst, 0x01);    \
                case 0x0:                                                  \
            }                                                              \
        }                                                                  \
    })

#define ZETA_Core_UIntToBytes(val, dst, len, endian)                           \
    ZETA_Core_UIntToBytes_(ZETA_Core_TmpName, ZETA_Core_TmpName,               \
                           ZETA_Core_TmpName, ZETA_Core_TmpName, (val), (dst), \
                           (len), (endian))

// -----------------------------------------------------------------------------

/**
 * @brief Read little endian stored length bytes occupying integer from data.
 *
 * @param src The byte array stores integer.
 * @param length The number of bytes the integer occupied.
 *
 * @return The read integer
 */
u128_t Zeta_Core_ReadLittleEndian(byte_t const* src, unsigned length);

/**
 * @brief Write an integer to data in little endian occupying length bytes.
 *
 * @param dst The destination byte array.
 * @param length The number of bytes the integer occupied.
 * @param val The target integer.
 *
 * @return The remaining value after
 */
u128_t Zeta_Core_WriteLittleEndian(byte_t* dst, u128_t val, unsigned length);

/**
 * @brief Read big endian stored length bytes occupying integer from data.
 *
 * @param src The byte array stores integer.
 * @param length The number of bytes the integer occupied.
 *
 * @return The read integer
 */
u128_t Zeta_Core_ReadBigEndian(byte_t const* src, unsigned length);

/**
 * @brief Write an integer to data in big endian occupying length bytes.
 *
 * @param dst The destination byte array.
 * @param length The number of bytes the integer occupied.
 * @param val The target integer.
 */
u128_t Zeta_Core_WriteBigEndian(byte_t* dst, u128_t val, unsigned length);

byte_t const* Zeta_Core_ReadStr(byte_t const* src, size_t src_size);

int Zeta_Core_FindPrevOne(unsigned long long active_map, int pos);

int Zeta_Core_FindNextOne(unsigned long long active_map, int pos);

unsigned long long Zeta_Core_ULLHash(unsigned long long x,
                                     unsigned long long salt);

unsigned long long Zeta_Core_LLHash(long long x, unsigned long long salt);

unsigned long long Zeta_Core_GetRandom();

unsigned long long Zeta_Core_SimpleRandom(unsigned long long x);

unsigned long long Zeta_Core_SimpleRandomRotate(unsigned long long* x);

unsigned long long Zeta_Core_GCD(unsigned long long x, unsigned long long y);

unsigned long long Zeta_Core_LCM(unsigned long long x, unsigned long long y);

/**
 * @brief Calculate base^exp.
 *
 * @param base The base of base^exp.
 * @param exp The exp of base^exp.
 *
 * @return base^exp.
 */
unsigned long long Zeta_Core_Power(unsigned long long base, unsigned exp);

/**
 * @brief Calculate base^exp % mod.
 *
 * @param base The base of base^exp % mod.
 * @param exp The exp of base^exp % mod.
 * @param mod The mod of base^exp % mod.
 *
 * @return base^exp % mod.
 */
unsigned long long Zeta_Core_PowerMod(unsigned long long base,
                                      unsigned long long exp,
                                      unsigned long long mod);

unsigned long long Zeta_Core_FixedPoint2Power(long long val);

unsigned Zeta_Core_FloorLog(unsigned long long val, unsigned long long base);

unsigned Zeta_Core_CeilLog(unsigned long long val, unsigned long long base);

long long Zeta_Core_FixedPointLog2(unsigned long long val);

unsigned long long Zeta_Core_FloorSqrt(unsigned long long val);

unsigned long long Zeta_Core_CeilSqrt(unsigned long long val);

unsigned long long Zeta_Core_FixedPointSqrt(unsigned long long val);

unsigned long long Zeta_Core_FindNextConMod(unsigned long long beg,
                                            unsigned long long target,
                                            unsigned long long mod);

#define ZETA_Core_FindNextConMod_(tmp_beg, tmp_target, tmp_mod, beg, target, \
                                  mod)                                       \
    ({                                                                       \
        ZETA_Core_AutoVar(tmp_beg, (beg));                                   \
        ZETA_Core_AutoVar(tmp_target, (target));                             \
        ZETA_Core_AutoVar(tmp_mod, (mod));                                   \
                                                                             \
        ZETA_Core_DebugAssert(0 <= tmp_target);                              \
        ZETA_Core_DebugAssert(tmp_target < tmp_mod);                         \
                                                                             \
        tmp_beg + 1 +                                                        \
            (tmp_target + tmp_mod - tmp_beg % tmp_mod - 1) % tmp_mod;        \
    })

#define ZETA_Core_FindNextConMod(beg, target, mod)                  \
    ZETA_Core_FindNextConMod_(ZETA_Core_TmpName, ZETA_Core_TmpName, \
                              ZETA_Core_TmpName, (beg), (target), (mod))

void* Zeta_Core_GetMostLink(void* n, void* (*GetLink)(void* n));

int Zeta_Choose2(bool_t a_cond, bool_t b_cond, unsigned long long* random_seed);

int Zeta_Choose3(bool_t a_cond, bool_t b_cond, bool_t c_cond,
                 unsigned long long* random_seed);

ZETA_Core_ExternC_End;
