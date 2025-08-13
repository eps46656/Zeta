#include <algorithm>
#include <cassert>
#include <set>
#include <vector>

template <typename Elem, typename Compare>
class DynamicSearchTable {
public:
    static constexpr size_t threshold{ 8 };
    static constexpr size_t decomposition_quata{ 6 };

    struct Node {
        union {
            std::vector<Elem>* seg;
            Elem elem;
        };

        bool is_finished;
    };

    DynamicSearchTable();

    DynamicSearchTable(const std::vector<Elem>& vec);

    DynamicSearchTable(const Compare& cmp);

    DynamicSearchTable(const std::vector<Elem>& vec, const Compare& cmp);

    ~DynamicSearchTable();

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
    struct SCompare {
        using is_transparent = std::true_type;

        const DynamicSearchTable& dt;

        bool operator()(Node* x, Node* y) const;

        template <typename Key>
        bool operator()(Node* node, const Key& key) const;

        template <typename Key>
        bool operator()(const Key& key, Node* node) const;
    };

    Compare cmp_;

    mutable std::multiset<Node*, SCompare> s_;

    size_t size_;

    static const Elem* GetElem_(const Node* n);

    static void DestroyNode_(Node* n);

    template <typename Key>
    const Elem* LinearLowerBound_(const Elem* seg, size_t seg_size,
                                  const Key& key) const;

    template <typename Key>
    const Elem* LinearUpperBound_(const Elem* seg, size_t seg_size,
                                  const Key& key) const;

    void Decompose_(
        typename std::multiset<Node*, SCompare>::iterator iter) const;
};

// -----------------------------------------------------------------------------

template <typename Elem, typename Compare>
DynamicSearchTable<Elem, Compare>::DynamicSearchTable()
    : s_{ SCompare{ *this } }, size_{ 0 } {}

template <typename Elem, typename Compare>
DynamicSearchTable<Elem, Compare>::DynamicSearchTable(
    const std::vector<Elem>& vec)
    : s_{ SCompare{ *this } }, size_{ vec.size() } {
    if (vec.size() <= threshold) {
        for (const auto& elem : vec) {
            this->s_.insert(new Node{ .elem = elem, .is_finished = true });
        }
    } else {
        Node* node{ new Node{ .seg = new std::vector<Elem>{ vec },
                              .is_finished = false } };

        std::vector<Elem>& seg{ *node->seg };

        std::swap(seg[0], *std::min_element(seg.begin(), seg.end()));

        this->s_.insert(node);
    }
}

template <typename Elem, typename Compare>
DynamicSearchTable<Elem, Compare>::DynamicSearchTable(const Compare& cmp)
    : cmp_{ cmp }, s_{ SCompare{ *this } }, size_{ 0 } {}

template <typename Elem, typename Compare>
DynamicSearchTable<Elem, Compare>::DynamicSearchTable(
    const std::vector<Elem>& vec, const Compare& cmp)
    : cmp_{ cmp }, s_{ SCompare{ *this } }, size_{ vec.size() } {
    if (vec.size() <= threshold) {
        for (const auto& elem : vec) {
            this->s_.insert(new Node{ .elem = elem, .is_finished = true });
        }
    } else {
        Node* node{ new Node{ .seg = new std::vector<Elem>{ vec },
                              .is_finished = false } };

        std::vector<Elem>& seg{ *node->seg };

        std::swap(seg[0], *std::min_element(seg.begin(), seg.end()));

        this->s_.insert(node);
    }
}

template <typename Elem, typename Compare>
DynamicSearchTable<Elem, Compare>::~DynamicSearchTable() {
    while (!this->s_.empty()) {
        auto iter{ this->s_.begin() };

        Node* n{ *iter };

        this->s_.erase(iter);

        DestroyNode_(n);
    }
}

template <typename Elem, typename Compare>
size_t DynamicSearchTable<Elem, Compare>::size() const {
    return this->size_;
}

template <typename Elem, typename Compare>
template <typename Key>
const Elem* DynamicSearchTable<Elem, Compare>::LowerBound(
    const Key& key) const {
    auto iter{ this->s_.lower_bound(key) };

    Node* n{ nullptr };
    const Elem* ret{ nullptr };

    for (size_t quata{ decomposition_quata };;) {
        if (iter != this->s_.end()) {
            n = *iter;
            ret = GetElem_(n);
        }

        if (iter == this->s_.begin()) { return ret; }

        auto prv_iter{ std::prev(iter) };

        Node* prv_n{ *prv_iter };

        if (!this->cmp_(*GetElem_(prv_n), key)) {
            iter = prv_iter;
            continue;
        }

        if (prv_n->is_finished) { return ret; }

        std::vector<Elem>& seg{ *prv_n->seg };
        size_t seg_size{ seg.size() };

        if (quata == 0) {
            const Elem* cur = this->LinearLowerBound_(&seg[0], seg_size, key);

            return cur == nullptr ? ret : cur;
        }

        this->Decompose_(prv_iter);

        --quata;
    }

    return ret;
}

