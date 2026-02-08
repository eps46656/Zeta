#pragma once

#include <unordered_map>
#include <zeta/core/assoc_cntr.ipp>
#include <zeta/core/debug_utils.ipp>

namespace zeta::core_test::assoc_cntr_utils {

using AssocCntrVTable = core::assoc_cntr::VTable;

// -----------------------------------------------------------------------------

inline auto& GetSanitizeFuncs() {
    static std::unordered_map<void const*, void (*)(void const* ac)> instance;
    return instance;
}

inline void AddSanitizeFunc(void const* ac, void (*Sanitize)(void const* ac)) {
    auto& map{ GetSanitizeFuncs() };

    auto iter{ map.insert({ ac, Sanitize }).first };

    ZETA_Core_DebugAssert(iter->second == Sanitize);
}

inline void Sanitize(void* ac) {
    if (ac == nullptr) { return; }

    auto& map{ GetSanitizeFuncs() };

    auto iter{ map.find(ac) };
    ZETA_Core_DebugAssert(iter != map.end());
    ZETA_Core_DebugAssert(iter->second != nullptr);

    iter->second(ac);
}

inline void Sanitize(core::assoc_cntr::Ref const* ac) { Sanitize(ac->cntr); }

inline auto& GetDestroyFuncs() {
    static std::unordered_map<void*, void (*)(void* sc)> instance;
    return instance;
}

inline void AddDestroyFunc(void* ac, void (*Destroy)(void* ac)) {
    auto& map{ GetDestroyFuncs() };

    auto iter{ map.insert({ ac, Destroy }).first };

    ZETA_Core_DebugAssert(iter->second == Destroy);
}

inline void Destroy(void* ac) {
    if (ac == nullptr) { return; }

    auto& map{ GetDestroyFuncs() };

    auto iter{ map.find(ac) };

    ZETA_Core_DebugAssert(iter != map.end());
    ZETA_Core_DebugAssert(iter->second != nullptr);

    iter->second(ac);
}

inline void Destroy(core::assoc_cntr::Ref* ac) { Destroy(ac->cntr); }

template <typename AssocCntr, typename Key, typename Elem>
Elem* Find(AssocCntr* ac, Key const& key) {
    void* cursor{ ZETA_Core_AssocCntr_AllocaCursor(ac) };

    auto elem{ static_cast<Elem*>(AssocCntr::Find(
        ac, &key, core::hash::TypeErasedHash<Key>,
        core::compare::TypeErasedCompare<Key, Elem>, true, cursor, nullptr)) };

    Sanitize(ac);

    ZETA_Core_DebugAssert(AssocCntr::Derefer(ac, cursor, true, nullptr) ==
                          elem);

    if (elem == nullptr) { return nullptr; }

    ZETA_Core_DebugAssert(core::compare::Compare(key, *elem) == 0);

    return elem;
}

template <typename AssocCntr, typename Elem>
Elem* Insert(AssocCntr* ac, Elem const& elem) {
    void* cursor{ ZETA_Core_AssocCntr_AllocaCursor(ac) };

    auto ins_elem{ static_cast<Elem*>(AssocCntr::Insert(ac, &elem, cursor)) };

    Sanitize(ac);

    ZETA_Core_DebugAssert(AssocCntr::Derefer(ac, cursor, true, nullptr) ==
                          ins_elem);

    ZETA_Core_DebugAssert(ins_elem != nullptr);

    ZETA_Core_DebugAssert(core::compare::Compare(elem, *ins_elem) == 0);

    return ins_elem;
}

template <typename AssocCntr, typename Key, typename Elem>
bool Erase(AssocCntr* ac, Key const& key) {
    void* cursor{ ZETA_Core_AssocCntr_AllocaCursor(ac) };

    auto elem{ static_cast<Elem*>(AssocCntr::Find(
        ac, &key, core::hash::TypeErasedHash<Key>,
        core::compare::TypeErasedCompare<Key, Elem>, true, cursor, nullptr)) };

    Sanitize(ac);

    ZETA_Core_DebugAssert(AssocCntr::Derefer(ac, cursor, true, nullptr) ==
                          elem);

    if (elem == nullptr) { return false; }

    ZETA_Core_DebugAssert(core::compare::Compare(key, *elem) == 0);

    size_t old_size{ AssocCntr::GetSize(ac) };
    ZETA_Core_DebugAssert(0 < old_size);

    AssocCntr::Erase(ac, cursor);

    Sanitize(ac);

    size_t new_size{ AssocCntr::GetSize(ac) };

    ZETA_Core_DebugAssert(old_size - 1 == new_size);

    return true;
}

// -----------------------------------------------------------------------------

template <typename AssocCntr>
inline size_t SyncGetSize(std::vector<AssocCntr*> const& acs) {
    ZETA_Core_DebugAssert(!acs.empty());

    size_t size{ AssocCntr::GetSize(acs[0]) };

    for (auto ac : acs) {
        ZETA_Core_DebugAssert(size == AssocCntr::GetSize(ac));
    }

    return size;
}

template <typename AssocCntr, typename Key, typename Elem>
Elem* SyncFind(std::vector<AssocCntr*> const& acs, Key const& key) {
    ZETA_Core_DebugAssert(!acs.empty());

    size_t size{ SyncGetSize(acs) };
    ZETA_Core_Unused(size);

    Elem* elem{ Find<AssocCntr, Key, Elem>(acs[0], key) };

    for (auto ac : acs) {
        Elem* cur_elem{ Find<AssocCntr, Key, Elem>(ac, key) };

        ZETA_Core_DebugAssert((elem == nullptr) == (cur_elem == nullptr));

        if (elem != nullptr) {
            ZETA_Core_DebugAssert(core::compare::Compare(*elem, *cur_elem) ==
                                  0);
        }
    }

    return elem;
}

template <typename AssocCntr, typename Elem>
bool SyncInsert(std::vector<AssocCntr*> const& acs, Elem const& elem) {
    if (SyncFind<AssocCntr, Elem, Elem>(acs, elem) != nullptr) { return false; }

    for (auto ac : acs) { Insert(ac, elem); }

    ZETA_Core_DebugAssert(
        (SyncFind<AssocCntr, Elem, Elem>(acs, elem) != nullptr));

    return true;
}

template <typename AssocCntr, typename Elem>
bool SyncErase(std::vector<AssocCntr*> const& acs, Elem const& elem) {
    if (SyncFind<AssocCntr, Elem, Elem>(acs, elem) == nullptr) { return false; }

    for (auto ac : acs) { Erase<AssocCntr, Elem, Elem>(ac, elem); }

    ZETA_Core_DebugAssert(
        (SyncFind<AssocCntr, Elem, Elem>(acs, elem) == nullptr));

    return true;
}

// -----------------------------------------------------------------------------

template <typename AssocCntrA, typename AssocCntrB, typename Elem>
void Contain(AssocCntrA* a, AssocCntrB* b) {
    void* a_iter{ ZETA_Core_AssocCntr_AllocaCursor(a) };
    void* a_end{ ZETA_Core_AssocCntr_AllocaCursor(a) };

    AssocCntrA::PeekL(a, true, a_iter, nullptr);
    AssocCntrA::GetRBCursor(a, a_end);

    size_t size{ 0 };

    while (!AssocCntrA::AreEqualCursor(a, a_iter, a_end)) {
        ++size;

        auto a_elem{ static_cast<Elem*>(
            AssocCntrA::Derefer(a, a_iter, true, nullptr)) };

        ZETA_Core_DebugAssert(a_elem != nullptr);

        Elem* b_elem{ Find<AssocCntrB, Elem, Elem>(b, *a_elem) };

        ZETA_Core_DebugAssert(b_elem != nullptr);

        ZETA_Core_DebugAssert(*a_elem == *b_elem);

        AssocCntrA::CursorStepR(a, a_iter);
    }

    ZETA_Core_DebugAssert(AssocCntrA::Derefer(a, a_iter, true, nullptr) ==
                          nullptr);

    ZETA_Core_DebugAssert(AssocCntrA::GetSize(a) == size);
}

template <typename AssocCntrA, typename AssocCntrB, typename Elem>
void Equal(AssocCntrA* a, AssocCntrB* b) {
    Contain<AssocCntrA, AssocCntrB, Elem>(a, b);
    Contain<AssocCntrA, AssocCntrB, Elem>(b, a);
}

template <typename AssocCntr, typename Elem>
void Equal(std::vector<AssocCntr*> const& acs) {
    size_t size{ acs.size() };

    for (size_t i{ 1 }; i < size; ++i) {
        Equal<AssocCntr, AssocCntr, Elem>(acs[i - 1], acs[i]);
    }
}

// -----------------------------------------------------------------------------

template <typename AssocCntr, typename Key, typename Elem>
Elem* SyncRandomFind(std::vector<AssocCntr*> const& acs) {
    return SyncFind(acs, Zeta_CoreTest_GetRandom<Key>());
}

template <typename AssocCntr, typename Elem>
bool SyncRandomInsert(std::vector<AssocCntr*> const& acs) {
    return SyncInsert(acs, Zeta_CoreTest_GetRandom<Elem>());
}

}  // namespace zeta::core_test::assoc_cntr_utils
