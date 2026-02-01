#pragma once

#include <zeta/core/bin_tree.hpp>
#include <zeta/core/mem_check_utils.hpp>

namespace zeta::core::rbtree {

constexpr unsigned black{ 1 };
constexpr unsigned red{ 2 };

// -----------------------------------------------------------------------------

template <typename RBTreeNode>
void CheckContract();

// -----------------------------------------------------------------------------

template <typename RBTreeNode>
RBTreeNode* InsertL(RBTreeNode* pos, RBTreeNode* n);

template <typename RBTreeNode>
RBTreeNode* InsertR(RBTreeNode* pos, RBTreeNode* n);

template <typename RBTreeNode>
RBTreeNode* Insert(RBTreeNode* pos_l, RBTreeNode* pos_r, RBTreeNode* n);

template <typename RBTreeNode>
RBTreeNode* GeneralInsertL(RBTreeNode* root, RBTreeNode* pos, RBTreeNode* n);

template <typename RBTreeNode>
RBTreeNode* GeneralInsertR(RBTreeNode* root, RBTreeNode* pos, RBTreeNode* n);

template <typename RBTreeNode>
RBTreeNode* Extract(RBTreeNode* pos);

template <typename RBTreeNode>
void Sanitize(MemRecorder* dst_mr, RBTreeNode* root);

}  // namespace zeta::core::rbtree
