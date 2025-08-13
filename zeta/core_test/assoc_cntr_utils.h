#pragma once

#include <zeta/core/assoc_cntr.h>
#include <zeta/core/debugger.h>
#include <zeta/core_test/cmp_utils.h>
#include <zeta/core_test/hash_utils.h>

#include <unordered_map>

auto& AssocCntrUtils_GetSanitizeFuncs() {
    static std::unordered_map<Zeta_Core_AssocCntr_VTable const*,
                              void (*)(Zeta_Core_AssocCntr)>
        instance;
    return instance;
}

void AssocCntrUtils_AddSanitizeFunc(
    Zeta_Core_AssocCntr_VTable const* assoc_cntr_vtable,
    void (*Sanitize)(Zeta_Core_AssocCntr assoc_cntr)) {
    auto& map{ AssocCntrUtils_GetSanitizeFuncs() };

    auto iter{ map.insert({ assoc_cntr_vtable, Sanitize }).first };

    ZETA_Core_DebugAssert(iter->second == Sanitize);
}

void AssocCntrUtils_Sanitize(Zeta_Core_AssocCntr assoc_cntr) {
    if (assoc_cntr.context == nullptr) { return; }

    auto& map{ AssocCntrUtils_GetSanitizeFuncs() };

    auto iter{ map.find(assoc_cntr.vtable) };
    ZETA_Core_DebugAssert(iter != map.end());
    ZETA_Core_DebugAssert(iter->second != NULL);

    iter->second(assoc_cntr);
}

// -----------------------------------------------------------------------------

auto& AssocCntrUtils_GetDestroyFuncs() {
    static std::unordered_map<Zeta_Core_AssocCntr_VTable const*,
                              void (*)(Zeta_Core_AssocCntr assoc_cntr)>
        instance;
    return instance;
}

void AssocCntrUtils_AddDestroyFunc(
    Zeta_Core_AssocCntr_VTable const* assoc_cntr_vtable,
    void (*Destroy)(Zeta_Core_AssocCntr)) {
    auto& map{ AssocCntrUtils_GetDestroyFuncs() };

    auto iter{ map.insert({ assoc_cntr_vtable, Destroy }).first };

    ZETA_Core_DebugAssert(iter->second == Destroy);
}

void AssocCntrUtils_Destroy(Zeta_Core_AssocCntr assoc_cntr) {
    if (assoc_cntr.context == nullptr) { return; }

    auto& map{ AssocCntrUtils_GetDestroyFuncs() };

    auto iter{ map.find(assoc_cntr.vtable) };

    ZETA_Core_DebugAssert(iter != map.end());
    ZETA_Core_DebugAssert(iter->second != NULL);

    iter->second(assoc_cntr);
}

// -----------------------------------------------------------------------------

template <typename Key, typename Elem>
Elem* AssocCntrUtils_Find(Zeta_Core_AssocCntr assoc_cntr, const Key& key) {
    void* cursor{ ZETA_Core_AssocCntr_AllocaCursor(assoc_cntr) };

    Elem* elem{ (Elem*)ZETA_Core_AssocCntr_Find(
        assoc_cntr, &key, &ZetaHash<Key>, nullptr, (&ZetaCompare<Key, Elem>),
        nullptr, cursor) };

    AssocCntrUtils_Sanitize(assoc_cntr);

    ZETA_Core_DebugAssert(ZETA_Core_AssocCntr_Refer(assoc_cntr, cursor) ==
                          elem);

    if (elem == nullptr) { return nullptr; }

    ZETA_Core_DebugAssert(Compare(nullptr, key, *elem) == 0);

    return elem;
}

