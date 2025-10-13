#include <zeta/core/cascade_allocator.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/llist.ipp>
#include <zeta/core/llist_node_tpl.ipp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

void CascadeAllocator::Init(void* ca_) {
    auto ca{ static_cast<CascadeAllocator*>(ca_) };
    ZETA_Core_DebugAssert(CheckAllocator(ca));

    size_t align{ ca->align };
    ZETA_Core_DebugAssert(0 < align);

    align = LCM(alignof(LListNode), align);

    char* mem{ static_cast<char*>(ca->mem) };

    char* mem_beg{ static_cast<char*>(
        __builtin_align_up(mem + sizeof(LListNode), align)) };
    char* mem_end{ static_cast<char*>(mem + ca->size) };

    ZETA_Core_DebugAssert(mem_beg <= mem_end);

    size_t size{ static_cast<size_t>(mem_end - mem_beg) };

    ZETA_Core_DebugAssert(sizeof(void*) * 1024 <= size);

    LListNode* last_node{ reinterpret_cast<LListNode*>(mem_beg) - 1 };

    last_node->Init();

    ca->align = align;

    ca->mem = mem_beg;
    ca->size = size;

    ca->last_node = last_node;
}

size_t CascadeAllocator::GetAlign(void const* ca_) {
    auto ca{ static_cast<CascadeAllocator const*>(ca_) };
    ZETA_Core_DebugAssert(CheckAllocator(ca));

    return ca->align;
}

size_t CascadeAllocator::Query(void const* ca_, size_t size) {
    auto ca{ static_cast<CascadeAllocator const*>(ca_) };
    ZETA_Core_DebugAssert(CheckAllocator(ca));

    size_t align{ ca->align };

    return UIntAlignUp(size, align);
}

void* CascadeAllocator::Allocate(void* ca_, size_t size) {
    auto ca{ static_cast<CascadeAllocator*>(ca_) };
    ZETA_Core_DebugAssert(CheckAllocator(ca));

    if (size == 0) { return nullptr; }

    LListNode* last_node{ ca->last_node };

    size = UIntAlignUp(size + sizeof(LListNode), ca->align);

    if (static_cast<size_t>(static_cast<char*>(ca->mem) + ca->size -
                            reinterpret_cast<char*>(last_node)) -
            sizeof(LListNode) <
        size) {
        return nullptr;
    }

    LListNode* new_last_node{ reinterpret_cast<LListNode*>(
        reinterpret_cast<char*>(last_node) + size) };

    new_last_node->Init();

    llist::InsertR(LListNodeTplOperator{}, last_node, new_last_node);

    ca->last_node = new_last_node;

    return last_node + 1;
}

void CascadeAllocator::Deallocate(void* ca_, void* ptr) {
    auto ca{ static_cast<CascadeAllocator*>(ca_) };
    ZETA_Core_DebugAssert(CheckAllocator(ca));

    if (ptr == nullptr) { return; }

    LListNode* node{ reinterpret_cast<LListNode*>(ptr) - 1 };

    LListNode* last_node{ ca->last_node };

    if (node->GetRPtr() == last_node) {
        llist::Extract(LListNodeTplOperator{}, last_node);
        ca->last_node = last_node = node;
    } else {
        llist::Extract(LListNodeTplOperator{}, node);
    }

    if (last_node->GetLPtr() != last_node) { return; }

    LListNode* init_last_node{ reinterpret_cast<LListNode*>(ca->mem) - 1 };

    if (init_last_node != last_node) {
        init_last_node->Init();
        ca->last_node = init_last_node;
    }
}

bool CascadeAllocator::CheckAllocator(void const* ca_) {
    auto ca{ static_cast<CascadeAllocator const*>(ca_) };
    if (!(ca != nullptr)) { return false; }

    size_t align{ ca->align };

    if (!(0 < align)) { return false; }
    if (!(align % alignof(LListNode) == 0)) { return false; }

    char* mem_beg{ static_cast<char*>(ca->mem) };

    if (!__builtin_is_aligned(mem_beg, align)) { return false; }

    return true;
}

allocator::AllocatorRef CascadeAllocator::GetAlloccatorRef(void* ca_) {
    auto ca{ reinterpret_cast<CascadeAllocator*>(ca_) };
    ZETA_Core_DebugAssert(CheckAllocator(ca));

    return {
        .inst = ca,
        .align = static_cast<unsigned short>(ca->align),
        .vtable = &allocator::AllocatorVTable::Make<CascadeAllocator>(),
    };
}

allocator::ConstAllocatorRef CascadeAllocator::GetAlloccatorRef(
    void const* ca_) {
    auto ca{ reinterpret_cast<CascadeAllocator const*>(ca_) };
    ZETA_Core_DebugAssert(CheckAllocator(ca));

    return {
        .inst = ca,
        .align = static_cast<unsigned short>(ca->align),
        .vtable = &allocator::AllocatorVTable::Make<CascadeAllocator>(),
    };
}

void CascadeAllocator::Sanitize(void const* ca_, MemRecorder* dst) {
    auto ca{ reinterpret_cast<CascadeAllocator const*>(ca_) };
    ZETA_Core_DebugAssert(CheckAllocator(ca));

    char* mem_beg{ static_cast<char*>(ca->mem) };
    char* mem_end{ static_cast<char*>(ca->mem) + ca->size };

    size_t align{ ca->align };

    LListNode* last_node{ ca->last_node };

    ZETA_Core_DebugAssert(
        mem_beg - sizeof(LListNode) * 1 <= reinterpret_cast<char*>(last_node) &&
        reinterpret_cast<char*>(last_node) + sizeof(LListNode) <= mem_end);

    if (last_node->GetRPtr() == last_node) {
        ZETA_Core_DebugAssert(reinterpret_cast<LListNode*>(
                                  mem_beg - sizeof(LListNode)) == last_node);
    }

    for (LListNode* node{ last_node };;) {
        LListNode* prv_node{ node->GetLPtr() };
        ZETA_Core_DebugAssert(prv_node->GetRPtr() == node);

        if (prv_node == last_node) { break; }

        ZETA_Core_DebugAssert(
            __builtin_is_aligned(reinterpret_cast<char*>(node), align));

        ZETA_Core_DebugAssert(
            mem_beg - sizeof(LListNode) * 1 <=
                reinterpret_cast<char*>(prv_node) &&
            reinterpret_cast<char*>(prv_node) + sizeof(LListNode) <= mem_end);

        auto del{ reinterpret_cast<char*>(node) -
                  reinterpret_cast<char*>(prv_node) };

        ZETA_Core_DebugAssert(static_cast<long long>(sizeof(LListNode)) < del);

        if (dst != nullptr) {
            MemRecorder::Record(dst, prv_node + 1, static_cast<size_t>(del));
        }

        node = prv_node;
    }
}

}  // namespace zeta::core
