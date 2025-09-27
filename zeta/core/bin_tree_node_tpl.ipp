#pragma once

#include <zeta/core/bin_tree_node_tpl.hpp>
#include <zeta/core/debug_utils.ipp>

namespace zeta::core {

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
void BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::Init() {
    if constexpr (EnPColor) {
        this->p.SetPtrColor(alignof(BinTreeNodeTpl), this, this, 0);
    } else {
        this->p.SetPtr(alignof(BinTreeNodeTpl), this,
                       EnRelLink ? this : nullptr);
    }

    if constexpr (EnLColor) {
        this->l.SetPtrColor(alignof(BinTreeNodeTpl), this, this, 0);
    } else {
        this->l.SetPtr(alignof(BinTreeNodeTpl), this,
                       EnRelLink ? this : nullptr);
    }

    if constexpr (EnRColor) {
        this->r.SetPtrColor(alignof(BinTreeNodeTpl), this, this, 0);
    } else {
        this->r.SetPtr(alignof(BinTreeNodeTpl), EnRelLink ? this : nullptr,
                       nullptr);
    }
}

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>*
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::GetPPtr() {
    auto m{ static_cast<BinTreeNodeTpl*>(
        this->p.GetPtr(alignof(BinTreeNodeTpl), this)) };

    if constexpr (EnRelLink || EnPColor) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>*
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::GetLPtr() {
    auto m{ static_cast<BinTreeNodeTpl*>(
        this->l.GetPtr(alignof(BinTreeNodeTpl), this)) };

    if constexpr (EnRelLink || EnLColor) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>*
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::GetRPtr() {
    auto m{ static_cast<BinTreeNodeTpl*>(
        this->r.GetPtr(alignof(BinTreeNodeTpl), this)) };

    if constexpr (EnRelLink || EnRColor) {
        if (m == this) { return nullptr; }
    }

    return m;
}

// -----------------------------------------------------------------------------

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize> const*
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::GetPPtr()
    const {
    return const_cast<BinTreeNodeTpl*>(this)->GetPPtr();
}

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize> const*
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::GetLPtr()
    const {
    return const_cast<BinTreeNodeTpl*>(this)->GetLPtr();
}

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize> const*
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::GetRPtr()
    const {
    return const_cast<BinTreeNodeTpl*>(this)->GetRPtr();
}

// -----------------------------------------------------------------------------

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
int BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor,
                   EnAccSize>::GetPColor() const {
    return this->p.GetColor(alignof(BinTreeNodeTpl), this);
}

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
int BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor,
                   EnAccSize>::GetLColor() const {
    return this->l.GetColor(alignof(BinTreeNodeTpl), this);
}

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
int BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor,
                   EnAccSize>::GetRColor() const {
    return this->r.GetColor(alignof(BinTreeNodeTpl), this);
}

// -----------------------------------------------------------------------------

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
void BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::SetPPtr(
    BinTreeNodeTpl* m) {
    if constexpr (EnRelLink || EnPColor) { m = m == nullptr ? this : m; }

    this->p.SetPtr(alignof(BinTreeNodeTpl), this, m);
}

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
void BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::SetLPtr(
    BinTreeNodeTpl* m) {
    if constexpr (EnRelLink || EnLColor) { m = m == nullptr ? this : m; }

    this->l.SetPtr(alignof(BinTreeNodeTpl), this, m);
}

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
void BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::SetRPtr(
    BinTreeNodeTpl* m) {
    if constexpr (EnRelLink || EnRColor) { m = m == nullptr ? this : m; }

    this->r.SetPtr(alignof(BinTreeNodeTpl), this, m);
}

// -----------------------------------------------------------------------------

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
void BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor,
                    EnAccSize>::SetPColor(int color) {
    this->p.SetColor(alignof(BinTreeNodeTpl), this, color);
}

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
void BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor,
                    EnAccSize>::SetLColor(int color) {
    this->l.SetColor(alignof(BinTreeNodeTpl), this, color);
}

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
void BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor,
                    EnAccSize>::SetRColor(int color) {
    this->r.SetColor(alignof(BinTreeNodeTpl), this, color);
}

// -----------------------------------------------------------------------------

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
EnableIf<EnAccSize, size_t> BinTreeNodeTpl<
    LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::GetAccSize() const {
    ZETA_Core_StaticAssert(EnAccSize);

    return this->acc_size;
}

template <typename LinkType, bool EnPColor, bool EnLColor, bool EnRColor,
          bool EnAccSize>
EnableIf<EnAccSize, void>
BinTreeNodeTpl<LinkType, EnPColor, EnLColor, EnRColor, EnAccSize>::SetAccSize(
    size_t acc_size) {
    ZETA_Core_StaticAssert(EnAccSize);

    this->acc_size = acc_size;
}

}  // namespace zeta::core
