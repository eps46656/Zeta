#include <zeta/core/debugger.h>
#include <zeta/core/mem_check_utils.h>
#include <zeta/core/utils.h>

Zeta_Core_MemRecorder* Zeta_Core_MemRecorder_Create() {
    return new Zeta_Core_MemRecorder{};
}

void Zeta_Core_MemRecorder_Destroy(Zeta_Core_MemRecorder* mem_recorder) {
    delete mem_recorder;
}

size_t Zeta_Core_MemRecorder_GetSize(Zeta_Core_MemRecorder* mem_recorder) {
    ZETA_Core_DebugAssert(mem_recorder != NULL);

    return mem_recorder->records.size();
}

size_t Zeta_Core_MemRecorder_GetRecordSize(Zeta_Core_MemRecorder* mem_recorder,
                                           void const* ptr) {
    ZETA_Core_DebugAssert(mem_recorder != NULL);

    auto iter{ mem_recorder->records.find(ptr) };

    return iter == mem_recorder->records.end() ? ZETA_Core_size_max
                                               : iter->second;
}

bool_t Zeta_Core_MemRecorder_IsRecorded(Zeta_Core_MemRecorder* mem_recorder,
                                        void const* ptr) {
    ZETA_Core_DebugAssert(mem_recorder != NULL);

    return mem_recorder->records.find(ptr) != mem_recorder->records.end();
}

void Zeta_Core_MemRecorder_Record(Zeta_Core_MemRecorder* mem_recorder,
                                  void const* ptr, size_t size) {
    ZETA_Core_DebugAssert(mem_recorder != NULL);

    auto iter{ mem_recorder->records.lower_bound(ptr) };

    if (iter != mem_recorder->records.end()) {
        ZETA_Core_DebugAssert(iter->first != ptr);
        ZETA_Core_DebugAssert((unsigned char const*)ptr + size <= iter->first);
    }

    if (iter != mem_recorder->records.begin()) {
        --iter;
        ZETA_Core_DebugAssert(
            (unsigned char const*)iter->first + iter->second <= ptr);
    }

    mem_recorder->records.insert({ ptr, size });
}

bool_t Zeta_Core_MemRecorder_Unrecord(Zeta_Core_MemRecorder* mem_recorder,
                                      void const* ptr) {
    ZETA_Core_DebugAssert(mem_recorder != NULL);

    return mem_recorder->records.erase(ptr) != 0;
}

void Zeta_Core_MemRecorder_Clear(Zeta_Core_MemRecorder* mem_recorder) {
    ZETA_Core_DebugAssert(mem_recorder != NULL);

    mem_recorder->records.clear();
}

void Zeta_Core_MemCheck_MatchRecords(
    Zeta_Core_MemRecorder const* src_mem_recorder,
    Zeta_Core_MemRecorder const* dst_mem_recorder) {
    ZETA_Core_DebugAssert(src_mem_recorder != NULL);
    ZETA_Core_DebugAssert(dst_mem_recorder != NULL);

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
