#pragma once

#include <zeta/core/define.h>

#if defined(__cplusplus)

#include <map>

#endif

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_MemRecorder);

struct Zeta_Core_MemRecorder;

#if defined(__cplusplus)

struct Zeta_Core_MemRecorder {
    std::map<void const*, size_t> records;
};

#endif

Zeta_Core_MemRecorder* Zeta_Core_MemRecorder_Create();

void Zeta_Core_MemRecorder_Destroy(Zeta_Core_MemRecorder* mem_recorder);

size_t Zeta_Core_MemRecorder_GetSize(Zeta_Core_MemRecorder* mem_recorder);

size_t Zeta_Core_MemRecorder_GetRecordSize(Zeta_Core_MemRecorder* mem_recorder,
                                           void const* ptr);

bool_t Zeta_Core_MemRecorder_IsRecorded(Zeta_Core_MemRecorder* mem_recorder,
                                        void const* ptr);

void Zeta_Core_MemRecorder_Record(Zeta_Core_MemRecorder* mem_recorder,
                                  void const* ptr, size_t size);

bool_t Zeta_Core_MemRecorder_Unrecord(Zeta_Core_MemRecorder* mem_recorder,
                                      void const* ptr);

void Zeta_Core_MemRecorder_Clear(Zeta_Core_MemRecorder* mem_recorder);

void Zeta_Core_MemCheck_MatchRecords(
    Zeta_Core_MemRecorder const* src_mem_recorder,
    Zeta_Core_MemRecorder const* dst_mem_recorder);

ZETA_Core_ExternC_End;
