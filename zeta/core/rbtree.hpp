#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_recorder.hpp>

namespace zeta::core::rbtree {

template <typename RBTreeNode, typename = void>
struct NodeTraits;  // IWYU pragma: export

constexpr unsigned black{ 1 };
constexpr unsigned red{ 2 };

constexpr size_t max_height{ integral::WidthOf<size_t> * 2 };

template <typename BinTreeNode>
unsigned GetColor(BinTreeNode* n);

template <typename BinTreeNode>
void SetColor(BinTreeNode* n, unsigned color);

template <typename RBTreeNode>
void CheckContract();

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
void Sanitize(mem_recorder::MemRecorder* dst_mr, RBTreeNode* root);

}  // namespace zeta::core::rbtree
