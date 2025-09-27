#pragma once

#include <zeta/core/llist_node_tpl.hpp>

namespace zeta::core {

template <typename LinkType, bool EnLColor, bool EnRColor>
void LListNodeTpl<LinkType, EnLColor, EnRColor>::Init() {
    if constexpr (EnLColor) {
        this->l.SetPtrColor(alignof(LListNodeTpl), this, this, 0);
    } else {
        this->l.SetPtr(alignof(LListNodeTpl), this, EnRelLink ? this : nullptr);
    }

    if constexpr (EnRColor) {
        this->r.SetPtrColor(alignof(LListNodeTpl), this, this, 0);
    } else {
        this->r.SetPtr(alignof(LListNodeTpl), EnRelLink ? this : nullptr,
                       nullptr);
    }
}

// -----------------------------------------------------------------------------

template <typename LinkType, bool EnLColor, bool EnRColor>
LListNodeTpl<LinkType, EnLColor, EnRColor>*
LListNodeTpl<LinkType, EnLColor, EnRColor>::GetLPtr() {
    auto m{ static_cast<LListNodeTpl*>(
        this->l.GetPtr(alignof(LListNodeTpl), this)) };

    if constexpr (EnRelLink || EnLColor) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <typename LinkType, bool EnLColor, bool EnRColor>
LListNodeTpl<LinkType, EnLColor, EnRColor>*
LListNodeTpl<LinkType, EnLColor, EnRColor>::GetRPtr() {
    auto m{ static_cast<LListNodeTpl*>(
        this->r.GetPtr(alignof(LListNodeTpl), this)) };

    if constexpr (EnRelLink || EnRColor) {
        if (m == this) { return nullptr; }
    }

    return m;
}

// -----------------------------------------------------------------------------

template <typename LinkType, bool EnLColor, bool EnRColor>
LListNodeTpl<LinkType, EnLColor, EnRColor> const*
LListNodeTpl<LinkType, EnLColor, EnRColor>::GetLPtr() const {
    return const_cast<LListNodeTpl*>(this)->GetLPtr();
}

template <typename LinkType, bool EnLColor, bool EnRColor>
LListNodeTpl<LinkType, EnLColor, EnRColor> const*
LListNodeTpl<LinkType, EnLColor, EnRColor>::GetRPtr() const {
    return const_cast<LListNodeTpl*>(this)->GetRPtr();
}

// -----------------------------------------------------------------------------

template <typename LinkType, bool EnLColor, bool EnRColor>
int LListNodeTpl<LinkType, EnLColor, EnRColor>::GetLColor() const {
    return this->l.GetColor(alignof(LListNodeTpl), this);
}

template <typename LinkType, bool EnLColor, bool EnRColor>
int LListNodeTpl<LinkType, EnLColor, EnRColor>::GetRColor() const {
    return this->r.GetColor(alignof(LListNodeTpl), this);
}

// -----------------------------------------------------------------------------

template <typename LinkType, bool EnLColor, bool EnRColor>
void LListNodeTpl<LinkType, EnLColor, EnRColor>::SetLPtr(LListNodeTpl* m) {
    if constexpr (EnRelLink || EnLColor) { m = m == nullptr ? this : m; }

    this->l.SetPtr(alignof(LListNodeTpl), this, m);
}

template <typename LinkType, bool EnLColor, bool EnRColor>
void LListNodeTpl<LinkType, EnLColor, EnRColor>::SetRPtr(LListNodeTpl* m) {
    if constexpr (EnRelLink || EnRColor) { m = m == nullptr ? this : m; }

    this->r.SetPtr(alignof(LListNodeTpl), this, m);
}

// -----------------------------------------------------------------------------

template <typename LinkType, bool EnLColor, bool EnRColor>
void LListNodeTpl<LinkType, EnLColor, EnRColor>::SetLColor(int color) {
    this->l.SetColor(alignof(LListNodeTpl), this, color);
}

template <typename LinkType, bool EnLColor, bool EnRColor>
void LListNodeTpl<LinkType, EnLColor, EnRColor>::SetRColor(int color) {
    this->r.SetColor(alignof(LListNodeTpl), this, color);
}

}  // namespace zeta::core
