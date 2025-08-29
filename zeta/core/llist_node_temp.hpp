#pragma once

#include <zeta/core/define.h>

#include <zeta/core/ptr_utils.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

template <typename Link, bool LColor, bool RColor>
struct LListNodeTemp;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <typename Link, bool LColor, bool RColor>
struct LListNodeTemp {
    AdvancedPtr<Link, LColor> l;
    AdvancedPtr<Link, RColor> r;

    void Init();

    LListNodeTemp* GetLPtr();
    LListNodeTemp* GetRPtr();

    LListNodeTemp const* GetLPtr() const;
    LListNodeTemp const* GetRPtr() const;

    int GetLColor() const;
    int GetRColor() const;

    void SetLPtr(LListNodeTemp* m);
    void SetRPtr(LListNodeTemp* m);

    void SetLColor(int color);
    void SetRColor(int color);
} __attribute__((aligned(2)));

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <typename Link, bool LColor, bool RColor>
void LListNodeTemp<Link, LColor, RColor>::Init() {
    if constexpr (LColor) {
        this->l.SetPtrColor(alignof(LListNodeTemp), this, this, 0);
    } else {
        this->l.SetPtr(alignof(LListNodeTemp), this, rel_link ? this : nullptr);
    }

    if constexpr (RColor) {
        this->r.SetPtrColor(alignof(LListNodeTemp), this, this, 0);
    } else {
        this->r.SetPtr(alignof(LListNodeTemp), rel_link ? this : nullptr,
                       nullptr);
    }
}

// -----------------------------------------------------------------------------

template <typename Link, bool LColor, bool RColor>
LListNodeTemp<Link, LColor, RColor>*
LListNodeTemp<Link, LColor, RColor>::GetLPtr() {
    LListNodeTemp* m{ static_cast<LListNodeTemp*>(
        this->l.GetPtr(alignof(LListNodeTemp), this)) };

    if constexpr (rel_link || LColor) {
        if (m == this) { return nullptr; }
    }

    return m;
}

template <typename Link, bool LColor, bool RColor>
LListNodeTemp<Link, LColor, RColor>*
LListNodeTemp<Link, LColor, RColor>::GetRPtr() {
    LListNodeTemp* m{ static_cast<LListNodeTemp*>(
        this->r.GetPtr(alignof(LListNodeTemp), this)) };

    if constexpr (rel_link || RColor) {
        if (m == this) { return nullptr; }
    }

    return m;
}

// -----------------------------------------------------------------------------

template <typename Link, bool LColor, bool RColor>
LListNodeTemp<Link, LColor, RColor> const*
LListNodeTemp<Link, LColor, RColor>::GetLPtr() const {
    return const_cast<LListNodeTemp*>(this)->GetLPtr();
}

template <typename Link, bool LColor, bool RColor>
LListNodeTemp<Link, LColor, RColor> const*
LListNodeTemp<Link, LColor, RColor>::GetRPtr() const {
    return const_cast<LListNodeTemp*>(this)->GetRPtr();
}

// -----------------------------------------------------------------------------

template <typename Link, bool LColor, bool RColor>
int LListNodeTemp<Link, LColor, RColor>::GetLColor() const {
    return this->l.GetColor(alignof(LListNodeTemp), this);
}

template <typename Link, bool LColor, bool RColor>
int LListNodeTemp<Link, LColor, RColor>::GetRColor() const {
    return this->r.GetColor(alignof(LListNodeTemp), this);
}

// -----------------------------------------------------------------------------

template <typename Link, bool LColor, bool RColor>
void LListNodeTemp<Link, LColor, RColor>::SetLPtr(LListNodeTemp* m) {
    if constexpr (rel_link || LColor) { m = m == nullptr ? this : m; }
    this->l.SetPtr(alignof(LListNodeTemp), this, m);
}

template <typename Link, bool LColor, bool RColor>
void LListNodeTemp<Link, LColor, RColor>::SetRPtr(LListNodeTemp* m) {
    if constexpr (rel_link || RColor) { m = m == nullptr ? this : m; }
    this->r.SetPtr(alignof(LListNodeTemp), this, m);
}

// -----------------------------------------------------------------------------

template <typename Link, bool LColor, bool RColor>
void LListNodeTemp<Link, LColor, RColor>::SetLColor(int color) {
    this->l.SetColor(alignof(LListNodeTemp), this, color);
}

template <typename Link, bool LColor, bool RColor>
void LListNodeTemp<Link, LColor, RColor>::SetRColor(int color) {
    this->r.SetColor(alignof(LListNodeTemp), this, color);
}

}  // namespace zeta::core
