#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/ptr_utils.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
struct BinTreeNodeTpl;

// -----------------------------------------------------------------------------

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
struct BinTreeNodeTpl {
    ZETA_Core_StaticAssert(
        IsAnyOf<EnAccSize, value_wrapper::StaticValueWrapper<true>,
                value_wrapper::StaticValueWrapper<false>>);

    ptr_utils::AugPtrTpl<LinkType, EnPColor> p;
    ptr_utils::AugPtrTpl<LinkType, EnLColor> l;
    ptr_utils::AugPtrTpl<LinkType, EnRColor> r;

    static constexpr bool EnRelLink{
        ptr_utils::AugPtrTpl<LinkType, EnPColor>::EnRelLink
    };

    Conditional<EnAccSize::value, size_t, Monostate> acc_size;

    void Init();

    BinTreeNodeTpl* GetPPtr();
    BinTreeNodeTpl* GetLPtr();
    BinTreeNodeTpl* GetRPtr();

    BinTreeNodeTpl const* GetPPtr() const;
    BinTreeNodeTpl const* GetLPtr() const;
    BinTreeNodeTpl const* GetRPtr() const;

    int GetPColor() const;
    int GetLColor() const;
    int GetRColor() const;

    void SetPPtr(BinTreeNodeTpl* m);
    void SetLPtr(BinTreeNodeTpl* m);
    void SetRPtr(BinTreeNodeTpl* m);

    void SetPColor(int color);
    void SetLColor(int color);
    void SetRColor(int color);

    template <typename _ = void>
    EnableIf<EnAccSize::value, size_t, _> GetAccSize() const;

    template <typename _ = void>
    EnableIf<EnAccSize::value, void, _> SetAccSize(size_t acc_size);
}
#if ZETA_Core_ullong_width == 32
__attribute__((aligned(4)));
#elif ZETA_Core_ullong_width == 64
__attribute__((aligned(8)));
#else
#error "Unsupported architecture."
#endif

}  // namespace zeta::core
