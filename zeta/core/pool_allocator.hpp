#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/mem_recorder.hpp>

#pragma push_macro("AllocatorTplParamList")
#define AllocatorTplParamList                               \
    typename ReuseStrategyTag, typename ReleaseStrategyTag, \
        typename SrcAllocatorLike

#pragma push_macro("AllocatorTplArgList")
#define AllocatorTplArgList \
    ReuseStrategyTag, ReleaseStrategyTag, SrcAllocatorLike

namespace zeta::core::pool_allocator {

struct ReuseStrategy {
    struct Oldest {};
    struct Latest {};
};

struct ReleaseStrategy {
    struct Never {};
    struct Oldest {};
    struct Latest {};
};

template <typename ReuseStrategyTag_, typename ReleaseStrategyTag_,
          typename SrcAllocatorLike_>
struct Allocator {
    using ReuseStrategyTag = ReuseStrategyTag_;
    using ReleaseStrategyTag = ReleaseStrategyTag_;
    using SrcAllocatorLike = SrcAllocatorLike_;

    ZETA_Core_StaticAssert(
        meta::IsAnySame<ReuseStrategyTag, ReuseStrategy::Oldest,
                        ReuseStrategy::Latest>);

    ZETA_Core_StaticAssert(
        meta::IsAnySame<ReleaseStrategyTag, ReleaseStrategy::Oldest,
                        ReleaseStrategy::Latest>);

    size_t cnt;
    size_t capacity;
    void* head;
    void* tail;
    SrcAllocatorLike src_alctr;
};

template <typename ReuseStrategyTag_, typename SrcAllocatorLike_>
struct Allocator<ReuseStrategyTag_, ReleaseStrategy::Never, SrcAllocatorLike_> {
    using ReuseStrategyTag = ReuseStrategyTag_;
    using ReleaseStrategyTag = ReleaseStrategy::Never;
    using SrcAllocatorLike = SrcAllocatorLike_;

    ZETA_Core_StaticAssert(
        meta::IsAnySame<ReuseStrategyTag, ReuseStrategy::Oldest,
                        ReuseStrategy::Latest>);

    SrcAllocatorLike src_allocator;
    void* head;
    void* tail;
};

template <typename ReuseStrategyTag_>
struct Allocator<ReuseStrategyTag_, ReleaseStrategy::Never, void> {
    using ReuseStrategyTag = ReuseStrategyTag_;
    using ReleaseStrategyTag = ReleaseStrategy::Never;
    using SrcAllocatorLike = void;

    ZETA_Core_StaticAssert(
        meta::IsAnySame<ReuseStrategyTag, ReuseStrategy::Oldest,
                        ReuseStrategy::Latest>);

    void* head;
    void* tail;
};

template <typename ReuseStrategyTag, typename ReleaseStrategyTag,
          typename SrcAllocatorLike, typename SrcAllocatorLikeInitArg,
          typename = meta::EnableIf<!meta::IsSame<SrcAllocatorLike, void>>>
void Init(Allocator<ReuseStrategyTag, ReleaseStrategyTag, SrcAllocatorLike>& pa,
          SrcAllocatorLikeInitArg&& src_allocator_like_init_arg);

template <typename ReuseStrategyTag, typename ReleaseStrategyTag>
void Init(Allocator<ReuseStrategyTag, ReleaseStrategyTag, void>& pa);

template <AllocatorTplParamList>
void Deinit(Allocator<AllocatorTplArgList>& pa);

template <AllocatorTplParamList>
size_t GetAlign(Allocator<AllocatorTplArgList> const& pa);

template <AllocatorTplParamList>
void* Allocate(Allocator<AllocatorTplArgList>& pa, size_t size);

template <AllocatorTplParamList>
void Deallocate(Allocator<AllocatorTplArgList>& pa, void* ptr);

template <AllocatorTplParamList,
          typename = meta::EnableIf<
              !meta::IsSame<ReleaseStrategyTag, ReleaseStrategy::Never>>>
void Release(Allocator<AllocatorTplArgList>& pa, size_t cnt);

template <AllocatorTplParamList>
void Sanitize(Allocator<AllocatorTplArgList>& pa,
              mem_recorder::MemRecorder* mr);

}  // namespace zeta::core::pool_allocator

namespace zeta::core {

template <AllocatorTplParamList>
struct lifecycle::Traits<pool_allocator::Allocator<AllocatorTplArgList>> {
    template <typename... Args>
    static void Init(pool_allocator::Allocator<AllocatorTplArgList>& pa,
                     Args&&... args);

    static void Deinit(pool_allocator::Allocator<AllocatorTplArgList>& pa);
};

}  // namespace zeta::core

#pragma pop_macro("AllocatorTplParamList")
#pragma pop_macro("AllocatorTplArgList")

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
