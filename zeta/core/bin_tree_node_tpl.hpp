#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/ptr_utils.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
struct BinTreeNodeTpl;

// -----------------------------------------------------------------------------

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
struct BinTreeNodeTpl {
    ptr_utils::AugPtrTpl<LinkType, EnPColor> p;
    ptr_utils::AugPtrTpl<LinkType, EnLColor> l;
    ptr_utils::AugPtrTpl<LinkType, EnRColor> r;

    static constexpr bool EnRelLink{
        ptr_utils::AugPtrTpl<LinkType, EnPColor>::EnRelLink
    };

    Conditional<EnAccSize, size_t, Monostate> acc_size;

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

    EnableIf<EnAccSize, size_t> GetAccSize() const;
    EnableIf<EnAccSize, void> SetAccSize(size_t acc_size);
}
#if ZETA_Core_ullong_width == 32
__attribute__((aligned(4)));
#elif ZETA_Core_ullong_width == 64
__attribute__((aligned(8)));
#else
#error "Unsupported architecture."
#endif

}  // namespace zeta::core
