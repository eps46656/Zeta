#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/ptr_utils.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

template <typename LinkType, typename EnLColor, typename EnRColor>
struct LListNodeTpl;

// -----------------------------------------------------------------------------

template <typename LinkType, typename EnLColor, typename EnRColor>
struct LListNodeTpl {
    ptr_utils::AugPtrTpl<LinkType, EnLColor> l;
    ptr_utils::AugPtrTpl<LinkType, EnRColor> r;

    static constexpr bool EnRelLink{
        ptr_utils::AugPtrTpl<LinkType, EnLColor>::EnRelLink
    };

    void Init();

    LListNodeTpl* GetLPtr();
    LListNodeTpl* GetRPtr();

    LListNodeTpl const* GetLPtr() const;
    LListNodeTpl const* GetRPtr() const;

    int GetLColor() const;
    int GetRColor() const;

    void SetLPtr(LListNodeTpl* m);
    void SetRPtr(LListNodeTpl* m);

    void SetLColor(int color);
    void SetRColor(int color);
}
#if ZETA_Core_ullong_width == 32
__attribute__((aligned(4)));
#elif ZETA_Core_ullong_width == 64
__attribute__((aligned(8)));
#else
#error "Unsupported architecture."
#endif

struct LListNodeTplOperator {
    template <typename LinkType, typename EnLColor, typename EnRColor>
    LListNodeTpl<LinkType, EnLColor, EnRColor>* GetL(
        LListNodeTpl<LinkType, EnLColor, EnRColor>* n) const;

    template <typename LinkType, typename EnLColor, typename EnRColor>
    LListNodeTpl<LinkType, EnLColor, EnRColor>* GetR(
        LListNodeTpl<LinkType, EnLColor, EnRColor>* n) const;

    template <typename LinkType, typename EnLColor, typename EnRColor>
    void SetL(LListNodeTpl<LinkType, EnLColor, EnRColor>* n,
              LListNodeTpl<LinkType, EnLColor, EnRColor>* m) const;

    template <typename LinkType, typename EnLColor, typename EnRColor>
    void SetR(LListNodeTpl<LinkType, EnLColor, EnRColor>* n,
              LListNodeTpl<LinkType, EnLColor, EnRColor>* m) const;
};

}  // namespace zeta::core
