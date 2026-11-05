#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/mem_recording.hpp>

namespace zeta::core::debug_utils {

constexpr mem_recording::MemRecorder::MemRecorder() : byte_cnt{ 0 } {}

constexpr size_t mem_recording::MemRecorder::GetBlockCnt(
    this MemRecorder const& self) {
    return self.blocks.size();
}

constexpr size_t mem_recording::MemRecorder::GetByteCnt(
    this MemRecorder const& self) {
    return self.byte_cnt;
}

constexpr size_t mem_recording::MemRecorder::GetSize(
    this MemRecorder const& self, void const* ptr) {
    auto iter{ self.blocks.find(ptr) };

    return iter == self.blocks.end() ? 0 : iter->second;
}

constexpr void mem_recording::MemRecorder::Add(this MemRecorder& self,
                                               void const* ptr, size_t size) {
    if (ptr == nullptr || size == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(ptr != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < size);

    auto iter{ self.blocks.lower_bound(ptr) };

    if (iter != self.blocks.end()) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            static_cast<char const*>(ptr) + size <= iter->first);
    }

    if (iter != self.blocks.begin()) {
        --iter;

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            static_cast<char const*>(iter->first) + iter->second <= ptr);
    }

    bool b{ self.blocks.insert({ ptr, size }).second };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(b);

    self.byte_cnt += size;
}

constexpr void mem_recording::MemRecorder::Remove(this MemRecorder& self,
                                                  void const* ptr) {
    if (ptr == nullptr) { return; }

    auto iter{ self.blocks.find(ptr) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter != self.blocks.end());

    self.byte_cnt -= iter->second;

    self.blocks.erase(iter);
}

constexpr void mem_recording::MemRecorder::Clear(this MemRecorder& self) {
    self.blocks.clear();
    self.byte_cnt = 0;
}

constexpr void mem_recording::MemRecorder::Contain(this MemRecorder const& self,
                                                   void const* ptr,
                                                   size_t size) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(ptr != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < size);

    auto iter{ self.blocks.find(ptr) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter != self.blocks.end());

    ZETA_Core_DebugUtils_Diag_PromiseAssert(size <= iter->second);
}

constexpr void mem_recording::MemRecorder::InChargeOf(
    this MemRecorder const& self, MemRecorder const& resp_mr) {
    auto mr_iter{ self.blocks.begin() };
    auto mr_end{ self.blocks.end() };
    auto resp_mr_iter{ resp_mr.blocks.begin() };
    auto resp_mr_end{ resp_mr.blocks.end() };

    for (;; ++mr_iter, ++resp_mr_iter) {
        bool mr_is_end{ mr_iter == mr_end };

        bool resp_mr_is_end{ resp_mr_iter == resp_mr_end };

        if (mr_is_end && resp_mr_is_end) { break; }

        if (!mr_is_end && resp_mr_is_end) {
            ZETA_Core_DebugUtils_Diag_LogVar(mr_iter->first);
            ZETA_Core_DebugUtils_Diag_LogVar(mr_iter->second);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                !(!mr_is_end && resp_mr_is_end));

            // hallucination or use after free: mr uses unallocated
            // mem_recording.
        }

        if (mr_iter->first < resp_mr_iter->first) {
            ZETA_Core_DebugUtils_Diag_LogVar(mr_iter->first);
            ZETA_Core_DebugUtils_Diag_LogVar(mr_iter->second);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                !(mr_iter->first < resp_mr_iter->first));

            // hallucination or use after free: mr uses unallocated
            // mem_recording.
        }

        if (mr_is_end && !resp_mr_is_end) {
            ZETA_Core_DebugUtils_Diag_LogVar(resp_mr_iter->first);
            ZETA_Core_DebugUtils_Diag_LogVar(resp_mr_iter->second);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                !(mr_is_end && !resp_mr_is_end));

            // mem_recording leak: mr misses allocated mem_recording.
        }

        if (resp_mr_iter->first < mr_iter->first) {
            ZETA_Core_DebugUtils_Diag_LogVar(resp_mr_iter->first);
            ZETA_Core_DebugUtils_Diag_LogVar(resp_mr_iter->second);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                !(resp_mr_iter->first < mr_iter->first));

            // mem_recording leak: mr misses allocated mem_recording.
        }

        if (resp_mr_iter->second < mr_iter->second) {
            ZETA_Core_DebugUtils_Diag_LogVar(resp_mr_iter->first);
            ZETA_Core_DebugUtils_Diag_LogVar(resp_mr_iter->second);
            ZETA_Core_DebugUtils_Diag_LogVar(mr_iter->first);
            ZETA_Core_DebugUtils_Diag_LogVar(mr_iter->second);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                !(resp_mr_iter->second < mr_iter->second));

            // overflow: mr uses more than allocated mem_recording.
        }
    }
}

constexpr void mem_recording::MemRecorder::InChargeOf(
    this MemRecorder const& self, MemRecorderClient const& resp_mrc) {
    self.InChargeOf(resp_mrc.GetMemRecorder());
}

