#pragma once

#include <zeta/core/define.hpp>

namespace zeta::core::llist {

template <typename LListNode, typename = void>
struct NodeTraits;  // IWYU pragma: export

template <typename LListNode>
constexpr bool IsConst();

template <typename LListNode>
constexpr LListNode* GetL(LListNode* n);

template <typename LListNode>
constexpr LListNode* GetR(LListNode* n);

template <typename LListNode>
void SetL(LListNode* n, LListNode* m);

template <typename LListNode>
void SetR(LListNode* n, LListNode* m);

template <typename LListNode>
void CheckContract();

template <typename LListNode>
size_t Count(LListNode* n);

template <typename LListNode>
void InsertL(LListNode* n, LListNode* m);

template <typename LListNode>
void InsertR(LListNode* n, LListNode* m);

template <typename LListNode>
void Extract(LListNode* n);

template <typename LListNode>
void InsertSegL(LListNode* n, LListNode* m_beg, LListNode* m_end);

template <typename LListNode>
void InsertSegR(LListNode* n, LListNode* m_beg, LListNode* m_end);

template <typename LListNode>
void ExtractSeg(LListNode* n_beg, LListNode* n_end);

}  // namespace zeta::core::llist
