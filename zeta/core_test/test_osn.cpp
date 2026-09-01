#include <fstream>
#include <vector>
#include <zeta/core/object_state_notation.ipp>

/*
struct Node {
    zeta::core::object_state_notation::NodeHeader node_header;

    union {
        unsigned _BitInt(64) u64_integral;
        signed _BitInt(64) s64_integral;

        std::vector<unsigned _BitInt(64)> u64_integral_list;
        std::vector<signed _BitInt(64)> s64_integral_list;

        std::vector<Node> node_list;
    };

    Node(zeta::core::object_state_notation::NodeHeader const& node_header_)
        : node_header{ node_header_ } {
        switch (this->node_header.node_tag.node_type) {
        case zeta::core::object_state_notation::NodeTypeEnum::IntegralList::
            value:

            if (this->node_header.integral_descriptor.signedness) {
                new (
                    &this->s64_integral_list)
decltype(this->s64_integral_list){}; } else { new ( &this->u64_integral_list)
decltype(this->u64_integral_list){};
            }

            break;

        case zeta::core::object_state_notation::NodeTypeEnum::NodeList::value:
            new (&this->node_list) decltype(this->node_list){};
            break;
        }
    }

    Node(Node&& node) : node_header{ node.node_header } {
        switch (this->node_header.node_tag.node_type) {
        case zeta::core::object_state_notation::NodeTypeEnum::IntegralList::
            value:

            if (this->node_header.integral_descriptor.signedness) {
                new (
                    &this->s64_integral_list) decltype(this->s64_integral_list){
                    std::move(node.s64_integral_list)
                };
            } else {
                new (
                    &this->u64_integral_list) decltype(this->u64_integral_list){
                    std::move(node.u64_integral_list)
                };
            }

            break;

        case zeta::core::object_state_notation::NodeTypeEnum::NodeList::value:
            new (&this->node_list) decltype(this->node_list){ std::move(
                node.node_list) };
            break;
        }
    }

    ~Node() {
        switch (this->node_header.node_tag.node_type) {
        case zeta::core::object_state_notation::NodeTypeEnum::IntegralList::
            value:

            if (this->node_header.integral_descriptor.signedness) {
                this->s64_integral_list.~vector();
            } else {
                this->u64_integral_list.~vector();
            }

            break;

        case zeta::core::object_state_notation::NodeTypeEnum::NodeList::value:
            this->node_list.~vector();
            break;
        }
    }
};
*/

#define ZetaDir "D:/ZetaDevelops/ZetaDevelop/Zeta"

struct BinFSProvider {
    std::ifstream& bin_fs;

    static constexpr size_t GetElemSize() { return 1; }

    static constexpr bool IsEnd() { return false; }

    size_t Transfer(this BinFSProvider& self, void* data, size_t elem_size,
                    size_t elem_stride, size_t elem_cnt) {
        ZETA_Core_DebugAssert(elem_size == 1);
        ZETA_Core_DebugAssert(elem_stride == 1);

        if (elem_stride == 1) {
            self.bin_fs.read(static_cast<char*>(data),
                             static_cast<long long>(elem_stride * elem_cnt));
        } else {
            unsigned char read_buffer[1024];

            self.bin_fs.read(reinterpret_cast<char*>(read_buffer),
                             static_cast<long long>(elem_cnt));

            zeta::core::utils::ElemCopy(data, read_buffer, 1, elem_stride, 1,
                                        elem_cnt);
        }

        return elem_cnt;
    }
};

struct BinFSAcceptor {
    std::istream& re_bin_fs;
    std::ofstream& bin_fs;

    static constexpr size_t GetElemSize() { return 1; }

    static constexpr bool IsEnd() { return false; }

