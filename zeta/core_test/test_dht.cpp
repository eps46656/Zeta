#include <zeta/core/debug_deque.h>
#include <zeta/core/debugger.h>
#include <zeta/core/mem_check_utils.h>
#include <zeta/core_test/assoc_cntr_utils.h>
#include <zeta/core_test/debug_hash_table_utils.h>
#include <zeta/core_test/dynamic_hash_table_utils.h>
#include <zeta/core_test/pair.h>
#include <zeta/core_test/pod_value.h>
#include <zeta/core_test/random.h>
#include <zeta/core_test/std_allocator.h>
#include <zeta/core_test/timer.h>

#include <memory>
#include <unordered_set>

void main1() {
    unsigned random_seed = time(NULL);
    unsigned fixed_seed = 1729615114;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    Zeta_CoreTest_SetRandomSeed(seed);

    using Elem = Pair<unsigned long long, unsigned long long>;

    Zeta_Core_AssocCntr assoc_cntr_a =
        Zeta_CoreTest_DebugHashTable_Create<Elem>();

    Zeta_Core_AssocCntr assoc_cntr_b = DynamicHashTable_Create<Elem>();

    std::shared_ptr<void> cursor;

    std::vector<Zeta_Core_AssocCntr> assoc_cntrs{ assoc_cntr_a, assoc_cntr_b };

    std::unordered_set<Elem> elems_s;
    std::vector<Elem> elems_v;

    auto GenerateUniqueElem{ [&] {
        Elem ret{ GetRandom<Elem>() };
        while (!elems_s.insert(ret).second) { ret = GetRandom<Elem>(); }
        elems_v.push_back(ret);
        return ret;
    } };

    auto PopRecordedElem = [&] {
        ZETA_Core_DebugAssert(!elems_s.empty());

        size_t idx{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(
            0, elems_v.size() - 1) };

        auto iter{ elems_s.find(elems_v[idx]) };

        ZETA_Core_DebugAssert(iter != elems_s.end());

        auto ret{ *iter };

        elems_s.erase(iter);

        elems_v[idx] = elems_v.back();
        elems_v.pop_back();

        return ret;
    };

    for (unsigned long long _{ 0 }; _ < 4; ++_) {
        for (unsigned long long i{ 0 }; i < 128; ++i) {
            ZETA_Core_PrintVar(i);
            AssocCntrUtils_SyncInsert<Elem>(assoc_cntrs, GenerateUniqueElem());
            AssocCntrUtils_Equal<Elem>(assoc_cntrs);
        }

        for (size_t p{ 0 }; p < 4; ++p) {
            for (unsigned long long i{ 0 }; i < 32; ++i) {
                ZETA_Core_PrintVar(i);
                ZETA_Core_DebugAssert(AssocCntrUtils_SyncInsert<Elem>(
                    assoc_cntrs, GenerateUniqueElem()));
                AssocCntrUtils_Equal<Elem>(assoc_cntrs);
            }

            for (unsigned long long i{ 0 }; i < 32; ++i) {
                ZETA_Core_PrintVar(i);
                ZETA_Core_DebugAssert(AssocCntrUtils_SyncErase<Elem>(
                    assoc_cntrs, PopRecordedElem()));
                AssocCntrUtils_Equal<Elem>(assoc_cntrs);
            }
        }
    }

    AssocCntrUtils_Destroy(assoc_cntr_a);
    AssocCntrUtils_Destroy(assoc_cntr_b);
}

int main() {
    printf("main start\n");
    main1();
    printf("ok\n");
    return 0;
}
