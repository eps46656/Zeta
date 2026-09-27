#include <vector>
#include <zeta/core/integral.hpp>
#include <zeta/core/object_state_notation.ipp>
#include <zeta/core_test/random.hpp>

using NodeUnsignedIntegral = unsigned _BitInt(64);
using NodeSignedIntegral = signed _BitInt(64);

constexpr size_t region_attr_size{ 16 };

using Region = unsigned _BitInt(8 * region_attr_size);

constexpr Region region_range_min{ 0 };
constexpr Region region_range_max{ zeta::core::integral::RangeMaxOf<Region> };

struct OSNNode {
    zeta::core::object_state_notation::NodeType node_type;
    bool has_name;
    bool has_obj_type;
    bool has_region;

    std::string name;

    std::string obj_type;

    size_t region_beg;
    size_t region_size;

    zeta::core::object_state_notation::IntegralDescriptor integral_descriptor;

    size_t list_elem_cnt;

    NodeUnsignedIntegral u_integral;
    NodeSignedIntegral s_integral;

    union {
        std::vector<NodeUnsignedIntegral> u_integral_list;
        std::vector<NodeSignedIntegral> s_integral_list;

        std::vector<OSNNode*> node_list;
    };

    constexpr OSNNode(zeta::core::object_state_notation::NodeType node_type,
                      bool integral_is_signed, bool integral_size)
        : node_type{ node_type },
          has_name{ false },
          has_obj_type{ false },
          has_region{ false },
          integral_descriptor{
              .is_signed = integral_is_signed,
              .size = integral_size,
          } {
        switch (this->node_type) {
        case zeta::core::object_state_notation::NodeType::Null: break;

        case zeta::core::object_state_notation::NodeType::Integral: break;

        case zeta::core::object_state_notation::NodeType::IntegralList:
            if (this->integral_descriptor.is_signed) {
                new (&this->s_integral_list) decltype(this->s_integral_list){};
            } else {
                new (&this->u_integral_list) decltype(this->u_integral_list){};
            }

            break;

        case zeta::core::object_state_notation::NodeType::NodeList:
            new (&this->node_list) decltype(this->node_list){};

            break;

        case zeta::core::object_state_notation::NodeType::Terminator:
            ZETA_Core_DebugUtils_Diag_Unreachable();
        }
    }
};

constexpr bool operator==(OSNNode const& lhs, OSNNode const& rhs) {
    if (lhs.node_type != rhs.node_type) { return false; }

    if (lhs.has_name != rhs.has_name) { return false; }

    if (lhs.has_name) {
        if (lhs.name != rhs.name) { return false; }
    }

    if (lhs.has_obj_type != rhs.has_obj_type) { return false; }

    if (lhs.has_obj_type) {
        if (lhs.obj_type != rhs.obj_type) { return false; }
    }

    if (lhs.has_region != rhs.has_region) { return false; }

    if (lhs.has_region) {
        if (lhs.region_beg != rhs.region_beg) { return false; }
        if (lhs.region_size != rhs.region_size) { return false; }
    }

    if (lhs.node_type ==
            zeta::core::object_state_notation::NodeType::Integral ||
        lhs.node_type ==
            zeta::core::object_state_notation::NodeType::IntegralList) {
        if (lhs.integral_descriptor.is_signed !=
            rhs.integral_descriptor.is_signed) {
            return false;
        }

        if (lhs.integral_descriptor.size != rhs.integral_descriptor.size) {
            return false;
        }
    }

    if (lhs.node_type ==
        zeta::core::object_state_notation::NodeType::Integral) {
        if (lhs.integral_descriptor.is_signed) {
            if (lhs.s_integral != rhs.s_integral) { return false; }
        } else {
            if (lhs.u_integral != rhs.u_integral) { return false; }
        }
    }

    if (lhs.node_type ==
        zeta::core::object_state_notation::NodeType::IntegralList) {
        if (lhs.list_elem_cnt != rhs.list_elem_cnt) { return false; }

        if (lhs.integral_descriptor.is_signed) {
            if (lhs.s_integral_list.size() != rhs.s_integral_list.size()) {
                return false;
            }

            if (lhs.list_elem_cnt != static_cast<size_t>(-1)) {
                ZETA_Core_DebugUtils_Diag_PromiseAssert(
                    lhs.s_integral_list.size() == lhs.list_elem_cnt);
            }

            if (lhs.s_integral_list != rhs.s_integral_list) { return false; }
        } else {
            if (lhs.u_integral_list.size() != rhs.u_integral_list.size()) {
                return false;
            }

            if (lhs.list_elem_cnt != static_cast<size_t>(-1)) {
                ZETA_Core_DebugUtils_Diag_PromiseAssert(
                    lhs.u_integral_list.size() == lhs.list_elem_cnt);
            }

            if (lhs.u_integral_list != rhs.u_integral_list) { return false; }
        }
    }

    if (lhs.node_type ==
        zeta::core::object_state_notation::NodeType::NodeList) {
        if (lhs.list_elem_cnt != rhs.list_elem_cnt) { return false; }

        if (lhs.node_list.size() != rhs.node_list.size()) { return false; }

        if (lhs.list_elem_cnt != static_cast<size_t>(-1)) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(lhs.node_list.size() ==
                                                    lhs.list_elem_cnt);
        }

        for (size_t i{ 0 }; i < lhs.node_list.size(); ++i) {
            if (*lhs.node_list[i] != *rhs.node_list[i]) { return false; }
        }
    }

    return true;
}

