#pragma once

#include <zeta/core/define.h>

#include <array>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace std {

template <typename First, typename Second>
struct hash<pair<First, Second>> {
    hash<First> first_hash;
    hash<Second> second_hash;

    auto operator()(const pair<First, Second>& p) const {
        return this->first_hash(p.first) * 23 + this->second_hash(p.second);
    }
};

template <typename Val>
struct SeqCntrHash {
    hash<Val> hash;

    template <typename Iterator>
    unsigned long long operator()(Iterator beg, Iterator end) const {
        unsigned long long ret{ static_cast<unsigned long long>(end - beg) };

        for (; beg != end; ++beg) {
            ret ^= this->hash(*beg) + 0x9e3779b9 + (ret << 6) + (ret >> 2);
        }

        return ret;
    }

    template <typename SeqCntr>
    unsigned long long operator()(const SeqCntr& seq_cntr) const {
        return (*this)(seq_cntr.begin(), seq_cntr.end());
    }
};

template <typename Val, size_t N>
struct hash<array<Val, N>> : public SeqCntrHash<Val> {};

template <typename Val, typename Allocator>
struct hash<vector<Val, Allocator>> : public SeqCntrHash<Val> {};

template <typename Val, typename Allocator>
struct hash<deque<Val, Allocator>> : public SeqCntrHash<Val> {};

template <typename Val, typename Allocator>
struct hash<list<Val, Allocator>> : public SeqCntrHash<Val> {};

template <typename Val>
struct AssocCntrHash {
    hash<Val> hash;

    template <typename Iterator>
    unsigned long long operator()(Iterator beg, Iterator end) const {
        unsigned long long a{ 0 };
        unsigned long long b{ 0 };

        for (; beg != end; ++beg) {
            unsigned long long x{ static_cast<unsigned long long>(
                this->hash(*beg)) };

            a ^= x;
            b += x;
        }

        return a ^ b;
    }

    template <typename AssocCntr>
    unsigned long long operator()(const AssocCntr& assoc_cntr) const {
        return (*this)(assoc_cntr.begin(), assoc_cntr.end());
    }
};

template <typename Key, typename Compare, typename Allocator>
struct hash<set<Key, Compare, Allocator>> : public AssocCntrHash<Key> {};

template <typename Key, typename Compare, typename Allocator>
struct hash<multiset<Key, Compare, Allocator>> : public AssocCntrHash<Key> {};

template <typename Key, typename Val, typename Compare, typename Allocator>
struct hash<map<Key, Val, Compare, Allocator>> : public AssocCntrHash<Key> {};

template <typename Key, typename Val, typename Compare, typename Allocator>
struct hash<multimap<Key, Val, Compare, Allocator>>
    : public AssocCntrHash<Key> {};

template <typename Key, typename Hash, typename Pred, typename Allocator>
struct hash<unordered_set<Key, Hash, Pred, Allocator>>
    : public AssocCntrHash<Key> {};

template <typename Key, typename Hash, typename Pred, typename Allocator>
struct hash<unordered_multiset<Key, Hash, Pred, Allocator>>
    : public AssocCntrHash<Key> {};

template <typename Key, typename Val, typename Hash, typename Pred,
          typename Allocator>
struct hash<unordered_map<Key, Val, Hash, Pred, Allocator>>
    : public AssocCntrHash<Key> {};

template <typename Key, typename Val, typename Hash, typename Pred,
          typename Allocator>
struct hash<unordered_multimap<Key, Val, Hash, Pred, Allocator>>
    : public AssocCntrHash<Key> {};

}  // namespace std

template <typename Value>
struct HashCore {
    unsigned long long operator()(void const* context, const Value& a,
                                  unsigned long long salt) const {
        ZETA_Core_Unused(context);
        return std::hash<Value>{}(a) * 23 + salt;
    }
};

template <typename Value>
unsigned long long Hash(void const* context, const Value& a,
                        unsigned long long salt) {
    return HashCore<Value>{}(context, a, salt);
}

template <typename Value>
unsigned long long ZetaHash(void const* context, void const* a,
                            unsigned long long salt) {
    return Hash(context, *(Value const*)a, salt);
}
