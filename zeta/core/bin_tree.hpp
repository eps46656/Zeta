#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core::bin_tree {

template <typename BinTreeNode, typename = void>
struct Traits;

namespace ops {

template <typename BinTreeNode>
constexpr bool IsConst();

template <typename BinTreeNode>
constexpr bool HasAccSize();

template <typename BinTreeNode>
BinTreeNode* GetP(BinTreeNode* n);

template <typename BinTreeNode>
BinTreeNode* GetL(BinTreeNode* n);

template <typename BinTreeNode>
BinTreeNode* GetR(BinTreeNode* n);

template <typename BinTreeNode>
void SetP(BinTreeNode* n, BinTreeNode* m);

template <typename BinTreeNode>
void SetL(BinTreeNode* n, BinTreeNode* m);

template <typename BinTreeNode>
void SetR(BinTreeNode* n, BinTreeNode* m);

template <typename BinTreeNode>
constexpr size_t GetNullAccSize();

template <typename BinTreeNode>
size_t GetAccSize(BinTreeNode* n);

template <typename BinTreeNode>
void SetAccSize(BinTreeNode* n, size_t acc_size);

template <typename BinTreeNode>
void CheckContract();

template <typename BinTreeNode>
utils::Pair<BinTreeNode*, size_t> GetMostP(BinTreeNode* n);

template <typename BinTreeNode>
utils::Pair<BinTreeNode*, size_t> GetMostL(BinTreeNode* n);

template <typename BinTreeNode>
utils::Pair<BinTreeNode*, size_t> GetMostR(BinTreeNode* n);

template <typename BinTreeNode>
size_t Count(BinTreeNode* n);

template <typename BinTreeNode>
void AddDiffSize(BinTreeNode* n, size_t diff_size);

template <typename BinTreeNode>
size_t GetSize(BinTreeNode* n);

template <typename BinTreeNode>
void SetSize(BinTreeNode* n, size_t size);

template <typename BinTreeNode>
void AttatchL(BinTreeNode* pos, BinTreeNode* n);

template <typename BinTreeNode>
void AttatchR(BinTreeNode* pos, BinTreeNode* n);

template <typename BinTreeNode>
void Detach(BinTreeNode* n);

template <typename BinTreeNode>
void Swap(BinTreeNode* n, BinTreeNode* m);

template <typename BinTreeNode>
void RotateL(BinTreeNode* n);

template <typename BinTreeNode>
void RotateR(BinTreeNode* n);

template <typename BinTreeNode>
utils::Pair<BinTreeNode*, size_t> AccessL(BinTreeNode* n, size_t idx);

template <typename BinTreeNode>
utils::Pair<BinTreeNode*, size_t> AccessR(BinTreeNode* n, size_t idx);

template <typename BinTreeNode>
BinTreeNode* StepPL(BinTreeNode* n);

template <typename BinTreeNode>
BinTreeNode* StepPR(BinTreeNode* n);

template <typename BinTreeNode>
BinTreeNode* StepL(BinTreeNode* n);

template <typename BinTreeNode>
BinTreeNode* StepR(BinTreeNode* n);

template <typename BinTreeNode>
utils::Pair<BinTreeNode*, size_t> AdvanceL(BinTreeNode* n, size_t step);

template <typename BinTreeNode>
utils::Pair<BinTreeNode*, size_t> AdvanceR(BinTreeNode* n, size_t step);

template <typename BinTreeNode>
utils::Pair<size_t, size_t> GetLRAccSize(BinTreeNode* n);

template <typename BinTreeNode>
void Sanitize(BinTreeNode* root);

}  // namespace ops

}  // namespace zeta::core::bin_tree
