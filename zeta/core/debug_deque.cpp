#include <zeta/core/debug_deque.h>
#include <zeta/core/debugger.h>
#include <zeta/core/utils.h>

#include <deque>

extern "C" void Zeta_Core_DebugDeque_Init(void* dd_) {
    Zeta_Core_DebugDeque* dd = (Zeta_Core_DebugDeque*)dd_;
    ZETA_Core_DebugAssert(dd != NULL);

    dd->deque = new std::deque<void*>;

    ZETA_Core_DebugAssert(0 < dd->width);
}

extern "C" void Zeta_Core_DebugDeque_Deinit(void* dd_) {
    Zeta_Core_DebugDeque* dd = (Zeta_Core_DebugDeque*)dd_;
    Zeta_Core_DebugDeque_Check(dd);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    for (auto iter{ deque->begin() }, end{ deque->end() }; iter != end;
         ++iter) {
        delete[] (unsigned char*)(*iter);
    }

    delete deque;
}

extern "C" size_t Zeta_Core_DebugDeque_GetWidth(void const* dd_) {
    Zeta_Core_DebugDeque const* dd = (Zeta_Core_DebugDeque const*)dd_;
    Zeta_Core_DebugDeque_Check(dd);

    return dd->width;
}

extern "C" size_t Zeta_Core_DebugDeque_GetSize(void const* dd_) {
    Zeta_Core_DebugDeque const* dd = (Zeta_Core_DebugDeque const*)dd_;
    Zeta_Core_DebugDeque_Check(dd);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    return deque->size();
}

extern "C" size_t Zeta_Core_DebugDeque_GetCapacity(void const* dd_) {
    Zeta_Core_DebugDeque const* dd = (Zeta_Core_DebugDeque const*)dd_;
    Zeta_Core_DebugDeque_Check(dd);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    return deque->max_size();
}

extern "C" void Zeta_Core_DebugDeque_GetLBCursor(void const* dd_,
                                                 void* dst_cursor_) {
    Zeta_Core_DebugDeque const* dd = (Zeta_Core_DebugDeque const*)dd_;
    Zeta_Core_DebugDeque_Check(dd);

    size_t* dst_cursor = (size_t*)dst_cursor_;

    if (dst_cursor == NULL) { return; }

    *dst_cursor = (size_t)(-1);
}

extern "C" void Zeta_Core_DebugDeque_GetRBCursor(void const* dd_,
                                                 void* dst_cursor_) {
    Zeta_Core_DebugDeque const* dd = (Zeta_Core_DebugDeque const*)dd_;
    Zeta_Core_DebugDeque_Check(dd);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    size_t* dst_cursor = (size_t*)dst_cursor_;

    if (dst_cursor == NULL) { return; }

    *dst_cursor = deque->size();
}

extern "C" void* Zeta_Core_DebugDeque_PeekL(void* dd_, void* dst_cursor_,
                                            void* dst_elem) {
    Zeta_Core_DebugDeque* dd = (Zeta_Core_DebugDeque*)dd_;
    Zeta_Core_DebugDeque_Check(dd);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    size_t* dst_cursor = (size_t*)dst_cursor_;
    if (dst_cursor != NULL) { *dst_cursor = 0; }

    if (deque->empty()) { return NULL; }

    void* elem = deque->front();

    if (dst_elem != NULL) { Zeta_Core_MemCopy(dst_elem, elem, dd->width); }

    return elem;
}

extern "C" void* Zeta_Core_DebugDeque_PeekR(void* dd_, void* dst_cursor_,
                                            void* dst_elem) {
    Zeta_Core_DebugDeque* dd = (Zeta_Core_DebugDeque*)dd_;
    Zeta_Core_DebugDeque_Check(dd);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    size_t* dst_cursor = (size_t*)dst_cursor_;
    if (dst_cursor != NULL) { *dst_cursor = deque->size() - 1; }

    if (deque->empty()) { return NULL; }

    void* elem = deque->back();

    if (dst_elem != NULL) { Zeta_Core_MemCopy(dst_elem, elem, dd->width); }

    return elem;
}

