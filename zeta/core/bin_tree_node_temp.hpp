#pragma once

#include <zeta/core/define.h>

#include <zeta/core/ptr_utils.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
struct BinTreeNodeTemp;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
struct BinTreeNodeTemp {
    static constexpr bool rel_link{ !is_same<Link, void*> };

    static constexpr bool has_acc_size{ !is_same<AccSize, void> };

    static constexpr AccSize null_acc_size{ 0 };

    AdvancedPtr<Link, PColor> p;
    AdvancedPtr<Link, LColor> l;
    AdvancedPtr<Link, RColor> r;

    conditional_t<has_acc_size, AccSize, Monostate> acc_size;

    void Init();

    BinTreeNodeTemp* GetPPtr();
    BinTreeNodeTemp* GetLPtr();
    BinTreeNodeTemp* GetRPtr();

    BinTreeNodeTemp const* GetPPtr() const;
    BinTreeNodeTemp const* GetLPtr() const;
    BinTreeNodeTemp const* GetRPtr() const;

    int GetPColor() const;
    int GetLColor() const;
    int GetRColor() const;

    void SetPPtr(BinTreeNodeTemp* m);
    void SetLPtr(BinTreeNodeTemp* m);
    void SetRPtr(BinTreeNodeTemp* m);

    void SetPColor(int color);
    void SetLColor(int color);
    void SetRColor(int color);

    AccSize GetAccSize() const;
    void SetAccSize(AccSize acc_size);
} __attribute__((aligned(2)));

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
void BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::Init() {
    if constexpr (PColor) {
        this->p.SetPtrColor(alignof(BinTreeNodeTemp), this, this, 0);
    } else {
        this->p.SetPtr(alignof(BinTreeNodeTemp), this,
                       rel_link ? this : nullptr);
    }

    if constexpr (LColor) {
        this->l.SetPtrColor(alignof(BinTreeNodeTemp), this, this, 0);
    } else {
        this->l.SetPtr(alignof(BinTreeNodeTemp), this,
                       rel_link ? this : nullptr);
    }

    if constexpr (RColor) {
        this->r.SetPtrColor(alignof(BinTreeNodeTemp), this, this, 0);
    } else {
        this->r.SetPtr(alignof(BinTreeNodeTemp), rel_link ? this : nullptr,
                       nullptr);
    }
}

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>*
BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::GetPPtr() {
    BinTreeNodeTemp* m{ static_cast<BinTreeNodeTemp*>(
        this->p.GetPtr(alignof(BinTreeNodeTemp), this)) };

    if constexpr (rel_link || PColor) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>*
BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::GetLPtr() {
    BinTreeNodeTemp* m{ static_cast<BinTreeNodeTemp*>(
        this->l.GetPtr(alignof(BinTreeNodeTemp), this)) };

    if constexpr (rel_link || LColor) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>*
BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::GetRPtr() {
    BinTreeNodeTemp* m{ static_cast<BinTreeNodeTemp*>(
        this->r.GetPtr(alignof(BinTreeNodeTemp), this)) };

    if constexpr (rel_link || RColor) {
        if (m == this) { return nullptr; }
    }

    return m;
}

// -----------------------------------------------------------------------------

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize> const*
BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::GetPPtr() const {
    return const_cast<BinTreeNodeTemp*>(this)->GetPPtr();
}

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize> const*
BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::GetLPtr() const {
    return const_cast<BinTreeNodeTemp*>(this)->GetLPtr();
}

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize> const*
BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::GetRPtr() const {
    return const_cast<BinTreeNodeTemp*>(this)->GetRPtr();
}

// -----------------------------------------------------------------------------

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
int BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::GetPColor() const {
    return this->p.GetColor(alignof(BinTreeNodeTemp), this);
}

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
int BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::GetLColor() const {
    return this->l.GetColor(alignof(BinTreeNodeTemp), this);
}

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
int BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::GetRColor() const {
    return this->r.GetColor(alignof(BinTreeNodeTemp), this);
}

// -----------------------------------------------------------------------------

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
void BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::SetPPtr(
    BinTreeNodeTemp* m) {
    if constexpr (rel_link || PColor) { m = m == nullptr ? this : m; }
    this->p.SetPtr(alignof(BinTreeNodeTemp), this, m);
}

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
void BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::SetLPtr(
    BinTreeNodeTemp* m) {
    if constexpr (rel_link || LColor) { m = m == nullptr ? this : m; }
    this->l.SetPtr(alignof(BinTreeNodeTemp), this, m);
}

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
void BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::SetRPtr(
    BinTreeNodeTemp* m) {
    if constexpr (rel_link || RColor) { m = m == nullptr ? this : m; }
    this->r.SetPtr(alignof(BinTreeNodeTemp), this, m);
}

// -----------------------------------------------------------------------------

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
void BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::SetPColor(
    int color) {
    this->p.SetColor(alignof(BinTreeNodeTemp), this, color);
}

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
void BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::SetLColor(
    int color) {
    this->l.SetColor(alignof(BinTreeNodeTemp), this, color);
}

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
void BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::SetRColor(
    int color) {
    this->r.SetColor(alignof(BinTreeNodeTemp), this, color);
}

// -----------------------------------------------------------------------------

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
AccSize BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::GetAccSize()
    const {
    ZETA_Core_StaticAssert(has_acc_size);
    return this->acc_size;
}

template <typename Link, bool PColor, bool LColor, bool RColor,
          typename AccSize>
void BinTreeNodeTemp<Link, PColor, LColor, RColor, AccSize>::SetAccSize(
    AccSize acc_size) {
    ZETA_Core_StaticAssert(has_acc_size);
    this->acc_size = acc_size;
}

}  // namespace zeta::core
