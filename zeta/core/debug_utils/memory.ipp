#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/memory.hpp>

namespace zeta::core::debug_utils {

constexpr memory::MemRecorder::MemRecorder() : byte_cnt{ 0 } {}

constexpr size_t memory::MemRecorder::GetBlockCnt(
    this MemRecorder const& self) {
    return self.blocks.size();
}

constexpr size_t memory::MemRecorder::GetByteCnt(this MemRecorder const& self) {
    return self.byte_cnt;
}

constexpr size_t memory::MemRecorder::GetSize(this MemRecorder const& self,
                                              void const* ptr) {
    auto iter{ self.blocks.find(ptr) };

    return iter == self.blocks.end() ? 0 : iter->second;
}

constexpr void memory::MemRecorder::Add(this MemRecorder& self, void const* ptr,
                                        size_t size) {
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

constexpr void memory::MemRecorder::Remove(this MemRecorder& self,
                                           void const* ptr) {
    auto iter{ self.blocks.find(ptr) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter != self.blocks.end());

    self.byte_cnt -= iter->second;

    self.blocks.erase(iter);
}

constexpr void memory::MemRecorder::Clear(this MemRecorder& self) {
    self.blocks.clear();
    self.byte_cnt = 0;
}

constexpr void memory::MemRecorder::Contain(this MemRecorder const& self,
                                            void const* ptr, size_t size) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(ptr != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < size);

    auto iter{ self.blocks.find(ptr) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(iter != self.blocks.end());

    ZETA_Core_DebugUtils_Diag_PromiseAssert(size <= iter->second);
}

constexpr void memory::MemRecorder::InChargeOf(this MemRecorder const& self,
                                               MemRecorder const& resp_mr) {
    ZETA_Core_DebugUtils_Diag_LogVar(self.blocks.size());
    ZETA_Core_DebugUtils_Diag_LogVar(resp_mr.blocks.size());

    auto mr_iter{ self.blocks.begin() };
    auto mr_end{ self.blocks.end() };
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
    this MemRecorder const& self, MemRecorderClient const& resp_mrc) {
    self.InChargeOf(resp_mrc.GetMemRecorder());
}

constexpr size_t memory::MemRecorderServer::GetBlockCnt(
    this MemRecorderServer const& self, identity_graph::Id group_id) {
    if (group_id == identity_graph::null_id) { return 0; }

    auto iter{ self.group_id_to_mem_recorder.find(group_id) };

    return iter == self.group_id_to_mem_recorder.end()
               ? static_cast<size_t>(-1)
               : iter->second.GetBlockCnt();
}

constexpr size_t memory::MemRecorderServer::GetByteCnt(
    this MemRecorderServer const& self, identity_graph::Id group_id) {
    if (group_id == identity_graph::null_id) { return 0; }

    auto iter{ self.group_id_to_mem_recorder.find(group_id) };

    return iter == self.group_id_to_mem_recorder.end()
               ? static_cast<size_t>(-1)
               : iter->second.GetByteCnt();
}

constexpr std::pair<memory::MemRecorder&, bool>
memory::MemRecorderServer::AddGroup(this MemRecorderServer& self,
                                    identity_graph::Id group_id) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(group_id !=
                                            identity_graph::null_id);

    auto [iter, added]{ self.group_id_to_mem_recorder.try_emplace(group_id) };

    return { iter->second, added };
}

constexpr void memory::MemRecorderServer::AddBlock(this MemRecorderServer& self,
                                                   identity_graph::Id group_id,
                                                   void const* ptr,
                                                   size_t size) {
    if (group_id == identity_graph::null_id) { return; }

    self.AddGroup(group_id).first.Add(ptr, size);
}

constexpr bool memory::MemRecorderServer::RemoveGroup(
    this MemRecorderServer& self, identity_graph::Id group_id) {
    if (group_id == identity_graph::null_id) { return 0; }

    return self.group_id_to_mem_recorder.erase(group_id) != 0;
}

constexpr void memory::MemRecorderServer::RemoveBlock(
    this MemRecorderServer& self, identity_graph::Id group_id,
    void const* ptr) {
    if (group_id == identity_graph::null_id) { return; }

    auto iter{ self.group_id_to_mem_recorder.find(group_id) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        iter != self.group_id_to_mem_recorder.end());

    iter->second.Remove(ptr);
}

constexpr void memory::MemRecorderServer::Contain(
    this MemRecorderServer const& self, identity_graph::Id group_id,
    void const* ptr, size_t size) {
    if (group_id == identity_graph::null_id) { return; }

    auto iter{ self.group_id_to_mem_recorder.find(group_id) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        iter != self.group_id_to_mem_recorder.end());

    iter->second.Contain(ptr, size);
}

constexpr memory::MemRecorderClient
memory::MemRecorderServer::MakeMemRecorderClient(this MemRecorderServer& self,
                                                 identity_graph::Id group_id) {
    return { &self, group_id };
}

constexpr memory::MemRecorderClient::MemRecorderClient(
    MemRecorderServer* server, identity_graph::Id group_id)
    : server{ server }, group_id{ group_id } {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(server != nullptr);
}

constexpr memory::MemRecorderClient
memory::MemRecorderClient::GetSubGroupClient(this MemRecorderClient const& self,
                                             std::string const& name) {
    identity_graph::Node* sub_group_node{ identity_graph::GetAdj(
        identity_graph::GetNode(self.group_id), true, name) };

    return {
        self.server,
        sub_group_node == nullptr ? identity_graph::null_id
                                  : sub_group_node->id,
    };
}

constexpr memory::MemRecorder& memory::MemRecorderClient::GetMemRecorder(
    this MemRecorderClient const& self) {
    return self.server->AddGroup(self.group_id).first;
}

constexpr size_t memory::MemRecorderClient::GetBlockCnt(
    this MemRecorderClient const& self) {
    return self.server->GetBlockCnt(self.group_id);
}

constexpr size_t memory::MemRecorderClient::GetByteCnt(
    this MemRecorderClient const& self) {
    return self.server->GetByteCnt(self.group_id);
}

constexpr void memory::MemRecorderClient::Add(this MemRecorderClient& self,
                                              void const* ptr, size_t size) {
    self.server->AddBlock(self.group_id, ptr, size);
}

constexpr void memory::MemRecorderClient::Remove(this MemRecorderClient& self,
                                                 void const* ptr) {
    self.server->RemoveBlock(self.group_id, ptr);
}

constexpr void memory::MemRecorderClient::Clear(this MemRecorderClient& self) {
    self.server->RemoveGroup(self.group_id);
}

constexpr void memory::MemRecorderClient::Contain(
    this MemRecorderClient const& self, void const* ptr, size_t size) {
    self.server->Contain(self.group_id, ptr, size);
}

constexpr void memory::MemRecorderClient::InChargeOf(
    this MemRecorderClient const& self, MemRecorder const& resp_mr) {
    self.GetMemRecorder().InChargeOf(resp_mr);
}

constexpr void memory::MemRecorderClient::InChargeOf(
    this MemRecorderClient const& self, MemRecorderClient const& resp_mrc) {
    self.GetMemRecorder().InChargeOf(resp_mrc.GetMemRecorder());
}

}  // namespace zeta::core::debug_utils
