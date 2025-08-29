#pragma once

#include <zeta/core/define.h>

#define ZETA_Core_ColorPtr_GetPtr(color_ptr, align) \
    __builtin_align_down(*(color_ptr), (align))

#define ZETA_Core_ColorPtr_GetColor_(tmp, color_ptr, align)            \
    ({                                                                 \
        char* tmp{ ZETA_Core_ToCharPtr(*(color_ptr)) };                \
        tmp - ZETA_Core_ToCharPtr(__builtin_align_down(tmp, (align))); \
    })

#define ZETA_Core_ColorPtr_GetColor(color_ptr, align) \
    ZETA_Core_ColorPtr_GetColor_(ZETA_Core_TmpName, (color_ptr), (align))

#define ZETA_Core_ColorPtr_Set_(tmp_color_ptr, tmp_color, color_ptr, align, \
                                ptr, color)                                 \
    {                                                                       \
        ZETA_Core_AutoVar(tmp_color_ptr, (color_ptr));                      \
        ZETA_Core_AutoVar(tmp_color, (color));                              \
        ZETA_Core_DebugAssert(0 <= tmp_color &&                             \
                              (unsigned long long)tmp_color < (align));     \
        *tmp_color_ptr = ZETA_Core_ToCharPtr((ptr)) + tmp_color;            \
    }                                                                       \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_ColorPtr_Set(color_ptr, align, ptr, color)                   \
    ZETA_Core_ColorPtr_Set_(ZETA_Core_TmpName, ZETA_Core_TmpName, (color_ptr), \
                            (align), (ptr), (color))

#define ZETA_Core_ColorPtr_SetPtr_(tmp_color_ptr, tmp_align, tmp_ptr, \
                                   color_ptr, align, ptr)             \
    {                                                                 \
        ZETA_Core_AutoVar(tmp_color_ptr, (color_ptr));                \
        ZETA_Core_AutoVar(tmp_align, (align));                        \
        ZETA_Core_AutoVar(tmp_ptr, (ptr));                            \
        ZETA_Core_ColorPtr_Set(                                       \
            tmp_color_ptr, tmp_align, tmp_ptr,                        \
            (ZETA_Core_ColorPtr_GetColor(tmp_color_ptr, tmp_align))); \
    }                                                                 \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_ColorPtr_SetPtr(color_ptr, align, ptr)             \
    ZETA_Core_ColorPtr_SetPtr_(ZETA_Core_TmpName, ZETA_Core_TmpName, \
                               ZETA_Core_TmpName, (color_ptr), (align), (ptr))

#define ZETA_Core_ColorPtr_SetColor_(tmp_color_ptr, tmp_align, tmp_color,      \
                                     color_ptr, align, color)                  \
    {                                                                          \
        ZETA_Core_AutoVar(tmp_color_ptr, (color_ptr));                         \
        ZETA_Core_AutoVar(tmp_align, (align));                                 \
        ZETA_Core_AutoVar(tmp_color, (color));                                 \
        ZETA_Core_ColorPtr_Set(                                                \
            tmp_color_ptr, tmp_align,                                          \
            (ZETA_Core_ColorPtr_GetPtr(tmp_color_ptr, tmp_align)), tmp_color); \
    }                                                                          \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_ColorPtr_SetColor(color_ptr, align, color)              \
    ZETA_Core_ColorPtr_SetColor_(ZETA_Core_TmpName, ZETA_Core_TmpName,    \
                                 ZETA_Core_TmpName, (color_ptr), (align), \
                                 (color))

// -----------------------------------------------------------------------------

#define ZETA_Core_RelPtr_GetPtr(rel_ptr, base) \
    (ZETA_Core_ToCharPtr((base)) + *(rel_ptr))

#define ZETA_Core_RelPtr_SetPtr_(tmp_rel_ptr, tmp_val, rel_ptr, base, ptr) \
    {                                                                      \
        ZETA_Core_AutoVar(tmp_rel_ptr, (rel_ptr));                         \
        ZETA_Core_AutoVar(tmp_val, (ZETA_Core_ToCharPtr((ptr)) -           \
                                    ZETA_Core_ToCharPtr((base))));         \
        *tmp_rel_ptr = tmp_val;                                            \
        ZETA_Core_DebugAssert(*tmp_rel_ptr == tmp_val);                    \
    }                                                                      \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_RelPtr_SetPtr(rel_ptr, base, ptr)                           \
    ZETA_Core_RelPtr_SetPtr_(ZETA_Core_TmpName, ZETA_Core_TmpName, (rel_ptr), \
                             (base), (ptr))

// -----------------------------------------------------------------------------

#define ZETA_Core_RelColorPtr_GetPtr(rel_color_ptr, align, base)           \
    __builtin_align_down((ZETA_Core_ToCharPtr((base)) + *(rel_color_ptr)), \
                         (align))

