
#include <zeta/core/debugger.h>
#include <zeta/core/define.h>

#include <iostream>
#include <random>
#include <vector>

#define PRINT_VAR(var)                                                 \
    std::cout << __FILE__ << ":" << __LINE__ << "\t\t" << #var << ": " \
              << (var) << "\n";

std::mt19937_64 en;

std::vector<unichar_t> StrToVec(std::string const& str) {
    return { str.begin(), str.end() };
}

std::ostream& operator<<(std::ostream& os, std::vector<unichar_t> const& vec) {
    size_t size{ vec.size() };
    for (size_t i{ 0 }; i < size; ++i) { os << (char)vec[i]; }
    return os;
}

std::ostream& operator<<(std::ostream& os, std::vector<size_t> const& vec) {
    size_t size{ vec.size() };
    for (size_t i{ 0 }; i < size; ++i) { os << vec[i] << " "; }
    return os;
}

template <typename Iterator>
std::vector<long long> GetLPSTableBasic(Iterator beg, Iterator end) {
    long long n{ static_cast<long long>(end - beg) };

    ZETA_Core_DebugAssert(0 <= n);

    std::vector<long long> ret;
    ret.resize(n + 1);

    ret[0] = -1;

    for (long long i{ 1 }; i < n; ++i) {
        for (long long j{ 1 }; j <= i; ++j) {
            long long l{ i - j };

            bool_t match{ TRUE };

            for (long long k{ 0 }; k < l; ++k) {
                if (str[j + k] != str[k]) {
                    match = FALSE;
                    break;
                }
            }

            if (match) {
                ret[i] = l;
                break;
            }
        }
    }

    return ret;
}

template <typename Container>
std::vector<long long> GetLPSTableBasic(const Container& cntr) {
    return GetLPSTableBasic(cntr.begin(), cntr.end());
}

template <typename Iterator>
std::vector<long long> GetLPSTable(Iterator beg, Iterator end) {
    long long n{ static_cast<long long>(end - beg) };

    std::vector<long long> ret;
    ret.resize(n + 1);

    ret[0] = -1;
    ret[1] = 0;

    for (long long i{ 2 }, l{ 0 }; i < n;) {
        if (beg[l] == beg[i - 1]) {
            ret[i++] = ++l;
        } else if (l == 0) {
            ret[i++] = 0;
        } else {
            l = ret[l];
        }
    }

    return ret;
}

template <typename Container>
std::vector<long long> GetLPSTable(const Container& cntr) {
    return GetLPSTable(cntr.begin(), cntr.end());
}

template <typename IteratorA, typename IteratorB>
long long GetMatchingCnt(IteratorA beg_a, IteratorB beg_b, long long n) {
    long long ret{ 0 };

    for (; 0 < n-- && *beg_a == *beg_b; ++beg_a, ++beg_b) { ++ret; }

    return ret;
}

template <typename Iterator>
std::vector<long long> GetLPPTableBasic(Iterator beg, Iterator end) {
    long long n{ static_cast<long long>(end - beg) };

    ZETA_Core_DebugAssert(0 <= n);

    if (n == 0) { return {}; }

    std::vector<long long> ret;
    ret.resize(n);

    ret[0] = n;

    for (long long i{ 1 }; i < n; ++i) {
        ret[i] = GetMatchingCnt(beg + i, beg, n - i);
    }
}

template <typename Container>
std::vector<long long> GetLPPTableBasic(const Container& cntr) {
    return GetLPPTableBasic(cntr.begin(), cntr.end());
}

template <typename Iterator>
std::vector<long long> GetLPPTable(Iterator beg, Iterator end) {
    long long n{ static_cast<long long>(end - beg) };

    ZETA_Core_DebugAssert(0 <= n);

    if (n == 0) { return {}; }

    std::vector<long long> ret;
    ret.resize(n);

    ret[0] = n;

    for (long long p_beg{ 1 }, p_end{ 1 }, i{ 1 }; i < n; ++i) {
        if (p_end < i) {
            p_beg = i;
            p_end = i;
        }

        long long i_end{ i + ret[i - p_beg] };

        if (i_end < p_end) {
            ret[i] = i_end - i;
            continue;
        }

        p_beg = i;

        while (p_end < n && dat[p_end - p_beg] == dat[p_end]) { ++p_end; }

        ret[i] = p_end - i;
    }
}

template <typename Container>
std::vector<long long> GetLPPTable(const Container& cntr) {
    return GetLPPTable(cntr.begin(), cntr.end());
}

std::vector<size_t> GetLSPBasic(const std::vector<unichar_t>& str) {
    size_t size{ str.size() };

    std::vector<size_t> ret;
    ret.resize(size, 0);

    for (size_t i{ size - 1 }; 0 < i--;) {
        for (size_t s{ size - i - 1 }; 0 < s; --s) {
            bool_t match{ TRUE };

            for (size_t j{ 0 }; j < s; ++j) {
                if (str[i + j] != str[size - s + j]) {
                    match = FALSE;
                    break;
                }
            }

            if (match) {
                ret[i] = s;
                break;
            }
        }
    }

    return ret;
}

std::vector<size_t> GetLSP(const std::vector<unichar_t>& str) {
    size_t size{ str.size() };

    std::vector<size_t> ret;
    ret.resize(size);

    ret[size - 1] = 0;

    for (size_t i{ size - 1 }; 0 < i--;) {
        size_t lsp{ ret[i + 1] };

        for (;;) {
            if (str[size - lsp - 1] == str[i]) {
                ret[i] = lsp + 1;
                break;
            }

            if (lsp == 0) {
                ret[i] = 0;
                break;
            }

            lsp = ret[size - lsp];
        }
    }

    return ret;
}

void main1() {
    unsigned seed = time(NULL);
    ZETA_Core_PrintVar(seed);

    en.seed(seed);

    std::uniform_int_distribution<int> size_generator{ 0, 1024 * 1024 * 1024 };

    std::vector<unichar_t> str{ StrToVec("1234512378") };

    for (size_t test_i{ 0 }; test_i < 16; ++test_i) {
        str.resize(0);

        for (size_t i{ 0 }; i < 512; ++i) {
            str.push_back('a' + size_generator(en) % 2);
        }

        // PRINT_VAR(str);

        std::vector<long long> lps_basic{ GetLPSTableBasic(str) };
        std::vector<long long> lps{ GetLPSTable(str) };

        // PRINT_VAR(lps_basic);
        // PRINT_VAR(lps);

        ZETA_Core_DebugAssert(lps_basic == lps);
    }
}

void main2() {
    unsigned seed = time(NULL);
    ZETA_Core_PrintVar(seed);

    en.seed(seed);

    std::uniform_int_distribution<int> size_generator{ 0, 1024 * 1024 * 1024 };

    std::vector<unichar_t> str{ StrToVec("1234512378") };

    for (size_t test_i{ 0 }; test_i < 16; ++test_i) {
        str.resize(0);

        for (size_t i{ 0 }; i < 512; ++i) {
            str.push_back('a' + size_generator(en) % 2);
        }

        PRINT_VAR(str);

        std::vector<long long> lpp_basic{ GetLPPTableBasic(str) };

        std::vector<long long> lpp{ GetLPPTable(str) };

        ZETA_Core_DebugAssert(lpp_basic == lpp);
    }
}

int main() {
    main2();
    printf("ok\n");
    return 0;
}
