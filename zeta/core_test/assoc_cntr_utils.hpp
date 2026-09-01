#pragma once

#include <stddef.h>

#include <list>
#include <unordered_map>
#include <utility>
#include <vector>
#include <zeta/core/assoc_cntr.hpp>
#include <zeta/core/assoc_cntr.ipp>
#include <zeta/core/assoc_cntr_ref.hpp>
#include <zeta/core/comparison.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/hash.hpp>

namespace zeta::core_test::assoc_cntr_utils {

using AssocCntrVTable = core::assoc_cntr::VTable;

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

inline void Sanitize(core::assoc_cntr_ref::Cntr const* ac) {
    Sanitize(ac->target_cntr);
}

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

inline void Destroy(core::assoc_cntr_ref::Cntr* ac) {
    Destroy(ac->target_cntr);
}

template <typename AssocCntr, typename Key, typename Elem>
Elem* Find(AssocCntr* ac, Key const& key) {
    core::assoc_cntr::CursorLimit* cursor;

    auto elem_find_with_key{ static_cast<Elem*>(core::assoc_cntr::Find(
        *ac, &key, core::hash::TypeErasedBasicHash<Key>,
        core::comparison::TypeErasedBasicCompare<Key, Elem>, true, nullptr,
        &cursor, nullptr)) };

    Sanitize(ac);

    if constexpr (core::meta::IsSame<core::meta::RemoveCVRef<Key>,
                                     core::meta::RemoveCVRef<Elem> >) {
        //
    }

    ZETA_Core_DebugAssert(
        core::assoc_cntr::Derefer(*ac, &cursor, true, nullptr, nullptr) ==
        elem_find_with_key);

    if (elem_find_with_key == nullptr) { return nullptr; }

    if constexpr (core::meta::IsSame<core::meta::RemoveCVRef<Key>,
                                     core::meta::RemoveCVRef<Elem> >) {
        ZETA_Core_DebugAssert(core::comparison::BasicCompare(
            core::comparison::OpTag::Equal{}, key, *elem_find_with_key));
    }

    return elem_find_with_key;
}

template <typename AssocCntr, typename Elem>
Elem* Insert(AssocCntr* ac, Elem const& elem) {
    core::assoc_cntr::CursorLimit cursor;

    auto ins_elem{ static_cast<Elem*>(
        core::assoc_cntr::Insert(*ac, &elem, &cursor)) };

    Sanitize(ac);

    ZETA_Core_DebugAssert(core::assoc_cntr::Derefer(*ac, &cursor, true, nullptr,
                                                    nullptr) == ins_elem);

    ZETA_Core_DebugAssert(ins_elem != nullptr);

    ZETA_Core_DebugAssert(core::comparison::BasicCompare(
        core::comparison::OpTag::Equal{}, elem, *ins_elem));

    return ins_elem;
}

template <typename AssocCntr, typename Key, typename Elem>
bool Erase(AssocCntr* ac, Key const& key) {
    core::assoc_cntr::CursorLimit cursor;

    auto elem{ static_cast<Elem*>(core::assoc_cntr::Find(
        *ac, &key, core::hash::TypeErasedBasicHash<Key>,
        core::comparison::TypeErasedBasicCompare<Key, Elem>, true, &cursor,
        nullptr)) };

    Sanitize(ac);

    ZETA_Core_DebugAssert(
        core::assoc_cntr::Derefer(*ac, &cursor, true, nullptr) == elem);

    if (elem == nullptr) { return false; }

    ZETA_Core_DebugAssert(core::comparison::BasicCompare(key, *elem) == 0);

    size_t old_size{ core::assoc_cntr::GetElemCnt(*ac) };
    ZETA_Core_DebugAssert(0 < old_size);

    core::assoc_cntr::Erase(*ac, &cursor);

    Sanitize(ac);

    size_t new_size{ core::assoc_cntr::GetElemCnt(*ac) };

    ZETA_Core_DebugAssert(old_size - 1 == new_size);

    return true;
}

template <typename AssocCntr>
size_t SyncGetElemCnt(std::vector<AssocCntr*> const& acs) {
    ZETA_Core_DebugAssert(!acs.empty());

    size_t size{ core::assoc_cntr::GetElemCnt(*acs[0]) };

    for (auto ac : acs) {
        ZETA_Core_DebugAssert(size == core::assoc_cntr::GetElemCnt(*ac));
    }

    return size;
}

template <typename AssocCntr, typename Key, typename Elem>
Elem* SyncFind(std::vector<AssocCntr*> const& acs, Key const& key) {
    ZETA_Core_DebugAssert(!acs.empty());

    size_t size{ SyncGetElemCnt(acs) };
    ZETA_Core_Unused(size);

    Elem* elem{ Find<AssocCntr, Key, Elem>(acs[0], key) };

    for (auto ac : acs) {
        Elem* cur_elem{ Find<AssocCntr, Key, Elem>(ac, key) };

        ZETA_Core_DebugAssert((elem == nullptr) == (cur_elem == nullptr));

        if (elem != nullptr) {
            ZETA_Core_DebugAssert(core::comparison::BasicCompare(
                core::comparison::OpTag::Equal{}, *elem, *cur_elem));
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

template <typename AssocCntrA, typename AssocCntrB, typename Elem>
void Contain(AssocCntrA* a, AssocCntrB* b) {
    core::assoc_cntr::CursorLimit a_iter;
    core::assoc_cntr::CursorLimit a_end;

    core::assoc_cntr::PeekL(*a, true, &a_iter, nullptr);
    core::assoc_cntr::GetRBCursor(*a, &a_end);

    size_t size{ 0 };

    while (!core::assoc_cntr::AreEqualCursor(*a, &a_iter, &a_end)) {
        ++size;

        auto a_elem{ static_cast<Elem*>(
            core::assoc_cntr::Derefer(*a, &a_iter, true, nullptr)) };

        ZETA_Core_DebugAssert(a_elem != nullptr);

        Elem* b_elem{ Find<AssocCntrB, Elem, Elem>(b, *a_elem) };

        ZETA_Core_DebugAssert(b_elem != nullptr);

        ZETA_Core_DebugAssert(*a_elem == *b_elem);

        core::assoc_cntr::CursorStepR(*a, &a_iter);
    }

    ZETA_Core_DebugAssert(
        core::assoc_cntr::Derefer(*a, &a_iter, true, nullptr) == nullptr);

    ZETA_Core_DebugAssert(core::assoc_cntr::GetElemCnt(*a) == size);
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

template <typename AssocCntr, typename Key, typename Elem>
Elem* SyncRandomFind(std::vector<AssocCntr*> const& acs) {
    return SyncFind(acs, Zeta_CoreTest_GetRandom<Key>());
}

template <typename AssocCntr, typename Elem>
bool SyncRandomInsert(std::vector<AssocCntr*> const& acs) {
    return SyncInsert(acs, Zeta_CoreTest_GetRandom<Elem>());
}

}  // namespace zeta::core_test::assoc_cntr_utils
