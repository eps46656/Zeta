#pragma once

#include <zeta/core/bin_tree.hpp>
#include <zeta/core/mem_check_utils.hpp>

namespace zeta::core::rbtree {

constexpr unsigned black{ 1 };
constexpr unsigned red{ 2 };

// -----------------------------------------------------------------------------

template <typename RBTreeNodeOperator, typename RBTreeNode>
void CheckContract(RBTreeNodeOperator const& rbtn_opr);

// -----------------------------------------------------------------------------

template <typename RBTreeNodeOperator, typename RBTreeNode>
RBTreeNode* InsertL(RBTreeNodeOperator const& rbtn_opr, RBTreeNode* pos,
                    RBTreeNode* n);

template <typename RBTreeNodeOperator, typename RBTreeNode>
RBTreeNode* InsertR(RBTreeNodeOperator const& rbtn_opr, RBTreeNode* pos,
                    RBTreeNode* n);

template <typename RBTreeNodeOperator, typename RBTreeNode>
RBTreeNode* Insert(RBTreeNodeOperator const& rbtn_opr, RBTreeNode* pos_l,
                   RBTreeNode* pos_r, RBTreeNode* n);

template <typename RBTreeNodeOperator, typename RBTreeNode>
RBTreeNode* GeneralInsertL(RBTreeNodeOperator const& rbtn_opr, RBTreeNode* root,
                           RBTreeNode* pos, RBTreeNode* n);

template <typename RBTreeNodeOperator, typename RBTreeNode>
RBTreeNode* GeneralInsertR(RBTreeNodeOperator const& rbtn_opr, RBTreeNode* root,
                           RBTreeNode* pos, RBTreeNode* n);

template <typename RBTreeNodeOperator, typename RBTreeNode>
RBTreeNode* Extract(RBTreeNodeOperator const& rbtn_opr, RBTreeNode* pos);

template <typename RBTreeNodeOperator, typename RBTreeNode>
void Sanitize(RBTreeNodeOperator const& rbtn_opr, MemRecorder* dst_mr,
              RBTreeNode* root);

}  // namespace zeta::core::rbtree
