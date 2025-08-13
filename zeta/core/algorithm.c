#include <zeta/core/algorithm.h>
#include <zeta/core/allocator.h>
#include <zeta/core/debugger.h>
#include <zeta/core/memory.h>

#define InsertionThreshold (8)

static void Insertion_(void* data, size_t width, size_t stride,
                       void* cmp_context, Zeta_Core_Compare Compare) {
    void* pre_data = data - stride;

    if (Compare(cmp_context, pre_data, data) <= 0) { return; }

    void* tmp =
        __builtin_alloca_with_align(width, __CHAR_BIT__ * alignof(max_align_t));

    Zeta_Core_MemCopy(tmp, data, width);

    do {
        Zeta_Core_MemCopy(data, pre_data, width);

        data = pre_data;
        pre_data -= stride;
    } while (Compare(cmp_context, tmp, pre_data) < 0);

    Zeta_Core_MemCopy(data, tmp, width);
}

static void InsertionSort_(void* data, size_t width, size_t stride, size_t size,
                           void* cmp_context, Zeta_Core_Compare Compare) {
    if (size <= 1) { return; }

    void* end = data + stride * size;

    void* min = data;

    for (void* i = data + stride; i < end; i += stride) {
        if (Compare(cmp_context, i, min) < 0) { min = i; }
    }

    Zeta_Core_MemSwap(data, min, width);

    for (void* i = data + stride * 2; i < end; i += stride) {
        Insertion_(i, width, stride, cmp_context, Compare);
    }
}

static void* Mo5_(void* a, void* b, void* c, void* d, void* e,
                  void* cmp_context, Zeta_Core_Compare Compare) {
    for (int state = 0; state <= 3; ++state) {
        switch (state) {
            case 3:
                if (Compare(cmp_context, d, e) <= 0) { break; }
                ZETA_Core_Swap(d, e);

            case 2:
                if (Compare(cmp_context, c, d) <= 0) { break; }
                ZETA_Core_Swap(c, d);

            case 1:
                if (Compare(cmp_context, b, c) <= 0) { break; }
                ZETA_Core_Swap(b, c);

            case 0:
                if (Compare(cmp_context, a, b) <= 0) { break; }
                ZETA_Core_Swap(a, b);
        }
    }

    ZETA_Core_DebugAssert(Compare(cmp_context, a, b) <= 0);
    ZETA_Core_DebugAssert(Compare(cmp_context, b, c) <= 0);
    ZETA_Core_DebugAssert(Compare(cmp_context, c, d) <= 0);
    ZETA_Core_DebugAssert(Compare(cmp_context, d, e) <= 0);

    return c;
}

static void* Partition_(void* data, size_t width, size_t stride, size_t size,
                        void* cmp_context, Zeta_Core_Compare Compare) {
    void* i = data;
    void* j = data + stride * size;

    int i_cmp = -1;
    int j_cmp = 1;

    for (;;) {
        while (i_cmp < 0) { i_cmp = Compare(cmp_context, i += stride, data); }
        while (0 < j_cmp) { j_cmp = Compare(cmp_context, j -= stride, data); }

        if (j <= i) { break; }

        if (i_cmp != 0 || j_cmp != 0) { Zeta_Core_MemSwap(i, j, width); }

        i_cmp = Compare(cmp_context, i += stride, data);
        j_cmp = Compare(cmp_context, j -= stride, data);
    }

    Zeta_Core_MemSwap(data, j, width);

    return j;
}

static void* NaivePartition_(void* data, size_t width, size_t stride,
                             size_t size, void* cmp_context,
                             Zeta_Core_Compare Compare) {
    if (size <= InsertionThreshold) {
        InsertionSort_(data, width, stride, size, cmp_context, Compare);
        return data + stride * (size / 2);
    }

    void* a = data + stride * (size * 1 / 6);
    void* b = data + stride * (size * 2 / 6);
    void* c = data + stride * (size * 3 / 6);
    void* d = data + stride * (size * 4 / 6);
    void* e = data + stride * (size * 5 / 6);

    Zeta_Core_MemSwap(data, Mo5_(a, b, c, d, e, cmp_context, Compare), width);

    return Partition_(data, width, stride, size, cmp_context, Compare);
}

