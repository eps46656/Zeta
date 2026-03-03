#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_recorder.hpp>

#pragma push_macro("AllocatorTplDeclParamList")
#define AllocatorTplDeclParamList                           \
    typename ReuseStrategyTag, typename ReleaseStrategyTag, \
        typename SrcAllocatorLike

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

template <AllocatorTplDeclParamList>
struct Allocator {
    ZETA_Core_StaticAssert(
        meta::IsAnyOf<ReuseStrategyTag, ReuseStrategy::Oldest,
                      ReuseStrategy::Latest>);

    ZETA_Core_StaticAssert(
        meta::IsAnyOf<ReleaseStrategyTag, ReleaseStrategy::Oldest,
                      ReleaseStrategy::Latest>);

    SrcAllocatorLike src_alctr;
    size_t cnt;
    size_t capacity;
    void* head;
    void* tail;
};

template <typename ReuseStrategyTag, typename SrcAllocatorLike>
struct Allocator<ReuseStrategyTag, ReleaseStrategy::Never, SrcAllocatorLike> {
    ZETA_Core_StaticAssert(
        meta::IsAnyOf<ReuseStrategyTag, ReuseStrategy::Oldest,
                      ReuseStrategy::Latest>);

    SrcAllocatorLike src_allocator;
    void* head;
    void* tail;
};

template <typename ReuseStrategyTag>
struct Allocator<ReuseStrategyTag, ReleaseStrategy::Never, void> {
    ZETA_Core_StaticAssert(
        meta::IsAnyOf<ReuseStrategyTag, ReuseStrategy::Oldest,
                      ReuseStrategy::Latest>);

    void* head;
    void* tail;
};

namespace ops {

template <AllocatorTplParamList>
void Init(Allocator<AllocatorTplArgList>* pa);

template <AllocatorTplParamList>
void Deinit(Allocator<AllocatorTplArgList>* pa);

template <AllocatorTplParamList>
size_t GetAlign(Allocator<AllocatorTplArgList> const* pa);

template <AllocatorTplParamList>
void* Allocate(Allocator<AllocatorTplArgList>* pa, size_t size);

template <AllocatorTplParamList>
void Deallocate(Allocator<AllocatorTplArgList>* pa, void* ptr);

template <AllocatorTplParamList,
          typename = meta::EnableIf<
              !meta::IsAnyOf<ReleaseStrategyTag, ReleaseStrategy::Never>>>
void Release(Allocator<AllocatorTplArgList>* pa, size_t cnt);

template <AllocatorTplParamList>
void Sanitize(Allocator<AllocatorTplArgList>* pa,
              mem_recorder::MemRecorder* mr);

}  // namespace ops

}  // namespace zeta::core::pool_allocator

#pragma pop_macro("AllocatorTplDeclParamList")
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
