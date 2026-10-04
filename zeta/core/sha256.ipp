#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/sha256.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

namespace sha256::detail {

constexpr unsigned _BitInt(32) ks[]{
    0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,  //
    0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,  //
    0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,  //
    0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,  //
    0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,  //
    0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,  //
    0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,  //
    0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,  //
    0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,  //
    0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,  //
    0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,  //
    0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,  //
    0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,  //
    0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,  //
    0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,  //
    0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2,  //
};

constexpr void ChunkRotate_(unsigned _BitInt(32) * hs,
                            unsigned char const* chunk) {
    unsigned _BitInt(32) ws[64];

    for (unsigned i{ 0 }; i < 16; ++i) {
        unsigned _BitInt(32) w{ 0 };

        for (unsigned j{ 0 }; j < 4; ++j) {
            w <<= 8;
            w += chunk[i * 4 + j];
        }

        ws[i] = w;
    }

    for (unsigned i{ 16 }; i < 64; ++i) {
        unsigned _BitInt(32) w0{ ws[i - 15] };
        unsigned _BitInt(32) w1{ ws[i - 2] };

        unsigned _BitInt(32) s0{ __builtin_rotateright32(w0, 7) ^
                                 __builtin_rotateright32(w0, 18) ^ (w0 >> 3) };

        unsigned _BitInt(32) s1{ __builtin_rotateright32(w1, 17) ^
                                 __builtin_rotateright32(w1, 19) ^ (w1 >> 10) };

        ws[i] = ws[i - 16] + s0 + ws[i - 7] + s1;
    }

    unsigned _BitInt(32) a{ hs[0] };
    unsigned _BitInt(32) b{ hs[1] };
    unsigned _BitInt(32) c{ hs[2] };
    unsigned _BitInt(32) d{ hs[3] };
    unsigned _BitInt(32) e{ hs[4] };
    unsigned _BitInt(32) f{ hs[5] };
    unsigned _BitInt(32) g{ hs[6] };
    unsigned _BitInt(32) h{ hs[7] };

    for (unsigned i{ 0 }; i < 64; ++i) {
        unsigned _BitInt(32)
            s0{ __builtin_rotateright32(a, 2) ^ __builtin_rotateright32(a, 13) ^
                __builtin_rotateright32(a, 22) };

        unsigned _BitInt(32) maj{ (a & b) ^ (a & c) ^ (b & c) };

        unsigned _BitInt(32) t2{ s0 + maj };

        unsigned _BitInt(32)
            s1{ __builtin_rotateright32(e, 6) ^ __builtin_rotateright32(e, 11) ^
                __builtin_rotateright32(e, 25) };

        unsigned _BitInt(32) ch{ (e & f) ^ ((~e) & g) };

        unsigned _BitInt(32) t1{ h + s1 + ch + ks[i] + ws[i] };

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

constexpr void CheckHasher_(Hasher const& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.octet_cnt <= max_octet_cnt);
}

constexpr void HasherRotate_(Hasher& hasher, unsigned char const* octets,
                             size_t cnt) {
    size_t buffer_octet_cnt{ hasher.octet_cnt % 64 };

    hasher.octet_cnt += cnt;

    if (0 < buffer_octet_cnt) {
        size_t buffer_res_octet_cnt{ chunk_octet_cnt - buffer_octet_cnt };

        if (cnt < buffer_res_octet_cnt) {
            utils::MemCopy(hasher.buffer + buffer_octet_cnt, octets, cnt);
            return;
        }

        utils::MemCopy(hasher.buffer + buffer_octet_cnt, octets,
                       buffer_res_octet_cnt);

        octets += buffer_res_octet_cnt;
        cnt -= buffer_res_octet_cnt;

        (ChunkRotate_)(hasher.hs, hasher.buffer);
    }

    for (; chunk_octet_cnt <= cnt;
         cnt -= chunk_octet_cnt, octets += chunk_octet_cnt) {
        (ChunkRotate_)(hasher.hs, octets);
    }

    if (0 < cnt) { utils::MemCopy(hasher.buffer, octets, cnt); }
}

constexpr void GetDigest_(Hasher const& hasher, unsigned _BitInt(32) * dst_hs) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_hs != nullptr);

    size_t buffer_res_octet_cnt{ chunk_octet_cnt -
                                 hasher.octet_cnt % chunk_octet_cnt };

    Hasher tmp{ hasher };

    constexpr unsigned char val0x80{ 0x80 };

    detail::HasherRotate_(tmp, &val0x80, 1);

    constexpr unsigned char zeros[chunk_octet_cnt + chunk_octet_cnt]{ 0 };

    if (buffer_res_octet_cnt < 9) {
        detail::HasherRotate_(tmp, zeros,
                              buffer_res_octet_cnt + chunk_octet_cnt - 9);
    } else {
        detail::HasherRotate_(tmp, zeros, buffer_res_octet_cnt - 9);
    }

    unsigned char len_octets[8];

    {
        size_t k{ hasher.octet_cnt * 8 };

        for (unsigned i{ 0 }; i < 8; ++i) {
            len_octets[7 - i] = k & 0xFF;
            k >>= 8;
        }
    }

    detail::HasherRotate_(tmp, len_octets, 8);

    for (unsigned i{ 0 }; i < 8; ++i) { dst_hs[i] = tmp.hs[i]; }
}

}  // namespace sha256::detail

constexpr sha256::Hasher::Hasher()
    : octet_cnt{ 0 },
      buffer{},
      hs{
          0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,  //
          0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19,  //
      } {}

constexpr void sha256::Hasher::GetDigest(this Hasher& self,
                                         unsigned char* dst_digest) {
    detail::CheckHasher_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_digest != nullptr);

    unsigned _BitInt(32) tmp_hs[8];

    detail::GetDigest_(self, tmp_hs);

    for (unsigned i{ 0 }; i < 8; ++i) {
        unsigned _BitInt(32) h{ tmp_hs[i] };

        dst_digest += 4;

        for (unsigned j{ 0 }; j < 4; ++j) {
            *(--dst_digest) = h & 0xFF;
            h >>= 8;
        }

        dst_digest += 4;
    }
}

constexpr void sha256::Hasher::GetDigest(this Hasher& self,
                                         Digest& dst_digest) {
    detail::CheckHasher_(self);

    unsigned _BitInt(32) tmp_hs[8];

    detail::GetDigest_(self, tmp_hs);

    Digest tmp_digest{ 0 };

    for (unsigned i{ 0 }; i < 8; ++i) {
        tmp_digest <<= 32;
        tmp_digest += tmp_hs[i];
    }

    dst_digest = tmp_digest;
}

constexpr void sha256::Hasher::Rotate(this Hasher& self,
                                      unsigned char const* octets, size_t cnt) {
    detail::CheckHasher_(self);

    if (cnt == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(octets != nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <=
                                            max_octet_cnt - self.octet_cnt);

    detail::HasherRotate_(self, octets, cnt);
}

}  // namespace zeta::core