#define ZETA_Core_RelColorPtr_GetColor_(tmp, rel_color_ptr, align, base) \
    ({                                                                   \
        char* tmp{ ZETA_Core_ToCharPtr((base)) + *(rel_color_ptr) };     \
        tmp - ZETA_Core_ToCharPtr(__builtin_align_down(tmp, (align)));   \
    })

#define ZETA_Core_RelColorPtr_GetColor(rel_color_ptr, align, base)      \
    ZETA_Core_RelColorPtr_GetColor_(ZETA_Core_TmpName, (rel_color_ptr), \
                                    (align), (base))

#define ZETA_Core_RelColorPtr_Set_(tmp_rel_color_ptr, tmp_color, tmp_val,    \
                                   rel_color_ptr, align, base, ptr, color)   \
    {                                                                        \
        ZETA_Core_AutoVar(tmp_rel_color_ptr, (rel_color_ptr));               \
        ZETA_Core_AutoVar(tmp_color, (color));                               \
        ZETA_Core_DebugAssert(0 <= tmp_color &&                              \
                              (unsigned long long)tmp_color < (align));      \
        ZETA_Core_AutoVar(tmp_val, (ZETA_Core_ToCharPtr((ptr)) + tmp_color - \
                                    ZETA_Core_ToCharPtr((base))));           \
        *tmp_rel_color_ptr = tmp_val;                                        \
        ZETA_Core_DebugAssert(*tmp_rel_color_ptr == tmp_val);                \
    }                                                                        \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_RelColorPtr_Set(rel_color_ptr, align, base, ptr, color)   \
    ZETA_Core_RelColorPtr_Set_(ZETA_Core_TmpName, ZETA_Core_TmpName,        \
                               ZETA_Core_TmpName, (rel_color_ptr), (align), \
                               (base), (ptr), (color))

#define ZETA_Core_RelColorPtr_SetPtr_(tmp_rel_color_ptr, tmp_align, tmp_base, \
                                      tmp_ptr, rel_color_ptr, align, base,    \
                                      ptr)                                    \
    {                                                                         \
        ZETA_Core_AutoVar(tmp_rel_color_ptr, (rel_color_ptr));                \
        ZETA_Core_AutoVar(tmp_align, (align));                                \
        ZETA_Core_AutoVar(tmp_base, (base));                                  \
        ZETA_Core_AutoVar(tmp_ptr, (ptr));                                    \
        ZETA_Core_RelColorPtr_Set(                                            \
            tmp_rel_color_ptr, tmp_align, tmp_base, tmp_ptr,                  \
            (ZETA_Core_RelColorPtr_GetColor(tmp_rel_color_ptr, tmp_align,     \
                                            tmp_base)));                      \
    }                                                                         \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_RelColorPtr_SetPtr(rel_color_ptr, align, base, ptr)   \
    ZETA_Core_RelColorPtr_SetPtr_(ZETA_Core_TmpName, ZETA_Core_TmpName, \
                                  ZETA_Core_TmpName, ZETA_Core_TmpName, \
                                  (rel_color_ptr), (align), (base), (ptr))

#define ZETA_Core_RelColorPtr_SetColor_(tmp_rel_color_ptr, tmp_align,       \
                                        tmp_base, tmp_color, rel_color_ptr, \
                                        align, base, color)                 \
    {                                                                       \
        ZETA_Core_AutoVar(tmp_rel_color_ptr, (rel_color_ptr));              \
        ZETA_Core_AutoVar(tmp_align, (align));                              \
        ZETA_Core_AutoVar(tmp_base, (base));                                \
        ZETA_Core_AutoVar(tmp_color, (color));                              \
        ZETA_Core_RelColorPtr_Set(                                          \
            tmp_rel_color_ptr, tmp_align, tmp_base,                         \
            (ZETA_Core_RelColorPtr_GetPtr(tmp_rel_color_ptr, tmp_align,     \
                                          tmp_base)),                       \
            tmp_color);                                                     \
    }                                                                       \
    ZETA_Core_StaticAssert(true)

#define ZETA_Core_RelColorPtr_SetColor(rel_color_ptr, align, base, color) \
    ZETA_Core_RelColorPtr_SetColor_(ZETA_Core_TmpName, ZETA_Core_TmpName, \
                                    ZETA_Core_TmpName, ZETA_Core_TmpName, \
                                    (rel_color_ptr), (align), (base), (color))

namespace zeta::core {

template <typename Link, bool Color>
struct AdvancedPtr;

template <typename Link, bool Color>
struct AdvancedPtr {
    static constexpr bool rel_link{ !is_same<Link, void*> };

    Link link;

    void* GetPtr() const;
    void* GetPtr(size_t align) const;
    void* GetPtr(void* base) const;
    void* GetPtr(size_t align, void* base) const;

    int GetColor(size_t align) const;
    int GetColor(size_t align, void* base) const;

    void SetPtr(void* ptr);
    void SetPtr(size_t align, void* ptr);
    void SetPtr(void* base, void* ptr);
    void SetPtr(size_t align, void* base, void* ptr);

