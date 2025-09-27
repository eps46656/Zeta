#pragma once

#include <map>
#include <zeta/core/define.hpp>

namespace zeta::core {

struct MemRecorder;

struct MemRecorder {
    std::map<void const*, size_t> records;

    static MemRecorder* Create();
    static void Destroy(void* mem_recorder);

    static size_t GetSize(void const* mem_recorder);
    static size_t GetRecordSize(void const* mem_recorder, void const* ptr);
    static bool IsRecorded(void const* mem_recorder, void const* ptr);

    static void Record(void* mem_recorder, void const* ptr, size_t size);
    static bool Unrecord(void* mem_recorder, void const* ptr);

    static void Clear(void* mem_recorder);

    static void MatchRecords(MemRecorder const* src_mem_recorder,
                             MemRecorder const* dst_mem_recorder);
};

}  // namespace zeta::core
