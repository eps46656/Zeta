#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <zeta/core/debug_utils/identity_graph.ipp>

namespace zeta::core::debug_utils::memory {

struct MemRecorder;
struct MemRecorderServer;
struct MemRecorderClient;

struct MemRecorder {
    std::map<void const*, size_t> blocks;
    size_t byte_cnt;

    constexpr MemRecorder();

    constexpr MemRecorder(MemRecorder const& mr) = default;

    constexpr size_t GetBlockCnt(this MemRecorder const& self);

    constexpr size_t GetByteCnt(this MemRecorder const& self);

    constexpr size_t GetSize(this MemRecorder const& self, void const* ptr);

    constexpr void Add(this MemRecorder& self, void const* ptr, size_t size);

    constexpr void Remove(this MemRecorder& self, void const* ptr);

    constexpr void Clear(this MemRecorder& self);

    constexpr void Contain(this MemRecorder const& self, void const* ptr,
                           size_t size);

    constexpr void InChargeOf(this MemRecorder const& self,
                              MemRecorder const& resp_mr);

    constexpr void InChargeOf(this MemRecorder const& self,
                              MemRecorderClient const& resp_mrc);
};

struct MemRecorderServer {
    std::unordered_map<identity_graph::Id, MemRecorder>
        group_id_to_mem_recorder;

    constexpr size_t GetBlockCnt(this MemRecorderServer const& self,
                                 identity_graph::Id group_id);

    constexpr size_t GetByteCnt(this MemRecorderServer const& self,
                                identity_graph::Id group_id);

    constexpr std::pair<memory::MemRecorder&, bool> AddGroup(
        this MemRecorderServer& self, identity_graph::Id group_id);

    constexpr void AddBlock(this MemRecorderServer& self,
                            identity_graph::Id group_id, void const* ptr,
                            size_t size);

    constexpr bool RemoveGroup(this MemRecorderServer& self,
                               identity_graph::Id group_id);

    constexpr void RemoveBlock(this MemRecorderServer& self,
                               identity_graph::Id group_id, void const* ptr);

    constexpr void Contain(this MemRecorderServer const& self,
                           identity_graph::Id group_id, void const* ptr,
                           size_t size);

    constexpr MemRecorderClient MakeMemRecorderClient(
        this MemRecorderServer& self, identity_graph::Id group_id);
};

struct MemRecorderClient {
    MemRecorderServer* server;
    identity_graph::Id group_id;

    constexpr MemRecorderClient(MemRecorderServer* server,
                                identity_graph::Id group_id);

    constexpr MemRecorderClient(MemRecorderClient const& mrc) = default;

    constexpr MemRecorderClient GetSubGroupClient(
        this MemRecorderClient const& self, std::string const& name);

    constexpr MemRecorder& GetMemRecorder(this MemRecorderClient const& self);

    constexpr size_t GetBlockCnt(this MemRecorderClient const& self);

    constexpr size_t GetByteCnt(this MemRecorderClient const& self);

    constexpr void Add(this MemRecorderClient& self, void const* ptr,
                       size_t size);

    constexpr void Remove(this MemRecorderClient& self, void const* ptr);

    constexpr void Clear(this MemRecorderClient& self);

    constexpr void Contain(this MemRecorderClient const& self, void const* ptr,
                           size_t size);

    constexpr void InChargeOf(this MemRecorderClient const& self,
                              MemRecorder const& resp_mr);

    constexpr void InChargeOf(this MemRecorderClient const& self,
                              MemRecorderClient const& resp_mrc);
};

inline MemRecorderServer default_mem_recorder_server;

}  // namespace zeta::core::debug_utils::memory
