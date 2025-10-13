#pragma once

#include <zeta/core/integral.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::ptr_utils {

namespace color_ptr {

void* GetPtr(void* const& color_ptr, size_t align);

void SetPtr(void*& color_ptr, size_t align, void* ptr);

int GetColor(void* const& color_ptr, size_t align);

void SetColor(void*& color_ptr, size_t align, int color);

void SetPtrColor(void*& color_ptr, size_t align, void* ptr, int color);

}  // namespace color_ptr

// -----------------------------------------------------------------------------

namespace rel_ptr {

template <typename SignedIntegral>
void* GetPtr(SignedIntegral& rel_ptr, void const* base);

template <typename SignedIntegral>
void* SetPtr(SignedIntegral const& rel_ptr, void const* base, void* ptr);

}  // namespace rel_ptr

// -----------------------------------------------------------------------------

namespace rel_color_ptr {

template <typename SignedIntegral>
void* GetPtr(SignedIntegral& rel_color_ptr, size_t align, void const* base);

template <typename SignedIntegral>
void SetPtr(SignedIntegral& rel_color_ptr, size_t align, void const* base,
            void* ptr);

template <typename SignedIntegral>
int GetColor(SignedIntegral const& rel_color_ptr, size_t align,
             void const* base);

template <typename SignedIntegral>
void SetColor(SignedIntegral& rel_color_ptr, size_t align, void const* base,
              int color);

template <typename SignedIntegral>
void SetPtrColor(SignedIntegral& rel_color_ptr, size_t align, void const* base,
                 void* ptr, int color);

}  // namespace rel_color_ptr

// -----------------------------------------------------------------------------

template <typename LinkType, typename EnColor>
struct AugPtrTpl;

template <typename LinkType, typename EnColor>
struct AugPtrTpl {
    ZETA_Core_StaticAssert(!IsConst<LinkType>);
    ZETA_Core_StaticAssert(!IsRef<LinkType>);

    ZETA_Core_StaticAssert(IsAnyOf<RemoveVolatile<LinkType>, void*> ||
                           IsSignedIntegral<RemoveVolatile<LinkType>>);

    ZETA_Core_StaticAssert(
        IsAnyOf<EnColor, value_wrapper::StaticValueWrapper<true>,
                value_wrapper::StaticValueWrapper<false>>);

    static constexpr bool EnRelLink{
        !IsAnyOf<RemoveVolatile<LinkType>, void*>
    };

    LinkType link;

    // -------------------------------------------------------------------------

    template <typename _ = void>
    EnableIf<!EnRelLink && !EnColor::value, void*, _> GetPtr() const;

    template <typename _ = void>
    EnableIf<!EnRelLink, void*, _> GetPtr(size_t align) const;

    template <typename _ = void>
    EnableIf<!EnColor::value, void*, _> GetPtr(void const* base) const;

    void* GetPtr(size_t align, void const* base) const;

    // -------------------------------------------------------------------------

    template <typename _ = void>
    EnableIf<!EnRelLink && EnColor::value, int, _> GetColor(size_t align) const;

    template <typename _ = void>
    EnableIf<EnColor::value, int, _> GetColor(size_t align,
                                              void const* base) const;

    // -------------------------------------------------------------------------

    template <typename _ = void>
    EnableIf<!EnRelLink && !EnColor::value, void, _> SetPtr(void* ptr);

    template <typename _ = void>
    EnableIf<!EnRelLink, void, _> SetPtr(size_t align, void* ptr);

    template <typename _ = void>
    EnableIf<!EnColor::value, void, _> SetPtr(void const* base, void* ptr);

    void SetPtr(size_t align, void const* base, void* ptr);

    // -------------------------------------------------------------------------

    template <typename _ = void>
    EnableIf<!EnRelLink && EnColor::value, void, _> SetColor(size_t align,
                                                             int color);

    template <typename _ = void>
    EnableIf<EnColor::value, void, _> SetColor(size_t align, void const* base,
                                               int color);

    template <typename _ = void>
    EnableIf<!EnRelLink && EnColor::value, void, _> SetPtrColor(size_t align,
                                                                void* ptr,
                                                                int color);

    template <typename _ = void>
    EnableIf<EnColor::value, void, _> SetPtrColor(size_t align,
                                                  void const* base, void* ptr,
                                                  int color);
};

}  // namespace zeta::core::ptr_utils