static void KthElement_(void* data, size_t width, size_t stride, size_t mid,
                        size_t size, void* cmp_context,
                        Zeta_Core_Compare Compare, size_t quata);

static void* MoMPartition_(void* data, size_t width, size_t stride, size_t size,
                           void* cmp_context, Zeta_Core_Compare Compare) {
    if (size <= InsertionThreshold) {
        InsertionSort_(data, width, stride, size, cmp_context, Compare);
        return data + stride * (size / 2);
    }

    size_t group_cnt = size / 5;

    size_t group_stride = stride * group_cnt;

    for (size_t i = 0; i < group_cnt; ++i) {
        void* a = data + stride * i;
        void* b = a + group_stride;
        void* c = b + group_stride;
        void* d = c + group_stride;
        void* e = d + group_stride;

        Zeta_Core_MemSwap(a, Mo5_(a, b, c, d, e, cmp_context, Compare), width);
    }

    KthElement_(data, width, stride, group_cnt / 2, group_cnt, cmp_context,
                Compare, 0);

    Zeta_Core_MemSwap(data, data + stride * (group_cnt / 2), width);

    return Partition_(data, width, stride, size, cmp_context, Compare);
}

static void KthElement_(void* data, size_t width, size_t stride, size_t mid,
                        size_t size, void* cmp_context,
                        Zeta_Core_Compare Compare, size_t quata) {
    for (;;) {
        if (size <= InsertionThreshold) {
            InsertionSort_(data, width, stride, size, cmp_context, Compare);
            return;
        }

        void* pivot;

        if (0 < quata) {
            pivot = NaivePartition_(data, width, stride, size, cmp_context,
                                    Compare);
            quata -= size;
        } else {
            pivot =
                MoMPartition_(data, width, stride, size, cmp_context, Compare);
        }

        size_t pivot_idx = (pivot - data) / stride;

        if (pivot_idx < mid) {
            data = pivot + stride;
            mid -= pivot_idx + 1;
            continue;
        }

        if (mid < pivot_idx) {
            size = pivot_idx;
            continue;
        }

        return;
    }
}

void* Zeta_Core_NicePartition(void* data, size_t width, size_t stride,
                              size_t size, void* cmp_context,
                              Zeta_Core_Compare Compare) {
    ZETA_Core_DebugAssert(data != NULL);
    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= stride);

    ZETA_Core_DebugAssert(Compare != NULL);

    if (size <= 1) { return data; }

    size_t quata = size * 4;

    size_t lb = 0;
    size_t rb = size;

    for (;;) {
        size_t cur_size = rb - lb;

        if (cur_size <= InsertionThreshold) {
            InsertionSort_(data + stride * lb, width, stride, cur_size,
                           cmp_context, Compare);

            size_t mid = size / 2;

            if (mid < lb) { return data + stride * lb; }

            if (rb <= mid) { return data + stride * (rb - 1); }

            return data + stride * mid;
        }

        void* pivot;

        if (0 < quata) {
            pivot = NaivePartition_(data + stride * lb, width, stride, cur_size,
                                    cmp_context, Compare);
            quata -= cur_size;
        } else {
            pivot = MoMPartition_(data + stride * lb, width, stride, cur_size,
                                  cmp_context, Compare);
        }

        size_t pivot_idx = (pivot - data) / stride;

        size_t l_size = pivot_idx;
        size_t r_size = size - l_size - 1;

        if (l_size <= r_size * 4 && r_size <= l_size * 4) {
            return data + stride * pivot_idx;
        }

        if (l_size < r_size) {
            rb = pivot_idx;
        } else {
            lb = pivot_idx + 1;
        }
    }
}

