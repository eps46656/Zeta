#include <zeta/core/allocator.hpp>
#include <zeta/core/allocator.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/pool_allocator.hpp>

ZETA_Core_ClangdPreambleBarrier;

#pragma push_macro("AllocatorTplParamList")
#define AllocatorTplParamList                               \
    typename ReuseStrategyTag, typename ReleaseStrategyTag, \
        typename SrcAllocatorLike

#pragma push_macro("AllocatorTplArgList")
#define AllocatorTplArgList \
    ReuseStrategyTag, ReleaseStrategyTag, SrcAllocatorLike

namespace zeta::core {

namespace pool_allocator::detail {

template <AllocatorTplParamList>
constexpr void Check_  // NOLINT(misc-use-internal-linkage)
    (Allocator<AllocatorTplArgList>& pa) {
    if (meta::GetValueWrapperValue<ReleaseStrategyTag> !=
        ReleaseStrategy::Never) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < pa.capacity);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(pa.cnt <= pa.capacity);
    }

    if (pa.head == nullptr) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(pa.tail == nullptr);

        if (meta::GetValueWrapperValue<ReleaseStrategyTag> !=
            ReleaseStrategy::Never) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(pa.cnt == 0);
        }
    } else if (pa.head == pa.tail) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(pa.cnt == 1);
    }
}

}  // namespace pool_allocator::detail

template <AllocatorTplParamList>
constexpr pool_allocator::Allocator<AllocatorTplArgList>::Allocator()
    requires meta::IsSame<SrcAllocatorLike, void>
{
    if constexpr (release_strategy != ReleaseStrategy::Never) { this->cnt = 0; }

    this->head = nullptr;
    this->tail = nullptr;
}

template <AllocatorTplParamList>
template <typename SrcAllocatorLikeConstructArg>
constexpr pool_allocator::Allocator<AllocatorTplArgList>::Allocator(
    SrcAllocatorLikeConstructArg&& src_allocator_like_construct_arg)
    requires(!meta::IsSame<SrcAllocatorLike, void>)
    : AllocatorBase<ReuseStrategyTag, ReleaseStrategyTag, SrcAllocatorLike>{
          meta::Forward<SrcAllocatorLikeConstructArg>(
              src_allocator_like_construct_arg)
      } {
    if constexpr (release_strategy != ReleaseStrategy::Never) { this->cnt = 0; }

    this->head = nullptr;
    this->tail = nullptr;
}

template <AllocatorTplParamList>
constexpr pool_allocator::Allocator<AllocatorTplArgList>::~Allocator() {
    detail::Check_(*this);

    if constexpr (meta::IsSame<SrcAllocatorLike, void>) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(this->head == nullptr);
    } else if (this->head != nullptr) {
        this->Release(integral::RangeMaxOf<size_t>);
    }
}

template <AllocatorTplParamList>
constexpr size_t
pool_allocator::Allocator<AllocatorTplArgList>::Allocator::GetAlign(
    this Allocator const& self) {
    detail::Check_(self);

    return alignof(void*);
}

namespace pool_allocator::detail {

constexpr void PushTail_  // NOLINT(misc-use-internal-linkage)
    (void*& head, void*& tail, void* ptr) {
    if (tail == nullptr) {
        head = ptr;
        tail = ptr;
        return;
    }

    *static_cast<uintptr_t*>(tail) ^=
        reinterpret_cast<uintptr_t>(ptr) ^ reinterpret_cast<uintptr_t>(head);

    *static_cast<uintptr_t*>(ptr) =
        reinterpret_cast<uintptr_t>(tail) ^ reinterpret_cast<uintptr_t>(head);

    *static_cast<uintptr_t*>(head) ^=
        reinterpret_cast<uintptr_t>(tail) ^ reinterpret_cast<uintptr_t>(ptr);

    tail = ptr;
}

constexpr void PopHead_  // NOLINT(misc-use-internal-linkage)
    (void*& head, void*& tail) {
    void* head_nxt{
        reinterpret_cast<void*>  // NOLINT(performance-no-int-to-ptr)
        (reinterpret_cast<uintptr_t>(tail) ^ *static_cast<uintptr_t*>(head))
    };

    *static_cast<uintptr_t*>(tail) ^= reinterpret_cast<uintptr_t>(head) ^
                                      reinterpret_cast<uintptr_t>(head_nxt);

    *static_cast<uintptr_t*>(head_nxt) ^=
        reinterpret_cast<uintptr_t>(tail) ^ reinterpret_cast<uintptr_t>(head);

    head = head_nxt;
}

constexpr void PopTail_  // NOLINT(misc-use-internal-linkage)
    (void*& head, void*& tail) {
    void* tail_prv{
        reinterpret_cast<void*>  // NOLINT(performance-no-int-to-ptr)
        (*static_cast<uintptr_t*>(tail) ^ reinterpret_cast<uintptr_t>(head))
    };

    *static_cast<uintptr_t*>(tail_prv) ^=
        reinterpret_cast<uintptr_t>(tail) ^ reinterpret_cast<uintptr_t>(head);

    *static_cast<uintptr_t*>(head) ^= reinterpret_cast<uintptr_t>(tail_prv) ^
                                      reinterpret_cast<uintptr_t>(tail);

    tail = tail_prv;
}

}  // namespace pool_allocator::detail

