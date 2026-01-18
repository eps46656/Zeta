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

namespace zeta::core {

struct GenericHashTableNode;

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
struct GenericHashTable;

// -----------------------------------------------------------------------------

struct GenericHashTableNode {
    ZETA_Core_DebugStructPadding;

    BinTreeNodeTpl<void*, value_wrapper::StaticValueWrapper<true>,
                   value_wrapper::StaticValueWrapper<false>,
                   value_wrapper::StaticValueWrapper<false>,
                   value_wrapper::StaticValueWrapper<false>>
        n;

    ZETA_Core_DebugStructPadding;

    void Init();
};

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
struct GenericHashTable {
    using MLPT = MultiLevelPtrTable<TableNodeAllocator>;

    using Node = GenericHashTableNode;

    using TreeNode = decltype(Declval<GenericHashTableNode>().n);

    static constexpr unsigned max_level{ MLPT::max_level };

    static constexpr size_t branch_num{ MLPT::max_branch_num };

    static constexpr Array<unsigned short, max_level> branch_nums{ [] {
        Array<unsigned short, max_level> arr;
        for (size_t i{ 0 }; i < max_level; ++i) { arr.elems[i] = branch_num; }
        return arr;
    }() };

    static constexpr size_t capacities[]{
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

    static constexpr struct TreeNodeOperator {
        static constexpr bool en_acc_size{ false };

        TreeNode* GetP(TreeNode* n) const;
        TreeNode* GetL(TreeNode* n) const;
        TreeNode* GetR(TreeNode* n) const;

        TreeNode const* GetP(TreeNode const* n) const;
        TreeNode const* GetL(TreeNode const* n) const;
        TreeNode const* GetR(TreeNode const* n) const;

        void SetP(TreeNode* n, TreeNode* m) const;
        void SetL(TreeNode* n, TreeNode* m) const;
        void SetR(TreeNode* n, TreeNode* m) const;

        unsigned int GetColor(TreeNode* n) const;
        void SetColor(TreeNode* n, unsigned int color) const;
    } tn_opr;

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

    NodeHash node_hash;
    // Should be set before Init().

    NodeCompare node_compare;
    // Should be set before Init().

    TableNodeAllocator table_node_allocator;
    // Should be set before Init().

    // -------------------------------------------------------------------------

    static consteval Array<unsigned short, max_level> BranchNums_();

    static size_t GetBucketIdx_(unsigned long long hash_code, size_t capacity);

    static size_t GetIdx_(unsigned level, size_t const* idxes);

    static void SetIdxes_(unsigned level, size_t* idxes, size_t idx);

    static size_t FindPrvCapacity_(size_t capacity);

    static size_t FindNxtCapacity_(size_t capacity);

    template <typename KeyHash, typename KeyNodeCompare>
    static Node* Find_(unsigned long long salt, MLPT* table, size_t capacity,
                       void const* key, KeyHash const& key_hash,
                       KeyNodeCompare const& key_node_compare);

    static void Insert_(GenericHashTable* ght, unsigned long long salt,
                        MLPT* table, size_t capacity, Node* node);

    static bool TryExtract_(GenericHashTable* ght, unsigned long long salt,
                            MLPT* table, size_t capacity, Node* node,
                            TreeNode* root);

    static void TryTransfer_(GenericHashTable const* ght, MLPT* cur_table,
                             MLPT* nxt_table, size_t quata);

    static void TryRunPending_(GenericHashTable const* ght, MLPT* cur_table,
                               MLPT* nxt_table, size_t quata);

    static SanitizeTreeRet SanitizeTree_(GenericHashTable const* ght,
                                         MemRecorder* dst_node,
                                         unsigned long long salt,
                                         size_t capacity, size_t bucket_idx,
                                         TreeNode* n);

    // -------------------------------------------------------------------------

    static void Init(void* ght);

    static void Deinit(void* ght);

    static size_t GetSize(void const* ght);

    static bool Contain(void const* ght, void const* node);

    template <typename KeyHash, typename KeyNodeCompare>
    static void* Find(void const* ght, void const* key, KeyHash const& key_hash,
                      KeyNodeCompare const& key_node_compare);

    static void Insert(void* ght, void* node);

    static void Extract(void* ght, void* node);

    static void* ExtractAny(void* ght);

    static void ExtractAll(void* ght);

    static bool RunPending(void* ght, size_t quata);

    static unsigned long long GetEffFactor(void const* ght);

    static bool CheckCntr(void const* ght);

    static void Sanitize(void const* ght, MemRecorder* dst_table,
                         MemRecorder* dst_node);
};

}  // namespace zeta::core
