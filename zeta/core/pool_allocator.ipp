#include <zeta/core/allocator.hpp>
#include <zeta/core/allocator.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/pool_allocator.hpp>

#pragma push_macro("AllocatorTplParamList")
#define AllocatorTplParamList                               \
    typename ReuseStrategyTag, typename ReleaseStrategyTag, \
        typename SrcAllocatorLike

#pragma push_macro("AllocatorTplArgList")
#define AllocatorTplArgList \
    ReuseStrategyTag, ReleaseStrategyTag, SrcAllocatorLike

namespace zeta::core {

namespace pool_allocator::ops::detail {

template <AllocatorTplParamList>
void Check_  // NOLINT(misc-use-internal-linkage)
    (Allocator<AllocatorTplArgList>* pa) {
    ZETA_Core_DebugAssert(pa != nullptr);

    if (!meta::IsAnyOf<ReleaseStrategyTag, ReleaseStrategy::Never>) {
        ZETA_Core_DebugAssert(0 < pa->capacity);
        ZETA_Core_DebugAssert(pa->cnt <= pa->capacity);
    }

    if (pa->head == nullptr) {
        ZETA_Core_DebugAssert(pa->tail == nullptr);

        if (!meta::IsAnyOf<ReleaseStrategyTag, ReleaseStrategy::Never>) {
            ZETA_Core_DebugAssert(pa->cnt == 0);
        }
    } else if (pa->head == pa->tail) {
        ZETA_Core_DebugAssert(pa->cnt == 1);
    }
}

}  // namespace pool_allocator::ops::detail

template <AllocatorTplParamList>
void pool_allocator::ops::Init(Allocator<AllocatorTplArgList>* pa) {
    ZETA_Core_DebugAssert(pa != nullptr);

    if constexpr (!meta::IsAnyOf<ReleaseStrategyTag, ReleaseStrategy::Never>) {
        pa->cnt = 0;
    }

    pa->head = nullptr;
    pa->tail = nullptr;
}

template <AllocatorTplParamList>
void pool_allocator::ops::Deinit(Allocator<AllocatorTplArgList>* pa) {
    detail::Check_(pa);

    if constexpr (meta::IsAnyOf<SrcAllocatorLike, void>) {
        ZETA_Core_DebugAssert(pa->head == nullptr);
    } else {
        if (pa->head != nullptr) { Release(pa, integral::RangeMaxOf<size_t>); }
    }
}

template <AllocatorTplParamList>
size_t pool_allocator::ops::GetAlign(Allocator<AllocatorTplArgList> const* pa) {
    detail::Check_(pa);

    return alignof(void*);
}

namespace pool_allocator::ops::detail {

inline void PushTail_  // NOLINT(misc-use-internal-linkage)
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

inline void PopHead_  // NOLINT(misc-use-internal-linkage)
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

inline void PopTail_  // NOLINT(misc-use-internal-linkage)
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

}  // namespace pool_allocator::ops::detail

template <AllocatorTplParamList>
void* pool_allocator::ops::Allocate(Allocator<AllocatorTplArgList>* pa,
                                    size_t size) {
    detail::Check_(pa);

    if (size == 0) { return nullptr; }

    void* head{ pa->head };
    void* tail{ pa->tail };

    if (head == nullptr) {
        if constexpr (!meta::IsAnyOf<SrcAllocatorLike, void>) {
            return allocator::ops::Allocate(pa->src_alctr, size);
        }

        return nullptr;
    }

    if constexpr (!meta::IsAnyOf<ReleaseStrategyTag, ReleaseStrategy::Never>) {
        --pa->cnt;
    }

    if (head == tail) {
        pa->head = nullptr;
        pa->tail = nullptr;
        return head;
    }

    void* ret;

    if constexpr (meta::IsAnyOf<ReuseStrategyTag, ReuseStrategy::Oldest>) {
        ret = head;
        detail::PopHead_(head, tail);
        pa->head = head;
    } else if constexpr (meta::IsAnyOf<ReuseStrategyTag,
                                       ReuseStrategy::Latest>) {
        ret = tail;
        detail::PopTail_(head, tail);
        pa->tail = tail;
    } else {
        ZETA_Core_StaticAssert(false);
    }

    return ret;
}

template <AllocatorTplParamList>
void pool_allocator::ops::Deallocate(Allocator<AllocatorTplArgList>* pa,
                                     void* ptr) {
    detail::Check_(pa);

    if (ptr == nullptr) { return; }

    ZETA_Core_DebugAssert(__builtin_is_aligned(ptr, alignof(void*)));

    void* head{ pa->head };
    void* tail{ pa->tail };
    size_t capacity{ pa->capacity };

    if (head == nullptr) {
        pa->head = ptr;
        pa->tail = ptr;

        if (!meta::IsAnyOf<ReleaseStrategyTag, ReleaseStrategy::Never>) {
            pa->cnt = 1;
        }

        *static_cast<uintptr_t*>(ptr) = 0;
        return;
    }

    if constexpr (meta::IsAnyOf<ReleaseStrategyTag, ReleaseStrategy::Latest>) {
        if (pa->cnt == capacity) {
            allocator::ops::Deallocate(pa->src_alctr, ptr);
            return;
        }

        ++pa->cnt;
    }

    detail::PushTail_(head, tail, ptr);

    if constexpr (meta::IsAnyOf<ReleaseStrategyTag, ReleaseStrategy::Oldest>) {
        if (pa->cnt == capacity) {
            void* n{ head };
            detail::PopHead_(head, tail);
            allocator::ops::Deallocate(pa->src_alctr, n);
        } else {
            ++pa->cnt;
        }
    }

    pa->head = head;
    pa->tail = tail;
}

template <AllocatorTplParamList, typename>
void pool_allocator::ops::Release(Allocator<AllocatorTplArgList>* pa,
                                  size_t cnt) {
    detail::Check_(pa);

    void* head{ pa->head };
    void* tail{ pa->tail };

    if constexpr (!meta::IsAnyOf<ReleaseStrategyTag, ReleaseStrategy::Never>) {
        pa->cnt -= Min(cnt, pa->cnt);
    }

    for (; 0 < cnt && head != nullptr; --cnt) {
        if (head == tail) {
            head = nullptr;
            tail = nullptr;
            continue;
        }

        void* n;

        if constexpr (meta::IsAnyOf<ReleaseStrategyTag,
                                    ReleaseStrategy::Oldest>) {
            n = head;
            detail::PopHead_(head, tail);
        } else if constexpr (meta::IsAnyOf<ReleaseStrategyTag,
                                           ReleaseStrategy::Latest>) {
            n = tail;
            detail::PopTail_(head, tail);
        } else {
            ZETA_Core_StaticAssert(false);
        }

        allocator::ops::Deallocate(pa->src_alctr, n);
    }

    pa->head = head;
    pa->tail = tail;
}

template <AllocatorTplParamList>
void pool_allocator::ops::Sanitize(Allocator<AllocatorTplArgList>* pa,
                                   mem_recorder::MemRecorder* mr) {
    detail::Check_(pa);

    void* head{ pa->head };
    void* tail{ pa->tail };

    if (head == nullptr) { return; }

    void* n_prv{ pa->head };
    void* n{ n_prv };

    size_t cnt{ 0 };

    for (;;) {
        mem_recorder::ops::Record(mr, n, sizeof(void*));
        ++cnt;

        if (n == tail) { break; }

        void* n_nxt{
            reinterpret_cast<void*>  // NOLINT(performance-no-int-to-ptr)
            (reinterpret_cast<uintptr_t>(n_prv) ^ *static_cast<uintptr_t*>(n))
        };

        n_prv = n;
        n = n_nxt;
    }

    ZETA_Core_DebugAssert(cnt == pa->cnt);
}

}  // namespace zeta::core

#pragma pop_macro("AllocatorTplParamList")
#pragma pop_macro("AllocatorTplArgList")
