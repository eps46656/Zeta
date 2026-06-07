#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/pair.hpp>
#include <zeta/core/ptr_utils.hpp>

namespace zeta::core {

inline void* ptr_utils::color_ptr::GetPtr(void* color_ptr, size_t align) {
    return (GetPtrColor)(color_ptr, align).first;
}

inline void const* ptr_utils::color_ptr::GetPtr(void const* color_ptr,
                                                size_t align) {
    return (GetPtrColor)(color_ptr, align).first;
}

inline unsigned ptr_utils::color_ptr::GetColor(void const* color_ptr,
                                               size_t align) {
    return (GetPtrColor)(color_ptr, align).second;
}

inline pair::Pair<void*, unsigned> ptr_utils::color_ptr::GetPtrColor(
    void* color_ptr, size_t align) {
    void* ptr{ __builtin_align_down(color_ptr, align) };

    unsigned color{ static_cast<unsigned>(static_cast<char*>(color_ptr) -
                                          static_cast<char*>(ptr)) };

    return { ptr, color };
}

inline pair::Pair<void const*, unsigned> ptr_utils::color_ptr::GetPtrColor(
    void const* color_ptr, size_t align) {
    auto [ptr, color]{ (GetPtrColor)(const_cast<void*>(color_ptr), align) };
    return { ptr, color };
}

inline void ptr_utils::color_ptr::SetPtr(void*& color_ptr, size_t align,
                                         void* ptr) {
    (SetPtrColor)(color_ptr, align, ptr, (GetColor)(color_ptr, align));
}

inline void ptr_utils::color_ptr::SetColor(void*& color_ptr, size_t align,
                                           unsigned color) {
    (SetPtrColor)(color_ptr, align, (GetPtr)(color_ptr, align), color);
}

inline void ptr_utils::color_ptr::SetPtrColor(void*& color_ptr, size_t align,
                                              void* ptr, unsigned color) {
    ZETA_Core_DebugAssert(__builtin_is_aligned(ptr, align));

    ZETA_Core_DebugAssert(color < align);

    color_ptr = static_cast<char*>(ptr) + color;

    ZETA_Core_DebugAssert((GetPtr)(color_ptr, align) == ptr);
    ZETA_Core_DebugAssert((GetColor)(color_ptr, align) == color);
}

template <typename SignedIntegral>
void* ptr_utils::rel_ptr::GetPtr(SignedIntegral rel_ptr, void const* base) {
    ZETA_Core_StaticAssert(integral::IsSignedIntegral<SignedIntegral>);

    return const_cast<char*>(static_cast<char const*>(base) + rel_ptr);
}

template <typename SignedIntegral>
void ptr_utils::rel_ptr::SetPtr(SignedIntegral& rel_ptr, void const* base,
                                void* ptr) {
    ZETA_Core_StaticAssert(integral::IsSignedIntegral<SignedIntegral>);

    ptrdiff_t diff{ static_cast<char*>(ptr) - static_cast<char const*>(base) };

    ZETA_Core_DebugAssert(integral::RangeMinOf<SignedIntegral> <= diff &&
                          diff <= integral::RangeMaxOf<SignedIntegral>);

    rel_ptr = static_cast<SignedIntegral>(diff);

    ZETA_Core_DebugAssert((GetPtr)(rel_ptr, base) == ptr);
}

template <typename SignedIntegral>
void* ptr_utils::rel_color_ptr::GetPtr(SignedIntegral rel_color_ptr,
                                       size_t align, void const* base) {
    return (GetPtrColor)(rel_color_ptr, align, base).first;
}

template <typename SignedIntegral>
unsigned ptr_utils::rel_color_ptr::GetColor(SignedIntegral rel_color_ptr,
                                            size_t align, void const* base) {
    return (GetPtrColor)(rel_color_ptr, align, base).second;
}

template <typename SignedIntegral>
pair::Pair<void*, unsigned> ptr_utils::rel_color_ptr::GetPtrColor(
    SignedIntegral rel_color_ptr, size_t align, void const* base) {
    ZETA_Core_StaticAssert(integral::IsSignedIntegral<SignedIntegral>);

    void* x{ const_cast<void*>(__builtin_align_down(
        static_cast<char const*>(base) + rel_color_ptr, align)) };

    void* ptr{ __builtin_align_down(x, align) };

    unsigned color{ static_cast<unsigned>(static_cast<char const*>(x) -
                                          static_cast<char const*>(ptr)) };

    return { ptr, color };
}

template <typename SignedIntegral>
void ptr_utils::rel_color_ptr::SetPtr(SignedIntegral& rel_color_ptr,
                                      size_t align, void const* base,
                                      void* ptr) {
    (SetPtrColor)(rel_color_ptr, align, base, ptr,
                  (GetColor)(rel_color_ptr, align, base));
}

template <typename SignedIntegral>
void ptr_utils::rel_color_ptr::SetColor(SignedIntegral& rel_color_ptr,
                                        size_t align, void const* base,
                                        unsigned color) {
    (SetPtrColor)(rel_color_ptr, align, base,
                  (GetPtr)(rel_color_ptr, align, base), color);
}

template <typename SignedIntegral>
void ptr_utils::rel_color_ptr::SetPtrColor(SignedIntegral& rel_color_ptr,
                                           size_t align, void const* base,
                                           void* ptr, unsigned color) {
    ZETA_Core_StaticAssert(integral::IsSignedIntegral<SignedIntegral>);

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

template <typename LinkType, typename ColorTag>
template <typename, typename>
void* ptr_utils::AugPtrTpl<LinkType, ColorTag>::GetPtr() const {
    return this->link;
}

template <typename LinkType, typename ColorTag>
template <typename, typename>
void* ptr_utils::AugPtrTpl<LinkType, ColorTag>::GetPtr(size_t align) const {
    if constexpr (ColorTag::value) {
        return color_ptr::GetPtr(this->link, align);
    } else {
        return this->GetPtr();
    }
}

template <typename LinkType, typename ColorTag>
template <typename, typename>
void* ptr_utils::AugPtrTpl<LinkType, ColorTag>::GetPtr(void const* base) const {
    if constexpr (IsRelLink) {
        return rel_ptr::GetPtr(this->link, base);
    } else {
        return this->GetPtr();
    }
}

template <typename LinkType, typename ColorTag>
void* ptr_utils::AugPtrTpl<LinkType, ColorTag>::GetPtr(size_t align,
                                                       void const* base) const {
    if constexpr (!IsRelLink) {
        return this->GetPtr(align);
    } else if constexpr (!ColorTag::value) {
        return this->GetPtr(base);
    } else {
        return rel_color_ptr::GetPtr(this->link, align, base);
    }
}

template <typename LinkType, typename ColorTag>
template <typename, typename>
unsigned ptr_utils::AugPtrTpl<LinkType, ColorTag>::GetColor(
    size_t align) const {
    return color_ptr::GetColor(this->link, align);
}

template <typename LinkType, typename ColorTag>
template <typename, typename>
unsigned ptr_utils::AugPtrTpl<LinkType, ColorTag>::GetColor(
    size_t align, void const* base) const {
    if constexpr (IsRelLink) {
        return rel_color_ptr::GetColor(this->link, align, base);
    } else {
        return this->GetColor(align);
    }
}

template <typename LinkType, typename ColorTag>
template <typename, typename>
void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetPtr(void* ptr) {
    this->link = ptr;
}

template <typename LinkType, typename ColorTag>
template <typename, typename>
void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetPtr(size_t align, void* ptr) {
    if constexpr (ColorTag::value) {
        color_ptr::SetPtr(this->link, align, ptr);
    } else {
        this->SetPtr(ptr);
    }
}

template <typename LinkType, typename ColorTag>
template <typename, typename>
void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetPtr(void const* base,
                                                      void* ptr) {
    if constexpr (IsRelLink) {
        rel_ptr::SetPtr(this->link, base, ptr);
    } else {
        this->SetPtr(ptr);
    }
}

template <typename LinkType, typename ColorTag>
void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetPtr(size_t align,
                                                      void const* base,
                                                      void* ptr) {
    if constexpr (!IsRelLink) {
        this->SetPtr(align, ptr);
    } else if constexpr (!ColorTag::value) {
        this->SetPtr(base, ptr);
    } else {
        rel_color_ptr::SetPtr(this->link, align, base, ptr);
    }
}

template <typename LinkType, typename ColorTag>
template <typename, typename>
void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetColor(size_t align,
                                                        unsigned color) {
    color_ptr::SetColor(this->link, align, color);
}

template <typename LinkType, typename ColorTag>
template <typename, typename>
void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetColor(size_t align,
                                                        void const* base,
                                                        unsigned color) {
    if constexpr (IsRelLink) {
        rel_color_ptr::SetColor(this->link, align, base, color);
    } else {
        this->SetColor(align, color);
    }
}

template <typename LinkType, typename ColorTag>
template <typename, typename>
void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetPtrColor(size_t align,
                                                           void* ptr,
                                                           unsigned color) {
    color_ptr::SetPtrColor(this->link, align, ptr, color);
}

template <typename LinkType, typename ColorTag>
template <typename, typename>
void ptr_utils::AugPtrTpl<LinkType, ColorTag>::SetPtrColor(size_t align,
                                                           void const* base,
                                                           void* ptr,
                                                           unsigned color) {
    if constexpr (IsRelLink) {
        rel_color_ptr::SetPtrColor(this->link, align, base, ptr, color);
    } else {
        this->SetPtrColor(align, ptr, color);
    }
}

}  // namespace zeta::core