template <typename Integral>
constexpr Integral GenRandomUniformIntegral(size_t width) {
    Integral lb;
    Integral rb;

    if constexpr (zeta::core::integral::IsSignedIntegral<Integral>) {
        lb = static_cast<Integral>(-1) << (width - 1);
        rb = (static_cast<Integral>(1) << (width - 1)) - 1;
    } else {
        lb = 0;
        rb = (static_cast<Integral>(1) << width) - 1;
    }

    return zeta::core_test::GenUniformRandomInt<Integral, Integral>(lb, rb);
}

constexpr std::string GenRandomString() {
    size_t str_size{ zeta::core_test::GenUniformRandomInt<unsigned, unsigned>(
        0, 32) };

    std::string str;

    str.resize(str_size);

    zeta::core_test::GenRandomLinSeq(str.data(), 1, 1, str_size);

    return str;
}

constexpr OSNNode* GenRandomNode(size_t energy) {
    constexpr zeta::core::object_state_notation::NodeType node_types[]{
        zeta::core::object_state_notation::NodeType::Null,
        zeta::core::object_state_notation::NodeType::Integral,
        zeta::core::object_state_notation::NodeType::IntegralList,
        zeta::core::object_state_notation::NodeType::NodeList,
    };

    zeta::core::object_state_notation::NodeType node_type{
        energy == 1 ? node_types[zeta::core_test::GenUniformRandomInt<int, int>(
                          0, sizeof(node_types) / sizeof(node_types[0]) - 1)]
                    : zeta::core::object_state_notation::NodeType::NodeList
    };

    bool has_name{ zeta::core_test::GenUniformRandomInt<int, int>(0, 1) == 0 };
    bool has_obj_type{ zeta::core_test::GenUniformRandomInt<int, int>(0, 1) ==
                       0 };
    bool has_region{ zeta::core_test::GenUniformRandomInt<int, int>(0, 1) ==
                     0 };

    bool integral_is_signed{ zeta::core_test::GenUniformRandomInt<int, int>(
                                 0, 1) == 0 };
    size_t integral_size{
        zeta::core_test::GenUniformRandomInt<unsigned, unsigned>(0, 16)
    };

    OSNNode* node{ new OSNNode(node_type, integral_is_signed, integral_size) };

    node->has_name = has_name;
    node->has_obj_type = has_obj_type;
    node->has_region = has_region;

    if (has_name) {
        node->name = (GenRandomString)();
    } else {
        node->name.clear();
    }

    if (has_obj_type) {
        node->obj_type = (GenRandomString)();
    } else {
        node->obj_type.clear();
    }

    if (has_region) {
        Region a{ zeta::core_test::GenUniformRandomInt<Region>(
            region_range_min, region_range_max) };
        Region b{ zeta::core_test::GenUniformRandomInt<Region>(
            region_range_min, region_range_max) };

        if (b < a) { std::swap(a, b); }

        node->region_beg = a;
        node->region_size = b - a;
    } else {
        node->region_beg = 0;
        node->region_size = 0;
    }

    bool is_variable{ zeta::core_test::GenUniformRandomInt<int, int>(0, 1) ==
                      0 };

    size_t actual_list_elem_cnt{
        node_type == zeta::core::object_state_notation::NodeType::NodeList
            ? zeta::core_test::GenUniformRandomInt<unsigned, unsigned>(0, 8)
            : zeta::core_test::GenUniformRandomInt<unsigned, unsigned>(
                  0, 1024 * 16)
    };

    node->list_elem_cnt =
        is_variable ? static_cast<size_t>(-1) : actual_list_elem_cnt;

    if (node_type == zeta::core::object_state_notation::NodeType::Integral) {
        if (integral_is_signed) {
            node->s_integral =
                (GenRandomUniformIntegral<NodeSignedIntegral>)(integral_size *
                                                               8);
        } else {
            node->u_integral =
                (GenRandomUniformIntegral<NodeUnsignedIntegral>)(integral_size *
                                                                 8);
        }
    }

    if (node_type ==
        zeta::core::object_state_notation::NodeType::IntegralList) {
        for (size_t i{ 0 }; i < actual_list_elem_cnt; ++i) {
            if (integral_is_signed) {
                node->s_integral_list.push_back(
                    (GenRandomUniformIntegral<
                        NodeSignedIntegral>)(integral_size * 8));
            } else {
                node->u_integral_list.push_back(
                    (GenRandomUniformIntegral<
                        NodeUnsignedIntegral>)(integral_size * 8));
            }
        }
    }

    if (node_type == zeta::core::object_state_notation::NodeType::NodeList) {
        std::deque<size_t> partition{ zeta::core_test::GenRandomPartition(
            energy, actual_list_elem_cnt) };

        for (size_t i{ 0 }; i < actual_list_elem_cnt; ++i) {
            node->node_list.push_back((GenRandomNode)(partition[i]));
        }
    }

    return node;
}

