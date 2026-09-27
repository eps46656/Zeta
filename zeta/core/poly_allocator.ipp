#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/sanity.ipp>
#include <zeta/core/poly_allocator.hpp>

namespace zeta::core {

#pragma push_macro("CallMethod_")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod_(method_ptr, method, ...)                                   \
    {                                                                          \
        ZETA_Core_DebugUtils_Diag_PromiseAssert(self.target_alctr != nullptr); \
        ZETA_Core_DebugUtils_Diag_PromiseAssert(self.vtable != nullptr);       \
                                                                               \
        auto method_ptr{ self.vtable->method };                                \
        ZETA_Core_DebugUtils_Diag_PromiseAssert(method_ptr != nullptr);        \
                                                                               \
        return method_ptr(self.target_alctr, __VA_ARGS__);                     \
    }                                                                          \
    static_assert(true);

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(method, ...) \
    CallMethod_(ZETA_Core_TmpName, method, __VA_ARGS__)

constexpr poly_allocator::Allocator::Allocator()
    : align{ 0 }, vtable{ nullptr }, target_alctr{ nullptr } {
    debug_utils::sanity::RegisterSanityCheckFunc(
        this, debug_utils::sanity::DummySanityCheckFunc);
}

template <allocator::IsAllocator TargetAllocator>
constexpr poly_allocator::Allocator::Allocator(TargetAllocator& target_alctr) {
    debug_utils::sanity::RegisterSanityCheckFunc(
        this, debug_utils::sanity::DummySanityCheckFunc);

    this->Set(target_alctr);
}

constexpr poly_allocator::Allocator::~Allocator() {
    debug_utils::sanity::UnregisterSanityCheckFunc(this);
}

template <allocator::IsAllocator TargetAllocator>
constexpr void
    poly_allocator::Allocator::Set  // NOLINT(misc-use-internal-linkage)
    (this Allocator& self, TargetAllocator& target_alctr) {
    self.align = allocator::GetAlign(target_alctr);

    self.vtable = &allocator::GetVTable<TargetAllocator>();

    self.target_alctr =
        const_cast<void*>(static_cast<void const*>(&target_alctr));
}

constexpr void* poly_allocator::Allocator::GetReferedInstPtr(
    this Allocator const& self, allocator::Tag) {
    return self.target_alctr;
}

constexpr size_t poly_allocator::Allocator::GetAlign(this Allocator const& self,
                                                     allocator::Tag) {
    return self.align;
}

constexpr void* poly_allocator::Allocator::Allocate(this Allocator& self,
                                                    allocator::Tag,
                                                    size_t size) {
    CallMethod(Allocate, size);
}

constexpr void poly_allocator::Allocator::Deallocate(this Allocator& self,
                                                     allocator::Tag,
                                                     void* ptr) {
    CallMethod(Deallocate, ptr);
}

#pragma pop_macro("CallMethod")
#pragma pop_macro("CallMethod_")

constexpr void poly_allocator::Allocator::Check(this Allocator const& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < self.align);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.vtable != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.target_alctr != nullptr);

#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(method) \
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.vtable->method != nullptr);

    CheckMethod(Allocate);
    CheckMethod(Deallocate);

#pragma pop_macro("CheckMethod")
}

constexpr void poly_allocator::Allocator::SanityCheck(
    void const* alctr_, debug_utils::sanity::SanityCheckScope scope) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(alctr_ != nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        scope != debug_utils::sanity::SanityCheckScope::Basic ||
        scope != debug_utils::sanity::SanityCheckScope::Complete);

    auto& self{ *static_cast<poly_allocator::Allocator const*>(alctr_) };

    self.Check();

    debug_utils::sanity::ExpandFinishedSanityCheckScope(
        alctr_, debug_utils::sanity::SanityCheckScope::Basic);

    if (scope == debug_utils::sanity::SanityCheckScope::Complete) {
        debug_utils::sanity::SanityCheck(self.target_alctr, scope);
    }
}

}  // namespace zeta::core