template <typename Elem>
Elem* AssocCntrUtils_Insert(Zeta_Core_AssocCntr assoc_cntr, const Elem& elem) {
    void* cursor{ ZETA_Core_AssocCntr_AllocaCursor(assoc_cntr) };

    Elem* ins_elem{ (Elem*)ZETA_Core_AssocCntr_Insert(assoc_cntr, &elem,
                                                      cursor) };

    AssocCntrUtils_Sanitize(assoc_cntr);

    ZETA_Core_DebugAssert(ZETA_Core_AssocCntr_Refer(assoc_cntr, cursor) ==
                          ins_elem);

    ZETA_Core_DebugAssert(ins_elem != nullptr);

    ZETA_Core_DebugAssert(Compare(nullptr, elem, *ins_elem) == 0);

    return ins_elem;
}

template <typename Key, typename Elem>
bool_t AssocCntrUtils_Erase(Zeta_Core_AssocCntr assoc_cntr, const Key& key) {
    void* cursor{ ZETA_Core_AssocCntr_AllocaCursor(assoc_cntr) };

    Elem* elem{ (Elem*)ZETA_Core_AssocCntr_Find(
        assoc_cntr, &key, ZetaHash<Key>, nullptr, (ZetaCompare<Key, Elem>),
        nullptr, cursor) };

    AssocCntrUtils_Sanitize(assoc_cntr);

    ZETA_Core_DebugAssert(ZETA_Core_AssocCntr_Refer(assoc_cntr, cursor) ==
                          elem);

    if (elem == nullptr) { return FALSE; }

    ZETA_Core_DebugAssert(Compare(nullptr, key, *elem) == 0);

    size_t old_size{ ZETA_Core_AssocCntr_GetSize(assoc_cntr) };
    ZETA_Core_DebugAssert(0 < old_size);

    ZETA_Core_AssocCntr_Erase(assoc_cntr, cursor);

    AssocCntrUtils_Sanitize(assoc_cntr);

    size_t new_size{ ZETA_Core_AssocCntr_GetSize(assoc_cntr) };

    ZETA_Core_DebugAssert(old_size - 1 == new_size);

    return TRUE;
}

// -----------------------------------------------------------------------------

size_t AssocCntrUtils_SyncGetSize(
    const std::vector<Zeta_Core_AssocCntr>& assoc_cntrs) {
    ZETA_Core_DebugAssert(!assoc_cntrs.empty());

    for (auto assoc_cntr : assoc_cntrs) {
        ZETA_Core_DebugAssert(assoc_cntr.vtable != nullptr);
    }

    size_t size{ ZETA_Core_AssocCntr_GetSize(assoc_cntrs[0]) };

    for (auto assoc_cntr : assoc_cntrs) {
        ZETA_Core_DebugAssert(size == ZETA_Core_AssocCntr_GetSize(assoc_cntr));
    }

    return size;
}

template <typename Key, typename Elem>
Elem* AssocCntrUtils_SyncFind(
    const std::vector<Zeta_Core_AssocCntr> assoc_cntrs, const Key& key) {
    ZETA_Core_DebugAssert(!assoc_cntrs.empty());

    size_t size{ AssocCntrUtils_SyncGetSize(assoc_cntrs) };
    ZETA_Core_Unused(size);

    Elem* elem{ AssocCntrUtils_Find<Key, Elem>(assoc_cntrs[0], key) };

    for (auto assoc_cntr : assoc_cntrs) {
        Elem* cur_elem = AssocCntrUtils_Find<Key, Elem>(assoc_cntr, key);

        ZETA_Core_DebugAssert((elem == nullptr) == (cur_elem == nullptr));

        if (elem != nullptr) {
            ZETA_Core_DebugAssert(Compare(nullptr, *elem, *cur_elem) == 0);
        }
    }

    return elem;
}

template <typename Elem>
bool_t AssocCntrUtils_SyncInsert(
    const std::vector<Zeta_Core_AssocCntr> assoc_cntrs, const Elem& elem) {
    if (AssocCntrUtils_SyncFind<Elem, Elem>(assoc_cntrs, elem) != nullptr) {
        return FALSE;
    }

    for (auto assoc_cntr : assoc_cntrs) {
        AssocCntrUtils_Insert(assoc_cntr, elem);
    }

    ZETA_Core_DebugAssert(
        (AssocCntrUtils_SyncFind<Elem, Elem>(assoc_cntrs, elem) != nullptr));

    return TRUE;
}