extern "C" void* Zeta_Core_DebugDeque_Access(void* dd_, size_t idx,
                                             void* dst_cursor_,
                                             void* dst_elem) {
    Zeta_Core_DebugDeque* dd = (Zeta_Core_DebugDeque*)dd_;
    Zeta_Core_DebugDeque_Check(dd);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    ZETA_Core_DebugAssert(idx + 1 < deque->size() + 2);

    size_t* dst_cursor = (size_t*)dst_cursor_;

    if (dst_cursor != NULL) { *dst_cursor = idx; }

    if (deque->size() <= idx) { return NULL; }

    void* elem = (*deque)[idx];

    if (dst_elem != NULL) { Zeta_Core_MemCopy(dst_elem, elem, dd->width); }

    return elem;
}

extern "C" void* Zeta_Core_DebugDeque_Refer(void* dd_,
                                            void const* pos_cursor_) {
    Zeta_Core_DebugDeque* dd = (Zeta_Core_DebugDeque*)dd_;
    size_t const* pos_cursor = (size_t const*)pos_cursor_;
    Zeta_Core_DebugDeque_Cursor_Check(dd, pos_cursor);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    size_t idx = *pos_cursor;

    return idx < deque->size() ? (*deque)[idx] : NULL;
}

extern "C" void Zeta_Core_DebugDeque_Read(void const* dd_,
                                          void const* pos_cursor_, size_t cnt,
                                          void* dst_, size_t dst_stride,
                                          void* dst_cursor_) {
    Zeta_Core_DebugDeque const* dd = (Zeta_Core_DebugDeque const*)dd_;
    size_t const* pos_cursor = (size_t const*)pos_cursor_;
    size_t* dst_cursor = (size_t*)dst_cursor_;
    Zeta_Core_DebugDeque_Cursor_Check(dd, pos_cursor);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    unsigned char* dst = (unsigned char*)dst_;
    ZETA_Core_DebugAssert(dst != NULL);

    size_t beg = *pos_cursor;
    ZETA_Core_DebugAssert(beg <= deque->size());

    ZETA_Core_DebugAssert(cnt <= deque->size() - beg);

    size_t end = beg + cnt;

    if (dst_cursor != NULL) { *dst_cursor = end; }

    size_t dst_width = ZETA_Core_GetMinOf(dst_stride, dd->width);

    for (size_t idx = beg; idx < end; ++idx) {
        Zeta_Core_MemCopy(dst, (*deque)[idx], dst_width);
        dst += dst_stride;
    }
}

extern "C" void Zeta_Core_DebugDeque_Write(void* dd_, void* pos_cursor_,
                                           size_t cnt, void const* src_,
                                           size_t src_stride,
                                           void* dst_cursor_) {
    Zeta_Core_DebugDeque* dd = (Zeta_Core_DebugDeque*)dd_;
    size_t* pos_cursor = (size_t*)pos_cursor_;
    size_t* dst_cursor = (size_t*)dst_cursor_;
    Zeta_Core_DebugDeque_Cursor_Check(dd, pos_cursor);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    unsigned char const* src = (unsigned char*)src_;
    ZETA_Core_DebugAssert(src != NULL);

    size_t beg = *pos_cursor;
    ZETA_Core_DebugAssert(beg <= deque->size());

    ZETA_Core_DebugAssert(cnt <= deque->size() - beg);

    size_t end = beg + cnt;

    if (dst_cursor != NULL) { *dst_cursor = end; }

    size_t src_width = ZETA_Core_GetMinOf(src_stride, dd->width);

    for (size_t idx = beg; idx < end; ++idx) {
        Zeta_Core_MemCopy((*deque)[idx], src, src_width);
        src += src_stride;
    }
}

