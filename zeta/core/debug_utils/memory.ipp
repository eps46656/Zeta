#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/memory.hpp>

namespace zeta::core::debug_utils {

constexpr memory::MemRecorder::MemRecorder() : byte_cnt{ 0 } {}

constexpr size_t memory::MemRecorder::GetBlockCnt(this MemRecorder const& mr) {
    return mr.blocks.size();
}

constexpr size_t memory::MemRecorder::GetByteCnt(this MemRecorder const& mr) {
    return mr.byte_cnt;
}

constexpr void memory::MemRecorder::Add(this MemRecorder& mr, void const* ptr,
                                        size_t size) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(ptr != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < size);

    auto iter{ mr.blocks.lower_bound(ptr) };

    if (iter != mr.blocks.end()) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            static_cast<char const*>(ptr) + size <= iter->first);
    }

    if (iter != mr.blocks.begin()) {
        --iter;

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            static_cast<char const*>(iter->first) + iter->second <= ptr);
    }

    bool b{ mr.blocks.insert({ ptr, size }).second };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(b);

    mr.byte_cnt += size;
}

constexpr void memory::MemRecorder::Remove(this MemRecorder& mr,
                                           void const* ptr) {
    auto iter{ mr.blocks.find(ptr) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter != mr.blocks.end());

    mr.byte_cnt -= iter->second;

    mr.blocks.erase(iter);
}

constexpr void memory::MemRecorder::Clear(this MemRecorder& mr) {
    mr.blocks.clear();
    mr.byte_cnt = 0;
}

constexpr void memory::MemRecorder::Contain(this MemRecorder const& mr,
                                            void const* ptr, size_t size) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(ptr != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < size);

    auto iter{ mr.blocks.find(ptr) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter != mr.blocks.end());

    ZETA_Core_DebugUtils_Diag_PromiseAssert(size <= iter->second);
}

constexpr void memory::MemRecorder::InChargeOf(this MemRecorder const& mr,
                                               MemRecorder const& resp_mr) {
    ZETA_Core_DebugUtils_Diag_LogVar(mr.blocks.size());
    ZETA_Core_DebugUtils_Diag_LogVar(resp_mr.blocks.size());

    auto mr_iter{ mr.blocks.begin() };
    auto mr_end{ mr.blocks.end() };
    auto resp_mr_iter{ resp_mr.blocks.begin() };
    auto resp_mr_end{ resp_mr.blocks.end() };

    for (;; ++mr_iter, ++resp_mr_iter) {
        bool mr_is_end{ mr_iter == mr_end };

        bool resp_mr_is_end{ resp_mr_iter == resp_mr_end };

        if (mr_is_end && resp_mr_is_end) { break; }

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            !(!mr_is_end && resp_mr_is_end));
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            !(mr_iter->first < resp_mr_iter->first));
        // hallucination or use after free: mr uses unallocated memory.

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            !(mr_is_end && !resp_mr_is_end));
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            !(resp_mr_iter->first < mr_iter->first));
        // memory leak: mr misses allocated memory.

        ZETA_Core_DebugUtils_Diag_PromiseAssert(mr_iter->second <=
                                                resp_mr_iter->second);
        // overflow: mr uses more than allocated memory.
    }
}

constexpr void memory::MemRecorder::InChargeOf(
    this MemRecorder const& mr, MemRecorderClient const& resp_mrc) {
    mr.InChargeOf(resp_mrc.GetMemRecorder());
}

constexpr size_t memory::MemRecorderServer::GetBlockCnt(
    this MemRecorderServer const& mrs, std::string const& group_name) {
    auto iter{ mrs.group_name_to_mem_recorder.find(group_name) };

    return iter == mrs.group_name_to_mem_recorder.end()
               ? static_cast<size_t>(-1)
               : iter->second.GetBlockCnt();
}

constexpr size_t memory::MemRecorderServer::GetByteCnt(
    this MemRecorderServer const& mrs, std::string const& group_name) {
    auto iter{ mrs.group_name_to_mem_recorder.find(group_name) };

    return iter == mrs.group_name_to_mem_recorder.end()
               ? static_cast<size_t>(-1)
               : iter->second.GetByteCnt();
}

