#include <memory.h>
#include <stddef.h>
#include <zeta/core/algorithm.h>
#include <zeta/core/cascade_allocator.h>
#include <zeta/core/debugger.h>
#include <zeta/core_test/cascade_alloc_utils.h>
#include <zeta/core_test/random.h>
#include <zeta/core_test/timer.h>

#include <queue>
#include <random>
#include <vector>

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

std::mt19937_64& RandomEngine() {
    static std::mt19937_64 en;
    return en;
}

long long Zeta_CoreTest_GetRandomInt(long long lb, long long rb) {
    static std::uniform_int_distribution<long long> ll_generator{
        0, ZETA_Core_RangeMaxOf(long long)
    };

    return ll_generator(RandomEngine()) % (rb - lb + 1) + lb;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

struct Val {
    static constexpr size_t width{ 16 };

    unsigned char data[width];
};

int ValCompare(const Val& x, const Val& y) {
    for (size_t i{ 0 }; i < Val::width; ++i) {
        if (x.data[i] < y.data[i]) { return -1; }
        if (y.data[i] < x.data[i]) { return 1; }
    }

    return 0;
}

void GetRandomVal(Val& dst) {
    for (size_t i{ 0 }; i < Val::width; ++i) {
        dst.data[i] = (unsigned char)Zeta_CoreTest_GetRandomInt(0, 255);
    }
}

void GetRandomVals(std::vector<Val>& dst) {
    for (size_t i{ 0 }; i < dst.size(); ++i) { GetRandomVal(dst[i]); }
}

struct D {
    Val val;
    char padding[5];
};

int DCompare(void const* context, void const* x, void const* y) {
    ZETA_Core_Unused(context);

    return ValCompare(*(Val const*)x, *(Val const*)y);
};

struct DComparerCpp {
    bool operator()(const D& x, const D& y) const {
        return ValCompare(x.val, y.val) < 0;
    }
};

void main1() {
    unsigned random_seed = time(NULL);
    unsigned fixed_seed = 1721022724;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    Zeta_CoreTest_InitCascadeAllocator();

    Zeta_CoreTest_SetRandomSeed(seed);

    std::vector<D> vec_a;
    std::vector<D> vec_b;

    vec_a.resize(1024 * 16 * 256 - 23);

    unsigned long long beg_time{ GetTime() };

    for (size_t test_i{ 0 }; test_i < 64; ++test_i) {
        ZETA_Core_PrintVar(test_i);

        for (size_t i{ 0 }; i < vec_a.size(); ++i) {
            GetRandomVal(vec_a[i].val);
        }

        vec_b = vec_a;

        std::sort(vec_a.begin(), vec_a.end(), DComparerCpp{});

        Zeta_Core_Sort(&vec_b[0].val, sizeof(Val), sizeof(D), vec_b.size(),
                       NULL, DCompare);

        for (size_t i{ 0 }; i < vec_a.size(); ++i) {
            ZETA_Core_DebugAssert(DCompare(NULL, &vec_a[i], &vec_b[i]) == 0);
        }
    }

    unsigned long long end_time{ GetTime() };

    ZETA_Core_PrintVar(end_time - beg_time);
}

void main2() {
    unsigned random_seed = time(NULL);
    unsigned fixed_seed = 1721022724;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    Zeta_CoreTest_InitCascadeAllocator();

    Zeta_CoreTest_SetRandomSeed(seed);

    std::vector<D> vec_a;

    vec_a.resize(1024 * 1024 * 16);

    for (size_t i{ 0 }; i < vec_a.size(); ++i) { GetRandomVal(vec_a[i].val); }

    unsigned long long beg_time{ GetTime() };

    if (FALSE) {
        std::sort(vec_a.begin(), vec_a.end(), DComparerCpp{});
    } else {
        Zeta_Core_Sort(&vec_a[0].val, sizeof(Val), sizeof(D), vec_a.size(),
                       NULL, DCompare);
    }

    unsigned long long end_time{ GetTime() };

    ZETA_Core_PrintVar(end_time - beg_time);
}

int main() {
    printf("main start\n");

    unsigned long long beg{ GetTime() };

    main1();

    unsigned long long end{ GetTime() };

    printf("ok\n");

    printf("time difference = %llu\n", end - beg);

    return 0;
}