template <typename Elem, typename Compare>
template <typename Key>
const Elem* DynamicSearchTable<Elem, Compare>::UpperBound(
    const Key& key) const {
    auto iter{ this->s_.upper_bound(key) };

    Node* n{ nullptr };
    const Elem* ret{ nullptr };

    for (size_t quata{ decomposition_quata };;) {
        if (iter != this->s_.end()) {
            n = *iter;
            ret = GetElem_(n);
        }

        if (iter == this->s_.begin()) { return ret; }

        auto prv_iter{ std::prev(iter) };

        Node* prv_n{ *prv_iter };

        if (this->cmp_(key, *GetElem_(prv_n))) {
            iter = prv_iter;
            continue;
        }

        if (prv_n->is_finished) { return ret; }

        std::vector<Elem>& seg{ *prv_n->seg };
        size_t seg_size{ seg.size() };

        if (quata == 0) {
            const Elem* cur = this->LinearUpperBound_(&seg[0], seg_size, key);

            return cur == nullptr ? ret : cur;
        }

        this->Decompose_(prv_iter);

        --quata;
    }

    return ret;
}

template <typename Elem, typename Compare>
void DynamicSearchTable<Elem, Compare>::Insert(const Elem& elem) {
    auto iter{ this->s_.upper_bound(elem) };

    for (size_t quata{ decomposition_quata };;) {
        if (iter == this->s_.begin()) {
            this->s_.insert(new Node{ .elem = elem, .is_finished = true });
            ++this->size_;
            return;
        }

        auto prv_iter{ std::prev(iter) };

        Node* prv_n{ *prv_iter };

        if (this->cmp_(elem, *GetElem_(prv_n))) {
            iter = prv_iter;
            continue;
        }

        if (prv_n->is_finished) {
            this->s_.insert(new Node{ .elem = elem, .is_finished = true });
            ++this->size_;
            return;
        }

        if (quata == 0) {
            (*prv_n->seg).push_back(elem);
            ++this->size_;
            return;
        }

        this->Decompose_(prv_iter);

        --quata;
    }
}

template <typename Elem, typename Compare>
template <typename Key>
void DynamicSearchTable<Elem, Compare>::Erase(const Key& key) {
    assert(0 < this->size_);

    auto iter{ this->s_.upper_bound(key) };

    if (iter == this->s_.end()) { return; }

    for (size_t quata{ decomposition_quata };;) {
        if (iter == this->s_.begin()) { return; }

        auto prv_iter{ std::prev(iter) };

        Node* prv_n{ *prv_iter };

        if (this->cmp_(key, *GetElem_(prv_n))) {
            iter = prv_iter;
            continue;
        }

        if (prv_n->is_finished) {
            if (!this->cmp_(prv_n->elem, key)) {
                this->s_.erase(prv_iter);
                DestroyNode_(prv_n);
                --this->size_;
            }

            return;
        }

        if (0 < quata) {
            this->Decompose_(prv_iter);

            --quata;

            continue;
        }

        std::vector<Elem>& seg{ *prv_n->seg };

        auto v_beg{ seg.begin() };
        auto v_end{ seg.end() };

        for (auto v_iter{ v_beg }; v_iter != v_end; ++v_iter) {
            if (this->cmp_(*v_iter, key) || this->cmp_(key, *v_iter)) {
                continue;
            }

            v_end = std::prev(v_end);

            *v_iter = std::move(*v_end);

            if (v_beg == v_iter) {
                std::swap(*v_beg, *std::min_element(v_beg, v_end));
            }

            seg.pop_back();
            --this->size_;

            break;
        }

        if (seg.size() == 1) {
            new (&prv_n->elem) Elem{ std::move(seg[0]) };
            delete &seg;
            prv_n->is_finished = true;
        }

        return;
    }
}

