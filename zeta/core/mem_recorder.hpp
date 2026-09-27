#pragma once

#include <map>
#include <zeta/core/define.hpp>

namespace zeta::core::mem_recorder {

struct MemRecorder {
    std::map<void const*, size_t> records;

    static MemRecorder* Create();
    static void Destroy(MemRecorder* mr);

    size_t GetSize(this MemRecorder const& mr);
    size_t GetRecordSize(this MemRecorder const& mr, void const* ptr);
    bool IsRecorded(this MemRecorder const& mr, void const* ptr);

    void Record(this MemRecorder& mr, void const* ptr, size_t size);
    bool Unrecord(this MemRecorder& mr, void const* ptr);

    void Clear(this MemRecorder& mr);

    static void MatchRecords(MemRecorder const& src_mr,
                             MemRecorder const& dst_mr);
};

}  // namespace zeta::core::mem_recorder
