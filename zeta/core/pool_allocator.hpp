#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core::pool_allocator {

enum struct ReuseStrategy : unsigned char {
    Oldest = 0,
    Latest = 1,
};

enum struct ReleaseStrategy : unsigned char {
    Never = 0,
    Oldest = 1,
    Latest = 2,
};

template <typename ReuseStrategyTag_, typename ReleaseStrategyTag_,
          typename SrcAllocatorLike_>
struct AllocatorBase {
    using ReuseStrategyTag = ReuseStrategyTag_;
    using ReleaseStrategyTag = ReleaseStrategyTag_;
    using SrcAllocatorLike = SrcAllocatorLike_;

    static_assert(meta::IsValueWrapperT<ReuseStrategyTag, ReuseStrategy>);

    static_assert(meta::IsValueWrapperT<ReleaseStrategyTag, ReleaseStrategy>);

    static_assert(allocator::IsAllocator<meta::RemoveCVRef<SrcAllocatorLike>>);

    size_t cnt;
    size_t capacity;
    void* head;
    void* tail;
    SrcAllocatorLike src_alctr;
};

template <typename ReuseStrategyTag_, typename SrcAllocatorLike_>
struct AllocatorBase<ReuseStrategyTag_,
                     meta::AutoValueWrapper<ReleaseStrategy::Never>,
                     SrcAllocatorLike_> {
    using ReuseStrategyTag = ReuseStrategyTag_;
    using ReleaseStrategyTag = meta::AutoValueWrapper<ReleaseStrategy::Never>;
    using SrcAllocatorLike = SrcAllocatorLike_;

    static_assert(meta::IsValueWrapperT<ReuseStrategyTag, ReuseStrategy>);

    static_assert(allocator::IsAllocator<meta::RemoveCVRef<SrcAllocatorLike>>);

    void* head;
    void* tail;
    SrcAllocatorLike src_allocator;

    template <typename SrcAllocatorLikeConstructArg>
    constexpr AllocatorBase(
        SrcAllocatorLikeConstructArg&& src_allocator_like_construct_arg);
};

template <typename ReuseStrategyTag_>
struct AllocatorBase<ReuseStrategyTag_,
                     meta::AutoValueWrapper<ReleaseStrategy::Never>, void> {
    using ReuseStrategyTag = ReuseStrategyTag_;
    using ReleaseStrategyTag = meta::AutoValueWrapper<ReleaseStrategy::Never>;
    using SrcAllocatorLike = void;

    static_assert(meta::IsValueWrapperT<ReuseStrategyTag, ReuseStrategy>);

    void* head;
    void* tail;
};

template <typename ReuseStrategyTag, typename ReleaseStrategyTag,
          typename SrcAllocatorLike>
struct Allocator : public AllocatorBase<ReuseStrategyTag, ReleaseStrategyTag,
                                        SrcAllocatorLike> {
    static constexpr ReuseStrategy reuse_strategy{
        meta::GetValueWrapperValue<ReuseStrategyTag>
    };

    static constexpr ReleaseStrategy release_strategy{
        meta::GetValueWrapperValue<ReleaseStrategyTag>
    };

    constexpr Allocator()
        requires meta::IsSame<SrcAllocatorLike, void>;

    template <typename SrcAllocatorLikeConstructArg>
    constexpr Allocator(
        SrcAllocatorLikeConstructArg&& src_allocator_like_construct_arg)
        requires(!meta::IsSame<SrcAllocatorLike, void>);

    constexpr ~Allocator();

    constexpr size_t GetAlign(this Allocator const& self);

    constexpr void* Allocate(this Allocator& self, size_t size);

    constexpr void Deallocate(this Allocator& self, void* ptr);

    constexpr void Release(this Allocator& self, size_t cnt)
        requires(release_strategy != ReleaseStrategy::Never);

    constexpr void SanityCheck(this Allocator& self,
                               mem_recorder::MemRecorder* mr);
};

}  // namespace zeta::core::pool_allocator

/*

Oldest Oldest
allocate: head
deallocate: tail
release: head

Oldest Latest
allocate: head
deallocate: tail
release: tail(direct)

Latest Oldest
allocate: head
deallocate: tail
release: head

Latest Latest
allocate: head
deallocate: tail
release: tail(direct)




prv_tail tail head nxt_head

prv_tail tail nxt_head

*tail ^= head ^ nxt_head

tail head nxt_head

head tail head tail

*/
