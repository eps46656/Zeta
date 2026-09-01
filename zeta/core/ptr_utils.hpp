#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/pair.hpp>

namespace zeta::core::ptr_utils {

namespace color_ptr {

constexpr void* GetPtr(void* color_ptr, size_t align);

constexpr void const* GetPtr(void const* color_ptr, size_t align);

constexpr unsigned GetColor(void const* color_ptr, size_t align);

constexpr pair::Pair<void*, unsigned> GetPtrColor(void* color_ptr,
                                                  size_t align);

constexpr pair::Pair<void const*, unsigned> GetPtrColor(void const* color_ptr,
                                                        size_t align);

constexpr void SetPtr(void*& color_ptr, size_t align, void* ptr);

constexpr void SetColor(void*& color_ptr, size_t align, unsigned color);

constexpr void SetPtrColor(void*& color_ptr, size_t align, void* ptr,
                           unsigned color);

}  // namespace color_ptr

namespace rel_ptr {

template <integral::IsSignedIntegral SignedIntegral>
constexpr void* GetPtr(SignedIntegral rel_ptr, void const* base);

template <integral::IsSignedIntegral SignedIntegral>
constexpr void SetPtr(SignedIntegral& rel_ptr, void const* base, void* ptr);

}  // namespace rel_ptr

namespace rel_color_ptr {

template <integral::IsSignedIntegral SignedIntegral>
constexpr void* GetPtr(SignedIntegral rel_color_ptr, size_t align,
                       void const* base);

template <integral::IsSignedIntegral SignedIntegral>
constexpr unsigned GetColor(SignedIntegral rel_color_ptr, size_t align,
                            void const* base);

template <integral::IsSignedIntegral SignedIntegral>
constexpr pair::Pair<void*, unsigned> GetPtrColor(SignedIntegral rel_color_ptr,
                                                  size_t align,
                                                  void const* base);

template <integral::IsSignedIntegral SignedIntegral>
constexpr void SetPtr(SignedIntegral& rel_color_ptr, size_t align,
                      void const* base, void* ptr);

template <integral::IsSignedIntegral SignedIntegral>
constexpr void SetColor(SignedIntegral& rel_color_ptr, size_t align,
                        void const* base, unsigned color);

template <integral::IsSignedIntegral SignedIntegral>
constexpr void SetPtrColor(SignedIntegral& rel_color_ptr, size_t align,
                           void const* base, void* ptr, unsigned color);

}  // namespace rel_color_ptr

template <typename LinkType>
concept IsLinkType = requires {
    requires !meta::IsConst<LinkType>;

    requires !meta::IsRef<LinkType>;

    requires meta::IsSame<meta::RemoveVolatile<LinkType>, void*> ||
                 integral::IsSignedIntegral<meta::RemoveVolatile<LinkType>>;
};

template <IsLinkType LinkType_, typename ColorTag_>
struct AugPtrTpl {
    using LinkType = LinkType_;
    using ColorTag = ColorTag_;

    ZETA_Core_StaticAssert(meta::IsValueWrapperT<ColorTag, bool>);

    static constexpr bool IsRelLink{
        !meta::IsSame<meta::RemoveVolatile<LinkType>, void*>
    };

    LinkType link;

    constexpr void* GetPtr(this AugPtrTpl const& self)
        requires(!IsRelLink && !ColorTag::value);

    constexpr void* GetPtr(this AugPtrTpl const& self, size_t align)
        requires(!IsRelLink);

    constexpr void* GetPtr(this AugPtrTpl const& self, void const* base)
        requires ColorTag::value;

    constexpr void* GetPtr(this AugPtrTpl const& self, size_t align,
                           void const* base);

    constexpr unsigned GetColor(this AugPtrTpl const& self, size_t align)
        requires(!IsRelLink && ColorTag::value);

    constexpr unsigned GetColor(this AugPtrTpl const& self, size_t align,
                                void const* base)
        requires ColorTag::value;

    constexpr void SetPtr(this AugPtrTpl& self, void* ptr)
        requires(!IsRelLink && !ColorTag::value);

    constexpr void SetPtr(this AugPtrTpl& self, size_t align, void* ptr)
        requires(!IsRelLink);

    constexpr void SetPtr(this AugPtrTpl& self, void const* base, void* ptr)
        requires(!ColorTag::value);

    constexpr void SetPtr(this AugPtrTpl& self, size_t align, void const* base,
                          void* ptr);

    constexpr void SetColor(this AugPtrTpl& self, size_t align, unsigned color)
        requires(!IsRelLink && ColorTag::value);

    constexpr void SetColor(this AugPtrTpl& self, size_t align,
                            void const* base, unsigned color)
        requires ColorTag::value;

    constexpr void SetPtrColor(this AugPtrTpl& self, size_t align, void* ptr,
                               unsigned color)
        requires(!IsRelLink && ColorTag::value);

    constexpr void SetPtrColor(this AugPtrTpl& self, size_t align,
                               void const* base, void* ptr, unsigned color)
        requires ColorTag::value;
};

}  // namespace zeta::core::ptr_utils
