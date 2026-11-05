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

template <integral::IsSignedIntegralType SignedIntegral>
constexpr void* GetPtr(SignedIntegral rel_ptr, void const* base);

template <integral::IsSignedIntegralType SignedIntegral>
constexpr void SetPtr(SignedIntegral& rel_ptr, void const* base, void* ptr);

}  // namespace rel_ptr

namespace rel_color_ptr {

template <integral::IsSignedIntegralType SignedIntegral>
constexpr void* GetPtr(SignedIntegral rel_color_ptr, size_t align,
                       void const* base);

template <integral::IsSignedIntegralType SignedIntegral>
constexpr unsigned GetColor(SignedIntegral rel_color_ptr, size_t align,
                            void const* base);

template <integral::IsSignedIntegralType SignedIntegral>
constexpr pair::Pair<void*, unsigned> GetPtrColor(SignedIntegral rel_color_ptr,
                                                  size_t align,
                                                  void const* base);

template <integral::IsSignedIntegralType SignedIntegral>
constexpr void SetPtr(SignedIntegral& rel_color_ptr, size_t align,
                      void const* base, void* ptr);

template <integral::IsSignedIntegralType SignedIntegral>
constexpr void SetColor(SignedIntegral& rel_color_ptr, size_t align,
                        void const* base, unsigned color);

template <integral::IsSignedIntegralType SignedIntegral>
constexpr void SetPtrColor(SignedIntegral& rel_color_ptr, size_t align,
                           void const* base, void* ptr, unsigned color);

}  // namespace rel_color_ptr

template <typename Link>
concept IsLink = requires {
    requires !meta::IsConstType<Link>;

    requires !meta::IsReferenceType<Link>;

    requires meta::IsSame<meta::RemoveVolatile<Link>, void*> ||
                 integral::IsSignedIntegralType<meta::RemoveVolatile<Link>>;
};

template <IsLink Link_, bool color_en_>
struct AugPtrTpl {
    using Link = Link_;
    static constexpr bool color_en{ color_en_ };

    static constexpr bool is_rel_link{
        !meta::IsSame<meta::RemoveVolatile<Link>, void*>
    };

    Link link;

    constexpr void* GetPtr(this AugPtrTpl const& self)
        requires(!is_rel_link && !color_en);

    constexpr void* GetPtr(this AugPtrTpl const& self, size_t align)
        requires(!is_rel_link);

    constexpr void* GetPtr(this AugPtrTpl const& self, void const* base)
        requires color_en;

    constexpr void* GetPtr(this AugPtrTpl const& self, size_t align,
                           void const* base);

    constexpr unsigned GetColor(this AugPtrTpl const& self, size_t align)
        requires(!is_rel_link && color_en);

    constexpr unsigned GetColor(this AugPtrTpl const& self, size_t align,
                                void const* base)
        requires color_en;

    constexpr void SetPtr(this AugPtrTpl& self, void* ptr)
        requires(!is_rel_link && !color_en);

    constexpr void SetPtr(this AugPtrTpl& self, size_t align, void* ptr)
        requires(!is_rel_link);

    constexpr void SetPtr(this AugPtrTpl& self, void const* base, void* ptr)
        requires(!color_en);

    constexpr void SetPtr(this AugPtrTpl& self, size_t align, void const* base,
                          void* ptr);

    constexpr void SetColor(this AugPtrTpl& self, size_t align, unsigned color)
        requires(!is_rel_link && color_en);

    constexpr void SetColor(this AugPtrTpl& self, size_t align,
                            void const* base, unsigned color)
        requires color_en;

    constexpr void SetPtrColor(this AugPtrTpl& self, size_t align, void* ptr,
                               unsigned color)
        requires(!is_rel_link && color_en);

    constexpr void SetPtrColor(this AugPtrTpl& self, size_t align,
                               void const* base, void* ptr, unsigned color)
        requires color_en;
};

}  // namespace zeta::core::ptr_utils