void Zeta_Core_KthElement(void* data, size_t width, size_t stride, size_t mid,
                          size_t size, void* cmp_context,
                          Zeta_Core_Compare Compare) {
    ZETA_Core_DebugAssert(data != NULL);
    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= stride);

    ZETA_Core_DebugAssert(mid <= size);

    ZETA_Core_DebugAssert(Compare != NULL);

    if (mid != size) {
        KthElement_(data, width, stride, mid, size, cmp_context, Compare,
                    size * 4);
    }
}

static void Sort_(size_t credit, void* data, size_t width, size_t stride,
                  size_t size, void* cmp_context, Zeta_Core_Compare Compare) {
    for (;;) {
        if (size <= InsertionThreshold) {
            InsertionSort_(data, width, stride, size, cmp_context, Compare);
            return;
        }

        void* pivot;

        if (0 < credit) {
            pivot = NaivePartition_(data, width, stride, size, cmp_context,
                                    Compare);
            credit = credit / 2;
        } else {
            pivot =
                MoMPartition_(data, width, stride, size, cmp_context, Compare);
        }

        void* r_data = pivot + stride;

        size_t l_size = (pivot - data) / stride;
        size_t r_size = size - 1 - l_size;

        if (l_size < r_size) {
            Sort_(credit, data, width, stride, l_size, cmp_context, Compare);
            data = r_data;
            size = r_size;
        } else {
            Sort_(credit, r_data, width, stride, r_size, cmp_context, Compare);
            size = l_size;
        }
    }
}

void Zeta_Core_Sort(void* data, size_t width, size_t stride, size_t size,
                    void* cmp_context, Zeta_Core_Compare Compare) {
    ZETA_Core_DebugAssert(data != NULL);
    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= stride);

    Sort_(size, data, width, stride, size, cmp_context, Compare);
}

// -----------------------------------------------------------------------------

static void Merge_(void* src_a, void* src_b, void* dst, size_t width,
                   size_t stride, size_t size_a, size_t size_b,
                   void* cmp_context, Zeta_Core_Compare Compare) {
    while (0 < size_a && 0 < size_b) {
        if (Compare(cmp_context, src_a, src_b) <= 0) {
            Zeta_Core_MemCopy(dst, src_a, width);
            src_a += stride;
            --size_a;
        } else {
            Zeta_Core_MemCopy(dst, src_b, width);
            src_b += stride;
            --size_b;
        }

        dst += stride;
    }

    if (0 < size_a) {
        Zeta_Core_ElemMove(dst, src_a, width, stride, stride, size_a);
    }

    if (0 < size_b) {
        Zeta_Core_ElemMove(dst, src_b, width, stride, stride, size_b);
    }
}

static void MergeSortTo_(void* src, void* dst, size_t width, size_t stride,
                         size_t size, void* cmp_context,
                         Zeta_Core_Compare Compare) {
    if (size <= 1) {
        if (size == 1) { Zeta_Core_MemCopy(dst, src, width); }
        return;
    }

    size_t l_size = size / 2;
    size_t r_size = size - l_size;

    void* src_mid = src + stride * l_size;
    void* dst_mid = dst + stride * l_size;

    MergeSortTo_(src_mid, dst_mid, width, stride, r_size, cmp_context, Compare);

    MergeSortTo_(src, src_mid, width, stride, l_size, cmp_context, Compare);

    Merge_(src_mid, dst_mid, dst, width, stride, l_size, r_size, cmp_context,
           Compare);
}

void Zeta_Core_MergeSort(void* data, size_t width, size_t stride, size_t size,
                         void* cmp_context, Zeta_Core_Compare Compare) {
    ZETA_Core_DebugAssert(data != NULL);
    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= stride);

    if (size <= 1) { return; }

    size_t l_size = size / 2;
    size_t r_size = size - l_size;

    void* buffer =
        ZETA_Core_Allocator_SafeAllocate(Zeta_Core_Memory_weak_lifo_allocator,
                                         alignof(max_align_t), stride * r_size);

    void* l_src = data;
    void* l_dst = data + stride * r_size;
    void* r_src = data + stride * l_size;
    void* r_dst = buffer;

    MergeSortTo_(r_src, r_dst, width, stride, r_size, cmp_context, Compare);

    MergeSortTo_(l_src, l_dst, width, stride, l_size, cmp_context, Compare);

    Merge_(l_dst, r_dst, data, width, stride, l_size, r_size, cmp_context,
           Compare);

    ZETA_Core_Allocator_Deallocate(Zeta_Core_Memory_weak_lifo_allocator,
                                   buffer);
}