template <typename ProviderLike>
constexpr OSNNode* NodeDecodeFromOctets(
    zeta::core::object_state_notation::Config const& config,
    zeta::core::object_state_notation::Decoder<ProviderLike>& decoder) {
    auto node_tag_result{ decoder.ReceiveNodeTag() };

    if (!node_tag_result.HasValue()) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            decoder.state !=
            zeta::core::object_state_notation::DecoderState::Corrupted);

        return nullptr;
    }

    zeta::core::object_state_notation::NodeTag node_tag{
        node_tag_result.GetValue()
    };

    OSNNode* node{ new OSNNode{ node_tag.node_type, false, 0 } };

    node->has_name = node_tag.has_name;
    node->has_obj_type = node_tag.has_obj_type;
    node->has_region = node_tag.has_region;

    if (node->has_name) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            decoder.state ==
            zeta::core::object_state_notation::DecoderState::SendingNameString);

        while (decoder.state == zeta::core::object_state_notation::
                                    DecoderState::SendingNameString) {
            auto char_result{ decoder.ReceiveStringChar() };

            if (!char_result.HasValue()) {
                ZETA_Core_DebugUtils_Diag_PromiseAssert(
                    decoder.state !=
                    zeta::core::object_state_notation::DecoderState::Corrupted);

                break;
            }

            node->name.push_back(char_result.GetValue());
        }
    }

    if (node->has_obj_type) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            decoder.state == zeta::core::object_state_notation::DecoderState::
                                 SendingObjTypeString);

        while (decoder.state == zeta::core::object_state_notation::
                                    DecoderState::SendingObjTypeString) {
            auto char_result{ decoder.ReceiveStringChar() };

            if (!char_result.HasValue()) {
                ZETA_Core_DebugUtils_Diag_PromiseAssert(
                    decoder.state !=
                    zeta::core::object_state_notation::DecoderState::Corrupted);

                break;
            }

            node->obj_type.push_back(char_result.GetValue());
        }
    }

    if (node->has_region) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            decoder.state ==
            zeta::core::object_state_notation::DecoderState::SendingRegionAttr);

        auto region_result{ decoder.ReceiveRegionAttr(
            zeta::core::meta::TypeWrapper<Region>{}) };

        Region region{ region_result.GetValue() };

        node->region_beg = region;
        node->region_size = 0;
    }

    if (node->node_type ==
            zeta::core::object_state_notation::NodeType::Integral ||
        node->node_type ==
            zeta::core::object_state_notation::NodeType::IntegralList) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            decoder.state == zeta::core::object_state_notation::DecoderState::
                                 SendingIntegralDescriptor);

        node->integral_descriptor =
            decoder.ReceiveIntegralDescriptor().GetValue();
    }

    if (node->node_type ==
            zeta::core::object_state_notation::NodeType::IntegralList ||
        node->node_type ==
            zeta::core::object_state_notation::NodeType::NodeList) {
        node->list_elem_cnt = decoder.ReceiveListElemCnt().GetValue();
    }

    if (node->node_type ==
        zeta::core::object_state_notation::NodeType::Integral) {
        if (node->integral_descriptor.is_signed) {
            node->s_integral =
                decoder
                    .ReceiveIntegral(
                        zeta::core::meta::TypeWrapper<NodeSignedIntegral>{})
                    .GetValue();
        } else {
            node->u_integral =
                decoder
                    .ReceiveIntegral(
                        zeta::core::meta::TypeWrapper<NodeUnsignedIntegral>{})
                    .GetValue();
        }
    }

    if (node->node_type ==
        zeta::core::object_state_notation::NodeType::IntegralList) {
        for (;;) {
            if (node->integral_descriptor.is_signed) {
                auto integral_result{ decoder.ReceiveIntegral(
                    zeta::core::meta::TypeWrapper<NodeSignedIntegral>{}) };

                if (!integral_result.HasValue()) {
                    ZETA_Core_DebugUtils_Diag_PromiseAssert(
                        decoder.state != zeta::core::object_state_notation::
                                             DecoderState::Corrupted);

                    break;
                }

                node->s_integral_list.push_back(integral_result.GetValue());
            } else {
                auto integral_result{ decoder.ReceiveIntegral(
                    zeta::core::meta::TypeWrapper<NodeUnsignedIntegral>{}) };

                if (!integral_result.HasValue()) {
                    ZETA_Core_DebugUtils_Diag_PromiseAssert(
                        decoder.state != zeta::core::object_state_notation::
                                             DecoderState::Corrupted);

                    break;
                }

                node->u_integral_list.push_back(integral_result.GetValue());
            }
        }

        if (node->list_elem_cnt != static_cast<size_t>(-1)) {
            if (node->integral_descriptor.is_signed) {
                ZETA_Core_DebugUtils_Diag_PromiseAssert(
                    node->list_elem_cnt == node->s_integral_list.size());
            } else {
                ZETA_Core_DebugUtils_Diag_PromiseAssert(
                    node->list_elem_cnt == node->u_integral_list.size());
            }
        }
    }

    if (node->node_type ==
        zeta::core::object_state_notation::NodeType::NodeList) {
        for (;;) {
            OSNNode* child_node{ NodeDecodeFromOctets(config, decoder) };

            if (child_node == nullptr) {
                ZETA_Core_DebugUtils_Diag_PromiseAssert(
                    decoder.state !=
                    zeta::core::object_state_notation::DecoderState::Corrupted);

                break;
            }

            node->node_list.push_back(child_node);
        }

        if (node->list_elem_cnt != static_cast<size_t>(-1)) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(node->list_elem_cnt ==
                                                    node->node_list.size());
        }
    }

    return node;
}

