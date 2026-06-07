#include <utility>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_recorder.hpp>

namespace zeta::core {

mem_recorder::MemRecorder* mem_recorder::Create() { return new MemRecorder{}; }

void mem_recorder::Destroy(MemRecorder* mr) { delete mr; }

size_t mem_recorder::GetSize(MemRecorder const& mr) {
    return mr.records.size();
}

size_t mem_recorder::GetRecordSize(MemRecorder const& mr, void const* ptr) {
    auto iter{ mr.records.find(ptr) };

    return iter == mr.records.end() ? integral::RangeMaxOf<size_t>
                                    : iter->second;
}

bool mem_recorder::IsRecorded(MemRecorder const& mr, void const* ptr) {
    return mr.records.contains(ptr);
}

void mem_recorder::Record(MemRecorder& mr, void const* ptr, size_t size) {
    auto iter{ mr.records.lower_bound(ptr) };

    if (iter != mr.records.end()) {
        ZETA_Core_DebugAssert(iter->first != ptr);
        ZETA_Core_DebugAssert(static_cast<char const*>(ptr) + size <=
                              iter->first);
    }

    if (iter != mr.records.begin()) {
        --iter;
        ZETA_Core_DebugAssert(
            static_cast<char const*>(iter->first) + iter->second <= ptr);
    }

    bool b{ mr.records.insert({ ptr, size }).second };

    ZETA_Core_DebugAssert(b);
}

bool mem_recorder::Unrecord(MemRecorder& mr, void const* ptr) {
    return mr.records.erase(ptr) != 0;
}

void mem_recorder::Clear(MemRecorder& mr) { mr.records.clear(); }

void mem_recorder::MatchRecords(MemRecorder const& src_mr,
                                MemRecorder const& dst_mr) {
    if (src_mr.records.size() != dst_mr.records.size()) {
        ZETA_Core_PrintVar(src_mr.records.size());
        ZETA_Core_PrintVar(dst_mr.records.size());
    }

    ZETA_Core_DebugAssert(src_mr.records.size() == dst_mr.records.size());

    for (auto src_iter{ src_mr.records.begin() },
         src_end{ src_mr.records.end() }, dst_iter{ dst_mr.records.begin() };
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
