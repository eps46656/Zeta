#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core::mem_recorder {

namespace ops {

size_t GetSize(MemRecorder const* mr) { return mr->records.size(); }

size_t GetRecordSize(MemRecorder const* mr, void const* ptr) {
    ZETA_Core_DebugAssert(mr != nullptr);

    auto iter{ mr->records.find(ptr) };

    return iter == mr->records.end() ? integral::RangeMaxOf<size_t>
                                     : iter->second;
}

bool IsRecorded(MemRecorder const* mr, void const* ptr) {
    ZETA_Core_DebugAssert(mr != nullptr);

    return mr->records.contains(ptr);
}

void Record(MemRecorder* mr, void const* ptr, size_t size) {
    ZETA_Core_DebugAssert(mr != nullptr);

    auto iter{ mr->records.lower_bound(ptr) };

    if (iter != mr->records.end()) {
        ZETA_Core_DebugAssert(iter->first != ptr);
        ZETA_Core_DebugAssert(static_cast<char const*>(ptr) + size <=
                              iter->first);
    }

    if (iter != mr->records.begin()) {
        --iter;
        ZETA_Core_DebugAssert(
            static_cast<char const*>(iter->first) + iter->second <= ptr);
    }

    bool b{ mr->records.insert({ ptr, size }).second };

    ZETA_Core_DebugAssert(b);
}

bool Unrecord(MemRecorder* mr, void const* ptr) {
    ZETA_Core_DebugAssert(mr != nullptr);

    return mr->records.erase(ptr) != 0;
}

void Clear(MemRecorder* mr) {
    ZETA_Core_DebugAssert(mr != nullptr);

    mr->records.clear();
}

void MatchRecords(MemRecorder const* src_mr, MemRecorder const* dst_mr) {
    ZETA_Core_DebugAssert(src_mr != nullptr);
    ZETA_Core_DebugAssert(dst_mr != nullptr);

    if (src_mr->records.size() != dst_mr->records.size()) {
        ZETA_Core_PrintVar(src_mr->records.size());
        ZETA_Core_PrintVar(dst_mr->records.size());
    }

    ZETA_Core_DebugAssert(src_mr->records.size() == dst_mr->records.size());

    for (auto src_iter{ src_mr->records.begin() },
         src_end{ src_mr->records.end() }, dst_iter{ dst_mr->records.begin() };
         src_iter != src_end; ++src_iter, ++dst_iter) {
        ZETA_Core_DebugAssert(!(src_iter->first < dst_iter->first));
        // memory leek: dst misses allocated memory.

        ZETA_Core_DebugAssert(!(dst_iter->first < src_iter->first));
        // hallucination or use after free: dst uses unallocated memoty.

        ZETA_Core_DebugAssert(dst_iter->second <= src_iter->second);
        // overflow: dst uses more than allocated memory.
    }
}

}  // namespace ops
}  // namespace zeta::core::mem_recorder