extern "C" void* Zeta_Core_DebugDeque_PushL(void* dd_, size_t cnt,
                                            void* dst_cursor_) {
    Zeta_Core_DebugDeque* dd = (Zeta_Core_DebugDeque*)dd_;
    Zeta_Core_DebugDeque_Check(dd);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    size_t* dst_cursor = (size_t*)dst_cursor_;
    if (dst_cursor != NULL) { *dst_cursor = 0; }

    deque->insert(deque->begin(), cnt, nullptr);

    for (size_t idx{ 0 }; idx < cnt; ++idx) {
        (*deque)[idx] = new unsigned char[dd->width];
    }

    return deque->empty() ? NULL : deque->front();
}

extern "C" void* Zeta_Core_DebugDeque_PushR(void* dd_, size_t cnt,
                                            void* dst_cursor_) {
    Zeta_Core_DebugDeque* dd = (Zeta_Core_DebugDeque*)dd_;
    Zeta_Core_DebugDeque_Check(dd);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    size_t origin_size{ deque->size() };

    size_t* dst_cursor = (size_t*)dst_cursor_;
    if (dst_cursor != NULL) { *dst_cursor = origin_size; }

    deque->insert(deque->end(), cnt, nullptr);

    for (size_t idx{ deque->size() - cnt }; idx < deque->size(); ++idx) {
        (*deque)[idx] = new unsigned char[dd->width];
    }

    return origin_size < deque->size() ? (*deque)[origin_size] : NULL;
}

extern "C" void* Zeta_Core_DebugDeque_Insert(void* dd_, void* pos_cursor_,
                                             size_t cnt) {
    Zeta_Core_DebugDeque* dd = (Zeta_Core_DebugDeque*)dd_;
    size_t* pos_cursor = (size_t*)pos_cursor_;
    Zeta_Core_DebugDeque_Cursor_Check(dd, pos_cursor);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    size_t idx = *pos_cursor;
    ZETA_Core_DebugAssert(idx <= deque->size());

    deque->insert(deque->begin() + idx, cnt, NULL);

    size_t beg = idx;
    size_t end = beg + cnt;

    for (; idx < end; ++idx) { (*deque)[idx] = new unsigned char[dd->width]; }

    return beg < deque->size() ? (*deque)[beg] : NULL;
}

extern "C" void Zeta_Core_DebugDeque_PopL(void* dd_, size_t cnt) {
    Zeta_Core_DebugDeque* dd = (Zeta_Core_DebugDeque*)dd_;
    Zeta_Core_DebugDeque_Check(dd);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    ZETA_Core_DebugAssert(cnt <= deque->size());

    while (0 < cnt--) {
        delete[] (unsigned char*)deque->front();
        deque->pop_front();
    }
}

extern "C" void Zeta_Core_DebugDeque_PopR(void* dd_, size_t cnt) {
    Zeta_Core_DebugDeque* dd = (Zeta_Core_DebugDeque*)dd_;
    Zeta_Core_DebugDeque_Check(dd);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    ZETA_Core_DebugAssert(cnt <= deque->size());

    while (0 < cnt--) {
        delete[] (unsigned char*)deque->back();
        deque->pop_back();
    }
}

extern "C" void Zeta_Core_DebugDeque_Erase(void* dd_, void* pos_cursor_,
                                           size_t cnt) {
    Zeta_Core_DebugDeque* dd = (Zeta_Core_DebugDeque*)dd_;
    size_t* pos_cursor = (size_t*)pos_cursor_;
    Zeta_Core_DebugDeque_Cursor_Check(dd, pos_cursor);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    size_t beg = *pos_cursor;
    ZETA_Core_DebugAssert(beg <= deque->size());

    ZETA_Core_DebugAssert(cnt <= deque->size() - beg);

    size_t end = beg + cnt;

    for (size_t idx = beg; idx < end; ++idx) {
        delete[] (unsigned char*)((*deque)[idx]);
    }

    deque->erase(deque->begin() + beg, deque->begin() + end);
}

