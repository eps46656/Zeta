#include <zeta/core/cascade_allocator.h>
#include <zeta/core/debugger.h>
#include <zeta/core/llist_node.h>
#include <zeta/core/utils.h>

#if ZETA_Core_EnableDebug

#define Check_(ca) Zeta_CascadeAllocator_Check(ca)

#else

#define Check_(ca) ZETA_Core_Unused((ca))

#endif

void Zeta_CascadeAllocator_Init(void* ca_) {
    Zeta_CascadeAllocator* ca = ca_;
    Check_(ca);

    size_t align = ca->align;
    ZETA_Core_DebugAssert(0 < align);

    align = Zeta_Core_GCD(alignof(Zeta_Core_OrdLListNode), align);

    void* mem_beg = __builtin_align_up(ca->mem, align);
    void* mem_end = __builtin_align_down(ca->mem + ca->size, align);

    ZETA_Core_DebugAssert(mem_beg <= mem_end);

    size_t size = mem_end - mem_beg;

    ZETA_Core_DebugAssert(sizeof(void*) * 1024 <= size);

    size_t node_size =
        ZETA_Core_IntRoundUp(sizeof(Zeta_Core_OrdLListNode), align);

    void* first_node = mem_end - node_size;

    Zeta_Core_OrdLListNode_Init(first_node);

    ca->align = align;

    ca->mem = mem_beg;
    ca->size = size;

    ca->first_node = first_node;
}

size_t Zeta_CascadeAllocator_GetAlign(void const* ca_) {
    Zeta_CascadeAllocator const* ca = ca_;
    Check_(ca);

    return ca->align;
}

size_t Zeta_CascadeAllocator_Query(void const* ca_, size_t size) {
    Zeta_CascadeAllocator const* ca = ca_;
    Check_(ca);

    size_t align = ca->align;

    return ZETA_Core_IntRoundUp(size, align);
}

void* Zeta_CascadeAllocator_Allocate(void* ca_, size_t size) {
    Zeta_CascadeAllocator* ca = ca_;
    Check_(ca);

    if (size == 0) { return NULL; }

    size_t align = ca->align;

    void* mem_beg = ca->mem;

    size_t node_size =
        ZETA_Core_IntRoundUp(sizeof(Zeta_Core_OrdLListNode), align);

    size = ZETA_Core_IntRoundUp(size, align);

    void* first_node = ca->first_node;

    if ((size_t)((void*)first_node - mem_beg) < node_size + size) {
        return NULL;
    }

    void* new_first_node = first_node - (node_size + size);

    Zeta_Core_OrdLListNode_Init(new_first_node);

    Zeta_Core_LList(Zeta_Core_OrdLListNode, InsertL)(first_node,
                                                     new_first_node);

    ca->first_node = new_first_node;

    return (void*)new_first_node + node_size;
}

void Zeta_CascadeAllocator_Deallocate(void* ca_, void* ptr) {
    Zeta_CascadeAllocator* ca = ca_;
    Check_(ca);

    if (ptr == NULL) { return; }

    size_t align = ca->align;

    size_t node_size =
        ZETA_Core_IntRoundUp(sizeof(Zeta_Core_OrdLListNode), align);

    void* node = ptr - node_size;

    if (ca->first_node == node) {
        ca->first_node = Zeta_Core_OrdLListNode_GetR(ca->first_node);
    }

    Zeta_Core_LList(Zeta_Core_OrdLListNode, Extract)(ptr - node_size);
}

void Zeta_CascadeAllocator_Check(void const* ca_) {
    Zeta_CascadeAllocator const* ca = ca_;
    ZETA_Core_DebugAssert(ca != NULL);

    size_t align = ca->align;

    ZETA_Core_DebugAssert(0 < align);
    ZETA_Core_DebugAssert(align % alignof(Zeta_Core_OrdLListNode) == 0);

    void* mem_beg = ca->mem;
    void* mem_end = mem_beg + ca->size;

    ZETA_Core_DebugAssert(__builtin_is_aligned(mem_beg, align));
    ZETA_Core_DebugAssert(__builtin_is_aligned(mem_end, align));
}

void Zeta_CascadeAllocator_Sanitize(void const* ca_,
                                    Zeta_Core_MemRecorder* dst) {
    Zeta_CascadeAllocator const* ca = ca_;
    Check_(ca);

    size_t align = ca->align;

    size_t node_size =
        ZETA_Core_IntRoundUp(sizeof(Zeta_Core_OrdLListNode), align);

    void* first_node = ca->first_node;

    for (void* node = first_node;;) {
        void* nxt_node = Zeta_Core_OrdLListNode_GetR(node);
        ZETA_Core_DebugAssert(Zeta_Core_OrdLListNode_GetL(nxt_node) == node);

        if (nxt_node == first_node) { break; }

        ZETA_Core_DebugAssert(node < nxt_node);
        ZETA_Core_DebugAssert(node_size <= (size_t)(nxt_node - node));

        if (dst != NULL) {
            Zeta_Core_MemRecorder_Record(dst, node + node_size,
                                         nxt_node - node - node_size);
        }

        node = nxt_node;
    }

    ZETA_Core_DebugAssert(ca->mem + ca->size - node_size ==
                          Zeta_Core_OrdLListNode_GetL(first_node));
}

Zeta_Core_Allocator_VTable const Zeta_Core_CascadeAllocator_allocator_vtable = {
    .GetAlign = Zeta_CascadeAllocator_GetAlign,

    .Query = Zeta_CascadeAllocator_Query,

    .Allocate = Zeta_CascadeAllocator_Allocate,

    .Deallocate = Zeta_CascadeAllocator_Deallocate,
};
