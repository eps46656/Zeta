#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/ptr_utils.hpp>

namespace zeta::core::ptr_utils {

namespace color_ptr {

inline void* GetPtr(void* const& color_ptr, size_t align) {
    return __builtin_align_down(color_ptr, align);
}

inline void SetPtr(void*& color_ptr, size_t align, void* ptr) {
    ZETA_Core_DebugAssert(__builtin_is_aligned(ptr, align));

    color_ptr = static_cast<char*>(ptr) + GetColor(color_ptr, align);
}

inline int GetColor(void* const& color_ptr, size_t align) {
    return static_cast<int>(static_cast<char*>(color_ptr) -
                            static_cast<char*>(GetPtr(color_ptr, align)));
}

inline void SetColor(void*& color_ptr, size_t align, int color) {
    ZETA_Core_DebugAssert(0 <= color &&
                          static_cast<unsigned long long>(color) < align);

    color_ptr = static_cast<char*>(GetPtr(color_ptr, align)) + color;
}

inline void SetPtrColor(void*& color_ptr, size_t align, void* ptr, int color) {
    ZETA_Core_DebugAssert(__builtin_is_aligned(ptr, align));

    ZETA_Core_DebugAssert(0 <= color &&
                          static_cast<unsigned long long>(color) < align);

    color_ptr = static_cast<char*>(ptr) + color;
}

}  // namespace color_ptr

// -----------------------------------------------------------------------------

namespace rel_ptr {

template <typename SignedIntegral>
void* GetPtr(SignedIntegral& rel_ptr, void const* base) {
    ZETA_Core_StaticAssert(IsSignedIntegral<SignedIntegral>);

    return static_cast<char const*>(base) + rel_ptr;
}

template <typename SignedIntegral>
void* SetPtr(SignedIntegral const& rel_ptr, void const* base, void* ptr) {
    ZETA_Core_StaticAssert(IsSignedIntegral<SignedIntegral>);

    ptrdiff_t diff{ static_cast<char*>(ptr) - static_cast<char const*>(base) };

    ZETA_Core_DebugAssert(range_min<SignedIntegral>() <= diff &&
                          diff <= range_max<SignedIntegral>());

    rel_ptr = static_cast<SignedIntegral>(diff);
}

}  // namespace rel_ptr

// -----------------------------------------------------------------------------

namespace rel_color_ptr {

template <typename SignedIntegral>
void* GetPtr(SignedIntegral& rel_color_ptr, size_t align, void const* base) {
    ZETA_Core_StaticAssert(IsSignedIntegral<SignedIntegral>);

    return __builtin_align_down(static_cast<char const*>(base) + rel_color_ptr,
                                align);
}

template <typename SignedIntegral>
void SetPtr(SignedIntegral& rel_color_ptr, size_t align, void const* base,
            void* ptr) {
    SetPtrColor(rel_color_ptr, align, base, ptr,
                GetColor(rel_color_ptr, align, base));
}

template <typename SignedIntegral>
int GetColor(SignedIntegral const& rel_color_ptr, size_t align,
             void const* base) {
    ZETA_Core_StaticAssert(IsSignedIntegral<SignedIntegral>);

    void* ptr{ static_cast<char const*>(base) + rel_color_ptr };

    return static_cast<int>(
        static_cast<char*>(ptr) -
        static_cast<char*>(__builtin_align_down(ptr, align)));
}

template <typename SignedIntegral>
void SetColor(SignedIntegral& rel_color_ptr, size_t align, void const* base,
              int color) {
    SetPtrColor(rel_color_ptr, align, base, GetPtr(rel_color_ptr, align, base),
                color);
}

template <typename SignedIntegral>
void SetPtrColor(SignedIntegral& rel_color_ptr, size_t align, void const* base,
                 void* ptr, int color) {
    ZETA_Core_StaticAssert(IsSignedIntegral<SignedIntegral>);

    ZETA_Core_DebugAssert(__builtin_is_aligned(ptr, align));

    ZETA_Core_DebugAssert(0 <= color &&
                          static_cast<unsigned long long>(color) < align);

    ptrdiff_t diff{ static_cast<char*>(ptr) + color -
                    static_cast<char const*>(base) };

    ZETA_Core_DebugAssert(range_min<SignedIntegral>() <= diff &&
                          diff <= range_max<SignedIntegral>());

    rel_color_ptr = static_cast<SignedIntegral>(diff);
}

}  // namespace rel_color_ptr

// -----------------------------------------------------------------------------

template <typename LinkType, bool EnColor>
void* AugPtrTpl<LinkType, EnColor>::GetPtr() const {
    ZETA_Core_StaticAssert(!EnRelLink);
    ZETA_Core_StaticAssert(!EnColor);

    return this->link;
}

template <typename LinkType, bool EnColor>
void* AugPtrTpl<LinkType, EnColor>::GetPtr(size_t align) const {
    ZETA_Core_StaticAssert(!EnRelLink);

    if constexpr (EnColor) {
        return color_ptr::GetPtr(this->link, align);
    } else {
        return this->GetPtr();
    }
}

template <typename LinkType, bool EnColor>
void* AugPtrTpl<LinkType, EnColor>::GetPtr(void const* base) const {
    ZETA_Core_StaticAssert(!EnColor);

    if constexpr (EnRelLink) {
        return rel_ptr::GetPtr(this->link, base);
    } else {
        return this->GetPtr();
    }
}

template <typename LinkType, bool EnColor>
void* AugPtrTpl<LinkType, EnColor>::GetPtr(size_t align,
                                           void const* base) const {
    if constexpr (!EnRelLink) {
        return this->GetPtr(align);
    } else if constexpr (!EnColor) {
        return this->GetPtr(base);
    } else {
        return rel_color_ptr::GetPtr(&this->link, align, base);
    }
}

template <typename LinkType, bool EnColor>
int AugPtrTpl<LinkType, EnColor>::GetColor(size_t align) const {
    ZETA_Core_StaticAssert(!EnRelLink);
    ZETA_Core_StaticAssert(EnColor);

    return color_ptr::GetColor(this->link, align);
}

template <typename LinkType, bool EnColor>
int AugPtrTpl<LinkType, EnColor>::GetColor(size_t align,
                                           void const* base) const {
    ZETA_Core_StaticAssert(EnColor);

    if constexpr (EnRelLink) {
        return rel_color_ptr::GetColor(this->link, align, base);
    } else {
        return this->GetColor(align);
    }
}

template <typename LinkType, bool EnColor>
void AugPtrTpl<LinkType, EnColor>::SetPtr(void* ptr) {
    ZETA_Core_StaticAssert(!EnRelLink);
    ZETA_Core_StaticAssert(!EnColor);

    this->link = ptr;
}

template <typename LinkType, bool EnColor>
void AugPtrTpl<LinkType, EnColor>::SetPtr(size_t align, void* ptr) {
    ZETA_Core_StaticAssert(!EnRelLink);

    if constexpr (EnColor) {
        color_ptr::SetPtr(this->link, align, ptr);
    } else {
        this->SetPtr(ptr);
    }
}

template <typename LinkType, bool EnColor>
void AugPtrTpl<LinkType, EnColor>::SetPtr(void const* base, void* ptr) {
    ZETA_Core_StaticAssert(!EnColor);

    if constexpr (EnRelLink) {
        rel_ptr::SetPtr(this->link, base, ptr);
    } else {
        this->SetPtr(ptr);
    }
}

template <typename LinkType, bool EnColor>
void AugPtrTpl<LinkType, EnColor>::SetPtr(size_t align, void const* base,
                                          void* ptr) {
    if constexpr (!EnRelLink) {
        this->SetPtr(align, ptr);
    } else if constexpr (!EnColor) {
        this->SetPtr(base, ptr);
    } else {
        rel_color_ptr::SetPtr(&this->link, align, base, ptr);
    }
}

template <typename LinkType, bool EnColor>
void AugPtrTpl<LinkType, EnColor>::SetColor(size_t align, int color) {
    ZETA_Core_StaticAssert(!EnRelLink);
    ZETA_Core_StaticAssert(EnColor);

    color_ptr::SetColor(this->link, align, color);
}

template <typename LinkType, bool EnColor>
void AugPtrTpl<LinkType, EnColor>::SetColor(size_t align, void const* base,
                                            int color) {
    ZETA_Core_StaticAssert(EnColor);

    if constexpr (EnRelLink) {
        rel_color_ptr::SetColor(this->link, align, base, color);
    } else {
        this->SetColor(align, color);
    }
}

template <typename LinkType, bool EnColor>
void AugPtrTpl<LinkType, EnColor>::SetPtrColor(size_t align, void* ptr,
                                               int color) {
    ZETA_Core_StaticAssert(!EnRelLink);
    ZETA_Core_StaticAssert(EnColor);

    color_ptr::SetPtrColor(this->link, align, ptr, color);
}

template <typename LinkType, bool EnColor>
void AugPtrTpl<LinkType, EnColor>::SetPtrColor(size_t align, void const* base,
                                               void* ptr, int color) {
    ZETA_Core_StaticAssert(EnColor);

    if constexpr (EnRelLink) {
        rel_color_ptr::SetPtrColor(this->link, align, base, ptr, color);
    } else {
        this->SetPtrColor(align, ptr, color);
    }
}

}  // namespace zeta::core::ptr_utils