extern "C" void Zeta_Core_DebugDeque_EraseAll(void* dd_) {
    Zeta_Core_DebugDeque* dd = (Zeta_Core_DebugDeque*)dd_;
    Zeta_Core_DebugDeque_Check(dd);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    for (auto iter{ deque->begin() }, end{ deque->end() }; iter != end;
         ++iter) {
        delete[] (unsigned char*)(*iter);
    }

    deque->clear();
}

extern "C" void Zeta_Core_DebugDeque_Check(void const* dd_) {
    Zeta_Core_DebugDeque const* dd = (Zeta_Core_DebugDeque const*)dd_;
    ZETA_Core_DebugAssert(dd != NULL);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;
    ZETA_Core_DebugAssert(deque != NULL);
}

extern "C" bool_t Zeta_Core_DebugDeque_Cursor_AreEqual(void const* dd_,
                                                       void const* cursor_a_,
                                                       void const* cursor_b_) {
    Zeta_Core_DebugDeque const* dd = (Zeta_Core_DebugDeque const*)dd_;
    size_t* cursor_a = (size_t*)cursor_a_;
    size_t* cursor_b = (size_t*)cursor_b_;

    Zeta_Core_DebugDeque_Cursor_Check(dd, cursor_a);
    Zeta_Core_DebugDeque_Cursor_Check(dd, cursor_b);

    ZETA_Core_DebugAssert(cursor_a != NULL);
    ZETA_Core_DebugAssert(cursor_b != NULL);

    return *cursor_a == *cursor_b;
}

extern "C" int Zeta_Core_DebugDeque_Cursor_Compare(void const* dd_,
                                                   void const* cursor_a_,
                                                   void const* cursor_b_) {
    Zeta_Core_DebugDeque const* dd = (Zeta_Core_DebugDeque const*)dd_;

    size_t* cursor_a = (size_t*)cursor_a_;
    size_t* cursor_b = (size_t*)cursor_b_;

    Zeta_Core_DebugDeque_Cursor_Check(dd, cursor_a);
    Zeta_Core_DebugDeque_Cursor_Check(dd, cursor_b);

    return ZETA_Core_ThreeWayCompare(*cursor_a + 1, *cursor_b + 1);
}

extern "C" size_t Zeta_Core_DebugDeque_Cursor_GetDist(void const* dd_,
                                                      void const* cursor_a_,
                                                      void const* cursor_b_) {
    Zeta_Core_DebugDeque const* dd = (Zeta_Core_DebugDeque const*)dd_;

    size_t* cursor_a = (size_t*)cursor_a_;
    size_t* cursor_b = (size_t*)cursor_b_;

    Zeta_Core_DebugDeque_Cursor_Check(dd, cursor_a);
    Zeta_Core_DebugDeque_Cursor_Check(dd, cursor_b);

    return *cursor_b - *cursor_a;
}

extern "C" size_t Zeta_Core_DebugDeque_Cursor_GetIdx(void const* dd_,
                                                     void const* cursor_) {
    Zeta_Core_DebugDeque const* dd = (Zeta_Core_DebugDeque const*)dd_;
    size_t* cursor = (size_t*)cursor_;
    Zeta_Core_DebugDeque_Cursor_Check(dd, cursor);

    return *cursor;
}

extern "C" void Zeta_Core_DebugDeque_Cursor_StepL(void const* dd,
                                                  void* cursor) {
    Zeta_Core_DebugDeque_Cursor_AdvanceL(dd, cursor, 1);
}

extern "C" void Zeta_Core_DebugDeque_Cursor_StepR(void const* dd,
                                                  void* cursor) {
    Zeta_Core_DebugDeque_Cursor_AdvanceR(dd, cursor, 1);
}

