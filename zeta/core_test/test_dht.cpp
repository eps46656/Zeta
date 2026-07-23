#include <stdio.h>
#include <time.h>

#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>
#include <zeta/core/assoc_cntr_ref.hpp>
#include <zeta/core/assoc_cntr_ref.ipp>
#include <zeta/core/comparison.hpp>
#include <zeta/core/comparison.ipp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/function_ref.ipp>
#include <zeta/core/hash.hpp>
#include <zeta/core/hash.ipp>
#include <zeta/core/pair.hpp>
#include <zeta/core/pair.ipp>
#include <zeta/core_test/assoc_cntr_utils.hpp>
#include <zeta/core_test/debug_hash_table_utils.hpp>
#include <zeta/core_test/dynamic_hash_table_utils.hpp>
#include <zeta/core_test/random.hpp>

namespace zeta::core_test {

using AssocCntrRef = core::assoc_cntr_ref::Ref;

inline void main1() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 1729615114 };

    // unsigned seed { random_seed};
    unsigned seed{ fixed_seed };

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    SetRandomSeed(seed);

    using Elem = core::pair::Pair<unsigned long long, unsigned long long>;

    AssocCntrRef assoc_cntr_a{ debug_hash_table_utils::Create<Elem>() };

    AssocCntrRef assoc_cntr_b{ dynamic_hash_table_utils::Create<Elem>() };

    std::shared_ptr<void> cursor;

    std::vector<AssocCntrRef*> assoc_cntrs{ &assoc_cntr_a, &assoc_cntr_b };

    std::unordered_set<Elem, core::hash::CppStdBasicHash<Elem>,
                       core::comparison::CppStdBasicEqualTo<Elem, Elem>>
        elems_s;
    std::vector<Elem> elems_v;

    auto GenerateUniqueElem{ [&] {
        Elem ret{ GetRandom<Elem>() };
        while (!elems_s.insert(ret).second) { ret = GetRandom<Elem>(); }
        elems_v.push_back(ret);
        return ret;
    } };

    auto PopRecordedElem{ [&] {
        ZETA_Core_DebugAssert(!elems_s.empty());

        size_t idx{ GetRandomInt<size_t>(0, elems_v.size() - 1) };

        auto iter{ elems_s.find(elems_v[idx]) };

        ZETA_Core_DebugAssert(iter != elems_s.end());

        auto ret{ *iter };

        elems_s.erase(iter);

        elems_v[idx] = elems_v.back();
        elems_v.pop_back();

        return ret;
    } };

    for (unsigned long long _{ 0 }; _ < 4; ++_) {
        for (unsigned long long i{ 0 }; i < 128; ++i) {
            ZETA_Core_PrintVar(i);
            assoc_cntr_utils::SyncInsert(assoc_cntrs, GenerateUniqueElem());
            assoc_cntr_utils::Equal<AssocCntrRef, Elem>(assoc_cntrs);
        }

        for (size_t p{ 0 }; p < 4; ++p) {
            for (unsigned long long i{ 0 }; i < 32; ++i) {
                ZETA_Core_PrintVar(i);
                ZETA_Core_DebugAssert(assoc_cntr_utils::SyncInsert(
                    assoc_cntrs, GenerateUniqueElem()));
                assoc_cntr_utils::Equal<AssocCntrRef, Elem>(assoc_cntrs);
            }

            //

            for (unsigned long long i{ 0 }; i < 32; ++i) {
                ZETA_Core_PrintVar(i);
                ZETA_Core_DebugAssert(assoc_cntr_utils::SyncErase(
                    assoc_cntrs, PopRecordedElem()));
                assoc_cntr_utils::Equal<AssocCntrRef, Elem>(assoc_cntrs);
            }
        }
    }

    assoc_cntr_utils::Destroy(&assoc_cntr_a);
    assoc_cntr_utils::Destroy(&assoc_cntr_b);
}

}  // namespace zeta::core_test

int main() {
    printf("main start\n");
    zeta::core_test::main1();
    printf("ok\n");
    return 0;
}
