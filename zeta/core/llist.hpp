#pragma once

#include <zeta/core/define.hpp>

namespace zeta::core::llist {

template <typename LListNodeOperator, typename LListNode>
void CheckContract(LListNodeOperator const& lln_opr);

// -----------------------------------------------------------------------------

template <typename LListNodeOperator, typename LListNode>
size_t Count(LListNodeOperator const& lln_opr, LListNode* n);

template <typename LListNodeOperator, typename LListNode>
void InsertL(LListNodeOperator const& lln_opr, LListNode* n, LListNode* m);

template <typename LListNodeOperator, typename LListNode>
void InsertR(LListNodeOperator const& lln_opr, LListNode* n, LListNode* m);

template <typename LListNodeOperator, typename LListNode>
void Extract(LListNodeOperator const& lln_opr, LListNode* n);

template <typename LListNodeOperator, typename LListNode>
void InsertSegL(LListNodeOperator const& lln_opr, LListNode* n,
                LListNode* m_beg, LListNode* m_end);

template <typename LListNodeOperator, typename LListNode>
void InsertSegR(LListNodeOperator const& lln_opr, LListNode* n,
                LListNode* m_beg, LListNode* m_end);

template <typename LListNodeOperator, typename LListNode>
void ExtractSeg(LListNodeOperator const& lln_opr, LListNode* n_beg,
                LListNode* n_end);

}  // namespace zeta::core::llist
