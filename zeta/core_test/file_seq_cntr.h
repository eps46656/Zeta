#pragma once

#include <seq_cntr.h>
#include <utils.h>
#include <zeta/core/debugger.h>

#include <fstream>

struct FileSeqCntr {
    mutable std::fstream fs;
    size_t size;

    void Open(const std::string& filename) {
        this->fs.open(filename, std::fstream::in | std::fstream::out |
                                    std::fstream::binary);
        ZETA_Core_DebugAssert(this->fs);

        this->fs.seekg(0, std::fstream::end);

        this->size = this->fs.tellg();
    }

    void Close() { this->fs.close(); }
};

struct FileSeqCntrCursor {
    size_t idx;
};

void FileSeqCntr_Deinit(void* file_seq_cntr_) {
    FileSeqCntr* file_seq_cntr = (FileSeqCntr*)file_seq_cntr_;
    FileSeqCntr_Check(file_seq_cntr);

    file_seq_cntr->fs.close();
}

size_t FileSeqCntr_GetWidth(void const* file_seq_cntr_) {
    FileSeqCntr const* file_seq_cntr = (FileSeqCntr const*)file_seq_cntr_;
    FileSeqCntr_Check(file_seq_cntr);

    return 1;
}

size_t FileSeqCntr_GetSize(void const* file_seq_cntr_) {
    FileSeqCntr const* file_seq_cntr = (FileSeqCntr const*)file_seq_cntr_;
    FileSeqCntr_Check(file_seq_cntr);

    return file_seq_cntr->size;
}

size_t FileSeqCntr_GetCapacity(void const* file_seq_cntr_) {
    FileSeqCntr const* file_seq_cntr = (FileSeqCntr const*)file_seq_cntr_;
    FileSeqCntr_Check(file_seq_cntr);

    return file_seq_cntr->size;
}

void FileSeqCntr_GetLBCursor(void const* file_seq_cntr_, void* dst_cursor_) {
    FileSeqCntr const* file_seq_cntr = (FileSeqCntr const*)file_seq_cntr_;
    FileSeqCntr_Check(file_seq_cntr);

    FileSeqCntrCursor* dst_cursor = (FileSeqCntrCursor*)dst_cursor_;
    ZETA_Core_DebugAssert(dst_cursor != NULL);

    dst_cursor->idx = -1;
}

void FileSeqCntr_GetRBCursor(void const* file_seq_cntr_, void* dst_cursor_) {
    FileSeqCntr const* file_seq_cntr = (FileSeqCntr const*)file_seq_cntr_;
    FileSeqCntr_Check(file_seq_cntr);

    FileSeqCntrCursor* dst_cursor = (FileSeqCntrCursor*)dst_cursor_;
    ZETA_Core_DebugAssert(dst_cursor != NULL);

    dst_cursor->idx = file_seq_cntr->size;
}

void* FileSeqCntr_PeekL(void* file_seq_cntr_, void* dst_cursor_,
                        void* dst_elem) {
    FileSeqCntr* file_seq_cntr = (FileSeqCntr*)file_seq_cntr_;
    FileSeqCntr_Check(file_seq_cntr);

    FileSeqCntrCursor* dst_cursor = (FileSeqCntrCursor*)dst_cursor_;

    if (file_seq_cntr->size == 0) { return NULL; }

    if (dst_cursor != NULL) { dst_cursor->idx = 0; }

    if (dst_elem != NULL) {
        file_seq_cntr->fs.seekg(0, std::fstream::beg);
        file_seq_cntr->fs.read((char*)dst_elem, 1);
    }

    return NULL;
}

void* FileSeqCntr_PeekR(void* file_seq_cntr_, void* dst_cursor_,
                        void* dst_elem) {
    FileSeqCntr* file_seq_cntr = (FileSeqCntr*)file_seq_cntr_;
    FileSeqCntr_Check(file_seq_cntr);

    FileSeqCntrCursor* dst_cursor = (FileSeqCntrCursor*)dst_cursor_;

    if (file_seq_cntr->size == 0) { return NULL; }

    if (dst_cursor != NULL) { dst_cursor->idx = file_seq_cntr->size - 1; }

    if (dst_elem != NULL) {
        file_seq_cntr->fs.seekg(file_seq_cntr->size - 1, std::fstream::beg);
        file_seq_cntr->fs.read((char*)dst_elem, 1);
    }

    return NULL;
}

