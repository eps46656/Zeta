#include <zeta/core/debugger.h>
#include <zeta/core/generic_hash_table.h>
#include <zeta/core/mem_check_utils.h>
#include <zeta/core/multi_level_table.h>
#include <zeta/core/utils.h>

#if ZETA_Core_EnableDebug

#define CheckCntr_(ght) Zeta_Core_GenericHashTable_Check((ght))

#define CheckNode_(ght, node) \
    Zeta_Core_GenericHashTable_CheckNode((ght), (node))

#else

#define CheckCntr_(ght) ZETA_Core_Unused((ght))

#define CheckNode_(ght, node)     \
    {                             \
        ZETA_Core_Unused((ght));  \
        ZETA_Core_Unused((node)); \
    }                             \
    ZETA_Core_StaticAssert(TRUE)

#endif

#define branch_num ZETA_Core_MultiLevelTable_max_branch_num

static unsigned short branch_nums[] = {
    [0 ... ZETA_Core_MultiLevelTable_max_level - 1] = branch_num
};

static size_t capacities[] = {
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

static size_t GetBucketIdx_(unsigned long long hash_code, size_t capacity) {
    return Zeta_Core_ULLHash(hash_code, 0) % capacity;
}

static size_t GetIdx_(int level, size_t const* idxes) {
    size_t idx = 0;

    for (int level_i = level - 1; 0 <= level_i; --level_i) {
        idx = idx * branch_num + idxes[level_i];
    }

    return idx;
}

static void SetIdxes_(int level, size_t* idxes, size_t idx) {
    for (int level_i = 0; level_i < level; ++level_i) {
        idxes[level_i] = idx % branch_num;
        idx /= branch_num;
    }
}

static size_t FindPrvCapacity_(size_t capacity) {
    int len = sizeof(capacities) / sizeof(capacities[0]);

    if (capacity <= capacities[0]) { return 0; }

    int lb = 0;
    int rb = len - 1;

    while (lb < rb) {
        int mb = (lb + rb + 1) / 2;

        if (capacities[mb] < capacity) {
            lb = mb;
        } else {
            rb = mb - 1;
        }
    }

    return capacities[lb];
}

static size_t FindNxtCapacity_(size_t capacity) {
    int len = sizeof(capacities) / sizeof(capacities[0]);

    int lb = 0;
    int rb = len;

    while (lb < rb) {
        int mb = (lb + rb) / 2;

        if (capacity < capacities[mb]) {
            rb = mb;
        } else {
            lb = mb + 1;
        }
    }

    return lb < len ? capacities[lb] : 0;
}

static void* Find_(unsigned long long salt, Zeta_Core_MultiLevelTable* table,
                   size_t capacity, void const* key, Zeta_Core_Hash KeyHash,
                   void const* key_hash_context,
                   Zeta_Core_Compare KeyNodeCompare,
                   void const* key_node_cmp_context) {
    size_t idxes[ZETA_Core_MultiLevelTable_max_level];

    SetIdxes_(table->level, idxes,
              GetBucketIdx_(KeyHash(key_hash_context, key, salt), capacity));

    void* target_n = NULL;

    for (void* n = ({
             void** tmp = Zeta_Core_MultiLevelTable_Access(table, idxes);
             tmp == NULL ? NULL : *tmp;
         });
         n != NULL;) {
        int cmp = KeyNodeCompare(
            key_node_cmp_context, key,
            ZETA_Core_MemberToStruct(Zeta_Core_GenericHashTable_Node, n, n));

        if (cmp == 0) { target_n = n; }

        if (cmp <= 0) {
            n = Zeta_Core_OrdRBTreeNode_GetL(n);
        } else {
            n = Zeta_Core_OrdRBTreeNode_GetR(n);
        }
    }

    return target_n == NULL ? NULL
                            : ZETA_Core_MemberToStruct(
                                  Zeta_Core_GenericHashTable_Node, n, target_n);
}

static void Insert_(Zeta_Core_GenericHashTable* ght, unsigned long long salt,
                    Zeta_Core_MultiLevelTable* table, size_t capacity,
                    Zeta_Core_GenericHashTable_Node* node) {
    size_t idxes[ZETA_Core_MultiLevelTable_max_level];

    SetIdxes_(table->level, idxes,
              GetBucketIdx_(ght->NodeHash(ght->node_hash_context, node, salt),
                            capacity));

    void** root_entry = Zeta_Core_MultiLevelTable_Insert(table, idxes);

    void* n = *root_entry;

    void* le_n = NULL;
    void* gt_n = NULL;

    while (n != NULL) {
        int cmp = ght->NodeCompare(
            ght->node_cmp_context, node,
            ZETA_Core_MemberToStruct(Zeta_Core_GenericHashTable_Node, n, n));

        if (cmp < 0) {
            gt_n = n;
            n = Zeta_Core_OrdRBTreeNode_GetL(n);
        } else {
            le_n = n;
            n = Zeta_Core_OrdRBTreeNode_GetR(n);
        }
    }

    *root_entry =
        Zeta_Core_RBTree(Zeta_Core_OrdRBTreeNode, Insert)(le_n, gt_n, &node->n);
}

static bool_t TryExtract_(Zeta_Core_GenericHashTable* ght,
                          unsigned long long salt,
                          Zeta_Core_MultiLevelTable* table, size_t capacity,
                          Zeta_Core_GenericHashTable_Node* node, void* root) {
    if (capacity == 0) { return FALSE; }

    size_t idxes[ZETA_Core_MultiLevelTable_max_level];

    SetIdxes_(table->level, idxes,
              GetBucketIdx_(ght->NodeHash(ght->node_hash_context, node, salt),
                            capacity));

    void** root_entry = Zeta_Core_MultiLevelTable_Access(table, idxes);

    if (root_entry == NULL || *root_entry != root) { return FALSE; }

    if ((*root_entry = Zeta_Core_RBTree(Zeta_Core_OrdRBTreeNode,
                                        Extract)(&node->n)) == NULL) {
        Zeta_Core_MultiLevelTable_Erase(table, idxes);
    }

    return TRUE;
}

static void TryTransfer_(Zeta_Core_GenericHashTable const* ght,
                         Zeta_Core_MultiLevelTable* cur_table,
                         Zeta_Core_MultiLevelTable* nxt_table, size_t quata) {
    if (cur_table->level == 0 || nxt_table->level == 0 ||
        cur_table->size == 0) {
        return;
    }

    size_t idxes[ZETA_Core_MultiLevelTable_max_level];

    void** root_entry = NULL;

    for (; 0 < quata && 0 < cur_table->size; --quata) {
        if (root_entry == NULL) {
            root_entry = Zeta_Core_MultiLevelTable_FindFirst(cur_table, idxes);
        }

        Zeta_Core_GenericHashTable_Node* trans_node = ZETA_Core_MemberToStruct(
            Zeta_Core_GenericHashTable_Node, n, *root_entry);

        if ((*root_entry = Zeta_Core_RBTree(Zeta_Core_OrdRBTreeNode,
                                            Extract)(&trans_node->n)) == NULL) {
            Zeta_Core_MultiLevelTable_Erase(cur_table, idxes);
            root_entry = NULL;
        }

        Insert_((Zeta_Core_GenericHashTable*)ght, ght->nxt_salt, nxt_table,
                ght->nxt_capacity, trans_node);
    }
}

void Zeta_Core_GenericHashTable_Init(void* ght_) {
    Zeta_Core_GenericHashTable* ght = ght_;
    ZETA_Core_DebugAssert(ght != NULL);

    ght->cur_salt = Zeta_Core_GetRandom();

    ght->cur_table_root = NULL;
    ght->nxt_table_root = NULL;

    ght->cur_table_size = 0;
    ght->nxt_table_size = 0;

    ght->cur_capacity = capacities[0];
    ght->nxt_capacity = 0;

    ght->size = 0;

    ZETA_Core_Allocator_Check(ght->table_node_allocator, alignof(void*));
}

void Zeta_Core_GenericHashTable_Deinit(void* ght) {
    Zeta_Core_GenericHashTable_ExtractAll(ght);
}

size_t Zeta_Core_GenericHashTable_GetSize(void const* ght_) {
    Zeta_Core_GenericHashTable const* ght = ght_;
    CheckCntr_(ght);

    return ght->size;
}

#define TryDo_(quata)                                                         \
    for (;;) {                                                                \
        TryTransfer_(ght, &cur_table, &nxt_table, (quata));                   \
                                                                              \
        if (nxt_capacity == 0) {                                              \
            ((Zeta_Core_GenericHashTable*)ght)->cur_table_size =              \
                cur_table.size;                                               \
            ((Zeta_Core_GenericHashTable*)ght)->cur_table_root =              \
                cur_table.root;                                               \
                                                                              \
            if (cur_table.size * 2 < cur_capacity) {                          \
                ((Zeta_Core_GenericHashTable*)ght)->nxt_capacity =            \
                    FindPrvCapacity_(cur_capacity);                           \
                ((Zeta_Core_GenericHashTable*)ght)->nxt_salt =                \
                    Zeta_Core_GetRandom();                                    \
            } else if (cur_capacity * 8 <= ght->size) {                       \
                ((Zeta_Core_GenericHashTable*)ght)->nxt_capacity =            \
                    FindNxtCapacity_(cur_capacity);                           \
                ((Zeta_Core_GenericHashTable*)ght)->nxt_salt =                \
                    Zeta_Core_GetRandom();                                    \
            }                                                                 \
                                                                              \
            break;                                                            \
        }                                                                     \
                                                                              \
        if (0 < cur_table.size) {                                             \
            ((Zeta_Core_GenericHashTable*)ght)->cur_table_size =              \
                cur_table.size;                                               \
            ((Zeta_Core_GenericHashTable*)ght)->cur_table_root =              \
                cur_table.root;                                               \
                                                                              \
            ((Zeta_Core_GenericHashTable*)ght)->nxt_table_size =              \
                nxt_table.size;                                               \
            ((Zeta_Core_GenericHashTable*)ght)->nxt_table_root =              \
                nxt_table.root;                                               \
                                                                              \
            break;                                                            \
        }                                                                     \
                                                                              \
        if (nxt_table.size == 0) {                                            \
            ((Zeta_Core_GenericHashTable*)ght)->cur_salt =                    \
                Zeta_Core_GetRandom();                                        \
            ((Zeta_Core_GenericHashTable*)ght)->cur_table_size = 0;           \
            ((Zeta_Core_GenericHashTable*)ght)->cur_capacity = capacities[0]; \
            ((Zeta_Core_GenericHashTable*)ght)->cur_table_root = NULL;        \
        } else {                                                              \
            ((Zeta_Core_GenericHashTable*)ght)->cur_salt = ght->nxt_salt;     \
            ((Zeta_Core_GenericHashTable*)ght)->cur_table_size =              \
                nxt_table.size;                                               \
            ((Zeta_Core_GenericHashTable*)ght)->cur_capacity = nxt_capacity;  \
            ((Zeta_Core_GenericHashTable*)ght)->cur_table_root =              \
                nxt_table.root;                                               \
        }                                                                     \
                                                                              \
        ((Zeta_Core_GenericHashTable*)ght)->nxt_table_size = 0;               \
        ((Zeta_Core_GenericHashTable*)ght)->nxt_capacity = 0;                 \
        ((Zeta_Core_GenericHashTable*)ght)->nxt_table_root = NULL;            \
                                                                              \
        break;                                                                \
    }

bool_t Zeta_Core_GenericHashTable_Contain(void const* ght_, void const* node_) {
    Zeta_Core_GenericHashTable const* ght = ght_;
    CheckCntr_(ght);

    Zeta_Core_GenericHashTable_Node const* node = node_;

    if (node == NULL) { return FALSE; }

    size_t cur_capacity = ght->cur_capacity;
    size_t nxt_capacity = ght->nxt_capacity;

    Zeta_Core_MultiLevelTable cur_table;
    cur_table.level = Zeta_Core_CeilLog(cur_capacity, branch_num);
    cur_table.branch_nums = branch_nums;
    cur_table.size = ght->cur_table_size;
    cur_table.root = ght->cur_table_root;
    cur_table.node_allocator = ght->table_node_allocator;

    Zeta_Core_MultiLevelTable nxt_table;
    nxt_table.level = Zeta_Core_CeilLog(nxt_capacity, branch_num);
    nxt_table.branch_nums = branch_nums;
    nxt_table.size = ght->nxt_table_size;
    nxt_table.root = ght->nxt_table_root;
    nxt_table.node_allocator = ght->table_node_allocator;

    void* root =
        Zeta_Core_GetMostLink((void*)&node->n, Zeta_Core_OrdRBTreeNode_GetP);

    size_t idxes[ZETA_Core_MultiLevelTable_max_level];

    void** root_entry;

    bool_t ret;

    SetIdxes_(cur_table.level, idxes,
              GetBucketIdx_(
                  ght->NodeHash(ght->node_hash_context, node, ght->cur_salt),
                  cur_capacity));

    if (!(ret = ((root_entry = Zeta_Core_MultiLevelTable_Access(
                      &cur_table, idxes)) != NULL &&
                 *root_entry == root)) &&
        0 < nxt_capacity) {
        SetIdxes_(nxt_table.level, idxes,
                  GetBucketIdx_(ght->NodeHash(ght->node_hash_context, node,
                                              ght->nxt_salt),
                                nxt_capacity));

        ret = (root_entry = Zeta_Core_MultiLevelTable_Access(&nxt_table,
                                                             idxes)) != NULL &&
              *root_entry == root;
    }

    TryDo_(4);

    return ret;
}

void* Zeta_Core_GenericHashTable_Find(void const* ght_, void const* key,
                                      Zeta_Core_Hash KeyHash,
                                      void const* key_hash_context,
                                      Zeta_Core_Compare KeyNodeCompare,
                                      void const* key_node_cmp_context) {
    Zeta_Core_GenericHashTable const* ght = ght_;
    CheckCntr_(ght);

    size_t cur_capacity = ght->cur_capacity;
    size_t nxt_capacity = ght->nxt_capacity;

    Zeta_Core_MultiLevelTable cur_table;
    cur_table.level = Zeta_Core_CeilLog(cur_capacity, branch_num);
    cur_table.branch_nums = branch_nums;
    cur_table.size = ght->cur_table_size;
    cur_table.root = ght->cur_table_root;
    cur_table.node_allocator = ght->table_node_allocator;

    Zeta_Core_MultiLevelTable nxt_table;
    nxt_table.level = Zeta_Core_CeilLog(nxt_capacity, branch_num);
    nxt_table.branch_nums = branch_nums;
    nxt_table.size = ght->nxt_table_size;
    nxt_table.root = ght->nxt_table_root;
    nxt_table.node_allocator = ght->table_node_allocator;

    void* ret =
        nxt_capacity == 0
            ? NULL
            : Find_(ght->nxt_salt, &nxt_table, nxt_capacity, key, KeyHash,
                    key_hash_context, KeyNodeCompare, key_node_cmp_context);

    if (ret == NULL) {
        ret = Find_(ght->cur_salt, &cur_table, cur_capacity, key, KeyHash,
                    key_hash_context, KeyNodeCompare, key_node_cmp_context);
    }

    TryDo_(4);

    return ret;
}

void Zeta_Core_GenericHashTable_Insert(void* ght_, void* node_) {
    Zeta_Core_GenericHashTable* ght = ght_;
    CheckCntr_(ght);

    Zeta_Core_GenericHashTable_Node* node = node_;
    ZETA_Core_DebugAssert(node != NULL);

    size_t cur_capacity = ght->cur_capacity;
    size_t nxt_capacity = ght->nxt_capacity;

    Zeta_Core_MultiLevelTable cur_table;
    cur_table.level = Zeta_Core_CeilLog(cur_capacity, branch_num);
    cur_table.branch_nums = branch_nums;
    cur_table.size = ght->cur_table_size;
    cur_table.root = ght->cur_table_root;
    cur_table.node_allocator = ght->table_node_allocator;

    Zeta_Core_MultiLevelTable nxt_table;
    nxt_table.level = Zeta_Core_CeilLog(nxt_capacity, branch_num);
    nxt_table.branch_nums = branch_nums;
    nxt_table.size = ght->nxt_table_size;
    nxt_table.root = ght->nxt_table_root;
    nxt_table.node_allocator = ght->table_node_allocator;

    if (0 < nxt_capacity) {
        Insert_(ght, ght->nxt_salt, &nxt_table, nxt_capacity, node);
    } else {
        Insert_(ght, ght->cur_salt, &cur_table, cur_capacity, node);
    }

    ++ght->size;

    TryDo_(4);
}

void Zeta_Core_GenericHashTable_Extract(void* ght_, void* node_) {
    Zeta_Core_GenericHashTable* ght = ght_;
    CheckCntr_(ght);

    Zeta_Core_GenericHashTable_Node* node = node_;
    ZETA_Core_DebugAssert(node != NULL);

    size_t cur_capacity = ght->cur_capacity;
    size_t nxt_capacity = ght->nxt_capacity;

    Zeta_Core_MultiLevelTable cur_table;
    cur_table.level = Zeta_Core_CeilLog(cur_capacity, branch_num);
    cur_table.branch_nums = branch_nums;
    cur_table.size = ght->cur_table_size;
    cur_table.root = ght->cur_table_root;
    cur_table.node_allocator = ght->table_node_allocator;

    Zeta_Core_MultiLevelTable nxt_table;
    nxt_table.level = Zeta_Core_CeilLog(nxt_capacity, branch_num);
    nxt_table.branch_nums = branch_nums;
    nxt_table.size = ght->nxt_table_size;
    nxt_table.root = ght->nxt_table_root;
    nxt_table.node_allocator = ght->table_node_allocator;

    void* root = Zeta_Core_GetMostLink(&node->n, Zeta_Core_OrdRBTreeNode_GetP);

    if (!TryExtract_(ght, ght->cur_salt, &cur_table, cur_capacity, node,
                     root) &&
        !TryExtract_(ght, ght->nxt_salt, &nxt_table, nxt_capacity, node,
                     root)) {
        ZETA_Core_DebugAssert(FALSE);
    }

    --ght->size;

    TryDo_(4);
}

void* Zeta_Core_GenericHashTable_ExtractAny(void* ght_) {
    Zeta_Core_GenericHashTable* ght = ght_;
    CheckCntr_(ght);

    if (ght->size == 0) { return NULL; }

    size_t cur_capacity = ght->cur_capacity;
    size_t nxt_capacity = ght->nxt_capacity;

    Zeta_Core_MultiLevelTable cur_table;
    cur_table.level = Zeta_Core_CeilLog(cur_capacity, branch_num);
    cur_table.branch_nums = branch_nums;
    cur_table.size = ght->cur_table_size;
    cur_table.root = ght->cur_table_root;
    cur_table.node_allocator = ght->table_node_allocator;

    Zeta_Core_MultiLevelTable nxt_table;
    nxt_table.level = Zeta_Core_CeilLog(nxt_capacity, branch_num);
    nxt_table.branch_nums = branch_nums;
    nxt_table.size = ght->nxt_table_size;
    nxt_table.root = ght->nxt_table_root;
    nxt_table.node_allocator = ght->table_node_allocator;

    size_t idxes[ZETA_Core_MultiLevelTable_max_level];

    void** root_entry = Zeta_Core_MultiLevelTable_FindFirst(&cur_table, idxes);

    Zeta_Core_GenericHashTable_Node* node = ZETA_Core_MemberToStruct(
        Zeta_Core_GenericHashTable_Node, n, *root_entry);

    if ((*root_entry = Zeta_Core_RBTree(Zeta_Core_OrdRBTreeNode,
                                        Extract)(&node->n)) == NULL) {
        Zeta_Core_MultiLevelTable_Erase(&cur_table, idxes);
    }

    --ght->size;

    TryDo_(4);

    return node;
}

void Zeta_Core_GenericHashTable_ExtractAll(void* ght_) {
    Zeta_Core_GenericHashTable* ght = ght_;
    CheckCntr_(ght);

    size_t cur_capacity = ght->cur_capacity;
    size_t nxt_capacity = ght->nxt_capacity;

    Zeta_Core_MultiLevelTable cur_table;
    cur_table.level = Zeta_Core_CeilLog(cur_capacity, branch_num);
    cur_table.branch_nums = branch_nums;
    cur_table.size = ght->cur_table_size;
    cur_table.root = ght->cur_table_root;
    cur_table.node_allocator = ght->table_node_allocator;

    Zeta_Core_MultiLevelTable nxt_table;
    nxt_table.level = Zeta_Core_CeilLog(nxt_capacity, branch_num);
    nxt_table.branch_nums = branch_nums;
    nxt_table.size = ght->nxt_table_size;
    nxt_table.root = ght->nxt_table_root;
    nxt_table.node_allocator = ght->table_node_allocator;

    Zeta_Core_MultiLevelTable_Deinit(&cur_table);

    if (0 < nxt_capacity) { Zeta_Core_MultiLevelTable_Deinit(&nxt_table); }

    ght->cur_salt = Zeta_Core_GetRandom();

    ght->cur_table_root = NULL;
    ght->nxt_table_root = NULL;

    ght->cur_table_size = 0;
    ght->nxt_table_size = 0;

    ght->cur_capacity = capacities[0];

    ght->size = 0;
}

bool_t Zeta_Core_GenericHashTable_RunPending(void* ght_, size_t quata) {
    Zeta_Core_GenericHashTable* ght = ght_;
    CheckCntr_(ght);

    size_t cur_capacity = ght->cur_capacity;
    size_t nxt_capacity = ght->nxt_capacity;

    if (nxt_capacity == 0) { return FALSE; }

    Zeta_Core_MultiLevelTable cur_table;
    cur_table.level = Zeta_Core_CeilLog(cur_capacity, branch_num);
    cur_table.branch_nums = branch_nums;
    cur_table.size = ght->cur_table_size;
    cur_table.root = ght->cur_table_root;
    cur_table.node_allocator = ght->table_node_allocator;

    Zeta_Core_MultiLevelTable nxt_table;
    nxt_table.level = Zeta_Core_CeilLog(nxt_capacity, branch_num);
    nxt_table.branch_nums = branch_nums;
    nxt_table.size = ght->nxt_table_size;
    nxt_table.root = ght->nxt_table_root;
    nxt_table.node_allocator = ght->table_node_allocator;

    TryDo_(ZETA_Core_GetMaxOf(4ULL, quata));

    return 0 < nxt_capacity;
}

unsigned long long Zeta_Core_GenericHashTable_GetEffFactor(void const* ght_) {
    Zeta_Core_GenericHashTable const* ght = ght_;
    CheckCntr_(ght);

    if (ght->size == 0) { return 0; }

    size_t cur_capacity = ght->cur_capacity;
    size_t nxt_capacity = ght->nxt_capacity;

    Zeta_Core_MultiLevelTable cur_table;
    cur_table.level = Zeta_Core_CeilLog(cur_capacity, branch_num);
    cur_table.branch_nums = branch_nums;
    cur_table.size = ght->cur_table_size;
    cur_table.root = ght->cur_table_root;
    cur_table.node_allocator = ght->table_node_allocator;

    Zeta_Core_MultiLevelTable nxt_table;
    nxt_table.level = Zeta_Core_CeilLog(nxt_capacity, branch_num);
    nxt_table.branch_nums = branch_nums;
    nxt_table.size = ght->nxt_table_size;
    nxt_table.root = ght->nxt_table_root;
    nxt_table.node_allocator = ght->table_node_allocator;

    unsigned long long total_height = 0;

    size_t idxes[ZETA_Core_MultiLevelTable_max_level];

    if (0 < cur_capacity) {
        void** root_entry =
            Zeta_Core_MultiLevelTable_FindFirst(&cur_table, idxes);

        while (root_entry != NULL) {
            size_t tree_size =
                Zeta_Core_BinTree(Zeta_Core_OrdRBTreeNode, Count)(*root_entry);

            total_height += ZETA_CeilLog2(tree_size) * tree_size;

            root_entry =
                Zeta_Core_MultiLevelTable_FindNext(&cur_table, idxes, FALSE);
        }
    }

    if (0 < nxt_capacity) {
        void** root_entry =
            Zeta_Core_MultiLevelTable_FindFirst(&nxt_table, idxes);

        while (root_entry != NULL) {
            size_t tree_size =
                Zeta_Core_BinTree(Zeta_Core_OrdRBTreeNode, Count)(*root_entry);

            total_height += ZETA_CeilLog2(tree_size) * tree_size;

            root_entry =
                Zeta_Core_MultiLevelTable_FindNext(&nxt_table, idxes, FALSE);
        }
    }

    ZETA_Core_PrintVar(total_height);

    return total_height * ZETA_Core_FixedPoint_Base / ght->size;
}

void Zeta_Core_GenericHashTable_Check(void const* ght_) {
    Zeta_Core_GenericHashTable const* ght = ght_;
    ZETA_Core_DebugAssert(ght != NULL);

    size_t cur_capacity = ght->cur_capacity;

    ZETA_Core_DebugAssert(cur_capacity != 0);

    ZETA_Core_Allocator_Check(ght->table_node_allocator, alignof(void*));

    ZETA_Core_DebugAssert(ght->NodeHash != NULL);
    ZETA_Core_DebugAssert(ght->NodeCompare != NULL);
}

ZETA_Core_DeclareStruct(SanitizeTreeRet);

struct SanitizeTreeRet {
    size_t cnt;
    Zeta_Core_GenericHashTable_Node* most_l_n;
    Zeta_Core_GenericHashTable_Node* most_r_n;
};

static SanitizeTreeRet SanitizeTree_(Zeta_Core_GenericHashTable const* ght,
                                     Zeta_Core_MemRecorder* dst_node,
                                     unsigned long long salt, size_t capacity,
                                     size_t bucket_idx, void* n) {
    if (n == NULL) { return (SanitizeTreeRet){ 0, NULL, NULL }; }

    Zeta_Core_GenericHashTable_Node* node =
        ZETA_Core_MemberToStruct(Zeta_Core_GenericHashTable_Node, n, n);

    ZETA_Core_DebugAssert(
        GetBucketIdx_(ght->NodeHash(ght->node_hash_context, node, salt),
                      capacity) == bucket_idx);

    if (dst_node != NULL) {
        Zeta_Core_MemRecorder_Record(dst_node, node,
                                     sizeof(Zeta_Core_GenericHashTable_Node));
    }

    void* nl = Zeta_Core_OrdRBTreeNode_GetL(&node->n);
    void* nr = Zeta_Core_OrdRBTreeNode_GetR(&node->n);

    SanitizeTreeRet l_ret =
        SanitizeTree_(ght, dst_node, salt, capacity, bucket_idx, nl);

    SanitizeTreeRet r_ret =
        SanitizeTree_(ght, dst_node, salt, capacity, bucket_idx, nr);

    SanitizeTreeRet ret;
    ret.cnt = l_ret.cnt + 1 + r_ret.cnt;
    ret.most_l_n = n;
    ret.most_r_n = n;

    if (nl != NULL) {
        int cmp = ght->NodeCompare(
            ght->node_cmp_context,
            ZETA_Core_MemberToStruct(Zeta_Core_GenericHashTable_Node, n,
                                     l_ret.most_r_n),
            node);

        ZETA_Core_DebugAssert(cmp <= 0);

        ret.most_l_n = l_ret.most_l_n;
    }

    if (nr != NULL) {
        int cmp = ght->NodeCompare(
            ght->node_cmp_context, node,
            ZETA_Core_MemberToStruct(Zeta_Core_GenericHashTable_Node, n,
                                     r_ret.most_l_n));

        ZETA_Core_DebugAssert(cmp <= 0);

        ret.most_r_n = r_ret.most_r_n;
    }

    return ret;
}

void Zeta_Core_GenericHashTable_Sanitize(void const* ght_,
                                         Zeta_Core_MemRecorder* dst_table_node,
                                         Zeta_Core_MemRecorder* dst_node) {
    Zeta_Core_GenericHashTable const* ght = ght_;
    CheckCntr_(ght);

    size_t cur_capacity = ght->cur_capacity;
    size_t nxt_capacity = ght->nxt_capacity;

    Zeta_Core_MultiLevelTable cur_table;
    cur_table.level = Zeta_Core_CeilLog(cur_capacity, branch_num);
    cur_table.branch_nums = branch_nums;
    cur_table.size = ght->cur_table_size;
    cur_table.root = ght->cur_table_root;
    cur_table.node_allocator = ght->table_node_allocator;

    Zeta_Core_MultiLevelTable nxt_table;
    nxt_table.level = Zeta_Core_CeilLog(nxt_capacity, branch_num);
    nxt_table.branch_nums = branch_nums;
    nxt_table.size = ght->nxt_table_size;
    nxt_table.root = ght->nxt_table_root;
    nxt_table.node_allocator = ght->table_node_allocator;

    if (0 < cur_capacity) {
        Zeta_Core_MultiLevelTable_Sanitize(&cur_table, dst_table_node);
    }

    if (0 < nxt_capacity) {
        Zeta_Core_MultiLevelTable_Sanitize(&nxt_table, dst_table_node);
    }

    size_t total_size = 0;

    size_t idxes[ZETA_Core_MultiLevelTable_max_level];

    if (0 < cur_capacity) {
        void** root_entry =
            Zeta_Core_MultiLevelTable_FindFirst(&cur_table, idxes);

        while (root_entry != NULL) {
            total_size +=
                Zeta_Core_BinTree(Zeta_Core_OrdRBTreeNode, Count)(*root_entry);

            SanitizeTree_(ght, dst_node, ght->cur_salt, cur_capacity,
                          GetIdx_(cur_table.level, idxes), *root_entry);

            root_entry =
                Zeta_Core_MultiLevelTable_FindNext(&cur_table, idxes, FALSE);
        }
    }

    if (0 < nxt_capacity) {
        void** root_entry =
            Zeta_Core_MultiLevelTable_FindFirst(&nxt_table, idxes);

        while (root_entry != NULL) {
            total_size +=
                Zeta_Core_BinTree(Zeta_Core_OrdRBTreeNode, Count)(*root_entry);

            SanitizeTree_(ght, dst_node, ght->nxt_salt, nxt_capacity,
                          GetIdx_(nxt_table.level, idxes), *root_entry);

            root_entry =
                Zeta_Core_MultiLevelTable_FindNext(&nxt_table, idxes, FALSE);
        }
    }

    ZETA_Core_DebugAssert(ght->size == total_size);
}

void Zeta_Core_GenericHashTable_Node_Init(void* node_) {
    Zeta_Core_GenericHashTable_Node* node = node_;
    ZETA_Core_DebugAssert(node != NULL);

    Zeta_Core_OrdRBTreeNode_Init(&node->n);
}