constexpr std::pair<memory::MemRecorder&, bool>
memory::MemRecorderServer::AddGroup(this MemRecorderServer& mrs,
                                    std::string const& group_name) {
    auto [iter,
          added]{ mrs.group_name_to_mem_recorder.try_emplace(group_name) };

    return { iter->second, added };
}

constexpr void memory::MemRecorderServer::AddBlock(
    this MemRecorderServer& mrs, std::string const& group_name, void const* ptr,
    size_t size) {
    return mrs.AddGroup(group_name).first.Add(ptr, size);
}

constexpr bool memory::MemRecorderServer::RemoveGroup(
    this MemRecorderServer& mrs, std::string const& group_name) {
    return mrs.group_name_to_mem_recorder.erase(group_name) != 0;
}

constexpr void memory::MemRecorderServer::RemoveBlock(
    this MemRecorderServer& mrs, std::string const& group_name,
    void const* ptr) {
    auto iter{ mrs.group_name_to_mem_recorder.find(group_name) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        iter != mrs.group_name_to_mem_recorder.end());

    iter->second.Remove(ptr);
}

constexpr void memory::MemRecorderServer::Contain(
    this MemRecorderServer const& mrs, std::string const& group_name,
    void const* ptr, size_t size) {
    auto iter{ mrs.group_name_to_mem_recorder.find(group_name) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        iter != mrs.group_name_to_mem_recorder.end());

    iter->second.Contain(ptr, size);
}

constexpr memory::MemRecorderClient
memory::MemRecorderServer::MakeMemRecorderClient(
    this MemRecorderServer& mrs, std::string const& group_name) {
    return { &mrs, group_name };
}

constexpr memory::MemRecorderClient::MemRecorderClient(
    MemRecorderServer* server, std::string const& group_name)
    : server{ server }, group_name{ group_name } {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(server != nullptr);
}

constexpr memory::MemRecorderClient
memory::MemRecorderClient::GetSubGroupClient(
    this MemRecorderClient const& mrc, std::string const& sub_group_name) {
    return {
        mrc.server,
        mrc.group_name + "/" + sub_group_name,
    };
}

constexpr memory::MemRecorder& memory::MemRecorderClient::GetMemRecorder(
    this MemRecorderClient const& mrc) {
    return mrc.server->AddGroup(mrc.group_name).first;
}

constexpr size_t memory::MemRecorderClient::GetBlockCnt(
    this MemRecorderClient const& mrc) {
    return mrc.server->GetBlockCnt(mrc.group_name);
}

constexpr size_t memory::MemRecorderClient::GetByteCnt(
    this MemRecorderClient const& mrc) {
    return mrc.server->GetByteCnt(mrc.group_name);
}

constexpr void memory::MemRecorderClient::Add(this MemRecorderClient& mrc,
                                              void const* ptr, size_t size) {
    mrc.server->AddBlock(mrc.group_name, ptr, size);
}

constexpr void memory::MemRecorderClient::Remove(this MemRecorderClient& mrc,
                                                 void const* ptr) {
    mrc.server->RemoveBlock(mrc.group_name, ptr);
}

constexpr void memory::MemRecorderClient::Clear(this MemRecorderClient& mrc) {
    mrc.server->RemoveGroup(mrc.group_name);
}

constexpr void memory::MemRecorderClient::Contain(
    this MemRecorderClient const& mrc, void const* ptr, size_t size) {
    mrc.server->Contain(mrc.group_name, ptr, size);
}

constexpr void memory::MemRecorderClient::InChargeOf(
    this MemRecorderClient const& mrc, MemRecorder const& resp_mr) {
    mrc.GetMemRecorder().InChargeOf(resp_mr);
}

constexpr void memory::MemRecorderClient::InChargeOf(
    this MemRecorderClient const& mrc, MemRecorderClient const& resp_mrc) {
    mrc.GetMemRecorder().InChargeOf(resp_mrc.GetMemRecorder());
}

}  // namespace zeta::core::debug_utils