    size_t Transfer(this BinFSAcceptor& self, void const* data,
                    size_t elem_size, size_t elem_stride, size_t elem_cnt) {
        ZETA_Core_DebugAssert(elem_size == 1);

        unsigned char cri_buffer[1024];
        unsigned char write_buffer[1024];

        zeta::core::utils::ElemCopy(write_buffer, data, 1, 1, elem_stride,
                                    elem_cnt);

        self.re_bin_fs.read(reinterpret_cast<char*>(cri_buffer),
                            static_cast<long long>(elem_cnt));

        int cmp{ std::memcmp(write_buffer, cri_buffer, elem_cnt) };

        if (cmp != 0) {
            ZETA_Core_Debug_PrintVar(cri_buffer[0]);
            ZETA_Core_Debug_PrintVar(cri_buffer[1]);
            ZETA_Core_Debug_PrintVar(cri_buffer[2]);
            ZETA_Core_Debug_PrintVar(cri_buffer[3]);

            ZETA_Core_Debug_PrintVar(elem_cnt);

            ZETA_Core_Debug_PrintVar(
                static_cast<unsigned char const*>(write_buffer)[0]);

            ZETA_Core_Debug_PrintVar(
                static_cast<unsigned char const*>(write_buffer)[1]);

            ZETA_Core_Debug_PrintVar(
                static_cast<unsigned char const*>(write_buffer)[2]);

            ZETA_Core_Debug_PrintVar(
                static_cast<unsigned char const*>(write_buffer)[3]);

            ZETA_Core_DebugAssert(cmp == 0);
        }

        self.bin_fs.write(reinterpret_cast<char const*>(write_buffer),
                          static_cast<long long>(elem_cnt));

        return elem_cnt;
    }
};

