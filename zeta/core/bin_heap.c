#include <zeta/core/bin_heap.h>
#include <zeta/core/debugger.h>
#include <zeta/core/utils.h>

static void* DownAdjust_(void* data, size_t width, size_t stride, size_t idx,
                         size_t size, void* src_elem, void* cmp_context,
                         int (*Compare)(void* cmp_context, void const* x,
                                        void const* y)) {
    ZETA_Core_DebugAssert(data != NULL);

    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= stride);

    ZETA_Core_DebugAssert(idx < size);

    ZETA_Core_DebugAssert(Compare != NULL);

    if (size == 1) {
        if (src_elem != NULL) { Zeta_Core_MemCopy(data, src_elem, width); }

        return data;
    }

    for (size_t n_i = idx;;) {
        void* n_ref = data + stride * n_i;

        size_t l_i = n_i * 2 + 1;

        if (size <= l_i) {
            if (src_elem != NULL) { Zeta_Core_MemCopy(n_ref, src_elem, width); }
            return n_ref;
        }

        void* l_ref = data + stride * l_i;

        void* min = src_elem == NULL ? n_ref : src_elem;

        if (Compare(cmp_context, l_ref, min) < 0) { min = l_ref; }

        size_t r_i = n_i * 2 + 2;

        if (r_i < size) {
            void* r_ref = data + stride * r_i;

            if (Compare(cmp_context, r_ref, min) < 0) { min = r_ref; }
        }

        if (src_elem == NULL) {
            if (min == n_ref) { return n_ref; }

            src_elem = __builtin_alloca_with_align(
                width, __CHAR_BIT__ * alignof(max_align_t));

            Zeta_Core_MemCopy(src_elem, n_ref, width);
        }

        Zeta_Core_MemCopy(n_ref, min, width);

        if (min == src_elem) { return n_ref; }

        n_i = min == l_ref ? l_i : r_i;
    }
}

void Zeta_Core_BinHeap_Construct(void* data, size_t width, size_t stride,
                                 size_t size, void* cmp_context,
                                 int (*Compare)(void* cmp_context,
                                                void const* x, void const* y)) {
    ZETA_Core_DebugAssert(data != NULL);

    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= stride);

    ZETA_Core_DebugAssert(0 < stride);

    ZETA_Core_DebugAssert(Compare != NULL);

    for (size_t i = size; 0 < i--;) {
        DownAdjust_(data, width, stride, i, size, NULL, cmp_context, Compare);
    }
}

void* Zeta_Core_BinHeap_Push(void* data, size_t width, size_t stride,
                             size_t size, void const* src_elem,
                             void* cmp_context,
                             int (*Compare)(void* cmp_context, void const* x,
                                            void const* y)) {
    ZETA_Core_DebugAssert(data != NULL);

    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= stride);

    ZETA_Core_DebugAssert(0 < stride);

    ZETA_Core_DebugAssert(Compare != NULL);

    for (size_t n_i = size;;) {
        if (n_i == 0) {
            if (src_elem != NULL) { Zeta_Core_MemCopy(data, src_elem, width); }
            return data;
        }

        size_t p_i = (n_i - 1) / 2;

        void* n_ref = data + stride * n_i;

        void* p_ref = data + stride * p_i;

        void const* cur = src_elem == NULL ? n_ref : src_elem;

        if (Compare(cmp_context, p_ref, cur) <= 0) {
            Zeta_Core_MemCopy(n_ref, cur, width);
            return n_ref;
        }

        if (src_elem == NULL) {
            src_elem = __builtin_alloca_with_align(
                width, __CHAR_BIT__ * alignof(max_align_t));

            Zeta_Core_MemCopy((void*)src_elem, n_ref, width);
        }

        Zeta_Core_MemCopy(n_ref, p_ref, width);

        n_i = p_i;
    }
}

void Zeta_Core_BinHeap_Pop(void* data, size_t width, size_t stride, size_t size,
                           void* dst_elem, void* cmp_context,
                           int (*Compare)(void* cmp_context, void const* x,
                                          void const* y)) {
    ZETA_Core_DebugAssert(data != NULL);

    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= stride);

    ZETA_Core_DebugAssert(Compare != NULL);

    if (size <= 1) {
        if (size == 1 && dst_elem != NULL) {
            Zeta_Core_MemCopy(dst_elem, data, width);
        }

        return;
    }

    DownAdjust_(data, width, stride, 0, size - 1, data + stride * (size - 1),
                cmp_context, Compare);
}

void* Zeta_Core_BinHeap_Adjust(void* data, size_t width, size_t stride,
                               size_t idx, size_t size, void* cmp_context,
                               int (*Compare)(void* cmp_context, void const* x,
                                              void const* y)) {
    ZETA_Core_DebugAssert(data != NULL);

    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= stride);

    ZETA_Core_DebugAssert(idx < size);

    ZETA_Core_DebugAssert(Compare != NULL);

    if (size == 1) { return data; }

    void* p =
        DownAdjust_(data, width, stride, idx, size, NULL, cmp_context, Compare);

    return p == data + stride * idx
               ? Zeta_Core_BinHeap_Push(data, width, stride, idx + 1, NULL,
                                        cmp_context, Compare)
               : p;
}
