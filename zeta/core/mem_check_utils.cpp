#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_check_utils.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

MemRecorder* MemRecorder::Create() { return new MemRecorder{}; }

void MemRecorder::Destroy(void* mem_recorder_) {
    auto mem_recorder{ static_cast<MemRecorder const*>(mem_recorder_) };
    ZETA_Core_DebugAssert(mem_recorder != nullptr);

    delete mem_recorder;
}

size_t MemRecorder::GetSize(void const* mem_recorder_) {
    auto mem_recorder{ static_cast<MemRecorder const*>(mem_recorder_) };
    ZETA_Core_DebugAssert(mem_recorder != nullptr);

    return mem_recorder->records.size();
}

size_t MemRecorder::GetRecordSize(void const* mem_recorder_, void const* ptr) {
    auto mem_recorder{ static_cast<MemRecorder const*>(mem_recorder_) };
    ZETA_Core_DebugAssert(mem_recorder != nullptr);

    auto iter{ mem_recorder->records.find(ptr) };

    return iter == mem_recorder->records.end() ? ZETA_Core_size_max
                                               : iter->second;
}

bool MemRecorder::IsRecorded(void const* mem_recorder_, void const* ptr) {
    auto mem_recorder{ static_cast<MemRecorder const*>(mem_recorder_) };
    ZETA_Core_DebugAssert(mem_recorder != nullptr);

    return mem_recorder->records.find(ptr) != mem_recorder->records.end();
}

void MemRecorder::Record(void* mem_recorder_, void const* ptr, size_t size) {
    auto mem_recorder{ static_cast<MemRecorder*>(mem_recorder_) };
    ZETA_Core_DebugAssert(mem_recorder != nullptr);

    auto iter{ mem_recorder->records.lower_bound(ptr) };

    if (iter != mem_recorder->records.end()) {
        ZETA_Core_DebugAssert(iter->first != ptr);
        ZETA_Core_DebugAssert(static_cast<char const*>(ptr) + size <=
                              iter->first);
    }

    if (iter != mem_recorder->records.begin()) {
        --iter;
        ZETA_Core_DebugAssert(
            static_cast<char const*>(iter->first) + iter->second <= ptr);
    }

    bool b{ mem_recorder->records.insert({ ptr, size }).second };

    ZETA_Core_DebugAssert(b);
}

bool MemRecorder::Unrecord(void* mem_recorder_, void const* ptr) {
    auto mem_recorder{ static_cast<MemRecorder*>(mem_recorder_) };
    ZETA_Core_DebugAssert(mem_recorder != nullptr);

    return mem_recorder->records.erase(ptr) != 0;
}

void MemRecorder::Clear(void* mem_recorder_) {
    auto mem_recorder{ static_cast<MemRecorder*>(mem_recorder_) };
    ZETA_Core_DebugAssert(mem_recorder != nullptr);

    mem_recorder->records.clear();
}

void MemRecorder::MatchRecords(MemRecorder const* src_mem_recorder,
                               MemRecorder const* dst_mem_recorder) {
    ZETA_Core_DebugAssert(src_mem_recorder != nullptr);
    ZETA_Core_DebugAssert(dst_mem_recorder != nullptr);

    if (src_mem_recorder->records.size() != dst_mem_recorder->records.size()) {
        ZETA_Core_PrintVar(src_mem_recorder->records.size());
        ZETA_Core_PrintVar(dst_mem_recorder->records.size());
    }

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