inline void main1(int num) {
    constexpr size_t str_buffer_size{ 1024 };
    unsigned char str_buffer[1024];

    std::string num_str{ std::to_string(num) };

    std::ifstream bin_fs{ std::string{ ZetaDir "/osn/test_osn_" } + num_str +
                              ".bin",
                          std::ios::binary };

    std::ifstream cri_bin_fs{ std::string{ ZetaDir "/osn/test_osn_" } +
                                  num_str + ".bin",
                              std::ios::binary };

    std::ofstream re_bin_fs{ std::string{ ZetaDir "/osn/test_osn_re_" } +
                                 num_str + ".bin",
                             std::ios::binary };

    BinFSProvider bin_fs_provider{ bin_fs };

    BinFSAcceptor bin_fs_acceptor{ cri_bin_fs, re_bin_fs };

    zeta::core::object_state_notation::Header header;

    zeta::core::object_state_notation::DecodeHeaderFromOctets(bin_fs_provider,
                                                              header);

    auto config{ ({
        auto [is_valid, config]{
            zeta::core::object_state_notation::Config::FromHeader(header)
        };

        if (!is_valid) {
            ZETA_Core_PrintVar(header.magic[0]);
            ZETA_Core_PrintVar(header.magic[1]);
            ZETA_Core_PrintVar(header.magic[2]);
            ZETA_Core_PrintVar(header.magic[3]);
            ZETA_Core_PrintVar(header.region_attr_size);

            ZETA_Core_DebugAssert(false);
            return;
        }

        config;
    }) };

    {
        auto [is_valid, header]{ config.ToHeader() };

        if (!is_valid) {
            ZETA_Core_DebugAssert(false);
            return;
        }

        zeta::core::object_state_notation::EncodeHeaderToOctets(bin_fs_acceptor,
                                                                header);
    }

    zeta::core::object_state_notation::Decoder<decltype(bin_fs_provider)>
        decoder{ config, bin_fs_provider };

    unsigned char serializer_integral_chunk_buffer_data[255];
    unsigned short serializer_integral_chunk_buffer_max_octet_cnt{ sizeof(
        serializer_integral_chunk_buffer_data) };

    zeta::core::object_state_notation::Encoder<decltype(bin_fs_acceptor)>
        encoder{ config, serializer_integral_chunk_buffer_data,
                 serializer_integral_chunk_buffer_max_octet_cnt,
                 bin_fs_acceptor };

    zeta::core::object_state_notation::IntegralDescriptor integral_descriptor;
    unsigned _BitInt(128) unsigned_integral;
    signed _BitInt(128) signed_integral;

    using UnsignedIntegralType = unsigned _BitInt(128);
    using SignedIntegralType = signed _BitInt(128);

    while (decoder.state !=
           zeta::core::object_state_notation::DecoderState::Finished) {
        ZETA_Core_Debug_PrintCurPos;

        ZETA_Core_Debug_PrintVar(zeta::core::meta::ToUnderlying(decoder.state));
        ZETA_Core_Debug_PrintVar(zeta::core::meta::ToUnderlying(encoder.state));

        switch (decoder.state) {
        case zeta::core::object_state_notation::DecoderState::SendingNodeTag: {
            ZETA_Core_Debug_PrintCurPos;

            encoder.SendNodeTag(decoder.ReceiveNodeTag().GetValue())
                .CheckHasValue();

            break;
        }

        case zeta::core::object_state_notation::DecoderState::SendingNameString:
        case zeta::core::object_state_notation::DecoderState::
            SendingObjTypeString: {
            zeta::core::lin_seq_elem_stream::Acceptor str_reader{
                .data = str_buffer,
                .elem_size = 1,
                .elem_stride = 1,
                .elem_cnt = str_buffer_size,
            };

            size_t str_size{
                decoder.ReceiveStringOctet(str_reader, str_buffer_size)
                    .GetValue()
            };

            ZETA_Core_DebugAssert(str_size < str_buffer_size);

            if (!encoder.SendStringOctet(
                    zeta::core::lin_seq_elem_stream::Provider{
                        .data = str_buffer,
                        .elem_size = 1,
                        .elem_stride = 1,
                        .elem_cnt = str_size,
                    },
                    str_size)) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            encoder.SendFinish().CheckHasValue();

            break;
        }

        case zeta::core::object_state_notation::DecoderState::
            SendingRegionAttr: {
            auto [region_beg, region_size]{
                decoder.ReceiveRegionAttr<unsigned long long>().GetValue()
            };

            encoder.SendRegionAttr(region_beg, region_size).CheckHasValue();

            break;
        }

        case zeta::core::object_state_notation::DecoderState::
            SendingIntegralDescriptor: {
            integral_descriptor =
                decoder.ReceiveIntegralDescriptor().GetValue();

            encoder.SendIntegralDescriptor(integral_descriptor).CheckHasValue();

            ZETA_Core_Debug_PrintVar(integral_descriptor.is_signed);
            ZETA_Core_Debug_PrintVar(integral_descriptor.size);

            break;
        }

        case zeta::core::object_state_notation::DecoderState::
            SendingListElemCnt: {
            encoder.SendListElemCnt(decoder.ReceiveListElemCnt().GetValue())
                .CheckHasValue();

            break;
        }

        case zeta::core::object_state_notation::DecoderState::SendingIntegral: {
            if (integral_descriptor.is_signed) {
                encoder
                    .SendIntegral(decoder.ReceiveIntegral<SignedIntegralType>()
                                      .GetValue())
                    .CheckHasValue();
            } else {
                encoder
                    .SendIntegral(
                        decoder.ReceiveIntegral<UnsignedIntegralType>()
                            .GetValue())
                    .CheckHasValue();
            }

            break;
        }

        case zeta::core::object_state_notation::DecoderState::SendingFinish: {
            decoder.ReceiveFinish().CheckHasValue();

            encoder.SendFinish().CheckHasValue();

            break;
        }

        default:
            ZETA_Core_Debug_PrintVar(
                zeta::core::meta::ToUnderlying(decoder.state));

            ZETA_Core_DebugAssert(false);
            return;
        }

        ZETA_Core_DebugAssert(
            decoder.state !=
            zeta::core::object_state_notation::DecoderState::Corrupted);

        ZETA_Core_DebugAssert(
            encoder.state !=
            zeta::core::object_state_notation::EncoderState::Corrupted);

        zeta::core::debug_utils::ClearDebugStrStream();
    }

    ZETA_Core_DebugAssert(
        encoder.state ==
        zeta::core::object_state_notation::EncoderState::Finished);

    ZETA_Core_DebugAssert(
        decoder.state ==
        zeta::core::object_state_notation::DecoderState::Finished);

    {
        ZETA_Core_DebugAssert(!bin_fs.eof());
        char dummy;
        bin_fs.read(&dummy, 1);
        ZETA_Core_DebugAssert(bin_fs.eof());
    }

    {
        ZETA_Core_DebugAssert(!cri_bin_fs.eof());
        char dummy;
        cri_bin_fs.read(&dummy, 1);
        ZETA_Core_DebugAssert(cri_bin_fs.eof());
    }

    ZETA_Core_PrintVar("complete");

    bin_fs.close();
    re_bin_fs.close();
}

int main() {
    for (int num{ 0 }; num < 100; ++num) {
        ZETA_Core_PrintVar(num);
        main1(num);
    }

    ZETA_Core_PrintVar("ok");
    return 0;
}
