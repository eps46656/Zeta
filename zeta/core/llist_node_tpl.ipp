#pragma once

#include <zeta/core/debug_utils.hpp>
#include <zeta/core/llist_node_tpl.hpp>
#include <zeta/core/ptr_utils.ipp>

#define TplDefParamList \
    typename LinkType, typename LColorTag, typename RColorTag

#define TplParamList LinkType, LColorTag, RColorTag

namespace zeta::core {

template <TplDefParamList>
void LListNodeTpl<TplParamList>::Init() {
    if constexpr (LColorTag::value) {
        this->l.SetPtrColor(alignof(LListNodeTpl), this, this, 0);
    } else {
        this->l.SetPtr(alignof(LListNodeTpl), this, this);
    }

    if constexpr (RColorTag::value) {
        this->r.SetPtrColor(alignof(LListNodeTpl), this, this, 0);
    } else {
        this->r.SetPtr(alignof(LListNodeTpl), this, this);
    }
}

template <TplDefParamList>
LListNodeTpl<TplParamList>* LListNodeTpl<TplParamList>::GetLPtr() {
    return static_cast<LListNodeTpl*>(
        this->l.GetPtr(alignof(LListNodeTpl), this));
}

template <TplDefParamList>
LListNodeTpl<TplParamList>* LListNodeTpl<TplParamList>::GetRPtr() {
    return static_cast<LListNodeTpl*>(
        this->r.GetPtr(alignof(LListNodeTpl), this));
}

template <TplDefParamList>
LListNodeTpl<TplParamList> const* LListNodeTpl<TplParamList>::GetLPtr() const {
    return const_cast<LListNodeTpl*>(this)->GetLPtr();
}

template <TplDefParamList>
LListNodeTpl<TplParamList> const* LListNodeTpl<TplParamList>::GetRPtr() const {
    return const_cast<LListNodeTpl*>(this)->GetRPtr();
}

template <TplDefParamList>
int LListNodeTpl<TplParamList>::GetLColor() const {
    return this->l.GetColor(alignof(LListNodeTpl), this);
}

template <TplDefParamList>
int LListNodeTpl<TplParamList>::GetRColor() const {
    return this->r.GetColor(alignof(LListNodeTpl), this);
}

template <TplDefParamList>
void LListNodeTpl<TplParamList>::SetLPtr(LListNodeTpl* m) {
    ZETA_Core_DebugAssert(m != nullptr);

    this->l.SetPtr(alignof(LListNodeTpl), this, m);
}

template <TplDefParamList>
void LListNodeTpl<TplParamList>::SetRPtr(LListNodeTpl* m) {
    ZETA_Core_DebugAssert(m != nullptr);

    this->r.SetPtr(alignof(LListNodeTpl), this, m);
}

template <TplDefParamList>
void LListNodeTpl<TplParamList>::SetLColor(int color) {
    this->l.SetColor(alignof(LListNodeTpl), this, color);
}

template <TplDefParamList>
void LListNodeTpl<TplParamList>::SetRColor(int color) {
    this->r.SetColor(alignof(LListNodeTpl), this, color);
}

template <TplDefParamList>
LListNodeTplView<TplParamList>* LListNodeTpl<TplParamList>::AsView() {
    return reinterpret_cast<LListNodeTplView<TplParamList>*>(this);
}

template <TplDefParamList>
LListNodeTplView<TplParamList> const* LListNodeTpl<TplParamList>::AsView()
    const {
    return reinterpret_cast<LListNodeTplView<TplParamList> const*>(this);
}

// -----------------------------------------------------------------------------

template <TplDefParamList>
constexpr bool LListNodeTplView<TplParamList>::IsConst(
    type_wrapper::TypeWrapper<LListNodeTplView*>) {
    return false;
}

template <TplDefParamList>
constexpr bool LListNodeTplView<TplParamList>::IsConst(
    type_wrapper::TypeWrapper<LListNodeTplView const*>) {
    return true;
}

template <TplDefParamList>
LListNodeTplView<TplParamList>* LListNodeTplView<TplParamList>::GetL(
    LListNodeTplView* n_) {
    auto n{ reinterpret_cast<LListNodeTpl<TplParamList>*>(n_) };
    return reinterpret_cast<LListNodeTplView*>(n->GetLPtr());
}

template <TplDefParamList>
LListNodeTplView<TplParamList> const* LListNodeTplView<TplParamList>::GetL(
    LListNodeTplView const* n_) {
    auto n{ reinterpret_cast<LListNodeTpl<TplParamList> const*>(n_) };
    return reinterpret_cast<LListNodeTplView const*>(n->GetLPtr());
}

template <TplDefParamList>
LListNodeTplView<TplParamList>* LListNodeTplView<TplParamList>::GetR(
    LListNodeTplView* n_) {
    auto n{ reinterpret_cast<LListNodeTpl<TplParamList>*>(n_) };
    return reinterpret_cast<LListNodeTplView*>(n->GetRPtr());
}

template <TplDefParamList>
LListNodeTplView<TplParamList> const* LListNodeTplView<TplParamList>::GetR(
    LListNodeTplView const* n_) {
    auto n{ reinterpret_cast<LListNodeTpl<TplParamList> const*>(n_) };
    return reinterpret_cast<LListNodeTplView const*>(n->GetRPtr());
}

template <TplDefParamList>
void LListNodeTplView<TplParamList>::SetL(LListNodeTplView* n_,
                                          LListNodeTplView* m_) {
    auto n{ reinterpret_cast<LListNodeTpl<TplParamList>*>(n_) };
    auto m{ reinterpret_cast<LListNodeTpl<TplParamList>*>(m_) };

    n->SetLPtr(m);
}

template <TplDefParamList>
void LListNodeTplView<TplParamList>::SetR(LListNodeTplView* n_,
                                          LListNodeTplView* m_) {
    auto n{ reinterpret_cast<LListNodeTpl<TplParamList>*>(n_) };
    auto m{ reinterpret_cast<LListNodeTpl<TplParamList>*>(m_) };

    n->SetRPtr(m);
}

}  // namespace zeta::core
