#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/lin_seq_elem_stream.ipp>
#include <zeta/core/unicode.hpp>
#include <zeta/core/utf8.ipp>

inline void test_utf8(zeta::core::unicode::unichar_t cp_beg,
                      zeta::core::unicode::unichar_t cp_end) {
    constexpr size_t buffer_size{ 1024 };
    // test how many codepoints a time

    zeta::core::unicode::unichar_t cp_buffer[buffer_size];
    unsigned char octet_buffer[buffer_size * 4];
    zeta::core::unicode::unichar_t re_cp_buffer[buffer_size];
    // unsigned char re_octet_buffer[buffer_size * 4];

    for (; cp_beg <= cp_end;) {
        size_t cur_cp_cnt{ zeta::core::comparison_utils::BasicMin(
            cp_end - cp_beg + 1U, buffer_size) };

        for (size_t i{ 0 }; i < cur_cp_cnt; ++i) {
            cp_buffer[i] =
                static_cast<zeta::core::unicode::unichar_t>(cp_beg + i);
        }

        ZETA_Core_Debug_PrintVar(cp_beg);
        ZETA_Core_Debug_PrintVar(cur_cp_cnt);
        ZETA_Core_Debug_PrintVar(cp_beg + cur_cp_cnt);

        size_t cur_octet_cnt{ 0 };

        {
            zeta::core::utf8::Encoder encoder;

            zeta::core::lin_seq_elem_stream::Provider p{
                .data = cp_buffer,
                .elem_size = sizeof(zeta::core::unicode::unichar_t),
                .elem_stride = sizeof(zeta::core::unicode::unichar_t),
                .elem_cnt = cur_cp_cnt,
            };

            zeta::core::lin_seq_elem_stream::Acceptor a{
                .data = octet_buffer,
                .elem_size = 1,
                .elem_stride = 1,
                .elem_cnt = sizeof(octet_buffer) / sizeof(octet_buffer[0]),
            };

            size_t old_a_elem_cnt{ a.elem_cnt };

            encoder.EncodeAndPush(p, a);

            ZETA_Core_Debug_PrintVar(cp_buffer[0]);
            ZETA_Core_Debug_PrintVar(octet_buffer[0]);
            ZETA_Core_Debug_PrintVar(octet_buffer[1]);
            ZETA_Core_Debug_PrintVar(octet_buffer[2]);
            ZETA_Core_Debug_PrintVar(octet_buffer[3]);

            ZETA_Core_DebugAssert(p.elem_cnt == 0);

            cur_octet_cnt = old_a_elem_cnt - a.elem_cnt;
        }

        {
            zeta::core::utf8::Decoder decoder;

            zeta::core::lin_seq_elem_stream::Provider p{
                .data = octet_buffer,
                .elem_size = 1,
                .elem_stride = 1,
                .elem_cnt = cur_octet_cnt,
            };

            zeta::core::lin_seq_elem_stream::Acceptor a{
                .data = re_cp_buffer,
                .elem_size = sizeof(zeta::core::unicode::unichar_t),
                .elem_stride = sizeof(zeta::core::unicode::unichar_t),
                .elem_cnt = cur_cp_cnt,
            };

            decoder.DecodeAndPush(p, a);

            ZETA_Core_Debug_PrintVar(octet_buffer[0]);
            ZETA_Core_Debug_PrintVar(octet_buffer[1]);
            ZETA_Core_Debug_PrintVar(octet_buffer[2]);
            ZETA_Core_Debug_PrintVar(octet_buffer[3]);
            ZETA_Core_Debug_PrintVar(re_cp_buffer[0]);

            ZETA_Core_DebugAssert(p.elem_cnt == 0);

            ZETA_Core_DebugAssert(a.elem_cnt == 0);

            ZETA_Core_DebugAssert(zeta::core::utils::MemCompare(
                                      cp_buffer, re_cp_buffer, cur_octet_cnt) ==
                                  0);
        }

        cp_beg += cur_cp_cnt;
    }
}

inline void main1() {
    zeta::core::unicode::unichar_t cp_beg{
        zeta::core::unicode::codepoint_range_min
        // zeta::core::unicode::surrogate_range_max + 1
    };

    zeta::core::unicode::unichar_t cp_end{
        zeta::core::unicode::surrogate_range_min - 1
        // zeta::core::unicode::codepoint_range_max
    };

    ZETA_Core_PrintVar(cp_beg);
    ZETA_Core_PrintVar(cp_end);

    zeta::core::unicode::unichar_t stride{ 0x10000 };

    for (; cp_beg <= cp_end; cp_beg += stride) {
        ZETA_Core_Debug_PrintVar(cp_beg);
        test_utf8(cp_beg, std::min(cp_beg + stride - 1, cp_end));

        zeta::core::debug_utils::ClearDebugStrStream();
    }

    ZETA_Core_PrintVar("ok");
}

int main() {
    main1();

    return 0;
}
