#include <zeta/core/sha256.ipp>
#include <zeta/core/type_identity.hpp>
#include <zeta/core_test/random.hpp>
#include <zeta/core_test/timer.hpp>

constexpr void GenRandomOctets(unsigned char* dst, size_t& octet_cnt,
                               size_t max_octet_cnt) {
    octet_cnt = zeta::core_test::GenUniformRandomInt<size_t>(0, max_octet_cnt);

    zeta::core_test::GenRandomMem(dst, octet_cnt);
}

inline void main1() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 1790008707 };

    // unsigned seed{ random_seed };
    unsigned seed{ fixed_seed };

    ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

    ZETA_Core_DebugUtils_Logging_ImmLogVar(random_seed);
    ZETA_Core_DebugUtils_Logging_ImmLogVar(fixed_seed);
    ZETA_Core_DebugUtils_Logging_ImmLogVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    size_t buffer_octet_cnt{ 0 };
    unsigned char buffer[2048];

    if (false) {
        (GenRandomOctets)(buffer, buffer_octet_cnt, sizeof(buffer));
    } else {
        buffer_octet_cnt = 3;
        buffer[0] = 'a';
        buffer[1] = 'b';
        buffer[2] = 'c';
    }

    zeta::core::sha256::Hasher hasher;

    hasher.Rotate(buffer, buffer_octet_cnt);

    unsigned char digest[zeta::core::sha256::digest_octet_cnt];

    hasher.GetDigest(digest);

    std::cout << std::hex << std::setfill('0');

    for (size_t i{ 0 }; i < zeta::core::sha256::digest_octet_cnt; ++i) {
        std::cout << std::setw(2) << static_cast<int>(digest[i]);
    }

    std::cout << std::dec << std::endl;

    std::cout << zeta::core::type_identity::type_sha<int> << std::endl;
    std::cout << zeta::core::type_identity::type_sha<double> << std::endl;
    std::cout << zeta::core::type_identity::type_sha<long long> << std::endl;
}

int main() {
    unsigned long long beg_time{ zeta::core_test::GetTime() };
    ZETA_Core_DebugUtils_Logging_ImmLogVar(beg_time);

    main1();

    ZETA_Core_DebugUtils_Logging_ImmLogVar(beg_time);

    unsigned long long end_time{ zeta::core_test::GetTime() };
    ZETA_Core_DebugUtils_Logging_ImmLogVar(end_time);

    unsigned long long duration{ end_time - beg_time };
    ZETA_Core_DebugUtils_Logging_ImmLogVar(duration);

    ZETA_Core_DebugUtils_Logging_ImmLogVar("ok");

    return 0;
}
