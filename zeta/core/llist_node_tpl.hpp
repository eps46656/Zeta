#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/ptr_utils.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

template <typename LinkType, typename LColorTag, typename RColorTag>
struct LListNodeTpl;

template <typename LinkType, typename LColorTag, typename RColorTag>
struct LListNodeTplView;

// -----------------------------------------------------------------------------

template <typename LinkType, typename LColorTag, typename RColorTag>
struct LListNodeTpl {
    ptr_utils::AugPtrTpl<LinkType, LColorTag> l;
    ptr_utils::AugPtrTpl<LinkType, RColorTag> r;

    static constexpr bool IsRelLink{
        ptr_utils::AugPtrTpl<LinkType, LColorTag>::IsRelLink
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

    LListNodeTplView<LinkType, LColorTag, RColorTag>* AsView();
    LListNodeTplView<LinkType, LColorTag, RColorTag> const* AsView() const;
}
#if ZETA_Core_ullong_width == 32
__attribute__((aligned(4)));
#elif ZETA_Core_ullong_width == 64
__attribute__((aligned(8)));
#else
#error "Unsupported architecture."
#endif

template <typename LinkType, typename LColorTag, typename RColorTag>
struct LListNodeTplView {
    static constexpr bool IsConst(type_wrapper::TypeWrapper<LListNodeTplView*>);

    static constexpr bool IsConst(
        type_wrapper::TypeWrapper<LListNodeTplView const*>);

    static LListNodeTplView* GetL(LListNodeTplView* n);

    static LListNodeTplView const* GetL(LListNodeTplView const* n);

    static LListNodeTplView* GetR(LListNodeTplView* n);

    static LListNodeTplView const* GetR(LListNodeTplView const* n);

    static void SetL(LListNodeTplView* n, LListNodeTplView* m);

    static void SetR(LListNodeTplView* n, LListNodeTplView* m);
};

}  // namespace zeta::core
