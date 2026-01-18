#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core::bin_tree {

template <typename BinTreeNodeOperator, typename BinTreeNode>
void CheckBinTreeNode(BinTreeNodeOperator const& btn_opr);

// -----------------------------------------------------------------------------

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<BinTreeNode*, size_t> GetMostP(BinTreeNodeOperator const& btn_opr,
                                    BinTreeNode* n);

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<BinTreeNode*, size_t> GetMostL(BinTreeNodeOperator const& btn_opr,
                                    BinTreeNode* n);

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<BinTreeNode*, size_t> GetMostR(BinTreeNodeOperator const& btn_opr,
                                    BinTreeNode* n);

// -----------------------------------------------------------------------------

template <typename BinTreeNodeOperator, typename BinTreeNode>
size_t Count(BinTreeNodeOperator const& btn_opr, BinTreeNode* n);

template <typename BinTreeNodeOperator, typename BinTreeNode>
void AddDiffSize(BinTreeNodeOperator const& btn_opr, BinTreeNode* n,
                 size_t diff_size);

template <typename BinTreeNodeOperator, typename BinTreeNode>
size_t GetSize(BinTreeNodeOperator const& btn_opr, BinTreeNode* n);

template <typename BinTreeNodeOperator, typename BinTreeNode>
void SetSize(BinTreeNodeOperator const& btn_opr, BinTreeNode* n, size_t size);

// -----------------------------------------------------------------------------

template <typename BinTreeNodeOperator, typename BinTreeNode>
void AttatchL(BinTreeNodeOperator const& btn_opr, BinTreeNode* pos,
              BinTreeNode* n);

template <typename BinTreeNodeOperator, typename BinTreeNode>
void AttatchR(BinTreeNodeOperator const& btn_opr, BinTreeNode* pos,
              BinTreeNode* n);

template <typename BinTreeNodeOperator, typename BinTreeNode>
void Detach(BinTreeNodeOperator const& btn_opr, BinTreeNode* n);

template <typename BinTreeNodeOperator, typename BinTreeNode>
void Swap(BinTreeNodeOperator const& btn_opr, BinTreeNode* n, BinTreeNode* m);

// -----------------------------------------------------------------------------

template <typename BinTreeNodeOperator, typename BinTreeNode>
void RotateL(BinTreeNodeOperator const& btn_opr, BinTreeNode* n);

template <typename BinTreeNodeOperator, typename BinTreeNode>
void RotateR(BinTreeNodeOperator const& btn_opr, BinTreeNode* n);

// -----------------------------------------------------------------------------

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<BinTreeNode*, size_t> AccessL(BinTreeNodeOperator const& btn_opr,
                                   BinTreeNode* n, size_t idx);

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<BinTreeNode*, size_t> AccessR(BinTreeNodeOperator const& btn_opr,
                                   BinTreeNode* n, size_t idx);

template <typename BinTreeNodeOperator, typename BinTreeNode>
BinTreeNode* StepPL(BinTreeNodeOperator const& btn_opr, BinTreeNode* n);

template <typename BinTreeNodeOperator, typename BinTreeNode>
BinTreeNode* StepPR(BinTreeNodeOperator const& btn_opr, BinTreeNode* n);

template <typename BinTreeNodeOperator, typename BinTreeNode>
BinTreeNode* StepL(BinTreeNodeOperator const& btn_opr, BinTreeNode* n);

template <typename BinTreeNodeOperator, typename BinTreeNode>
BinTreeNode* StepR(BinTreeNodeOperator const& btn_opr, BinTreeNode* n);

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<BinTreeNode*, size_t> AdvanceL(BinTreeNodeOperator const& btn_opr,
                                    BinTreeNode* n, size_t step);

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<BinTreeNode*, size_t> AdvanceR(BinTreeNodeOperator const& btn_opr,
                                    BinTreeNode* n, size_t step);

template <typename BinTreeNodeOperator, typename BinTreeNode>
Pair<size_t, size_t> GetLRAccSize(BinTreeNodeOperator const& btn_opr,
                                  BinTreeNode* n);

// -------------------------------------------------------------------------

template <typename BinTreeNodeOperator, typename BinTreeNode>
void Sanitize(BinTreeNodeOperator const& btn_opr, BinTreeNode* n);

}  // namespace zeta::core::bin_tree