void* FileSeqCntr_Access(void* file_seq_cntr_, size_t idx, void* dst_cursor_,
                         void* dst_elem) {
    FileSeqCntr* file_seq_cntr = (FileSeqCntr*)file_seq_cntr_;
    FileSeqCntr_Check(file_seq_cntr);

    ZETA_Core_DebugAssert(idx + 1 < file_seq_cntr->size + 2);

    FileSeqCntrCursor* dst_cursor = (FileSeqCntrCursor*)dst_cursor_;

    if (dst_cursor != NULL) { dst_cursor->idx = idx; }

    if (idx < file_seq_cntr->size && dst_elem != NULL) {
        file_seq_cntr->fs.seekg(idx, std::fstream::beg);
        file_seq_cntr->fs.read((char*)dst_elem, 1);
    }

    return NULL;
}

void* FileSeqCntr_Refer(void* file_seq_cntr_, void const* pos_cursor_) {
    FileSeqCntr* file_seq_cntr = (FileSeqCntr*)file_seq_cntr_;
    FileSeqCntrCursor const* pos_cursor = (FileSeqCntrCursor const*)pos_cursor_;

    FileSeqCntr_CheckCursor(file_seq_cntr, pos_cursor);

    return NULL;
}

void FileSeqCntr_Read(void const* file_seq_cntr_, void const* pos_cursor_,
                      size_t cnt, void* dst, size_t dst_stride,
                      void* dst_cursor_) {
    FileSeqCntr const* file_seq_cntr = (FileSeqCntr const*)file_seq_cntr_;
    FileSeqCntrCursor const* pos_cursor = (FileSeqCntrCursor const*)pos_cursor_;

    FileSeqCntr_CheckCursor(file_seq_cntr, pos_cursor);

    FileSeqCntrCursor* dst_cursor = (FileSeqCntrCursor*)dst_cursor_;

    if (cnt == 0) { return; }

    size_t idx = pos_cursor->idx;

    ZETA_Core_DebugAssert(cnt <= file_seq_cntr->size - idx);

    char* buffer = (char*)std::malloc(sizeof(char) * cnt);

    file_seq_cntr->fs.seekg(idx, std::fstream::beg);
    file_seq_cntr->fs.read(buffer, cnt);

    Zeta_Core_ElemCopy(dst, buffer, 1, dst_stride, 1, cnt);

    std::free(buffer);
}

bool_t FileSeqCntr_Cursor_AreEqual(void const* file_seq_cntr_,
                                   void const* cursor_a_,
                                   void const* cursor_b_) {
    FileSeqCntr const* file_seq_cntr = (FileSeqCntr const*)file_seq_cntr_;
    FileSeqCntrCursor const* cursor_a = (FileSeqCntrCursor const*)cursor_a_;
    FileSeqCntrCursor const* cursor_b = (FileSeqCntrCursor const*)cursor_b_;

    FileSeqCntr_CheckCursor(file_seq_cntr, cursor_a);
    FileSeqCntr_CheckCursor(file_seq_cntr, cursor_b);

    return cursor_a->idx == cursor_b->idx;
}

int FileSeqCntr_Cursor_Compare(void const* file_seq_cntr_,
                               void const* cursor_a_, void const* cursor_b_) {
    FileSeqCntr const* file_seq_cntr = (FileSeqCntr const*)file_seq_cntr_;
    FileSeqCntrCursor const* cursor_a = (FileSeqCntrCursor const*)cursor_a_;
    FileSeqCntrCursor const* cursor_b = (FileSeqCntrCursor const*)cursor_b_;

    FileSeqCntr_CheckCursor(file_seq_cntr, cursor_a);
    FileSeqCntr_CheckCursor(file_seq_cntr, cursor_b);

    return ZETA_Core_ThreeWayCompare(cursor_a->idx, cursor_b->idx);
}

size_t FileSeqCntr_Cursor_GetDist(void const* file_seq_cntr_,
                                  void const* cursor_a_,
                                  void const* cursor_b_) {
    FileSeqCntr const* file_seq_cntr = (FileSeqCntr const*)file_seq_cntr_;
    FileSeqCntrCursor const* cursor_a = (FileSeqCntrCursor const*)cursor_a_;
    FileSeqCntrCursor const* cursor_b = (FileSeqCntrCursor const*)cursor_b_;

    FileSeqCntr_CheckCursor(file_seq_cntr, cursor_a);
    FileSeqCntr_CheckCursor(file_seq_cntr, cursor_b);

    return cursor_b->idx - cursor_a->idx;
}

