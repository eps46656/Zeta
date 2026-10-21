#pragma once

#include <unordered_map>

namespace zeta::core::debug_utils::lru {

template <typename Key, typename Value, typename KeyHash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>>
struct CacheManager {
    size_t max_size;

    mutable std::list<std::pair<Key, Value>> list;

    mutable std::unordered_map<
        Key, typename std::list<std::pair<Key, Value>>::iterator, KeyHash,
        KeyEqual>
        map;

    struct Iterator {
        std::unordered_map<Key,
                           typename std::list<std::pair<Key, Value>>::iterator,
                           KeyHash, KeyEqual>::iterator map_iter;

        constexpr std::pair<Key, Value> const& operator*();

        constexpr std::pair<Key, Value> const* operator->();

        friend constexpr bool operator==(Iterator const& lhs,
                                         Iterator const& rhs) = default;

        friend constexpr bool operator!=(Iterator const& lhs,
                                         Iterator const& rhs) = default;
    };

    constexpr CacheManager(size_t max_size);

    constexpr Iterator Begin(this CacheManager const& self);

    constexpr Iterator End(this CacheManager const& self);

    constexpr void SetMaxSize(this CacheManager& self, size_t max_size);

    template <typename FindKey>
    constexpr Iterator Find(this CacheManager const& self, FindKey const& key);

    constexpr Iterator Set(this CacheManager& self, Key const& key,
                           Value const& value);

    constexpr void DiscardLast(this CacheManager& self);

    constexpr void Clear(this CacheManager& self);
};

}  // namespace zeta::core::debug_utils::lru
