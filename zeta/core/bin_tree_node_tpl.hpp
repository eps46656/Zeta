#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/ptr_utils.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core {

template <typename LinkType, typename PColorTag, typename EnLColor,
          typename EnRColor, typename EnAccSize>
struct BinTreeNodeTpl;

// -----------------------------------------------------------------------------

template <typename LinkType, typename PColorTag, typename LColorTag,
          typename RColorTag, typename AccSizeTag>
struct BinTreeNodeTpl {
    ZETA_Core_StaticAssert(
        IsAnyOf<AccSizeTag, value_wrapper::TrueType, value_wrapper::FalseType>);

    ptr_utils::AugPtrTpl<LinkType, PColorTag> p;
    ptr_utils::AugPtrTpl<LinkType, LColorTag> l;
    ptr_utils::AugPtrTpl<LinkType, RColorTag> r;

    static constexpr bool IsRelLink{
        ptr_utils::AugPtrTpl<LinkType, PColorTag>::IsRelLink
    };

    Conditional<AccSizeTag::value, size_t, Monostate> acc_size;

    void Init();

    BinTreeNodeTpl* GetPPtr();
    BinTreeNodeTpl* GetLPtr();
    BinTreeNodeTpl* GetRPtr();

    BinTreeNodeTpl const* GetPPtr() const;
    BinTreeNodeTpl const* GetLPtr() const;
    BinTreeNodeTpl const* GetRPtr() const;

    unsigned GetPColor() const;
    unsigned GetLColor() const;
    unsigned GetRColor() const;

    void SetPPtr(BinTreeNodeTpl* m);
    void SetLPtr(BinTreeNodeTpl* m);
    void SetRPtr(BinTreeNodeTpl* m);

    void SetPColor(unsigned color);
    void SetLColor(unsigned color);
    void SetRColor(unsigned color);

    template <typename _ = void, typename = EnableIf<AccSizeTag::value, _>>
    size_t GetAccSize() const;

    template <typename _ = void, typename = EnableIf<AccSizeTag::value, _>>
    void SetAccSize(size_t acc_size);
}
#if ZETA_Core_ullong_width == 32
__attribute__((aligned(4)));
#elif ZETA_Core_ullong_width == 64
__attribute__((aligned(8)));
#else
#error "Unsupported architecture."
#endif

}  // namespace zeta::core
