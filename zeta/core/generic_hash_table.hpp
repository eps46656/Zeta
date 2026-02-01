#pragma once

#include <zeta/core/allocator.hpp>
#include <zeta/core/bin_tree_node_tpl.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_check_utils.hpp>
#include <zeta/core/multi_level_ptr_table.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

#pragma push_macro("CntrTplDeclParamList")
#pragma push_macro("CntrTplArgList")

#define CntrTplDeclParamList                         \
    typename NodeHashLike, typename NodeCompareLike, \
        typename TableNodeAllocatorLike

#define CntrTplArgList NodeHashLike, NodeCompareLike, TableNodeAllocatorLike

namespace zeta::core::generic_hash_table {

constexpr auto max_level{ multi_level_ptr_table::max_level };
constexpr auto branch_num{ multi_level_ptr_table::max_branch_num };

constexpr Array<unsigned short, max_level> branch_nums{ [] {
    Array<unsigned short, max_level> arr;
    for (size_t i{ 0 }; i < max_level; ++i) { arr.elems[i] = branch_num; }
    return arr;
}() };

struct Node;

template <typename NodeHashLike, typename NodeCompareLike,
          typename TableNodeAllocatorLike>
struct Cntr;

using TreeNode =
    BinTreeNodeTpl<void*, value_wrapper::TrueType, value_wrapper::FalseType,
                   value_wrapper::FalseType, value_wrapper::FalseType>;

struct TreeNodeView {
    static constexpr bool en_acc_size{ false };

    static constexpr bool IsConst(type_wrapper::TypeWrapper<TreeNodeView*>);

    static constexpr bool IsConst(
        type_wrapper::TypeWrapper<TreeNodeView const*>);

    static constexpr bool IsAccSizeEnabled(
        type_wrapper::TypeWrapper<TreeNodeView const*>);

    static TreeNodeView* GetP(TreeNodeView* n);
    static TreeNodeView* GetL(TreeNodeView* n);
    static TreeNodeView* GetR(TreeNodeView* n);

    static TreeNodeView const* GetP(TreeNodeView const* n);
    static TreeNodeView const* GetL(TreeNodeView const* n);
    static TreeNodeView const* GetR(TreeNodeView const* n);

    static void SetP(TreeNodeView* n, TreeNodeView* m);
    static void SetL(TreeNodeView* n, TreeNodeView* m);
    static void SetR(TreeNodeView* n, TreeNodeView* m);

    static unsigned int GetColor(TreeNodeView const* n);
    static void SetColor(TreeNodeView* n, unsigned int color);
};

struct Node {
    ZETA_Core_DebugStructPadding;

    TreeNode n;

    ZETA_Core_DebugStructPadding;

    void Init();
};

constexpr size_t capacities[]{
    61ULL,
    251ULL,
    1021ULL,
    4093ULL,
    16381ULL,
    65521ULL,

#if 32 <= ZETA_Core_size_width
    262139ULL,
    1048573ULL,
    4194301ULL,
    16777213ULL,
    67108859ULL,
    268435399ULL,
    1073741789ULL,
    4294967291ULL,
#endif

#if 48 <= ZETA_Core_size_width
    17179869143ULL,
    68719476731ULL,
    274877906899ULL,
    1099511627689ULL,
    4398046511093ULL,
    17592186044399ULL,
    70368744177643ULL,
    281474976710597ULL,
#endif

#if 64 <= ZETA_Core_size_width
    1125899906842597ULL,
    4503599627370449ULL,
    18014398509481951ULL,
    72057594037927931ULL,
    288230376151711717ULL,
    1152921504606846883ULL,
    4611686018427387847ULL,
    18446744073709551557ULL,
#endif

};

template <typename NodeHashLike, typename NodeCompareLike,
          typename TableNodeAllocatorLike>
struct Cntr {
    struct SanitizeTreeRet {
        size_t cnt;
        TreeNode* most_l_n;
        TreeNode* most_r_n;
    };

    // -------------------------------------------------------------------------

    unsigned long long cur_salt;
    unsigned long long nxt_salt;

    void* cur_table_root;
    void* nxt_table_root;

    size_t cur_table_size;
    size_t nxt_table_size;

    size_t cur_capacity;
    size_t nxt_capacity;

    size_t size;

    NodeHashLike node_hash;
    // Should be set before Init().

    NodeCompareLike node_compare;
    // Should be set before Init().

    TableNodeAllocatorLike table_node_alctr;
    // Should be set before Init().
};

namespace ops {

template <CntrTplDeclParamList>
void Check(Cntr<CntrTplArgList>* cntr);

template <CntrTplDeclParamList>
void Init(Cntr<CntrTplArgList>* ght);

template <CntrTplDeclParamList>
void Deinit(Cntr<CntrTplArgList>* ght);

template <CntrTplDeclParamList>
size_t GetSize(Cntr<CntrTplArgList> const* ght);

template <CntrTplDeclParamList>
bool Contain(Cntr<CntrTplArgList> const* ght, Node const* node);

template <CntrTplDeclParamList, typename KeyHash, typename KeyNodeCompare>
Node* Find(Cntr<CntrTplArgList> const* ght, void const* key,
           KeyHash const& key_hash, KeyNodeCompare const& key_node_compare);

template <CntrTplDeclParamList>
void Insert(Cntr<CntrTplArgList>* ght, Node* node);

template <CntrTplDeclParamList>
void Extract(Cntr<CntrTplArgList>* ght, Node* node);

template <CntrTplDeclParamList>
Node* ExtractAny(Cntr<CntrTplArgList>* ght);

template <CntrTplDeclParamList>
void ExtractAll(Cntr<CntrTplArgList>* ght);

template <CntrTplDeclParamList>
bool RunPending(Cntr<CntrTplArgList>* ght, size_t quata);

template <CntrTplDeclParamList>
unsigned long long GetEffFactor(Cntr<CntrTplArgList> const* ght);

template <CntrTplDeclParamList>
void Sanitize(Cntr<CntrTplArgList> const* ght, MemRecorder* dst_table,
              MemRecorder* dst_node);

}  // namespace ops

}  // namespace zeta::core::generic_hash_table

#pragma pop_macro("CntrTplDeclParamList")
#pragma pop_macro("CntrTplArgList")
