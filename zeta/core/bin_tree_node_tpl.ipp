#pragma once

#include <zeta/core/bin_tree_node_tpl.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/ptr_utils.ipp>
#include <zeta/core/type_traits.hpp>

namespace zeta::core {

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
void BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::Init() {
    if constexpr (EnPColor::value) {
        this->p.SetPtrColor(alignof(BinTreeNodeTpl), this, this, 0);
    } else {
        this->p.SetPtr(alignof(BinTreeNodeTpl), this,
                       EnRelLink ? this : nullptr);
    }

    if constexpr (EnLColor::value) {
        this->l.SetPtrColor(alignof(BinTreeNodeTpl), this, this, 0);
    } else {
        this->l.SetPtr(alignof(BinTreeNodeTpl), this,
                       EnRelLink ? this : nullptr);
    }

    if constexpr (EnRColor::value) {
        this->r.SetPtrColor(alignof(BinTreeNodeTpl), this, this, 0);
    } else {
        this->r.SetPtr(alignof(BinTreeNodeTpl), this,
                       EnRelLink ? this : nullptr);
    }

    if constexpr (EnAccSize::value) { this->acc_size = 0; }
}

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>*
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::GetPPtr() {
    auto m{ static_cast<BinTreeNodeTpl*>(
        this->p.GetPtr(alignof(BinTreeNodeTpl), this)) };

    if constexpr (EnRelLink || EnPColor::value) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>*
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::GetLPtr() {
    auto m{ static_cast<BinTreeNodeTpl*>(
        this->l.GetPtr(alignof(BinTreeNodeTpl), this)) };

    if constexpr (EnRelLink || EnLColor::value) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>*
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::GetRPtr() {
    auto m{ static_cast<BinTreeNodeTpl*>(
        this->r.GetPtr(alignof(BinTreeNodeTpl), this)) };

    if constexpr (EnRelLink || EnRColor::value) {
        if (m == this) { return nullptr; }
    }

    return m;
}

// -----------------------------------------------------------------------------

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize> const*
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::GetPPtr()
    const {
    return const_cast<BinTreeNodeTpl*>(this)->GetPPtr();
}

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize> const*
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::GetLPtr()
    const {
    return const_cast<BinTreeNodeTpl*>(this)->GetLPtr();
}

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize> const*
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::GetRPtr()
    const {
    return const_cast<BinTreeNodeTpl*>(this)->GetRPtr();
}

// -----------------------------------------------------------------------------

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
unsigned BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor,
                        EnAccSize>::GetPColor() const {
    return this->p.GetColor(alignof(BinTreeNodeTpl), this);
}

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
unsigned BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor,
                        EnAccSize>::GetLColor() const {
    return this->l.GetColor(alignof(BinTreeNodeTpl), this);
}

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
unsigned BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor,
                        EnAccSize>::GetRColor() const {
    return this->r.GetColor(alignof(BinTreeNodeTpl), this);
}

// -----------------------------------------------------------------------------

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
void BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::SetPPtr(
    BinTreeNodeTpl* m) {
    if constexpr (EnRelLink || EnPColor::value) { m = m == nullptr ? this : m; }

    this->p.SetPtr(alignof(BinTreeNodeTpl), this, m);
}

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
void BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::SetLPtr(
    BinTreeNodeTpl* m) {
    if constexpr (EnRelLink || EnLColor::value) { m = m == nullptr ? this : m; }

    this->l.SetPtr(alignof(BinTreeNodeTpl), this, m);
}

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
void BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::SetRPtr(
    BinTreeNodeTpl* m) {
    if constexpr (EnRelLink || EnRColor::value) { m = m == nullptr ? this : m; }

    this->r.SetPtr(alignof(BinTreeNodeTpl), this, m);
}

// -----------------------------------------------------------------------------

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
void BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor,
                    EnAccSize>::SetPColor(unsigned color) {
    this->p.SetColor(alignof(BinTreeNodeTpl), this, color);
}

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
void BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor,
                    EnAccSize>::SetLColor(unsigned color) {
    this->l.SetColor(alignof(BinTreeNodeTpl), this, color);
}

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
void BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor,
                    EnAccSize>::SetRColor(unsigned color) {
    this->r.SetColor(alignof(BinTreeNodeTpl), this, color);
}

// -----------------------------------------------------------------------------

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
template <typename, typename>
size_t BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor,
                      EnAccSize>::GetAccSize() const {
    return this->acc_size;
}

template <typename LinkType, typename EnPColor, typename EnLColor,
          typename EnRColor, typename EnAccSize>
template <typename, typename>
void BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor,
                    EnAccSize>::SetAccSize(size_t acc_size) {
    this->acc_size = acc_size;
}

}  // namespace zeta::core