constexpr size_t mem_recording::MemRecorderServer::GetBlockCnt(
    this MemRecorderServer const& self, identity_graph::Id group_id) {
    if (group_id == identity_graph::null_id) { return 0; }

    auto iter{ self.group_id_to_mem_recorder.find(group_id) };

    return iter == self.group_id_to_mem_recorder.end()
               ? 0
               : iter->second.GetBlockCnt();
}

constexpr size_t mem_recording::MemRecorderServer::GetByteCnt(
    this MemRecorderServer const& self, identity_graph::Id group_id) {
    if (group_id == identity_graph::null_id) { return 0; }

    auto iter{ self.group_id_to_mem_recorder.find(group_id) };

    return iter == self.group_id_to_mem_recorder.end()
               ? 0
               : iter->second.GetByteCnt();
}

constexpr std::pair<mem_recording::MemRecorder&, bool>
mem_recording::MemRecorderServer::AddGroup(this MemRecorderServer& self,
                                           identity_graph::Id group_id) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(group_id !=
                                            identity_graph::null_id);

    auto [iter, added]{ self.group_id_to_mem_recorder.try_emplace(group_id) };

    return { iter->second, added };
}

constexpr void mem_recording::MemRecorderServer::AddBlock(
    this MemRecorderServer& self, identity_graph::Id group_id, void const* ptr,
    size_t size) {
    if (group_id == identity_graph::null_id) { return; }

    self.AddGroup(group_id).first.Add(ptr, size);
}

constexpr bool mem_recording::MemRecorderServer::RemoveGroup(
    this MemRecorderServer& self, identity_graph::Id group_id) {
    if (group_id == identity_graph::null_id) { return 0; }

    return self.group_id_to_mem_recorder.erase(group_id) == 1;
}

constexpr void mem_recording::MemRecorderServer::RemoveBlock(
    this MemRecorderServer& self, identity_graph::Id group_id,
    void const* ptr) {
    if (group_id == identity_graph::null_id) { return; }

    auto iter{ self.group_id_to_mem_recorder.find(group_id) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        iter != self.group_id_to_mem_recorder.end());

    iter->second.Remove(ptr);
}

constexpr void mem_recording::MemRecorderServer::Contain(
    this MemRecorderServer const& self, identity_graph::Id group_id,
    void const* ptr, size_t size) {
    if (group_id == identity_graph::null_id) { return; }

    auto iter{ self.group_id_to_mem_recorder.find(group_id) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        iter != self.group_id_to_mem_recorder.end());

    iter->second.Contain(ptr, size);
}

constexpr mem_recording::MemRecorderClient
mem_recording::MemRecorderServer::MakeMemRecorderClient(
    this MemRecorderServer& self, identity_graph::Id group_id) {
    return { &self, group_id };
}

constexpr mem_recording::MemRecorderClient::MemRecorderClient(
    MemRecorderServer* server, identity_graph::Id group_id)
    : server{ server }, group_id{ group_id } {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(server != nullptr);
}

constexpr mem_recording::MemRecorderClient
mem_recording::MemRecorderClient::GetSubGroupClient(
    this MemRecorderClient const& self, std::string const& name) {
    identity_graph::Id sub_group_id{ identity_graph::GetAdj(self.group_id, true,
                                                            name) };

    return {
        self.server,
        sub_group_id == identity_graph::null_id ? identity_graph::null_id
                                                : sub_group_id,
    };
}

constexpr mem_recording::MemRecorder&
mem_recording::MemRecorderClient::GetMemRecorder(
    this MemRecorderClient const& self) {
    return self.server->AddGroup(self.group_id).first;
}

constexpr size_t mem_recording::MemRecorderClient::GetBlockCnt(
    this MemRecorderClient const& self) {
    return self.server->GetBlockCnt(self.group_id);
}

constexpr size_t mem_recording::MemRecorderClient::GetByteCnt(
    this MemRecorderClient const& self) {
    return self.server->GetByteCnt(self.group_id);
}

constexpr void mem_recording::MemRecorderClient::Add(
    this MemRecorderClient& self, void const* ptr, size_t size) {
    self.server->AddBlock(self.group_id, ptr, size);
}

constexpr void mem_recording::MemRecorderClient::Remove(
    this MemRecorderClient& self, void const* ptr) {
    self.server->RemoveBlock(self.group_id, ptr);
}

constexpr void mem_recording::MemRecorderClient::Clear(
    this MemRecorderClient& self) {
    self.server->RemoveGroup(self.group_id);
}

constexpr void mem_recording::MemRecorderClient::Contain(
    this MemRecorderClient const& self, void const* ptr, size_t size) {
    self.server->Contain(self.group_id, ptr, size);
}

constexpr void mem_recording::MemRecorderClient::InChargeOf(
    this MemRecorderClient const& self, MemRecorder const& resp_mr) {
    self.GetMemRecorder().InChargeOf(resp_mr);
}

constexpr void mem_recording::MemRecorderClient::InChargeOf(
    this MemRecorderClient const& self, MemRecorderClient const& resp_mrc) {
    self.GetMemRecorder().InChargeOf(resp_mrc.GetMemRecorder());
}

}  // namespace zeta::core::debug_utils