// -----------------------------------------------------------------------------

static void TransMerge_(void** src_a, void** src_b, void** dst, size_t size_a,
                        size_t size_b, void* cmp_context,
                        Zeta_Core_Compare Compare) {
    while (0 < size_a && 0 < size_b) {
        if (Compare(cmp_context, *src_a, *src_b) <= 0) {
            *dst = *src_a;
            ++src_a;
            --size_a;
        } else {
            *dst = *src_b;
            ++src_b;
            --size_b;
        }

        ++dst;
    }

    if (0 < size_a) { Zeta_Core_MemMove(dst, src_a, sizeof(void*) * size_a); }

    if (0 < size_b) { Zeta_Core_MemMove(dst, src_b, sizeof(void*) * size_b); }
}

static void TransMergeSortTo_(void** src, void** dst, size_t size,
                              void* cmp_context, Zeta_Core_Compare Compare) {
    if (size <= 1) {
        if (size == 1) { *dst = *src; }
        return;
    }

    size_t l_size = size / 2;
    size_t r_size = size - l_size;

    void** src_mid = src + l_size;
    void** dst_mid = dst + l_size;

    TransMergeSortTo_(src_mid, dst_mid, r_size, cmp_context, Compare);

    TransMergeSortTo_(src, src_mid, l_size, cmp_context, Compare);

    TransMerge_(src_mid, dst_mid, dst, l_size, r_size, cmp_context, Compare);
}

void Zeta_TransMergeSort(void* data, size_t width, size_t stride, size_t size,
                         void* cmp_context, Zeta_Core_Compare Compare) {
    ZETA_Core_DebugAssert(data != NULL);
    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= stride);

    if (size <= 1) { return; }

    size_t l_size = size / 2;
    size_t r_size = size - l_size;

    void** ptr = ZETA_Core_Allocator_SafeAllocate(
        Zeta_Core_Memory_weak_lifo_allocator, alignof(void*),
        sizeof(void*) * (size + r_size));

    void** data_ptr = ptr;
    void** buffer_ptr = ptr + size;

    void** l_src = data_ptr;
    void** l_dst = data_ptr + r_size;
    void** r_src = data_ptr + l_size;
    void** r_dst = buffer_ptr;

    for (size_t i = 0; i < size; ++i) { data_ptr[i] = data + stride * i; }

    TransMergeSortTo_(r_src, r_dst, r_size, cmp_context, Compare);

    TransMergeSortTo_(l_src, l_dst, l_size, cmp_context, Compare);

    TransMerge_(l_dst, r_dst, data_ptr, l_size, r_size, cmp_context, Compare);

    void* tmp = __builtin_alloca(width);

    for (size_t i = 0; i < size; ++i) {
        size_t j = (data_ptr[i] - data) / stride;

        if (i == j) { continue; }

        Zeta_Core_MemCopy(tmp, data + stride * i, width);

        Zeta_Core_MemCopy(data + stride * i, data + stride * j, width);
        data_ptr[i] = data + stride * i;

        for (;;) {
            size_t nxt_j = (data_ptr[j] - data) / stride;

            if (nxt_j == i) { break; }

            Zeta_Core_MemCopy(data + stride * j, data + stride * nxt_j, width);

            data_ptr[j] = data + stride * j;

            j = nxt_j;
        }

        Zeta_Core_MemCopy(data + stride * j, tmp, width);
        data_ptr[j] = data + stride * j;
    }

    ZETA_Core_Allocator_Deallocate(Zeta_Core_Memory_weak_lifo_allocator, ptr);
}
