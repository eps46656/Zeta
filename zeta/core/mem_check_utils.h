#pragma once

#include <zeta/core/define.h>

#include <map>

namespace zeta::core {

struct MemRecorder;

struct MemRecorder {
    std::map<void const*, size_t> records;

    static MemRecorder* Create();
    static void Destroy(MemRecorder* mem_recorder);

    size_t GetSize() const;
    size_t GetRecordSize(void const* ptr) const;
    bool IsRecorded(void const* ptr) const;

    void Record(void const* ptr, size_t size);
    bool Unrecord(void const* ptr);

    void Clear();

    static void MatchRecords(MemRecorder const* src_mem_recorder,
                             MemRecorder const* dst_mem_recorder);
};

}  // namespace zeta::core