    void SetColor(size_t align, int color);
    void SetColor(size_t align, void* base, int color);

    void SetPtrColor(size_t align, void* ptr, int color);
    void SetPtrColor(size_t align, void* base, void* ptr, int color);
};

template <typename Link, bool Color>
void* AdvancedPtr<Link, Color>::GetPtr() const {
    ZETA_Core_StaticAssert(!rel_link);
    ZETA_Core_StaticAssert(!Color);

    return this->link;
}

template <typename Link, bool Color>
void* AdvancedPtr<Link, Color>::GetPtr(size_t align) const {
    ZETA_Core_StaticAssert(!rel_link);

    if constexpr (Color) {
        return ZETA_Core_ColorPtr_GetPtr(&this->link, align);
    } else {
        return this->GetPtr();
    }
}

template <typename Link, bool Color>
void* AdvancedPtr<Link, Color>::GetPtr(void* base) const {
    ZETA_Core_StaticAssert(!Color);

    if constexpr (rel_link) {
        return ZETA_Core_RelPtr_GetPtr(&this->link, base);
    } else {
        return this->GetPtr();
    }
}

template <typename Link, bool Color>
void* AdvancedPtr<Link, Color>::GetPtr(size_t align, void* base) const {
    if constexpr (!rel_link) {
        return this->GetPtr(align);
    } else if constexpr (!Color) {
        return this->GetPtr(base);
    } else {
        return ZETA_Core_RelColorPtr_GetPtr(&this->link, align, base);
    }
}

template <typename Link, bool Color>
int AdvancedPtr<Link, Color>::GetColor(size_t align) const {
    ZETA_Core_StaticAssert(!rel_link);
    ZETA_Core_StaticAssert(Color);

    return ZETA_Core_ColorPtr_GetColor(&this->link, align);
}

template <typename Link, bool Color>
int AdvancedPtr<Link, Color>::GetColor(size_t align, void* base) const {
    ZETA_Core_StaticAssert(Color);

    if constexpr (rel_link) {
        return ZETA_Core_RelColorPtr_GetColor(&this->link, align, base);
    } else {
        return this->GetColor(align);
    }
}

template <typename Link, bool Color>
void AdvancedPtr<Link, Color>::SetPtr(void* ptr) {
    ZETA_Core_StaticAssert(!rel_link);
    ZETA_Core_StaticAssert(!Color);

    this->link = ptr;
}

template <typename Link, bool Color>
void AdvancedPtr<Link, Color>::SetPtr(size_t align, void* ptr) {
    ZETA_Core_StaticAssert(!rel_link);

    if constexpr (Color) {
        ZETA_Core_ColorPtr_SetPtr(&this->link, align, ptr);
    } else {
        this->SetPtr(ptr);
    }
}

template <typename Link, bool Color>
void AdvancedPtr<Link, Color>::SetPtr(void* base, void* ptr) {
    ZETA_Core_StaticAssert(!Color);

    if constexpr (rel_link) {
        ZETA_Core_RelPtr_SetPtr(&this->link, base, ptr);
    } else {
        this->SetPtr(ptr);
    }
}

template <typename Link, bool Color>
void AdvancedPtr<Link, Color>::SetPtr(size_t align, void* base, void* ptr) {
    if constexpr (!rel_link) {
        this->SetPtr(align, ptr);
    } else if constexpr (!Color) {
        this->SetPtr(base, ptr);
    } else {
        ZETA_Core_RelColorPtr_SetPtr(&this->link, align, base, ptr);
    }
}

template <typename Link, bool Color>
void AdvancedPtr<Link, Color>::SetColor(size_t align, int color) {
    ZETA_Core_StaticAssert(!rel_link);
    ZETA_Core_StaticAssert(Color);

    ZETA_Core_ColorPtr_SetColor(&this->link, align, color);
}

template <typename Link, bool Color>
void AdvancedPtr<Link, Color>::SetColor(size_t align, void* base, int color) {
    ZETA_Core_StaticAssert(Color);

    if constexpr (rel_link) {
        ZETA_Core_RelColorPtr_SetColor(&this->link, align, base, color);
    } else {
        this->SetColor(align, color);
    }
}

template <typename Link, bool Color>
void AdvancedPtr<Link, Color>::SetPtrColor(size_t align, void* ptr, int color) {
    ZETA_Core_StaticAssert(!rel_link);
    ZETA_Core_StaticAssert(Color);

    ZETA_Core_ColorPtr_Set(&this->link, align, ptr, color);
}

template <typename Link, bool Color>
void AdvancedPtr<Link, Color>::SetPtrColor(size_t align, void* base, void* ptr,
                                           int color) {
    ZETA_Core_StaticAssert(Color);

    if constexpr (rel_link) {
        ZETA_Core_RelColorPtr_Set(&this->link, align, base, ptr, color);
    } else {
        this->SetPtrColor(align, ptr, color);
    }
}

}  // namespace zeta::core