template <typename AcceptorLike>
constexpr void EncodeToOctets(
    zeta::core::object_state_notation::Encoder<AcceptorLike>& encoder,
    OSNNode const& node) {
    zeta::core::object_state_notation::NodeTag node_tag{
        .node_type = node.node_type,
        .has_name = node.has_name,
        .has_obj_type = node.has_obj_type,
        .has_region = node.has_region,
    };

    encoder.SendNodeTag(node_tag);

    if (node.has_name) {
        for (char c : node.name) { encoder.SendStringChar(c); }
    }

    if (node.has_obj_type) {
        for (char c : node.obj_type) { encoder.SendStringChar(c); }
    }

    if (node.has_region) {
        Region region{ node.region_beg };

        encoder.SendRegionAttr(region, 0);
    }

    if (node.node_type ==
            zeta::core::object_state_notation::NodeType::Integral ||
        node.node_type ==
            zeta::core::object_state_notation::NodeType::IntegralList) {
        encoder.SendIntegralDescriptor(node.integral_descriptor);
    }

    if (node.node_type ==
            zeta::core::object_state_notation::NodeType::IntegralList ||
        node.node_type ==
            zeta::core::object_state_notation::NodeType::NodeList) {
        encoder.SendListElemCnt(node.list_elem_cnt);
    }

    if (node.node_type ==
        zeta::core::object_state_notation::NodeType::Integral) {
        if (node.integral_descriptor.is_signed) {
            encoder.SendIntegral(node.s_integral);
        } else {
            encoder.SendIntegral(node.u_integral);
        }
    }

    if (node.node_type ==
        zeta::core::object_state_notation::NodeType::IntegralList) {
        if (node.integral_descriptor.is_signed) {
            for (auto integral : node.s_integral_list) {
                encoder.SendIntegral(integral);
            }
        } else {
            for (auto integral : node.u_integral_list) {
                encoder.SendIntegral(integral);
            }
        }
    }

    if (node.node_type ==
        zeta::core::object_state_notation::NodeType::NodeList) {
        for (auto child_node : node.node_list) {}
    }
}

