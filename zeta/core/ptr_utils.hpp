#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::ptr_utils {

namespace color_ptr {

void* GetPtr(void* const& color_ptr, size_t align);

void SetPtr(void*& color_ptr, size_t align, void* ptr);

unsigned GetColor(void* const& color_ptr, size_t align);

void SetColor(void*& color_ptr, size_t align, unsigned color);

void SetPtrColor(void*& color_ptr, size_t align, void* ptr, unsigned color);

}  // namespace color_ptr

namespace rel_ptr {

template <typename SignedIntegral>
void* GetPtr(SignedIntegral const& rel_ptr, void const* base);

template <typename SignedIntegral>
void SetPtr(SignedIntegral& rel_ptr, void const* base, void* ptr);

}  // namespace rel_ptr

namespace rel_color_ptr {

template <typename SignedIntegral>
void* GetPtr(SignedIntegral const& rel_color_ptr, size_t align,
             void const* base);

template <typename SignedIntegral>
void SetPtr(SignedIntegral& rel_color_ptr, size_t align, void const* base,
            void* ptr);

template <typename SignedIntegral>
unsigned GetColor(SignedIntegral const& rel_color_ptr, size_t align,
                  void const* base);

template <typename SignedIntegral>
void SetColor(SignedIntegral& rel_color_ptr, size_t align, void const* base,
              unsigned color);

template <typename SignedIntegral>
void SetPtrColor(SignedIntegral& rel_color_ptr, size_t align, void const* base,
                 void* ptr, unsigned color);

}  // namespace rel_color_ptr

template <typename LinkType_, typename ColorTag_>
struct AugPtrTpl {
    using LinkType = LinkType_;
    using ColorTag = ColorTag_;

    ZETA_Core_StaticAssert(!meta::IsConst<LinkType>);
    ZETA_Core_StaticAssert(!meta::IsRef<LinkType>);

    ZETA_Core_StaticAssert(
        meta::IsAnyOf<meta::RemoveVolatile<LinkType>, void*> ||
        integral::IsSigned<meta::RemoveVolatile<LinkType>>);

    ZETA_Core_StaticAssert(meta::IsAnyOf<ColorTag, value_wrapper::TrueType,
                                         value_wrapper::FalseType>);

    static constexpr bool IsRelLink{
        !meta::IsAnyOf<meta::RemoveVolatile<LinkType>, void*>
    };

    LinkType link;

    template <typename _ = void,
              typename = meta::EnableIf<!IsRelLink && !ColorTag::value, _>>
    void* GetPtr() const;

    template <typename _ = void, typename = meta::EnableIf<!IsRelLink, _>>
    void* GetPtr(size_t align) const;

    template <typename _ = void, typename = meta::EnableIf<!ColorTag::value, _>>
    void* GetPtr(void const* base) const;

    void* GetPtr(size_t align, void const* base) const;

    template <typename _ = void,
              typename = meta::EnableIf<!IsRelLink && ColorTag::value, _>>
    unsigned GetColor(size_t align) const;

    template <typename _ = void, typename = meta::EnableIf<ColorTag::value, _>>
    unsigned GetColor(size_t align, void const* base) const;

    template <typename _ = void,
              typename = meta::EnableIf<!IsRelLink && !ColorTag::value, _>>
    void SetPtr(void* ptr);

    template <typename _ = void, typename = meta::EnableIf<!IsRelLink, _>>
    void SetPtr(size_t align, void* ptr);

    template <typename _ = void, typename = meta::EnableIf<!ColorTag::value, _>>
    void SetPtr(void const* base, void* ptr);

    void SetPtr(size_t align, void const* base, void* ptr);

    template <typename _ = void,
              typename = meta::EnableIf<!IsRelLink && ColorTag::value, _>>
    void SetColor(size_t align, unsigned color);

    template <typename _ = void, typename = meta::EnableIf<ColorTag::value, _>>
    void SetColor(size_t align, void const* base, unsigned color);

    template <typename _ = void,
              typename = meta::EnableIf<!IsRelLink && ColorTag::value, _>>
    void SetPtrColor(size_t align, void* ptr, unsigned color);

    template <typename _ = void, typename = meta::EnableIf<ColorTag::value, _>>
    void SetPtrColor(size_t align, void const* base, void* ptr, unsigned color);
};

}  // namespace zeta::core::ptr_utils
