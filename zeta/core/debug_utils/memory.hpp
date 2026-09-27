#pragma once

#include <map>
#include <string>
#include <unordered_map>

namespace zeta::core::debug_utils::memory {

struct MemRecorder;
struct MemRecorderServer;
struct MemRecorderClient;

struct MemRecorder {
    std::map<void const*, size_t> blocks;
    size_t byte_cnt;

    constexpr MemRecorder();

    constexpr MemRecorder(MemRecorder const& mr) = default;

    constexpr size_t GetBlockCnt(this MemRecorder const& mr);

    constexpr size_t GetByteCnt(this MemRecorder const& mr);

    constexpr void Add(this MemRecorder& mr, void const* ptr, size_t size);

    constexpr void Remove(this MemRecorder& mr, void const* ptr);

    constexpr void Clear(this MemRecorder& mr);

    constexpr void Contain(this MemRecorder const& mr, void const* ptr,
                           size_t size);

    constexpr void InChargeOf(this MemRecorder const& mr,
                              MemRecorder const& resp_mr);

    constexpr void InChargeOf(this MemRecorder const& mr,
                              MemRecorderClient const& resp_mrc);
};

struct MemRecorderServer {
    std::unordered_map<std::string, MemRecorder> group_name_to_mem_recorder;

    constexpr size_t GetBlockCnt(this MemRecorderServer const& mrs,
                                 std::string const& group_name);

    constexpr size_t GetByteCnt(this MemRecorderServer const& mrs,
                                std::string const& group_name);

    constexpr std::pair<memory::MemRecorder&, bool> AddGroup(
        this MemRecorderServer& mrs, std::string const& group_name);

    constexpr void AddBlock(this MemRecorderServer& mrs,
                            std::string const& group_name, void const* ptr,
                            size_t size);

    constexpr bool RemoveGroup(this MemRecorderServer& mrs,
                               std::string const& group_name);

    constexpr void RemoveBlock(this MemRecorderServer& mrs,
                               std::string const& group_name, void const* ptr);

    constexpr void Contain(this MemRecorderServer const& mrs,
                           std::string const& group_name, void const* ptr,
                           size_t size);

    constexpr MemRecorderClient MakeMemRecorderClient(
        this MemRecorderServer& mrs, std::string const& group_name);
};

struct MemRecorderClient {
    MemRecorderServer* server;
    std::string group_name;

    constexpr MemRecorderClient(MemRecorderServer* server,
                                std::string const& group_name);

    constexpr MemRecorderClient(MemRecorderClient const& mrc) = default;

    constexpr MemRecorderClient GetSubGroupClient(
        this MemRecorderClient const& mrc, std::string const& sub_group_name);

    constexpr MemRecorder& GetMemRecorder(this MemRecorderClient const& mrc);

    constexpr size_t GetBlockCnt(this MemRecorderClient const& mrc);

    constexpr size_t GetByteCnt(this MemRecorderClient const& mrc);

    constexpr void Add(this MemRecorderClient& mrc, void const* ptr,
                       size_t size);

    constexpr void Remove(this MemRecorderClient& mrc, void const* ptr);

    constexpr void Clear(this MemRecorderClient& mrc);

    constexpr void Contain(this MemRecorderClient const& mrc, void const* ptr,
                           size_t size);

    constexpr void InChargeOf(this MemRecorderClient const& mrc,
                              MemRecorder const& resp_mr);

    constexpr void InChargeOf(this MemRecorderClient const& mr,
                              MemRecorderClient const& resp_mrc);
};

inline MemRecorderServer default_mem_recorder_server;

}  // namespace zeta::core::debug_utils::memory
