#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/basic_bin_tree_node.hpp>
#include <zeta/core/bin_tree.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/fixed_point.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/multi_level_ptr_table.hpp>
#include <zeta/core/rbtree.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

#pragma push_macro("CntrTplDeclParamList")
#define CntrTplDeclParamList                           \
    typename NodeHashLike_, typename NodeCompareLike_, \
        typename TableNodeAllocatorLike_

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                             \
    typename NodeHashLike, typename NodeCompareLike, \
        typename TableNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList NodeHashLike, NodeCompareLike, TableNodeAllocatorLike

namespace zeta::core::generic_hash_table {

constexpr auto max_level{ multi_level_ptr_table::max_level };
constexpr auto branch_num{ multi_level_ptr_table::max_branch_num };

constexpr utils::Array<unsigned short, max_level> branch_nums{ [] {
    utils::Array<unsigned short, max_level> arr;
    for (size_t i{ 0 }; i < max_level; ++i) { arr.elems[i] = branch_num; }
    return arr;
}() };

using UFP =
    fixed_point::FixedPoint<value_wrapper::FalseType,
                            value_wrapper::StaticValueWrapper<size_t, 16>,
                            value_wrapper::StaticValueWrapper<size_t, 16>>;

constexpr size_t min_bucket_size{ 37 };

#if 32 <= ZETA_Core_size_width
constexpr size_t max_bucket_size{ 1'000'000'007 };
#else
#error "Unsupported architecture."
#endif

constexpr size_t min_move_quata_per_op{ 2 };
constexpr size_t max_move_quata_per_op{ integral::RangeMaxOf<size_t> };

constexpr UFP min_center_load_ratio{ UFP::FromFraction(50U, 100U) };  // 0.5
constexpr UFP max_center_load_ratio{ UFP::FromIntegral(32U) };

constexpr UFP min_drift_ratio{ UFP::FromFraction(150U, 100U) };  // 1.5
constexpr UFP max_drift_ratio{ UFP::FromIntegral(16U) };         // 16

struct TreeNode : public basic_bin_tree_node::Node<
                      void*, value_wrapper::TrueType, value_wrapper::FalseType,
                      value_wrapper::FalseType, value_wrapper::FalseType> {};

}  // namespace zeta::core::generic_hash_table

namespace zeta::core {

template <>
struct bin_tree::Traits<generic_hash_table::TreeNode const> {
    static constexpr bool IsConst();

    static constexpr bool HasAccSize();

    static generic_hash_table::TreeNode const* GetP(
        generic_hash_table::TreeNode const* n);
    static generic_hash_table::TreeNode const* GetL(
        generic_hash_table::TreeNode const* n);
    static generic_hash_table::TreeNode const* GetR(
        generic_hash_table::TreeNode const* n);
};

template <>
struct bin_tree::Traits<generic_hash_table::TreeNode>
    : public bin_tree::Traits<generic_hash_table::TreeNode const> {
    static constexpr bool IsConst();

    static generic_hash_table::TreeNode* GetP(generic_hash_table::TreeNode* n);
    static generic_hash_table::TreeNode* GetL(generic_hash_table::TreeNode* n);
    static generic_hash_table::TreeNode* GetR(generic_hash_table::TreeNode* n);

    static void SetP(generic_hash_table::TreeNode* n,
                     generic_hash_table::TreeNode* m);
    static void SetL(generic_hash_table::TreeNode* n,
                     generic_hash_table::TreeNode* m);
    static void SetR(generic_hash_table::TreeNode* n,
                     generic_hash_table::TreeNode* m);
};

template <>
struct rbtree::Traits<generic_hash_table::TreeNode const> {
    static unsigned GetColor(generic_hash_table::TreeNode const* n);
};

template <>
struct rbtree::Traits<generic_hash_table::TreeNode>
    : public rbtree::Traits<generic_hash_table::TreeNode const> {
    static void SetColor(generic_hash_table::TreeNode* n, unsigned color);
};

}  // namespace zeta::core

namespace zeta::core::generic_hash_table {

struct RehashingConfig {
    unsigned move_quata_per_op;
    UFP center_load_ratio;
    UFP drift_ratio;
};

struct Node {
    ZETA_Core_DebugStructPadding;

    TreeNode n;

    ZETA_Core_DebugStructPadding;

    void Init();
};

template <CntrTplDeclParamList>
struct Cntr {
    using NodeHashLike = NodeHashLike_;
    using NodeCompareLike = NodeCompareLike_;
    using TableNodeAllocatorLike = TableNodeAllocatorLike_;

    struct SanitizeTreeRet {
        size_t cnt;
        TreeNode* most_l_n;
        TreeNode* most_r_n;
    };

    unsigned long long cur_salt;
    unsigned long long nxt_salt;

    void* cur_table_root;
    void* nxt_table_root;

    size_t cur_table_size;
    size_t nxt_table_size;

    size_t cur_bucket_size;
    size_t nxt_bucket_size;

    size_t size;

    RehashingConfig rehashing_config;

    NodeHashLike node_hash;
    // Should be set before Init().

    NodeCompareLike node_compare;
    // Should be set before Init().

    TableNodeAllocatorLike table_node_alctr;
    // Should be set before Init().

    // SaltRandomGenerator salt_random_gen;
};

namespace ops {

template <CntrTplParamList>
void Init(Cntr<CntrTplArgList>* ght);

template <CntrTplParamList>
void Deinit(Cntr<CntrTplArgList>* ght);

template <CntrTplParamList>
size_t GetSize(Cntr<CntrTplArgList> const* ght);

template <CntrTplParamList>
bool Contain(Cntr<CntrTplArgList> const* ght, Node const* node);

template <CntrTplParamList, typename KeyHash, typename KeyNodeCompare>
Node* Find(Cntr<CntrTplArgList> const* ght, void const* key,
           KeyHash const& key_hash, KeyNodeCompare const& key_node_compare);

template <CntrTplParamList>
void Insert(Cntr<CntrTplArgList>* ght, Node* node);

template <CntrTplParamList>
void Extract(Cntr<CntrTplArgList>* ght, Node* node);

template <CntrTplParamList>
Node* ExtractAny(Cntr<CntrTplArgList>* ght);

template <CntrTplParamList>
void ExtractAll(Cntr<CntrTplArgList>* ght);

template <CntrTplParamList>
bool RunPending(Cntr<CntrTplArgList>* ght, size_t quata);

template <CntrTplParamList>
unsigned long long GetEffFactor(Cntr<CntrTplArgList> const* ght);

template <CntrTplParamList>
void Sanitize(Cntr<CntrTplArgList> const* ght,
              mem_recorder::MemRecorder* dst_table,
              mem_recorder::MemRecorder* dst_node);

}  // namespace ops

}  // namespace zeta::core::generic_hash_table

#pragma pop_macro("CntrTplDeclParamList")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
