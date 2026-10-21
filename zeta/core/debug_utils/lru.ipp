#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/lru.hpp>

namespace zeta::core::debug_utils {

template <typename Key, typename Value, typename KeyHash, typename KeyEqual>
constexpr std::pair<Key, Value> const&
lru::CacheManager<Key, Value, KeyHash, KeyEqual>::Iterator::operator*() {
    return *this->map_iter->second;
}

template <typename Key, typename Value, typename KeyHash, typename KeyEqual>
constexpr std::pair<Key, Value> const*
lru::CacheManager<Key, Value, KeyHash, KeyEqual>::Iterator::operator->() {
    return &*this->map_iter->second;
}

template <typename Key, typename Value, typename KeyHash, typename KeyEqual>
constexpr lru::CacheManager<Key, Value, KeyHash, KeyEqual>::CacheManager(
    size_t max_size)
    : max_size{ max_size } {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < max_size);
}

template <typename Key, typename Value, typename KeyHash, typename KeyEqual>
constexpr typename lru::CacheManager<Key, Value, KeyHash, KeyEqual>::Iterator
lru::CacheManager<Key, Value, KeyHash, KeyEqual>::Begin(
    this CacheManager const& self) {
    return { self.map.begin() };
}

template <typename Key, typename Value, typename KeyHash, typename KeyEqual>
constexpr typename lru::CacheManager<Key, Value, KeyHash, KeyEqual>::Iterator
lru::CacheManager<Key, Value, KeyHash, KeyEqual>::End(
    this CacheManager const& self) {
    return { self.map.end() };
}

template <typename Key, typename Value, typename KeyHash, typename KeyEqual>
constexpr void lru::CacheManager<Key, Value, KeyHash, KeyEqual>::SetMaxSize(
    this CacheManager& self, size_t max_size) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < max_size);

    self.max_size = max_size;

    while (self.max_size < self.list.size()) { self.DiscardLast(); }
}

template <typename Key, typename Value, typename KeyHash, typename KeyEqual>
template <typename FindKey>
constexpr typename lru::CacheManager<Key, Value, KeyHash, KeyEqual>::Iterator
lru::CacheManager<Key, Value, KeyHash, KeyEqual>::Find(
    this CacheManager const& self, FindKey const& key) {
    auto map_iter{ self.map.find(key) };

    if (map_iter != self.map.end()) {
        self.list.splice(self.list.begin(), self.list, map_iter->second);
    }

    return { map_iter };
}

template <typename Key, typename Value, typename KeyHash, typename KeyEqual>
constexpr typename lru::CacheManager<Key, Value, KeyHash, KeyEqual>::Iterator
lru::CacheManager<Key, Value, KeyHash, KeyEqual>::Set(this CacheManager& self,
                                                      Key const& key,
                                                      Value const& value) {
    auto iter{ self.Find(key) };

    if (iter.map_iter != self.map.end()) {
        iter.map_iter->second->second = value;
        return iter;
    }

    while (self.max_size <= self.list.size()) { self.DiscardLast(); }

    self.list.push_front({ key, value });

    auto p{ self.map.insert({ key, self.list.begin() }) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(p.second);

    return { p.first };
}

template <typename Key, typename Value, typename KeyHash, typename KeyEqual>
constexpr void lru::CacheManager<Key, Value, KeyHash, KeyEqual>::DiscardLast(
    this CacheManager& self) {
    if (self.list.empty()) { return; }

    size_t cnt{ self.map.erase(self.list.back().first) };
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt == 1);

    self.list.pop_back();
}

template <typename Key, typename Value, typename KeyHash, typename KeyEqual>
constexpr void lru::CacheManager<Key, Value, KeyHash, KeyEqual>::Clear(
    this CacheManager& self) {
    self.list.clear();
    self.map.clear();
}

}  // namespace zeta::core::debug_utils
