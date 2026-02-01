#pragma once

#include <zeta/core/bin_tree_node_tpl.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/ptr_utils.ipp>

#pragma push_macro("TplDefParamList")
#pragma push_macro("TplParamList")

#define TplDefParamList                                       \
    typename LinkType, typename PColorTag, typename EnLColor, \
        typename EnRColor, typename EnAccSize

#define TplParamList LinkType, PColorTag, EnLColor, EnRColor, EnAccSize

namespace zeta::core {

template <TplDefParamList>
void BinTreeNodeTpl<TplParamList>::Init() {
    if constexpr (PColorTag::value) {
        this->p.SetPtrColor(alignof(BinTreeNodeTpl), this, this, 0);
    } else {
        this->p.SetPtr(alignof(BinTreeNodeTpl), this,
                       IsRelLink ? this : nullptr);
    }

    if constexpr (EnLColor::value) {
        this->l.SetPtrColor(alignof(BinTreeNodeTpl), this, this, 0);
    } else {
        this->l.SetPtr(alignof(BinTreeNodeTpl), this,
                       IsRelLink ? this : nullptr);
    }

    if constexpr (EnRColor::value) {
        this->r.SetPtrColor(alignof(BinTreeNodeTpl), this, this, 0);
    } else {
        this->r.SetPtr(alignof(BinTreeNodeTpl), this,
                       IsRelLink ? this : nullptr);
    }

    if constexpr (EnAccSize::value) { this->acc_size = 0; }
}

template <TplDefParamList>
BinTreeNodeTpl<TplParamList>* BinTreeNodeTpl<TplParamList>::GetPPtr() {
    auto m{ static_cast<BinTreeNodeTpl*>(
        this->p.GetPtr(alignof(BinTreeNodeTpl), this)) };

    if constexpr (IsRelLink || PColorTag::value) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <TplDefParamList>
BinTreeNodeTpl<TplParamList>* BinTreeNodeTpl<TplParamList>::GetLPtr() {
    auto m{ static_cast<BinTreeNodeTpl*>(
        this->l.GetPtr(alignof(BinTreeNodeTpl), this)) };

    if constexpr (IsRelLink || EnLColor::value) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <TplDefParamList>
BinTreeNodeTpl<TplParamList>* BinTreeNodeTpl<TplParamList>::GetRPtr() {
    auto m{ static_cast<BinTreeNodeTpl*>(
        this->r.GetPtr(alignof(BinTreeNodeTpl), this)) };

    if constexpr (IsRelLink || EnRColor::value) {
        if (m == this) { return nullptr; }
    }

    return m;
}

// -----------------------------------------------------------------------------

template <TplDefParamList>
BinTreeNodeTpl<TplParamList> const* BinTreeNodeTpl<TplParamList>::GetPPtr()
    const {
    return const_cast<BinTreeNodeTpl*>(this)->GetPPtr();
}

template <TplDefParamList>
BinTreeNodeTpl<TplParamList> const* BinTreeNodeTpl<TplParamList>::GetLPtr()
    const {
    return const_cast<BinTreeNodeTpl*>(this)->GetLPtr();
}

template <TplDefParamList>
BinTreeNodeTpl<TplParamList> const* BinTreeNodeTpl<TplParamList>::GetRPtr()
    const {
    return const_cast<BinTreeNodeTpl*>(this)->GetRPtr();
}

// -----------------------------------------------------------------------------

template <TplDefParamList>
unsigned BinTreeNodeTpl<TplParamList>::GetPColor() const {
    return this->p.GetColor(alignof(BinTreeNodeTpl), this);
}

template <TplDefParamList>
unsigned BinTreeNodeTpl<TplParamList>::GetLColor() const {
    return this->l.GetColor(alignof(BinTreeNodeTpl), this);
}

template <TplDefParamList>
unsigned BinTreeNodeTpl<TplParamList>::GetRColor() const {
    return this->r.GetColor(alignof(BinTreeNodeTpl), this);
}

// -----------------------------------------------------------------------------

template <TplDefParamList>
void BinTreeNodeTpl<TplParamList>::SetPPtr(BinTreeNodeTpl* m) {
    if constexpr (IsRelLink || PColorTag::value) {
        m = m == nullptr ? this : m;
    }

    this->p.SetPtr(alignof(BinTreeNodeTpl), this, m);
}

template <TplDefParamList>
void BinTreeNodeTpl<TplParamList>::SetLPtr(BinTreeNodeTpl* m) {
    if constexpr (IsRelLink || EnLColor::value) { m = m == nullptr ? this : m; }

    this->l.SetPtr(alignof(BinTreeNodeTpl), this, m);
}

template <TplDefParamList>
void BinTreeNodeTpl<TplParamList>::SetRPtr(BinTreeNodeTpl* m) {
    if constexpr (IsRelLink || EnRColor::value) { m = m == nullptr ? this : m; }

    this->r.SetPtr(alignof(BinTreeNodeTpl), this, m);
}

// -----------------------------------------------------------------------------

template <TplDefParamList>
void BinTreeNodeTpl<TplParamList>::SetPColor(unsigned color) {
    this->p.SetColor(alignof(BinTreeNodeTpl), this, color);
}

template <TplDefParamList>
void BinTreeNodeTpl<TplParamList>::SetLColor(unsigned color) {
    this->l.SetColor(alignof(BinTreeNodeTpl), this, color);
}

template <TplDefParamList>
void BinTreeNodeTpl<TplParamList>::SetRColor(unsigned color) {
    this->r.SetColor(alignof(BinTreeNodeTpl), this, color);
}

// -----------------------------------------------------------------------------

template <TplDefParamList>
template <typename, typename>
size_t BinTreeNodeTpl<TplParamList>::GetAccSize() const {
    return this->acc_size;
}

template <TplDefParamList>
template <typename, typename>
void BinTreeNodeTpl<TplParamList>::SetAccSize(size_t acc_size) {
    this->acc_size = acc_size;
}

}  // namespace zeta::core

#pragma pop_macro("TplDefParamList")
#pragma pop_macro("TplParamList")
