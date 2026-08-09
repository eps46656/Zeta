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

    template <typename _ = void>
        requires requires {
            requires meta::IsSame<_, void>;
            requires !IsRelLink && !ColorTag::value;
        }
    constexpr void* GetPtr() const;

    template <typename _ = void>
        requires requires {
            requires meta::IsSame<_, void>;
            requires !IsRelLink;
        }
    constexpr void* GetPtr(size_t align) const;

    template <typename _ = void>
        requires requires {
            requires meta::IsSame<_, void>;
            requires ColorTag::value;
        }
    constexpr void* GetPtr(void const* base) const;

    constexpr void* GetPtr(size_t align, void const* base) const;

    template <typename _ = void>
        requires requires {
            requires meta::IsSame<_, void>;
            requires !IsRelLink && ColorTag::value;
        }
    constexpr unsigned GetColor(size_t align) const;

    template <typename _ = void>
        requires requires {
            requires meta::IsSame<_, void>;
            requires ColorTag::value;
        }
    constexpr unsigned GetColor(size_t align, void const* base) const;

    template <typename _ = void>
        requires requires {
            requires meta::IsSame<_, void>;
            requires !IsRelLink && !ColorTag::value;
        }
    constexpr void SetPtr(void* ptr);

    template <typename _ = void>
        requires requires {
            requires meta::IsSame<_, void>;
            requires !IsRelLink;
        }
    constexpr void SetPtr(size_t align, void* ptr);

    template <typename _ = void>
        requires requires {
            requires meta::IsSame<_, void>;
            requires !ColorTag::value;
        }
    constexpr void SetPtr(void const* base, void* ptr);

    constexpr void SetPtr(size_t align, void const* base, void* ptr);

    template <typename _ = void>
        requires requires {
            requires meta::IsSame<_, void>;
            requires !IsRelLink && ColorTag::value;
        }
    constexpr void SetColor(size_t align, unsigned color);

    template <typename _ = void>
        requires requires {
            requires meta::IsSame<_, void>;
            requires ColorTag::value;
        }
    constexpr void SetColor(size_t align, void const* base, unsigned color);

    template <typename _ = void>
        requires requires {
            requires meta::IsSame<_, void>;
            requires !IsRelLink && ColorTag::value;
        }
    constexpr void SetPtrColor(size_t align, void* ptr, unsigned color);

    template <typename _ = void>
        requires requires {
            requires meta::IsSame<_, void>;
            requires ColorTag::value;
        }
    constexpr void SetPtrColor(size_t align, void const* base, void* ptr,
                               unsigned color);
};

}  // namespace zeta::core::ptr_utils
