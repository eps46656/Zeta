#include <zeta/core/mem_check_utils.h>

#include <zeta/core/integer.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

MemRecorder* MemRecorder::Create() { return new MemRecorder{}; }

void MemRecorder::Destroy(MemRecorder* mem_recorder) { delete mem_recorder; }

size_t MemRecorder::GetSize() const { return this->records.size(); }

size_t MemRecorder::GetRecordSize(void const* ptr) const {
    auto iter{ this->records.find(ptr) };
    return iter == this->records.end() ? ZETA_Core_size_max : iter->second;
}

bool MemRecorder::IsRecorded(void const* ptr) const {
    return this->records.find(ptr) != this->records.end();
}

void MemRecorder::Record(void const* ptr, size_t size) {
    auto iter{ this->records.lower_bound(ptr) };

    if (iter != this->records.end()) {
        ZETA_Core_DebugAssert(iter->first != ptr);
        ZETA_Core_DebugAssert((unsigned char const*)ptr + size <= iter->first);
    }

    if (iter != this->records.begin()) {
        --iter;
        ZETA_Core_DebugAssert(
            (unsigned char const*)iter->first + iter->second <= ptr);
    }

    this->records.insert({ ptr, size });
}

bool MemRecorder::Unrecord(void const* ptr) {
    return this->records.erase(ptr) != 0;
}

void MemRecorder::Clear() { this->records.clear(); }

void MemRecorder::MatchRecords(MemRecorder const* src_mem_recorder,
                               MemRecorder const* dst_mem_recorder) {
    ZETA_Core_DebugAssert(src_mem_recorder != nullptr);
    ZETA_Core_DebugAssert(dst_mem_recorder != nullptr);

    ZETA_Core_DebugAssert(src_mem_recorder->records.size() ==
                          dst_mem_recorder->records.size());

    for (auto src_iter{ src_mem_recorder->records.begin() },
         src_end{ src_mem_recorder->records.end() },
         dst_iter{ dst_mem_recorder->records.begin() };
         src_iter != src_end; ++src_iter, ++dst_iter) {
        ZETA_Core_DebugAssert(!(src_iter->first < dst_iter->first));
        // memory leek: dst misses allocated memory.

        ZETA_Core_DebugAssert(!(dst_iter->first < src_iter->first));
        // hallucination or use after free: dst uses unallocated memoty.

        ZETA_Core_DebugAssert(dst_iter->second <= src_iter->second);
        // overflow: dst uses more than allocated memory.
    }
}

}  // namespace zeta::core