struct OctetStream {
    std::deque<unsigned char> octets;

    constexpr bool IsEnd(this OctetStream& self,
                         zeta::core::seq_endpoint::provider::Tag) {
        return self.octets.empty();
    }

    static constexpr bool IsEnd(zeta::core::seq_endpoint::acceptor::Tag) {
        return false;
    }

    static constexpr size_t GetElemSize(
        zeta::core::seq_endpoint::provider::Tag) {
        return 1;
    }

    static constexpr size_t GetElemSize(
        zeta::core::seq_endpoint::acceptor::Tag) {
        return 1;
    }

    constexpr size_t Transfer(this OctetStream& self,
                              zeta::core::seq_endpoint::provider::Tag,
                              void* src, size_t src_elem_size,
                              ptrdiff_t src_elem_stride, size_t cnt) {
        size_t transferred_cnt{ std::min(cnt, self.octets.size()) };

        for (size_t i{ 0 }; i < transferred_cnt; ++i) {
            if (0 < src_elem_size) {
                *static_cast<unsigned char*>(src) = self.octets.front();
            }

            self.octets.pop_front();
            src = static_cast<unsigned char*>(src) + src_elem_stride;
        }

        return transferred_cnt;
    }

    constexpr size_t Transfer(this OctetStream& self,
                              zeta::core::seq_endpoint::acceptor::Tag,
                              void* src, size_t src_elem_size,
                              ptrdiff_t src_elem_stride, size_t cnt) {
        if (src_elem_size == 0) { return cnt; }

        for (size_t i{ 0 }; i < cnt; ++i) {
            self.octets.push_back(*static_cast<unsigned char*>(src));
            src = static_cast<unsigned char*>(src) + src_elem_stride;
        }

        return cnt;
    }
};

constexpr void main1() {
    zeta::core::object_state_notation::Config config{
        .version = { 0, 1, { 0, 0 } },
        .region_attr_size = region_attr_size,
    };

    //

    OSNNode* node{ (GenRandomNode)(20) };

    OctetStream octet_stream_a;

    //
}

int main() {
    ZETA_Core_DebugUtils_Logging_ImmLogVar("test_osn.cpp");

    return 0;
}
