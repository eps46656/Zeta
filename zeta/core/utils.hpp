#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/type_traits.hpp>

#define ZETA_Core_AreOverlapped(a_beg, a_end, b_beg, b_end) \
    (!((a_end) <= (b_beg) || (b_end) <= (a_beg)))

namespace zeta::core {

template <typename T>
T Declval() {};

// -----------------------------------------------------------------------------
struct Monostate {};

// -----------------------------------------------------------------------------

template <typename First, typename Second>
struct Pair {
    First first;
    Second second;
};

template <typename First, typename Second, typename Third>
struct Triplet {
    First first;
    Second second;
    Third third;
};

// -----------------------------------------------------------------------------

template <size_t N, typename... Args>
decltype(auto) GetNth(Args&&... args);

// -----------------------------------------------------------------------------

template <typename T>
RemoveRef<T>&& Move(T&& t);

template <typename T>
constexpr T&& Forward(RemoveRef<T>& t);

template <typename X, typename Y>
void Swap(X&& x, Y&& y);

// -----------------------------------------------------------------------------

template <typename T0, typename... Ts>
decltype(auto) Min(T0&& x0, Ts&&... xs);

template <typename T0, typename... Ts>
decltype(auto) Max(T0&& x0, Ts&&... xs);

// -----------------------------------------------------------------------------

template <typename X, typename Y>
int ThreeWayCompare(X const& x, Y const& y);

template <typename X, typename Y>
struct ThreeWayCompareCore {
    int operator()(X const& x, Y const& y) const;
};

// -----------------------------------------------------------------------------

template <typename Node, typename GetLinkFunc>
Pair<Node*, size_t> GetMostLink(Node* n, GetLinkFunc const& get_link);

// -----------------------------------------------------------------------------

template <typename Ret, typename Context, typename... Args>
struct ContextualFunction {
    ZETA_Core_StaticAssert(IsSame<Context, void> ||
                           !IsSame<Context, void const>);

    Context* context;
    Ret (*callable)(Context* context, Args... args);

    template <typename Callable>
    static ContextualFunction FromCallable(Callable& callable);

    template <typename... InvokeArgs>
    Ret operator()(InvokeArgs&&... invoke_args) const;
};

// -----------------------------------------------------------------------------

struct FunctionRefKind {
    static constexpr char Func{ 1 };
    static constexpr char ContextualFunc{ 2 };
    static constexpr char ConstContextualFunc{ 3 };
};

template <typename Sig>
struct FunctionRef;

template <typename Ret, typename... Args>
struct FunctionRef<Ret(Args...)> {
    union {
        struct {
            Ret (*ptr)(Args... args);
        } func;

        struct {
            void* context;
            Ret (*ptr)(void* context, Args... args);
        } contextual_func;

        struct {
            void const* context;
            Ret (*ptr)(void const* context, Args... args);
        } const_contextual_func;
    };

    char kind;

    // -------------------------------------------------------------------------

    FunctionRef() = default;

    FunctionRef(FunctionRef const&) = default;

    FunctionRef(FunctionRef&&) = default;

    FunctionRef(Ret (*func)(Args... args));

    FunctionRef(void* context,
                Ret (*contextual_func)(void* context, Args... args));

    template <typename Callable>
    FunctionRef(Callable& callable);

    // -------------------------------------------------------------------------

    Ret operator()(Args... args) const;
};

// -----------------------------------------------------------------------------

using HashFunctionRef = FunctionRef<unsigned long long(void const*)>;

using CompareFunctionRef = FunctionRef<int(void const*, void const*)>;

// -----------------------------------------------------------------------------

int MemCompare(void const* a, void const* b, size_t size);

void MemSwap(void* x, void* y, size_t size);

void MemCopy(void* dst, void const* src, size_t size);

void MemMove(void* dst, void const* src, size_t size);

void* MemRotate(void* data_, size_t l_size, size_t r_size);

unsigned long long MemHash(void const* data, size_t size,
                           unsigned long long salt);

// -----------------------------------------------------------------------------

int ElemCompare(void const* a, void const* b, size_t width, size_t a_stride,
                size_t b_stride, size_t cnt);

void ElemCopy(void* dst, void const* src, size_t width, size_t dst_stride,
              size_t src_stride, size_t cnt);

void ElemMove(void* dst, void const* src, size_t width, size_t dst_stride,
              size_t src_stride, size_t cnt);

unsigned long long ElemHash(void const* data, size_t width, size_t stride,
                            size_t cnt, unsigned long long salt);

void* ElemRotate(void* data, size_t width, size_t stride, size_t l_size,
                 size_t r_size);

// -----------------------------------------------------------------------------

template <typename XIterator, typename YIterator>
int SeqCompare(XIterator x, YIterator y, size_t size);

template <typename DstIterator, typename SrcIterator>
void SeqCopy(DstIterator dst, SrcIterator src, size_t size);

template <typename DstIterator, typename SrcIterator>
void SeqMove(DstIterator dst, SrcIterator src, size_t size);

template <typename Iterator>
Iterator SeqRotate(Iterator beg, Iterator mid, Iterator end);

// -----------------------------------------------------------------------------

constexpr unsigned long long UnsafeUIntCeilDiv(unsigned x, unsigned y);

// -----------------------------------------------------------------------------

constexpr unsigned long long UIntAlignDown(unsigned long long val,
                                           unsigned long long mod);

constexpr unsigned long long UIntAlignUp(unsigned long long val,
                                         unsigned long long mod);

// -----------------------------------------------------------------------------

constexpr int FloorLog2(unsigned long long x);

constexpr int CeilLog2(unsigned long long x);

// -----------------------------------------------------------------------------

constexpr int FindPrevOne(unsigned long long val, int pos);

constexpr int FindNextOne(unsigned long long val, int pos);

// -----------------------------------------------------------------------------

unsigned long long ULLHash(unsigned long long x, unsigned long long salt);
unsigned long long SLLHash(long long x, unsigned long long salt);

unsigned long long GetRandom();

unsigned long long SimpleRandomRotate(unsigned long long* random_seed);

// -----------------------------------------------------------------------------

int Choose2(bool cond0, bool cond1, unsigned long long* random_seed);

int Choose3(bool cond0, bool cond1, bool cond2,
            unsigned long long* random_seed);

}  // namespace zeta::core
