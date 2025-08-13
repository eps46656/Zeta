#include <zeta/core/debugger.h>
#include <zeta/core/utils.h>

#include <random>

std::mt19937_64 en;

std::uniform_int_distribution<unsigned long long> ull_generator;

unsigned long long randint(unsigned long long lb, unsigned long long rb) {
    ZETA_Core_DebugAssert(lb <= rb);
    return ull_generator(en) % (rb - lb + 1ULL) + lb;
}

void main1() {
    unsigned random_seed = time(NULL);
    unsigned fixed_seed = 1718129024;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    en.seed(seed);

    unsigned long long test_num = 1000 * 1000;

    for (unsigned long long test_i = 0; test_i < test_num; ++test_i) {
        unsigned long long base = randint(2, 32);
        unsigned long long val = randint(2, ZETA_ULLONG_MAX / 64);

        unsigned long long exp = Zeta_Core_FloorLog(val, base);

        unsigned long long x = Zeta_Power(base, exp);

        // ZETA_Core_PrintVar(base);
        // ZETA_Core_PrintVar(val);

        ZETA_Core_DebugAssert(x <= val);
        ZETA_Core_DebugAssert(val < x * base);

        ZETA_Core_PrintVar(test_i);
    }
}

void main2() {
    unsigned random_seed = time(NULL);
    unsigned fixed_seed = 1718129024;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    en.seed(seed);

    unsigned long long test_num = 1000 * 1000;

    for (unsigned long long test_i = 0; test_i < test_num; ++test_i) {
        unsigned long long base = randint(2, 32);
        unsigned long long val = randint(2, ZETA_ULLONG_MAX / 64);

        unsigned long long exp = Zeta_Core_CeilLog(val, base);

        unsigned long long x = Zeta_Power(base, exp);

        ZETA_Core_DebugAssert(val <= x);
        ZETA_Core_DebugAssert(x / base < val);

        ZETA_Core_PrintVar(test_i);
    }
}

void main3() {
    unsigned random_seed = time(NULL);
    unsigned fixed_seed = 1718129024;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    en.seed(seed);

    unsigned long long test_num = 1000 * 1000;

    for (unsigned long long test_i = 0; test_i < test_num; ++test_i) {
        unsigned long long base = 2;
        unsigned long long val = randint(2, ZETA_ULLONG_MAX);

        unsigned long long exp1 = ZETA_Core_FloorLog2(val);

        unsigned long long exp2 = Zeta_Core_FloorLog(val, base);

        // ZETA_Core_PrintVar(val);
        // ZETA_Core_PrintVar(exp1);
        // ZETA_Core_PrintVar(exp2);

        ZETA_Core_DebugAssert(exp1 == exp2);

        ZETA_Core_PrintVar(test_i);
    }
}

void main4() {
    unsigned random_seed = time(NULL);
    unsigned fixed_seed = 1718129024;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    en.seed(seed);

    unsigned long long test_num = 1000 * 1000;

    for (unsigned long long test_i = 0; test_i < test_num; ++test_i) {
        unsigned long long val = randint(1, ZETA_ULLONG_MAX / (1024 * 1024));

        ZETA_Core_PrintCurPos;

        unsigned long long x = Zeta_Core_FloorSqrt(val);

        ZETA_Core_PrintCurPos;

        ZETA_Core_DebugAssert(x * x <= val);
        ZETA_Core_DebugAssert(val / (x + 1) < x + 1);

        ZETA_Core_PrintVar(test_i);
    }
}

void main5() {
    unsigned random_seed = time(NULL);
    unsigned fixed_seed = 1718129024;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    en.seed(seed);

    unsigned long long test_num = 1000 * 1000;

    for (unsigned long long test_i = 0; test_i < test_num; ++test_i) {
        // unsigned long long val = randint(0, ZETA_ULLONG_MAX / (1024 * 1024));

        unsigned long long k = randint(0, 1024 * 1024);

        unsigned long long val = k * k;

        ZETA_Core_PrintCurPos;

        unsigned long long x = Zeta_Core_CeilSqrt(val);

        ZETA_Core_DebugAssert(val <= x * x);

        if (0 < x) { ZETA_Core_DebugAssert((x - 1) * (x - 1) < val); }

        ZETA_Core_PrintVar(test_i);
    }
}

int main() {
    main5();
    printf("ok\n");
    return 0;
}