extern "C" void Zeta_Core_DebugDeque_Cursor_AdvanceL(void const* dd_,
                                                     void* cursor_,
                                                     size_t step) {
    Zeta_Core_DebugDeque const* dd = (Zeta_Core_DebugDeque const*)dd_;
    size_t* cursor = (size_t*)cursor_;
    Zeta_Core_DebugDeque_Cursor_Check(dd, cursor);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    size_t idx = *cursor - step;

    ZETA_Core_DebugAssert(idx + 1 < deque->size() + 2);

    *cursor = idx;
}

extern "C" void Zeta_Core_DebugDeque_Cursor_AdvanceR(void const* dd_,
                                                     void* cursor_,
                                                     size_t step) {
    Zeta_Core_DebugDeque const* dd = (Zeta_Core_DebugDeque const*)dd_;
    size_t* cursor = (size_t*)cursor_;
    Zeta_Core_DebugDeque_Cursor_Check(dd, cursor);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;

    size_t idx = *cursor + step;

    ZETA_Core_DebugAssert(idx + 1 < deque->size() + 2);

    *cursor = idx;
}

extern "C" void Zeta_Core_DebugDeque_Cursor_Check(void const* dd_,
                                                  void const* cursor_) {
    Zeta_Core_DebugDeque const* dd = (Zeta_Core_DebugDeque const*)dd_;
    ZETA_Core_DebugAssert(dd != NULL);

    std::deque<void*>* deque = (std::deque<void*>*)dd->deque;
    ZETA_Core_DebugAssert(deque != NULL);

    size_t* cursor = (size_t*)cursor_;
    ZETA_Core_DebugAssert(cursor != NULL);

    ZETA_Core_DebugAssert(*cursor + 1 < deque->size() + 2);
}

extern "C" Zeta_Core_SeqCntr_VTable const
    Zeta_Core_DebugDeque_seq_cntr_vtable = {
        .cursor_size = sizeof(size_t),

        .Deinit = Zeta_Core_DebugDeque_Deinit,

        .GetWidth = Zeta_Core_DebugDeque_GetWidth,

        .GetSize = Zeta_Core_DebugDeque_GetSize,

        .GetCapacity = Zeta_Core_DebugDeque_GetCapacity,

        .GetLBCursor = Zeta_Core_DebugDeque_GetLBCursor,

        .GetRBCursor = Zeta_Core_DebugDeque_GetRBCursor,

        .PeekL = Zeta_Core_DebugDeque_PeekL,

        .PeekR = Zeta_Core_DebugDeque_PeekR,

        .Access = Zeta_Core_DebugDeque_Access,

        .Refer = Zeta_Core_DebugDeque_Refer,

        .Read = Zeta_Core_DebugDeque_Read,

        .Write = Zeta_Core_DebugDeque_Write,

        .PushL = Zeta_Core_DebugDeque_PushL,

        .PushR = Zeta_Core_DebugDeque_PushR,

        .Insert = Zeta_Core_DebugDeque_Insert,

        .PopL = Zeta_Core_DebugDeque_PopL,

        .PopR = Zeta_Core_DebugDeque_PopR,

        .Erase = Zeta_Core_DebugDeque_Erase,

        .EraseAll = Zeta_Core_DebugDeque_EraseAll,

        .Cursor_AreEqual = Zeta_Core_DebugDeque_Cursor_AreEqual,

        .Cursor_Compare = Zeta_Core_DebugDeque_Cursor_Compare,

        .Cursor_GetDist = Zeta_Core_DebugDeque_Cursor_GetDist,

        .Cursor_GetIdx = Zeta_Core_DebugDeque_Cursor_GetIdx,

        .Cursor_StepL = Zeta_Core_DebugDeque_Cursor_StepL,

        .Cursor_StepR = Zeta_Core_DebugDeque_Cursor_StepR,

        .Cursor_AdvanceL = Zeta_Core_DebugDeque_Cursor_AdvanceL,

        .Cursor_AdvanceR = Zeta_Core_DebugDeque_Cursor_AdvanceR,
    };
