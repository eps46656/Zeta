#pragma once

#include <map>
#include <zeta/core/define.hpp>

namespace zeta::core::mem_recorder {

struct MemRecorder {
    std::map<void const*, size_t> records;
};

MemRecorder* Create();
void Destroy(MemRecorder* mr);

size_t GetSize(MemRecorder const& mr);
size_t GetRecordSize(MemRecorder const& mr, void const* ptr);
bool IsRecorded(MemRecorder const& mr, void const* ptr);

void Record(MemRecorder& mr, void const* ptr, size_t size);
bool Unrecord(MemRecorder& mr, void const* ptr);

void Clear(MemRecorder& mr);

void MatchRecords(MemRecorder const& src_mr, MemRecorder const& dst_mr);

}  // namespace zeta::core::mem_recorder