template <typename Elem, typename Compare>
void DynamicSearchTable<Elem, Compare>::Sanitize() const {
    size_t re_size{ 0 };

    for (auto iter{ this->s_.begin() }, end{ this->s_.end() }; iter != end;) {
        Node* n{ *iter };

        if (n->is_finished) {
            ++re_size;
        } else {
            std::vector<Elem>& seg{ *n->seg };

            re_size += seg.size();

            assert(1 < seg.size());

            assert(std::min_element(seg.begin(), seg.end()) == seg.begin());
        }

        auto nxt_iter{ std::next(iter) };

        if (nxt_iter != end) {
            const Elem& nxt_elem{ *GetElem_(*nxt_iter) };

            if (n->is_finished) {
                assert(!this->cmp_(nxt_elem, n->elem));
            } else {
                std::vector<Elem>& seg{ *n->seg };

                assert(
                    std::all_of(seg.begin(), seg.end(), [&](const Elem& elem) {
                        return !this->cmp_(nxt_elem, elem);
                    }));
            }
        }

        iter = nxt_iter;
    }

    assert(this->size_ == re_size);
}

template <typename Elem, typename Compare>
bool DynamicSearchTable<Elem, Compare>::SCompare::operator()(Node* x,
                                                             Node* y) const {
    return this->dt.cmp_(*GetElem_(x), *GetElem_(y));
}

template <typename Elem, typename Compare>
template <typename Key>
bool DynamicSearchTable<Elem, Compare>::SCompare::operator()(
    Node* node, const Key& key) const {
    return this->dt.cmp_(*GetElem_(node), key);
}

template <typename Elem, typename Compare>
template <typename Key>
bool DynamicSearchTable<Elem, Compare>::SCompare::operator()(const Key& key,
                                                             Node* node) const {
    return this->dt.cmp_(key, *GetElem_(node));
}

template <typename Elem, typename Compare>
const Elem* DynamicSearchTable<Elem, Compare>::GetElem_(const Node* n) {
    return n->is_finished ? &n->elem : &(*n->seg)[0];
}

template <typename Elem, typename Compare>
void DynamicSearchTable<Elem, Compare>::DestroyNode_(Node* n) {
    if (n->is_finished) {
        n->elem.~Elem();
    } else {
        delete n->seg;
    }

    delete n;
}

template <typename Elem, typename Compare>
template <typename Key>
const Elem* DynamicSearchTable<Elem, Compare>::LinearLowerBound_(
    const Elem* seg, size_t seg_size, const Key& key) const {
    const Elem* ret{ nullptr };

    for (size_t i{ 0 }; i < seg_size; ++i) {
        const Elem* cur{ seg + i };

        if (!this->cmp_(*cur, key) &&
            (ret == nullptr || this->cmp_(*cur, *ret))) {
            ret = cur;
        }
    }

    return ret;
}

template <typename Elem, typename Compare>
template <typename Key>
const Elem* DynamicSearchTable<Elem, Compare>::LinearUpperBound_(
    const Elem* seg, size_t seg_size, const Key& key) const {
    const Elem* ret{ nullptr };

    for (size_t i{ 0 }; i < seg_size; ++i) {
        const Elem* cur{ seg + i };

        if (this->cmp_(key, *cur) &&
            (ret == nullptr || this->cmp_(*cur, *ret))) {
            ret = cur;
        }
    }

    return ret;
}

template <typename Elem, typename Compare>
void DynamicSearchTable<Elem, Compare>::Decompose_(
    typename std::multiset<Node*, SCompare>::iterator iter) const {
    Node* n{ *iter };

    if (n->is_finished) { return; }

    std::vector<Elem>& seg{ *n->seg };
    size_t seg_size{ seg.size() };

    size_t half_seg_size{ seg_size / 2 };

    Elem* beg{ &seg[0] };
    Elem* mid{ &seg[0] + half_seg_size };
    Elem* end{ &seg[0] + seg_size };

    std::nth_element(beg + 1, mid, end, this->cmp_);

    Node* new_n;

    if (end - mid == 1) {
        new_n = new Node{ .elem = std::move(*mid), .is_finished = true };
    } else {
        new_n =
            new Node{ .seg = new std::vector<Elem>{}, .is_finished = false };

        new_n->seg->insert(new_n->seg->end(), std::make_move_iterator(mid),
                           std::make_move_iterator(end));
    }

    seg.erase(seg.begin() + half_seg_size, seg.end());

    if (half_seg_size == 1) {
        new (&n->elem) Elem{ std::move(seg[0]) };
        delete &seg;
        n->is_finished = true;
    }

    this->s_.insert(std::next(iter), new_n);
}
