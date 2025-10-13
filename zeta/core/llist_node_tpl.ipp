#pragma once

#include <zeta/core/llist_node_tpl.hpp>
#include <zeta/core/ptr_utils.ipp>

namespace zeta::core {

template <typename LinkType, typename EnLColor, typename EnRColor>
void LListNodeTpl<LinkType, EnLColor, EnRColor>::Init() {
    if constexpr (EnLColor::value) {
        this->l.SetPtrColor(alignof(LListNodeTpl), this, this, 0);
    } else {
        this->l.SetPtr(alignof(LListNodeTpl), this, this);
    }

    if constexpr (EnRColor::value) {
        this->r.SetPtrColor(alignof(LListNodeTpl), this, this, 0);
    } else {
        this->r.SetPtr(alignof(LListNodeTpl), this, this);
    }
}

// -----------------------------------------------------------------------------

template <typename LinkType, typename EnLColor, typename EnRColor>
auto LListNodeTpl<LinkType, EnLColor, EnRColor>::GetLPtr() -> LListNodeTpl* {
    return static_cast<LListNodeTpl*>(
        this->l.GetPtr(alignof(LListNodeTpl), this));
}

template <typename LinkType, typename EnLColor, typename EnRColor>
auto LListNodeTpl<LinkType, EnLColor, EnRColor>::GetRPtr() -> LListNodeTpl* {
    return static_cast<LListNodeTpl*>(
        this->r.GetPtr(alignof(LListNodeTpl), this));
}

// -----------------------------------------------------------------------------

template <typename LinkType, typename EnLColor, typename EnRColor>
LListNodeTpl<LinkType, EnLColor, EnRColor> const*
LListNodeTpl<LinkType, EnLColor, EnRColor>::GetLPtr() const {
    return const_cast<LListNodeTpl*>(this)->GetLPtr();
}

template <typename LinkType, typename EnLColor, typename EnRColor>
LListNodeTpl<LinkType, EnLColor, EnRColor> const*
LListNodeTpl<LinkType, EnLColor, EnRColor>::GetRPtr() const {
    return const_cast<LListNodeTpl*>(this)->GetRPtr();
}

// -----------------------------------------------------------------------------

template <typename LinkType, typename EnLColor, typename EnRColor>
int LListNodeTpl<LinkType, EnLColor, EnRColor>::GetLColor() const {
    return this->l.GetColor(alignof(LListNodeTpl), this);
}

template <typename LinkType, typename EnLColor, typename EnRColor>
int LListNodeTpl<LinkType, EnLColor, EnRColor>::GetRColor() const {
    return this->r.GetColor(alignof(LListNodeTpl), this);
}

// -----------------------------------------------------------------------------

template <typename LinkType, typename EnLColor, typename EnRColor>
void LListNodeTpl<LinkType, EnLColor, EnRColor>::SetLPtr(LListNodeTpl* m) {
    if constexpr (EnRelLink || EnLColor::value) { m = m == nullptr ? this : m; }

    this->l.SetPtr(alignof(LListNodeTpl), this, m);
}

template <typename LinkType, typename EnLColor, typename EnRColor>
void LListNodeTpl<LinkType, EnLColor, EnRColor>::SetRPtr(LListNodeTpl* m) {
    if constexpr (EnRelLink || EnRColor::value) { m = m == nullptr ? this : m; }

    this->r.SetPtr(alignof(LListNodeTpl), this, m);
}

// -----------------------------------------------------------------------------

template <typename LinkType, typename EnLColor, typename EnRColor>
void LListNodeTpl<LinkType, EnLColor, EnRColor>::SetLColor(int color) {
    this->l.SetColor(alignof(LListNodeTpl), this, color);
}

template <typename LinkType, typename EnLColor, typename EnRColor>
void LListNodeTpl<LinkType, EnLColor, EnRColor>::SetRColor(int color) {
    this->r.SetColor(alignof(LListNodeTpl), this, color);
}

// -----------------------------------------------------------------------------

template <typename LinkType, typename EnLColor, typename EnRColor>
LListNodeTpl<LinkType, EnLColor, EnRColor>* LListNodeTplOperator::GetL(
    LListNodeTpl<LinkType, EnLColor, EnRColor>* n) const {
    return n->GetLPtr();
}

template <typename LinkType, typename EnLColor, typename EnRColor>
LListNodeTpl<LinkType, EnLColor, EnRColor>* LListNodeTplOperator::GetR(
    LListNodeTpl<LinkType, EnLColor, EnRColor>* n) const {
    return n->GetRPtr();
}

template <typename LinkType, typename EnLColor, typename EnRColor>
void LListNodeTplOperator::SetL(
    LListNodeTpl<LinkType, EnLColor, EnRColor>* n,
    LListNodeTpl<LinkType, EnLColor, EnRColor>* m) const {
    n->SetLPtr(m);
}

template <typename LinkType, typename EnLColor, typename EnRColor>
void LListNodeTplOperator::SetR(
    LListNodeTpl<LinkType, EnLColor, EnRColor>* n,
    LListNodeTpl<LinkType, EnLColor, EnRColor>* m) const {
    n->SetRPtr(m);
}

}  // namespace zeta::core
