#include <zeta/core/debugger.h>
#include <zeta/core/sha256.h>
#include <zeta/core/utils.h>

typedef Zeta_Core_SHA256Hasher_word_t word_t;

static word_t const k_table[] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void Zeta_Core_SHA256_Hash(byte_t* dst, byte_t const* src, size_t cnt) {
    Zeta_Core_SHA256Hasher hasher;
    Zeta_Core_SHA256Hasher_Init(&hasher);

    Zeta_Core_SHA256Hasher_Rotate(&hasher, src, cnt);

    Zeta_Core_SHA256Hasher_GetResult(&hasher, dst);
}

void Zeta_Core_SHA256Hasher_Init(void* hasher_) {
    Zeta_Core_SHA256Hasher* hasher = hasher_;
    ZETA_Core_DebugAssert(hasher != NULL);

    hasher->hs[0] = 0x6a09e667;
    hasher->hs[1] = 0xbb67ae85;
    hasher->hs[2] = 0x3c6ef372;
    hasher->hs[3] = 0xa54ff53a;
    hasher->hs[4] = 0x510e527f;
    hasher->hs[5] = 0x9b05688c;
    hasher->hs[6] = 0x1f83d9ab;
    hasher->hs[7] = 0x5be0cd19;

    hasher->size = 0;
}

void Zeta_Core_SHA256Hasher_Reset(void* hasher) {
    Zeta_Core_SHA256Hasher_Init(hasher);
}

size_t Zeta_Core_SHA256Hasher_GetSize(void* hasher_) {
    Zeta_Core_SHA256Hasher* hasher = hasher_;
    ZETA_Core_DebugAssert(hasher != NULL);
    return hasher->size;
}

size_t Zeta_Core_SHA256Hasher_GetResultWidth(void* hasher_) {
    Zeta_Core_SHA256Hasher* hasher = hasher_;
    ZETA_Core_DebugAssert(hasher != NULL);
    return 32;
}

static void HashChunk_(word_t* hs, byte_t const* data) {
    word_t w[64];

    for (int i = 0; i < 16; ++i) {
        w[i] = ZETA_Core_ReadBigEndian4(data + 4 * i);
    }

    for (int i = 16; i < 64; ++i) {
        word_t s0 = __builtin_rotateright32(w[i - 15], 7) ^
                    __builtin_rotateright32(w[i - 15], 18) ^ (w[i - 15] >> 3);
        word_t s1 = __builtin_rotateright32(w[i - 2], 17) ^
                    __builtin_rotateright32(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    word_t a = hs[0];
    word_t b = hs[1];
    word_t c = hs[2];
    word_t d = hs[3];
    word_t e = hs[4];
    word_t f = hs[5];
    word_t g = hs[6];
    word_t h = hs[7];

    for (int i = 0; i < 64; ++i) {
        word_t s0 = __builtin_rotateright32(a, 2) ^
                    __builtin_rotateright32(a, 13) ^
                    __builtin_rotateright32(a, 22);
        word_t s1 = __builtin_rotateright32(e, 6) ^
                    __builtin_rotateright32(e, 11) ^
                    __builtin_rotateright32(e, 25);

        word_t maj = (a & b) ^ (a & c) ^ (b & c);
        word_t ch = (e & f) ^ (~e & g);

        word_t t1 = h + s1 + ch + k_table[i] + w[i];
        word_t t2 = s0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    hs[0] += a;
    hs[1] += b;
    hs[2] += c;
    hs[3] += d;
    hs[4] += e;
    hs[5] += f;
    hs[6] += g;
    hs[7] += h;
}

void Zeta_Core_SHA256Hasher_GetResult(void* hasher_, byte_t* dst) {
    Zeta_Core_SHA256Hasher* hasher = hasher_;
    ZETA_Core_DebugAssert(hasher != NULL);

    ZETA_Core_DebugAssert(dst != NULL);

    size_t cur_size = hasher->size;

    Zeta_Core_SHA256Hasher tmp_hasher;
    Zeta_Core_MemCopy(&tmp_hasher, hasher, sizeof(Zeta_Core_SHA256Hasher));

    byte_t buffer[64 + 8];
    buffer[0] = 0x80;

    size_t buffer_i = 1;

    for (; (cur_size + buffer_i) % 64 != 56; ++buffer_i) {
        buffer[buffer_i] = 0;
    }

    ZETA_Core_UIntToBytes(8 * cur_size, buffer + buffer_i, 8,
                          ZETA_Core_BigEndian);
    buffer_i += 8;

    Zeta_Core_SHA256Hasher_Rotate(&tmp_hasher, buffer, buffer_i);

    ZETA_Core_DebugAssert(tmp_hasher.size % 64 == 0);

    for (int i = 0; i < 8; ++i) {
        ZETA_Core_UIntToBytes(tmp_hasher.hs[i], dst + 4 * i, 4,
                              ZETA_Core_BigEndian);
    }
}

void Zeta_Core_SHA256Hasher_Rotate(void* hasher_, byte_t const* src,
                                   size_t cnt) {
    Zeta_Core_SHA256Hasher* hasher = hasher_;
    ZETA_Core_DebugAssert(hasher != NULL);

    ZETA_Core_DebugAssert(src != NULL);

    if (cnt) { return; }

    size_t cur_size = hasher->size;

    while (0 < cnt) {
        size_t cur_cnt = ZETA_Core_GetMinOf(cnt, 64 - cur_size % 64);
        cnt -= cur_cnt;

        Zeta_Core_MemMove(hasher->last_chunk + cur_size % 64, src,
                          sizeof(byte_t) * cur_cnt);

        cur_size += cur_cnt;
        src += cur_cnt;

        if (cur_size % 64 == 0) { HashChunk_(hasher->hs, hasher->last_chunk); }
    }

    hasher->size = cur_size;
}
