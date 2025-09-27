#pragma once

#include <zeta/core/integral.hpp>
#include <zeta/core/utils.hpp>

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

template <typename LinkType, bool EnColor>
struct AugPtrTpl;

template <typename LinkType, bool EnColor>
struct AugPtrTpl {
    ZETA_Core_StaticAssert(!IsConst<LinkType>);
    ZETA_Core_StaticAssert(!IsRef<LinkType>);

    ZETA_Core_StaticAssert(IsSame<RemoveVolatile<LinkType>, void*> ||
                           IsSignedIntegral<RemoveVolatile<LinkType>>);

    static constexpr bool EnRelLink{ !IsSame<RemoveVolatile<LinkType>, void*> };

    LinkType link;

    void* GetPtr() const;
    void* GetPtr(size_t align) const;
    void* GetPtr(void const* base) const;
    void* GetPtr(size_t align, void const* base) const;

    int GetColor(size_t align) const;
    int GetColor(size_t align, void const* base) const;

    void SetPtr(void* ptr);
    void SetPtr(size_t align, void* ptr);
    void SetPtr(void const* base, void* ptr);
    void SetPtr(size_t align, void const* base, void* ptr);

    void SetColor(size_t align, int color);
    void SetColor(size_t align, void const* base, int color);

    void SetPtrColor(size_t align, void* ptr, int color);
    void SetPtrColor(size_t align, void const* base, void* ptr, int color);
};

}  // namespace zeta::core::ptr_utils
