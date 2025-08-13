#include <algorithm>
#include <set>
#include <vector>

template <typename Elem, typename Compare>
class NaiveSearchTable {
public:
    NaiveSearchTable() = default;

    NaiveSearchTable(const std::vector<Elem>& vec);

    NaiveSearchTable(const Compare& cmp);

    NaiveSearchTable(const std::vector<Elem>& vec, const Compare& cmp);

    size_t size() const;

    template <typename Key>
    const Elem* LowerBound(const Key& key) const;

    template <typename Key>
    const Elem* UpperBound(const Key& key) const;

    void Insert(const Elem& elem);

    template <typename Key>
    void Erase(const Key& key);

    void Sanitize() const;

private:
    std::multiset<Elem, Compare> s_;
};

// -----------------------------------------------------------------------------

template <typename Elem, typename Compare>
NaiveSearchTable<Elem, Compare>::NaiveSearchTable(const std::vector<Elem>& vec)
    : s_{ vec.begin(), vec.end() } {}

template <typename Elem, typename Compare>
NaiveSearchTable<Elem, Compare>::NaiveSearchTable(const Compare& cmp)
    : s_{ cmp } {}

template <typename Elem, typename Compare>
NaiveSearchTable<Elem, Compare>::NaiveSearchTable(const std::vector<Elem>& vec,
                                                  const Compare& cmp)
    : s_{ vec.begin(), vec.end(), cmp } {}

template <typename Elem, typename Compare>
size_t NaiveSearchTable<Elem, Compare>::size() const {
    return this->s_.size();
}

template <typename Elem, typename Compare>
template <typename Key>
const Elem* NaiveSearchTable<Elem, Compare>::LowerBound(const Key& key) const {
    auto iter{ this->s_.lower_bound(key) };

    return iter == this->s_.end() ? nullptr : &*iter;
}

template <typename Elem, typename Compare>
template <typename Key>
const Elem* NaiveSearchTable<Elem, Compare>::UpperBound(const Key& key) const {
    auto iter{ this->s_.upper_bound(key) };

    return iter == this->s_.end() ? nullptr : &*iter;
}

template <typename Elem, typename Compare>
void NaiveSearchTable<Elem, Compare>::Insert(const Elem& elem) {
    this->s_.insert(elem);
}

template <typename Elem, typename Compare>
template <typename Key>
void NaiveSearchTable<Elem, Compare>::Erase(const Key& key) {
    auto iter{ this->s_.find(key) };

    if (iter != this->s_.end()) { this->s_.erase(iter); }
}

template <typename Elem, typename Compare>
void NaiveSearchTable<Elem, Compare>::Sanitize() const {
    return;
}
