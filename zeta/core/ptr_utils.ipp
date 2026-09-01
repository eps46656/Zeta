#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/pair.hpp>
#include <zeta/core/ptr_utils.hpp>

namespace zeta::core {

constexpr void* ptr_utils::color_ptr::GetPtr(void* color_ptr, size_t align) {
    return (GetPtrColor)(color_ptr, align).first;
}

constexpr void const* ptr_utils::color_ptr::GetPtr(void const* color_ptr,
                                                   size_t align) {
    return (GetPtrColor)(color_ptr, align).first;
}

constexpr unsigned ptr_utils::color_ptr::GetColor(void const* color_ptr,
                                                  size_t align) {
    return (GetPtrColor)(color_ptr, align).second;
}

constexpr pair::Pair<void*, unsigned> ptr_utils::color_ptr::GetPtrColor(
    void* color_ptr, size_t align) {
    void* ptr{ __builtin_align_down(color_ptr, align) };

    unsigned color{ static_cast<unsigned>(static_cast<char*>(color_ptr) -
                                          static_cast<char*>(ptr)) };

    return { ptr, color };
}

constexpr pair::Pair<void const*, unsigned> ptr_utils::color_ptr::GetPtrColor(
    void const* color_ptr, size_t align) {
    auto [ptr, color]{ (GetPtrColor)(const_cast<void*>(color_ptr), align) };
    return { ptr, color };
}

constexpr void ptr_utils::color_ptr::SetPtr(void*& color_ptr, size_t align,
                                            void* ptr) {
    (SetPtrColor)(color_ptr, align, ptr, (GetColor)(color_ptr, align));
}

constexpr void ptr_utils::color_ptr::SetColor(void*& color_ptr, size_t align,
                                              unsigned color) {
    (SetPtrColor)(color_ptr, align, (GetPtr)(color_ptr, align), color);
}

constexpr void ptr_utils::color_ptr::SetPtrColor(void*& color_ptr, size_t align,
                                                 void* ptr, unsigned color) {
    ZETA_Core_DebugAssert(__builtin_is_aligned(ptr, align));

    ZETA_Core_DebugAssert(color < align);

    color_ptr = static_cast<char*>(ptr) + color;

    ZETA_Core_DebugAssert((GetPtr)(color_ptr, align) == ptr);
    ZETA_Core_DebugAssert((GetColor)(color_ptr, align) == color);
}

template <integral::IsSignedIntegral SignedIntegral>
constexpr void* ptr_utils::rel_ptr::GetPtr(SignedIntegral rel_ptr,
                                           void const* base) {
    return const_cast<char*>(static_cast<char const*>(base) + rel_ptr);
}

template <integral::IsSignedIntegral SignedIntegral>
constexpr void ptr_utils::rel_ptr::SetPtr(SignedIntegral& rel_ptr,
                                          void const* base, void* ptr) {
    ptrdiff_t diff{ static_cast<char*>(ptr) - static_cast<char const*>(base) };

    ZETA_Core_DebugAssert(integral::RangeMinOf<SignedIntegral> <= diff &&
                          diff <= integral::RangeMaxOf<SignedIntegral>);

    rel_ptr = static_cast<SignedIntegral>(diff);

    ZETA_Core_DebugAssert((GetPtr)(rel_ptr, base) == ptr);
}

template <integral::IsSignedIntegral SignedIntegral>
constexpr void* ptr_utils::rel_color_ptr::GetPtr(SignedIntegral rel_color_ptr,
                                                 size_t align,
                                                 void const* base) {
    return (GetPtrColor)(rel_color_ptr, align, base).first;
}

template <integral::IsSignedIntegral SignedIntegral>
constexpr unsigned ptr_utils::rel_color_ptr::GetColor(
    SignedIntegral rel_color_ptr, size_t align, void const* base) {
    return (GetPtrColor)(rel_color_ptr, align, base).second;
}

template <integral::IsSignedIntegral SignedIntegral>
constexpr pair::Pair<void*, unsigned> ptr_utils::rel_color_ptr::GetPtrColor(
    SignedIntegral rel_color_ptr, size_t align, void const* base) {
    void* x{ const_cast<void*>(__builtin_align_down(
        static_cast<char const*>(base) + rel_color_ptr, align)) };

    void* ptr{ __builtin_align_down(x, align) };

    unsigned color{ static_cast<unsigned>(static_cast<char const*>(x) -
                                          static_cast<char const*>(ptr)) };

    return { ptr, color };
}

template <integral::IsSignedIntegral SignedIntegral>
constexpr void ptr_utils::rel_color_ptr::SetPtr(SignedIntegral& rel_color_ptr,
                                                size_t align, void const* base,
                                                void* ptr) {
    (SetPtrColor)(rel_color_ptr, align, base, ptr,
                  (GetColor)(rel_color_ptr, align, base));
}

template <integral::IsSignedIntegral SignedIntegral>
constexpr void ptr_utils::rel_color_ptr::SetColor(SignedIntegral& rel_color_ptr,
                                                  size_t align,
                                                  void const* base,
                                                  unsigned color) {
    (SetPtrColor)(rel_color_ptr, align, base,
                  (GetPtr)(rel_color_ptr, align, base), color);
}

template <integral::IsSignedIntegral SignedIntegral>
constexpr void ptr_utils::rel_color_ptr::SetPtrColor(
    SignedIntegral& rel_color_ptr, size_t align, void const* base, void* ptr,
    unsigned color) {
    ZETA_Core_DebugAssert(__builtin_is_aligned(ptr, align));

    ZETA_Core_DebugAssert(color < align);

    ptrdiff_t diff{ static_cast<char*>(ptr) + color -
                    static_cast<char const*>(base) };

    ZETA_Core_DebugAssert(integral::RangeMinOf<SignedIntegral> <= diff &&
                          diff <= integral::RangeMaxOf<SignedIntegral>);

    rel_color_ptr = static_cast<SignedIntegral>(diff);

    ZETA_Core_DebugAssert((GetPtr)(rel_color_ptr, align, base) == ptr);
    ZETA_Core_DebugAssert((GetColor)(rel_color_ptr, align, base) == color);
}

template <ptr_utils::IsLinkType LinkType, typename ColorTag>
constexpr void* ptr_utils::AugPtrTpl<LinkType, ColorTag>::GetPtr(
    this AugPtrTpl const& self)
    requires(!IsRelLink && !ColorTag::value)
{
    return self.link;
}

template <ptr_utils::IsLinkType LinkType, typename ColorTag>
constexpr void* ptr_utils::AugPtrTpl<LinkType, ColorTag>::GetPtr(
    this AugPtrTpl const& self, size_t align)
    requires(!IsRelLink)
{
    if constexpr (ColorTag::value) {
        return color_ptr::GetPtr(self.link, align);
    } else {
        return self.GetPtr();
    }
}

template <ptr_utils::IsLinkType LinkType, typename ColorTag>
constexpr void* ptr_utils::AugPtrTpl<LinkType, ColorTag>::GetPtr(
    this AugPtrTpl const& self, void const* base)
    requires ColorTag::value
{
    if constexpr (IsRelLink) {
        return rel_ptr::GetPtr(self.link, base);
    } else {
        return self.GetPtr();
    }
}

template <ptr_utils::IsLinkType LinkType, typename ColorTag>
constexpr void* ptr_utils::AugPtrTpl<LinkType, ColorTag>::GetPtr(
    this AugPtrTpl const& self, size_t align, void const* base) {
    if constexpr (!IsRelLink) {
        return self.GetPtr(align);
    } else if constexpr (!ColorTag::value) {
        return self.GetPtr(base);
    } else {
        return rel_color_ptr::GetPtr(self.link, align, base);
    }
}

template <ptr_utils::IsLinkType LinkType, typename ColorTag>
constexpr unsigned ptr_utils::AugPtrTpl<LinkType, ColorTag>::GetColor(
    this AugPtrTpl const& self, size_t align)
    requires(!ptr_utils::AugPtrTpl<LinkType, ColorTag>::IsRelLink &&
             ColorTag::value)
{
    return color_ptr::GetColor(self.link, align);
}

template <ptr_utils::IsLinkType LinkType, typename ColorTag>
constexpr unsigned ptr_utils::AugPtrTpl<LinkType, ColorTag>::GetColor(
    this AugPtrTpl const& self, size_t align, void const* base)
    requires ColorTag::value
{
    if constexpr (IsRelLink) {
        return rel_color_ptr::GetColor(self.link, align, base);
    } else {
        return self.GetColor(align);
    }
}

template <ptr_utils::IsLinkType LinkType, typename ColorTag>
constexpr void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetPtr(
    this AugPtrTpl& self, void* ptr)
    requires(!ptr_utils::AugPtrTpl<LinkType, ColorTag>::IsRelLink &&
             !ColorTag::value)
{
    self.link = ptr;
}

template <ptr_utils::IsLinkType LinkType, typename ColorTag>
constexpr void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetPtr(
    this AugPtrTpl& self, size_t align, void* ptr)
    requires(!ptr_utils::AugPtrTpl<LinkType, ColorTag>::IsRelLink)
{
    if constexpr (ColorTag::value) {
        color_ptr::SetPtr(self.link, align, ptr);
    } else {
        self.SetPtr(ptr);
    }
}

template <ptr_utils::IsLinkType LinkType, typename ColorTag>
constexpr void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetPtr(
    this AugPtrTpl& self, void const* base, void* ptr)
    requires(!ColorTag::value)
{
    if constexpr (IsRelLink) {
        rel_ptr::SetPtr(self.link, base, ptr);
    } else {
        self.SetPtr(ptr);
    }
}

template <ptr_utils::IsLinkType LinkType, typename ColorTag>
constexpr void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetPtr(
    this AugPtrTpl& self, size_t align, void const* base, void* ptr) {
    if constexpr (!IsRelLink) {
        self.SetPtr(align, ptr);
    } else if constexpr (!ColorTag::value) {
        self.SetPtr(base, ptr);
    } else {
        rel_color_ptr::SetPtr(self.link, align, base, ptr);
    }
}

template <ptr_utils::IsLinkType LinkType, typename ColorTag>
constexpr void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetColor(
    this AugPtrTpl& self, size_t align, unsigned color)
    requires(!ptr_utils::AugPtrTpl<LinkType, ColorTag>::IsRelLink &&
             ColorTag::value)
{
    color_ptr::SetColor(self.link, align, color);
}

template <ptr_utils::IsLinkType LinkType, typename ColorTag>
constexpr void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetColor(
    this AugPtrTpl& self, size_t align, void const* base, unsigned color)
    requires ColorTag::value
{
    if constexpr (IsRelLink) {
        rel_color_ptr::SetColor(self.link, align, base, color);
    } else {
        self.SetColor(align, color);
    }
}

template <ptr_utils::IsLinkType LinkType, typename ColorTag>
constexpr void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetPtrColor(
    this AugPtrTpl& self, size_t align, void* ptr, unsigned color)
    requires(!ptr_utils::AugPtrTpl<LinkType, ColorTag>::IsRelLink &&
             ColorTag::value)
{
    color_ptr::SetPtrColor(self.link, align, ptr, color);
}

template <ptr_utils::IsLinkType LinkType, typename ColorTag>
constexpr void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetPtrColor(
    this AugPtrTpl& self, size_t align, void const* base, void* ptr,
    unsigned color)
    requires ColorTag::value
{
    if constexpr (IsRelLink) {
        rel_color_ptr::SetPtrColor(self.link, align, base, ptr, color);
    } else {
        self.SetPtrColor(align, ptr, color);
    }
}

}  // namespace zeta::core
