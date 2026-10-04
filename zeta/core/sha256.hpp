#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>

namespace zeta::core::sha256 {

constexpr size_t chunk_octet_cnt{ 64 };

constexpr size_t digest_octet_cnt{ 32 };

constexpr size_t max_octet_cnt{ integral::RangeMaxOf<size_t> / 8 -
                                chunk_octet_cnt - chunk_octet_cnt };

using Digest = unsigned _BitInt(digest_octet_cnt * 8);

struct Hasher {
    size_t octet_cnt;
    unsigned char buffer[chunk_octet_cnt];

    unsigned _BitInt(32) hs[8];

    constexpr Hasher();

    constexpr Hasher(Hasher const&) = default;

    constexpr void GetDigest(this Hasher& self, unsigned char* dst_digest);

    constexpr void GetDigest(this Hasher& self, Digest& dst_digest);

    constexpr void Rotate(this Hasher& self, unsigned char const* octets,
                          size_t cnt);
};

}  // namespace zeta::core::sha256