template <AllocatorTplParamList>
constexpr void*
pool_allocator::Allocator<AllocatorTplArgList>::Allocator::Allocate(
    this Allocator& self, size_t size) {
    detail::Check_(self);

    if (size == 0) { return nullptr; }

    void* head{ self.head };
    void* tail{ self.tail };

    if (head == nullptr) {
        if constexpr (!meta::IsSame<SrcAllocatorLike, void>) {
            return allocator::Allocate(self.src_alctr, size);
        }

        return nullptr;
    }

    if constexpr (release_strategy != ReleaseStrategy::Never) { --self.cnt; }

    if (head == tail) {
        self.head = nullptr;
        self.tail = nullptr;
        return head;
    }

    void* ret;

    if constexpr (reuse_strategy == ReuseStrategy::Oldest) {
        ret = head;
        detail::PopHead_(head, tail);
        self.head = head;
    } else if constexpr (reuse_strategy == ReuseStrategy::Latest) {
        ret = tail;
        detail::PopTail_(head, tail);
        self.tail = tail;
    } else {
        static_assert(false);
    }

    return ret;
}

template <AllocatorTplParamList>
constexpr void
pool_allocator::Allocator<AllocatorTplArgList>::Allocator::Deallocate(
    this Allocator& self, void* ptr) {
    detail::Check_(self);

    if (ptr == nullptr) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        __builtin_is_aligned(ptr, alignof(void*)));

    void* head{ self.head };
    void* tail{ self.tail };
    size_t capacity{ self.capacity };

    if (head == nullptr) {
        self.head = ptr;
        self.tail = ptr;

        if (release_strategy != ReleaseStrategy::Never) { self.cnt = 1; }

        *static_cast<uintptr_t*>(ptr) = 0;
        return;
    }

    if constexpr (release_strategy != ReleaseStrategy::Latest) {
        if (self.cnt == capacity) {
            allocator::Deallocate(self.src_alctr, ptr);
            return;
        }

        ++self.cnt;
    }

    detail::PushTail_(head, tail, ptr);

    if constexpr (release_strategy != ReleaseStrategy::Oldest) {
        if (self.cnt == capacity) {
            void* n{ head };
            detail::PopHead_(head, tail);
            allocator::Deallocate(self.src_alctr, n);
        } else {
            ++self.cnt;
        }
    }

    self.head = head;
    self.tail = tail;
}

template <AllocatorTplParamList>
constexpr void
pool_allocator::Allocator<AllocatorTplArgList>::Allocator::Release(
    this Allocator& self, size_t cnt)
    requires(release_strategy != ReleaseStrategy::Never)
{
    detail::Check_(self);

    void* head{ self.head };
    void* tail{ self.tail };

    self.cnt -= Min(cnt, self.cnt);

    for (; 0 < cnt && head != nullptr; --cnt) {
        if (head == tail) {
            head = nullptr;
            tail = nullptr;
            continue;
        }

        void* n;

        if constexpr (release_strategy == ReleaseStrategy::Oldest) {
            n = head;
            detail::PopHead_(head, tail);
        } else if constexpr (release_strategy == ReleaseStrategy::Latest) {
            n = tail;
            detail::PopTail_(head, tail);
        } else {
            static_assert(false);
        }

        allocator::Deallocate(self.src_alctr, n);
    }

    self.head = head;
    self.tail = tail;
}

template <AllocatorTplParamList>
constexpr void
pool_allocator::Allocator<AllocatorTplArgList>::Allocator::SanityCheck(
    this Allocator& self, mem_recorder::MemRecorder* mr) {
    detail::Check_(self);

    void* head{ self.head };
    void* tail{ self.tail };

    if (head == nullptr) { return; }

    void* n_prv{ self.head };
    void* n{ n_prv };

    size_t cnt{ 0 };

    for (;;) {
        if (mr != nullptr) { mr->Record(n, sizeof(void*)); }

        ++cnt;

        if (n == tail) { break; }

        void* n_nxt{
            reinterpret_cast<void*>  // NOLINT(performance-no-int-to-ptr)
            (reinterpret_cast<uintptr_t>(n_prv) ^ *static_cast<uintptr_t*>(n))
        };

        n_prv = n;
        n = n_nxt;
    }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt == self.cnt);
}

}  // namespace zeta::core

#pragma pop_macro("AllocatorTplParamList")
#pragma pop_macro("AllocatorTplArgList")