size_t FileSeqCntr_Cursor_GetIdx(void const* file_seq_cntr_,
                                 void const* cursor_) {
    FileSeqCntr const* file_seq_cntr = (FileSeqCntr const*)file_seq_cntr_;
    FileSeqCntrCursor const* cursor = (FileSeqCntrCursor const*)cursor_;

    FileSeqCntr_CheckCursor(file_seq_cntr, cursor);

    return cursor->idx;
}

void FileSeqCntr_Cursor_StepL(void const* file_seq_cntr, void* cursor) {
    FileSeqCntr_Cursor_AdvanceL(file_seq_cntr, cursor, 1);
}

void FileSeqCntr_Cursor_StepR(void const* file_seq_cntr, void* cursor) {
    FileSeqCntr_Cursor_AdvanceR(file_seq_cntr, cursor, 1);
}

void FileSeqCntr_Cursor_AdvanceL(void const* file_seq_cntr_, void* cursor_,
                                 size_t step) {
    FileSeqCntr const* file_seq_cntr = (FileSeqCntr const*)file_seq_cntr_;
    FileSeqCntrCursor* cursor = (FileSeqCntrCursor*)cursor_;

    FileSeqCntr_CheckCursor(file_seq_cntr, cursor);

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    cursor->idx -= step;
}

void FileSeqCntr_Cursor_AdvanceR(void const* file_seq_cntr_, void* cursor_,
                                 size_t step) {
    FileSeqCntr const* file_seq_cntr = (FileSeqCntr const*)file_seq_cntr_;
    FileSeqCntrCursor* cursor = (FileSeqCntrCursor*)cursor_;

    FileSeqCntr_CheckCursor(file_seq_cntr, cursor);

    ZETA_Core_DebugAssert(step <= file_seq_cntr->size - cursor->idx);

    cursor->idx += step;
}

void FileSeqCntr_Check(void const* file_seq_cntr_) {
    FileSeqCntr const* file_seq_cntr = (FileSeqCntr const*)file_seq_cntr_;
    ZETA_Core_DebugAssert(file_seq_cntr != NULL);

    file_seq_cntr->fs.seekg(0, std::fstream::end);

    ZETA_Core_DebugAssert(file_seq_cntr->fs.tellg() == file_seq_cntr->size);
}

void FileSeqCntr_CheckCursor(void const* file_seq_cntr_, void const* cursor_) {
    FileSeqCntr const* file_seq_cntr = (FileSeqCntr const*)file_seq_cntr_;
    FileSeqCntr_Check(file_seq_cntr);

    FileSeqCntrCursor* cursor = (FileSeqCntrCursor*)cursor_;
    ZETA_Core_DebugAssert(cursor != NULL);

    ZETA_Core_DebugAssert(cursor->idx + 1 < file_seq_cntr->size + 2);
}

extern Zeta_Core_SeqCntr file_seq_cntr = {
    .cursor_size = sizeof(FileSeqCntrCursor),

    .Deinit = FileSeqCntr_Deinit,

    .GetWidth = FileSeqCntr_GetWidth,

    .GetSize = FileSeqCntr_GetSize,

    .GetCapacity = FileSeqCntr_GetCapacity,

    .GetLBCursor = FileSeqCntr_GetLBCursor,

    .GetRBCursor = FileSeqCntr_GetRBCursor,

    .PeekL = FileSeqCntr_PeekL,

    .PeekR = FileSeqCntr_PeekR,

    .Access = FileSeqCntr_Access,

    .Refer = FileSeqCntr_Refer,

    .Read = FileSeqCntr_Read,

    .Write = NULL,

    .PushL = NULL,

    .PushR = NULL,

    .Insert = NULL,

    .PopL = NULL,

    .PopR = NULL,

    .Erase = NULL,

    .EraseAll = NULL,

    .Cursor_AreEqual = FileSeqCntr_Cursor_AreEqual,

    .Cursor_Compare = FileSeqCntr_Cursor_Compare,

    .Cursor_GetDist = FileSeqCntr_Cursor_GetDist,

    .Cursor_GetIdx = FileSeqCntr_Cursor_GetIdx,

    .Cursor_StepL = FileSeqCntr_Cursor_StepL,

    .Cursor_StepR = FileSeqCntr_Cursor_StepR,

    .Cursor_AdvanceL = FileSeqCntr_Cursor_AdvanceL,

    .Cursor_AdvanceR = FileSeqCntr_Cursor_AdvanceR,
};