template <typename Elem>
bool_t AssocCntrUtils_SyncErase(
    const std::vector<Zeta_Core_AssocCntr> assoc_cntrs, const Elem& elem) {
    if (AssocCntrUtils_SyncFind<Elem, Elem>(assoc_cntrs, elem) == nullptr) {
        return FALSE;
    }

    for (auto assoc_cntr : assoc_cntrs) {
        AssocCntrUtils_Erase<Elem, Elem>(assoc_cntr, elem);
    }

    ZETA_Core_DebugAssert(
        (AssocCntrUtils_SyncFind<Elem, Elem>(assoc_cntrs, elem) == nullptr));

    return TRUE;
}

// -----------------------------------------------------------------------------

template <typename Elem>
void AssocCntrUtils_Contain(Zeta_Core_AssocCntr a, Zeta_Core_AssocCntr b) {
    ZETA_Core_DebugAssert(a.vtable != nullptr);
    ZETA_Core_DebugAssert(b.vtable != nullptr);

    void* a_iter{ ZETA_Core_AssocCntr_AllocaCursor(a) };
    void* a_end{ ZETA_Core_AssocCntr_AllocaCursor(a) };

    ZETA_Core_AssocCntr_PeekL(a, a_iter);
    ZETA_Core_AssocCntr_GetRBCursor(a, a_end);

    size_t size{ 0 };

    while (!ZETA_Core_AssocCntr_Cursor_AreEqual(a, a_iter, a_end)) {
        ++size;

        Elem* a_elem = (Elem*)ZETA_Core_AssocCntr_Refer(a, a_iter);

        ZETA_Core_DebugAssert(a_elem != nullptr);

        Elem* b_elem{ AssocCntrUtils_Find<Elem, Elem>(b, *a_elem) };

        ZETA_Core_DebugAssert(b_elem != nullptr);

        ZETA_Core_DebugAssert(*a_elem == *b_elem);

        ZETA_Core_AssocCntr_Cursor_StepR(a, a_iter);
    }

    ZETA_Core_DebugAssert(ZETA_Core_AssocCntr_Refer(a, a_iter) == nullptr);

    ZETA_Core_DebugAssert(ZETA_Core_AssocCntr_GetSize(a) == size);
}

template <typename Elem>
void AssocCntrUtils_Equal(Zeta_Core_AssocCntr a, Zeta_Core_AssocCntr b) {
    ZETA_Core_DebugAssert(a.vtable != nullptr);
    ZETA_Core_DebugAssert(b.vtable != nullptr);

    AssocCntrUtils_Contain<Elem>(a, b);
    AssocCntrUtils_Contain<Elem>(b, a);
}

template <typename Elem>
void AssocCntrUtils_Equal(const std::vector<Zeta_Core_AssocCntr> assoc_cntrs) {
    size_t size{ assoc_cntrs.size() };

    for (size_t i{ 1 }; i < size; ++i) {
        AssocCntrUtils_Equal<Elem>(assoc_cntrs[i - 1], assoc_cntrs[i]);
    }
}

// -----------------------------------------------------------------------------

template <typename Key, typename Elem>
Elem* AssocCntrUtils_SyncRandomFind(
    const std::vector<Zeta_Core_AssocCntr>& assoc_cntrs) {
    return AssocCntrUtils_SyncFind(assoc_cntrs, Zeta_CoreTest_GetRandom<Key>());
}

template <typename Elem>
bool_t AssocCntrUtils_SyncRandomInsert(
    const std::vector<Zeta_Core_AssocCntr>& assoc_cntrs) {
    return AssocCntrUtils_SyncInsert(assoc_cntrs,
                                     Zeta_CoreTest_GetRandom<Elem>());
}
