#pragma once

#include <unordered_map>
#include <zeta/core/assoc_cntr.ipp>
#include <zeta/core/debug_utils.ipp>

namespace zeta::core_test::assoc_cntr_utils {

using AssocCntrRef = core::assoc_cntr::AssocCntrRef;
using AssocCntrVTable = core::assoc_cntr::AssocCntrVTable;

inline auto& GetSanitizeFuncs() {
    static std::unordered_map<AssocCntrVTable const*, void (*)(AssocCntrRef)>
        instance;
    return instance;
}

inline void AddSanitizeFunc(AssocCntrVTable const* assoc_cntr_vtable,
                            void (*Sanitize)(AssocCntrRef assoc_cntr)) {
    auto& map{ GetSanitizeFuncs() };

    auto iter{ map.insert({ assoc_cntr_vtable, Sanitize }).first };

    ZETA_Core_DebugAssert(iter->second == Sanitize);
}

inline void Sanitize(AssocCntrRef assoc_cntr) {
    if (assoc_cntr.inst == nullptr) { return; }

    auto& map{ GetSanitizeFuncs() };

    auto iter{ map.find(assoc_cntr.vtable) };
    ZETA_Core_DebugAssert(iter != map.end());
    ZETA_Core_DebugAssert(iter->second != nullptr);

    iter->second(assoc_cntr);
}

// -----------------------------------------------------------------------------

inline auto& GetDestroyFuncs() {
    static std::unordered_map<AssocCntrVTable const*,
                              void (*)(AssocCntrRef assoc_cntr)>
        instance;
    return instance;
}

inline void AddDestroyFunc(AssocCntrVTable const* assoc_cntr_vtable,
                           void (*Destroy)(AssocCntrRef)) {
    auto& map{ GetDestroyFuncs() };

    auto iter{ map.insert({ assoc_cntr_vtable, Destroy }).first };

    ZETA_Core_DebugAssert(iter->second == Destroy);
}

inline void Destroy(AssocCntrRef assoc_cntr) {
    if (assoc_cntr.inst == nullptr) { return; }

    auto& map{ GetDestroyFuncs() };

    auto iter{ map.find(assoc_cntr.vtable) };

    ZETA_Core_DebugAssert(iter != map.end());
    ZETA_Core_DebugAssert(iter->second != nullptr);

    iter->second(assoc_cntr);
}

// -----------------------------------------------------------------------------

template <typename Key, typename Elem>
Elem* Find(AssocCntrRef assoc_cntr, Key const& key) {
    void* cursor{ ZETA_Core_AssocCntr_AllocaCursor(&assoc_cntr) };

    auto elem{ static_cast<Elem*>(AssocCntrRef::FnFind(
        &assoc_cntr, &key, core::hash::TypeErasedHash<Key>,
        core::compare::TypeErasedCompare<Key, Elem>, cursor)) };

    Sanitize(assoc_cntr);

    ZETA_Core_DebugAssert(AssocCntrRef::Refer(&assoc_cntr, cursor) == elem);

    if (elem == nullptr) { return nullptr; }

    ZETA_Core_DebugAssert(core::compare::Compare(key, *elem) == 0);

    return elem;
}

template <typename Elem>
Elem* Insert(AssocCntrRef assoc_cntr, Elem const& elem) {
    void* cursor{ ZETA_Core_AssocCntr_AllocaCursor(&assoc_cntr) };

    auto ins_elem{ static_cast<Elem*>(
        AssocCntrRef::Insert(&assoc_cntr, &elem, cursor)) };

    Sanitize(assoc_cntr);

    ZETA_Core_DebugAssert(AssocCntrRef::Refer(&assoc_cntr, cursor) == ins_elem);

    ZETA_Core_DebugAssert(ins_elem != nullptr);

    ZETA_Core_DebugAssert(core::compare::Compare(elem, *ins_elem) == 0);

    return ins_elem;
}

template <typename Key, typename Elem>
bool Erase(AssocCntrRef assoc_cntr, Key const& key) {
    void* cursor{ ZETA_Core_AssocCntr_AllocaCursor(&assoc_cntr) };

    auto elem{ static_cast<Elem*>(AssocCntrRef::FnFind(
        &assoc_cntr, &key, core::hash::TypeErasedHash<Key>,
        core::compare::TypeErasedCompare<Key, Elem>, cursor)) };

    Sanitize(assoc_cntr);

    ZETA_Core_DebugAssert(AssocCntrRef::Refer(&assoc_cntr, cursor) == elem);

    if (elem == nullptr) { return false; }

    ZETA_Core_DebugAssert(core::compare::Compare(key, *elem) == 0);

    size_t old_size{ AssocCntrRef::GetSize(&assoc_cntr) };
    ZETA_Core_DebugAssert(0 < old_size);

    AssocCntrRef::Erase(&assoc_cntr, cursor);

    Sanitize(assoc_cntr);

    size_t new_size{ AssocCntrRef::GetSize(&assoc_cntr) };

    ZETA_Core_DebugAssert(old_size - 1 == new_size);

    return true;
}

// -----------------------------------------------------------------------------

inline size_t SyncGetSize(std::vector<AssocCntrRef> const& assoc_cntrs) {
    ZETA_Core_DebugAssert(!assoc_cntrs.empty());

    for (auto assoc_cntr : assoc_cntrs) {
        ZETA_Core_DebugAssert(assoc_cntr.vtable != nullptr);
    }

    size_t size{ AssocCntrRef::GetSize(&assoc_cntrs[0]) };

    for (auto assoc_cntr : assoc_cntrs) {
        ZETA_Core_DebugAssert(size == AssocCntrRef::GetSize(&assoc_cntr));
    }

    return size;
}

template <typename Key, typename Elem>
Elem* SyncFind(std::vector<AssocCntrRef> const& assoc_cntrs, Key const& key) {
    ZETA_Core_DebugAssert(!assoc_cntrs.empty());

    size_t size{ SyncGetSize(assoc_cntrs) };
    ZETA_Core_Unused(size);

    Elem* elem{ Find<Key, Elem>(assoc_cntrs[0], key) };

    for (auto assoc_cntr : assoc_cntrs) {
        Elem* cur_elem{ Find<Key, Elem>(assoc_cntr, key) };

        ZETA_Core_DebugAssert((elem == nullptr) == (cur_elem == nullptr));

        if (elem != nullptr) {
            ZETA_Core_DebugAssert(core::compare::Compare(*elem, *cur_elem) ==
                                  0);
        }
    }

    return elem;
}

template <typename Elem>
bool SyncInsert(std::vector<AssocCntrRef> const& assoc_cntrs,
                Elem const& elem) {
    if (SyncFind<Elem, Elem>(assoc_cntrs, elem) != nullptr) { return false; }

    for (auto assoc_cntr : assoc_cntrs) { Insert(assoc_cntr, elem); }

    ZETA_Core_DebugAssert((SyncFind<Elem, Elem>(assoc_cntrs, elem) != nullptr));

    return true;
}

template <typename Elem>
bool SyncErase(std::vector<AssocCntrRef> const& assoc_cntrs, Elem const& elem) {
    if (SyncFind<Elem, Elem>(assoc_cntrs, elem) == nullptr) { return false; }

    for (auto assoc_cntr : assoc_cntrs) { Erase<Elem, Elem>(assoc_cntr, elem); }

    ZETA_Core_DebugAssert((SyncFind<Elem, Elem>(assoc_cntrs, elem) == nullptr));

    return true;
}

// -----------------------------------------------------------------------------

template <typename Elem>
void Contain(AssocCntrRef a, AssocCntrRef b) {
    ZETA_Core_DebugAssert(a.vtable != nullptr);
    ZETA_Core_DebugAssert(b.vtable != nullptr);

    void* a_iter{ ZETA_Core_AssocCntr_AllocaCursor(&a) };
    void* a_end{ ZETA_Core_AssocCntr_AllocaCursor(&a) };

    AssocCntrRef::PeekL(&a, a_iter, nullptr);
    AssocCntrRef::GetRBCursor(&a, a_end);

    size_t size{ 0 };

    while (!AssocCntrRef::AreEqualCursor(&a, a_iter, a_end)) {
        ++size;

        auto a_elem{ static_cast<Elem*>(AssocCntrRef::Refer(&a, a_iter)) };

        ZETA_Core_DebugAssert(a_elem != nullptr);

        Elem* b_elem{ Find<Elem, Elem>(b, *a_elem) };

        ZETA_Core_DebugAssert(b_elem != nullptr);

        ZETA_Core_DebugAssert(*a_elem == *b_elem);

        AssocCntrRef::CursorStepR(&a, a_iter);
    }

    ZETA_Core_DebugAssert(AssocCntrRef::Refer(&a, a_iter) == nullptr);

    ZETA_Core_DebugAssert(AssocCntrRef::GetSize(&a) == size);
}

template <typename Elem>
void Equal(AssocCntrRef a, AssocCntrRef b) {
    ZETA_Core_DebugAssert(a.vtable != nullptr);
    ZETA_Core_DebugAssert(b.vtable != nullptr);

    Contain<Elem>(a, b);
    Contain<Elem>(b, a);
}

template <typename Elem>
void Equal(std::vector<AssocCntrRef> const& assoc_cntrs) {
    size_t size{ assoc_cntrs.size() };

    for (size_t i{ 1 }; i < size; ++i) {
        Equal<Elem>(assoc_cntrs[i - 1], assoc_cntrs[i]);
    }
}

// -----------------------------------------------------------------------------

template <typename Key, typename Elem>
Elem* SyncRandomFind(std::vector<AssocCntrRef> const& assoc_cntrs) {
    return SyncFind(assoc_cntrs, Zeta_CoreTest_GetRandom<Key>());
}

template <typename Elem>
bool SyncRandomInsert(std::vector<AssocCntrRef> const& assoc_cntrs) {
    return SyncInsert(assoc_cntrs, Zeta_CoreTest_GetRandom<Elem>());
}

}  // namespace zeta::core_test::assoc_cntr_utils
