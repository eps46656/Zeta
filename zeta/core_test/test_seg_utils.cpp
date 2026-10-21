#include <zeta/core/circular_array.ipp>
#include <zeta/core/debug_utils/memory.hpp>
#include <zeta/core/debug_utils/memory.ipp>
#include <zeta/core/debug_utils/revision.ipp>
#include <zeta/core/hash.ipp>
#include <zeta/core/hash_utils.ipp>
#include <zeta/core/lin_seq_endpoint.hpp>
#include <zeta/core/seg_utils.hpp>
#include <zeta/core/seg_utils.ipp>
#include <zeta/core_test/random.hpp>
#include <zeta/core_test/sample_object.hpp>
#include <zeta/core_test/timer.hpp>

template <typename Elem>
constexpr void InitCircularArray(zeta::core::circular_array::Cntr<Elem>& ca,
                                 std::vector<Elem> const& elems) {
    if (0 < ca.elem_cnt) {
        ca.EraseAll(zeta::core::seq_cntr::Tag{},
                    zeta::core::seq_endpoint::acceptor::BasicAcceptor{});
    }

    size_t rot;

    if (ca.slot_cnt == 0) {
        rot = 0;
    } else {
        rot = zeta::core_test::random::GenUniformRandomInt<size_t>(
            0, ca.slot_cnt - 1);
    }

    ca.rot = rot;

    ca.PushR(zeta::core::seq_cntr::Tag{}, elems.size(),
             zeta::core::lin_seq_endpoint::provider::Provider<
                 zeta::core::lifecycle::ConstLeftObjView<Elem>>{
                 .data{ elems.data() },
                 .elem_stride{ sizeof(Elem) },
                 .elem_cnt = elems.size(),
             },
             nullptr, nullptr);
}

template <typename Reader>
struct ReadCircularArrayAcceptor {
    size_t acc_cnt;
    size_t total_cnt;
    Reader&& reader;

    constexpr bool IsEnd(this ReadCircularArrayAcceptor& self,
                         zeta::core::seq_endpoint::acceptor::Tag) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(self.acc_cnt <= self.total_cnt);
        return self.acc_cnt == self.total_cnt;
    }

    template <zeta::core::lifecycle::IsSrcStorageView SrcStorageView>
    constexpr size_t Transfer(this ReadCircularArrayAcceptor& self,
                              zeta::core::seq_endpoint::acceptor::Tag,
                              SrcStorageView src, ptrdiff_t elem_stride,
                              size_t cnt) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(self.acc_cnt <= self.total_cnt);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            src.semantics == zeta::core::lifecycle::StorageSemantics::LeftObj ||
            src.semantics ==
                zeta::core::lifecycle::StorageSemantics::ConstLeftObj);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(cnt <=
                                                self.total_cnt - self.acc_cnt);

        for (size_t i{ 0 }; i < cnt; ++i) {
            self.reader(*src.ptr);
            src.ptr = zeta::core::utils::PtrInc(src.ptr, elem_stride);
        }

        self.acc_cnt += cnt;

        return cnt;
    }
};

template <typename Elem, typename Reader>
constexpr void ReadCircularArray(
    zeta::core::circular_array::Cntr<Elem> const& ca, size_t beg, size_t cnt,
    Reader&& reader) {
    ReadCircularArrayAcceptor<Reader> acceptor{
        .acc_cnt   = 0,
        .total_cnt = cnt,
        .reader    = std::forward<Reader>(reader),
    };

    zeta::core::circular_array::Cursor cursor;

    ca.Refer(
        zeta::core::seq_cntr::Tag{}, beg, true, nullptr, &cursor,
        zeta::core::lifecycle::LeftObjView<zeta::core::meta::RemoveConst<Elem>>{
            nullptr });

    ca.Read(zeta::core::seq_cntr::Tag{}, &cursor, cnt, acceptor, nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(acceptor.acc_cnt == cnt);
}

template <typename Elem>
constexpr size_t RollHash(size_t hash, std::vector<Elem> const& elems,
                          size_t beg, size_t cnt) {
    for (size_t i{ beg }, end{ beg + cnt }; i < end; ++i) {
        hash = zeta::core::hash::BasicHash(elems[i], hash);
    }

    return hash;
}

template <typename Elem>
constexpr size_t RollHash(size_t hash,
                          zeta::core::circular_array::Cntr<Elem> const& ca,
                          size_t beg, size_t cnt) {
    auto reader{ [&hash](Elem const& elem) {
        hash = zeta::core::hash::BasicHash(elem, hash);
    } };

    (ReadCircularArray)(ca, beg, cnt, reader);

    return hash;
}

template <typename Elem>
constexpr size_t GetRandomStride() {
    return sizeof(Elem) +
           zeta::core_test::random::GenUniformRandomInt<size_t>(
               0, std::max<size_t>(2, sizeof(Elem) * 2 / alignof(Elem))) *
               alignof(Elem);
}

constexpr size_t GenRandomSlotCnt() {
    return zeta::core_test::random::GenUniformRandomInt<size_t>(1, 64);
}

constexpr void GenRandomSampleObj(
    std::vector<zeta::core_test::sample_object::SampleObject>& elems,
    size_t elem_cnt) {
    elems.resize(elem_cnt);

    zeta::core_test::random::GenRandoms<
        zeta::core_test::sample_object::SampleObject>(elems.begin(),
                                                      elems.end());

    for (zeta::core_test::sample_object::SampleObject& elem : elems) {
        elem.statistics.Reset();
    }
}

constexpr void CommonInit(
    zeta::core::circular_array::Cntr<
        zeta::core_test::sample_object::SampleObject>& l_ca,
    zeta::core::circular_array::Cntr<
        zeta::core_test::sample_object::SampleObject>& r_ca,
    std::vector<zeta::core_test::sample_object::SampleObject>& l_elems,
    std::vector<zeta::core_test::sample_object::SampleObject>& r_elems,
    size_t l_elem_cnt, size_t r_elem_cnt) {
    (GenRandomSampleObj)(l_elems, l_elem_cnt);
    (GenRandomSampleObj)(r_elems, r_elem_cnt);

    (InitCircularArray)(l_ca, l_elems);
    (InitCircularArray)(r_ca, r_elems);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(l_ca.elem_cnt == l_elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(r_ca.elem_cnt == r_elem_cnt);

    {
        zeta::core_test::sample_object::Statictics rhs_expected_statistics{
            .default_construct = 0,

            .copy_construct_lhs = 0,
            .copy_construct_rhs = 1,

            .move_construct_lhs = 0,
            .move_construct_rhs = 0,

            .copy_assign_lhs = 0,
            .copy_assign_rhs = 0,

            .move_assign_lhs = 0,
            .move_assign_rhs = 0,

            .acc_copy_cnt = 0,
            .acc_move_cnt = 0,
        };

        for (zeta::core_test::sample_object::SampleObject& elem : l_elems) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(elem.statistics ==
                                                    rhs_expected_statistics);
        }

        for (zeta::core_test::sample_object::SampleObject& elem : r_elems) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(elem.statistics ==
                                                    rhs_expected_statistics);
        }
    }

    {
        zeta::core_test::sample_object::Statictics lhs_expected_statistics{
            .default_construct = 0,

            .copy_construct_lhs = 1,
            .copy_construct_rhs = 0,

            .move_construct_lhs = 0,
            .move_construct_rhs = 0,

            .copy_assign_lhs = 0,
            .copy_assign_rhs = 0,

            .move_assign_lhs = 0,
            .move_assign_rhs = 0,

            .acc_copy_cnt = 1,
            .acc_move_cnt = 0,
        };

        auto reader{
            [lhs_expected_statistics](
                zeta::core_test::sample_object::SampleObject const& elem) {
                ZETA_Core_DebugUtils_Diag_PromiseAssert(
                    elem.statistics == lhs_expected_statistics);

                elem.statistics.Reset();
            }
        };

        (ReadCircularArray)(l_ca, 0, l_elem_cnt, reader);
        (ReadCircularArray)(r_ca, 0, r_elem_cnt, reader);
    }
}

constexpr void TestSegInsertShoveL() {
    using SampleObject = zeta::core_test::sample_object::SampleObject;

    size_t l_elem_stride{ (GetRandomStride<SampleObject>)() };
    size_t r_elem_stride{ (GetRandomStride<SampleObject>)() };

    size_t l_slot_cnt{ (GenRandomSlotCnt)() };
    size_t r_slot_cnt{ (GenRandomSlotCnt)() };

    size_t l_elem_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, l_slot_cnt) };

    size_t r_elem_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, r_slot_cnt) };

    size_t ins_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, l_slot_cnt + r_slot_cnt - l_elem_cnt - r_elem_cnt) };

    size_t shove_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        std::max(r_elem_cnt + ins_cnt, r_slot_cnt) - r_slot_cnt,
        std::min(r_elem_cnt + ins_cnt, l_slot_cnt - l_elem_cnt)) };

    size_t rl_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, r_elem_cnt) };

    zeta::core::circular_array::Cntr<
        zeta::core_test::sample_object::SampleObject>
        l_ca{
            .data = zeta::core::debug_utils::memory::Malloc<SampleObject>(
                l_elem_stride * l_slot_cnt),
            .elem_stride = l_elem_stride,
            .elem_cnt    = 0,
            .slot_cnt    = l_slot_cnt,
            .rot         = 0,
#if ZETA_Core_DebugEnable
            .stamp = zeta::core::debug_utils::revision::GetStamp(),
#endif
        };

    zeta::core::circular_array::Cntr<
        zeta::core_test::sample_object::SampleObject>
        r_ca{
            .data = zeta::core::debug_utils::memory::Malloc<SampleObject>(
                r_elem_stride * r_slot_cnt),
            .elem_stride = r_elem_stride,
            .elem_cnt    = 0,
            .slot_cnt    = r_slot_cnt,
            .rot         = 0,
#if ZETA_Core_DebugEnable
            .stamp = zeta::core::debug_utils::revision::GetStamp(),
#endif
        };

    std::vector<SampleObject> l_elems;
    std::vector<SampleObject> r_elems;

    (CommonInit)(l_ca, r_ca, l_elems, r_elems, l_elem_cnt, r_elem_cnt);

    std::vector<SampleObject> ins_elems;
    (GenRandomSampleObj)(ins_elems, ins_cnt);

    {
        size_t l_vec_hash{ RollHash(0, l_elems, 0, l_elem_cnt) };
        size_t r_vec_hash{ RollHash(0, r_elems, 0, r_elem_cnt) };

        size_t l_ca_hash{ RollHash(0, l_ca, 0, l_elem_cnt) };
        size_t r_ca_hash{ RollHash(0, r_ca, 0, r_elem_cnt) };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(l_vec_hash == l_ca_hash);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(r_vec_hash == r_ca_hash);
    }

    zeta::core::seg_utils::SegInsertShoveL(
        l_ca,       // l_ca
        r_ca,       // r_ca
        rl_cnt,     // rl_cnt
        ins_cnt,    // ins_cnt
        shove_cnt,  // shove_cnt
        zeta::core::lin_seq_endpoint::provider::Provider<
            zeta::core::lifecycle::ConstLeftObjView<SampleObject>>{
            .data{ ins_elems.data() },
            .elem_stride = sizeof(SampleObject),
            .elem_cnt    = ins_cnt,
        });

    ZETA_Core_DebugUtils_Diag_PromiseAssert(l_ca.elem_cnt ==
                                            l_elem_cnt + shove_cnt);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(r_ca.elem_cnt ==
                                            r_elem_cnt + ins_cnt - shove_cnt);

    {
        auto reader{ [](SampleObject const& elem) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                elem.statistics.default_construct == 0);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                elem.statistics.copy_construct_lhs +
                    elem.statistics.move_construct_lhs +
                    elem.statistics.copy_assign_lhs +
                    elem.statistics.move_assign_lhs <=
                1);
        } };

        (ReadCircularArray)(l_ca, 0, l_ca.elem_cnt, reader);
        (ReadCircularArray)(r_ca, 0, r_ca.elem_cnt, reader);
    }

    size_t expected_final_hash{ 0 };

    expected_final_hash =
        (RollHash)(expected_final_hash, l_elems, 0, l_elem_cnt);
    expected_final_hash = (RollHash)(expected_final_hash, r_elems, 0, rl_cnt);
    expected_final_hash =
        (RollHash)(expected_final_hash, ins_elems, 0, ins_cnt);
    expected_final_hash =
        (RollHash)(expected_final_hash, r_elems, rl_cnt, r_elem_cnt - rl_cnt);

    size_t op_final_hash{ 0 };

    op_final_hash = (RollHash)(op_final_hash, l_ca, 0, l_ca.elem_cnt);
    op_final_hash = (RollHash)(op_final_hash, r_ca, 0, r_ca.elem_cnt);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(expected_final_hash ==
                                            op_final_hash);

    {
        std::vector<SampleObject> k;
        (GenRandomSampleObj)(k, l_ca.elem_cnt + r_ca.elem_cnt);

        size_t k_hash{ RollHash(0, k, 0, k.size()) };

        if (0 < k.size()) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                k_hash != expected_final_hash && k_hash != op_final_hash);
        }
    }

    l_ca.EraseAll(zeta::core::seq_cntr::Tag{},
                  zeta::core::seq_endpoint::acceptor::BasicAcceptor{});

    r_ca.EraseAll(zeta::core::seq_cntr::Tag{},
                  zeta::core::seq_endpoint::acceptor::BasicAcceptor{});

    zeta::core::debug_utils::memory::Free(l_ca.data);
    zeta::core::debug_utils::memory::Free(r_ca.data);
}

constexpr void TestSegInsertShoveR() {
    using SampleObject = zeta::core_test::sample_object::SampleObject;

    size_t l_elem_stride{ (GetRandomStride<SampleObject>)() };
    size_t r_elem_stride{ (GetRandomStride<SampleObject>)() };

    size_t l_slot_cnt{ (GenRandomSlotCnt)() };
    size_t r_slot_cnt{ (GenRandomSlotCnt)() };

    size_t l_elem_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, l_slot_cnt) };

    size_t r_elem_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, r_slot_cnt) };

    size_t ins_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, l_slot_cnt + r_slot_cnt - l_elem_cnt - r_elem_cnt) };

    size_t shove_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        std::max(l_elem_cnt + ins_cnt, l_slot_cnt) - l_slot_cnt,
        std::min(l_elem_cnt + ins_cnt, r_slot_cnt - r_elem_cnt)) };

    size_t lr_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, l_elem_cnt) };

    zeta::core::circular_array::Cntr<
        zeta::core_test::sample_object::SampleObject>
        l_ca{
            .data = zeta::core::debug_utils::memory::Malloc<SampleObject>(
                l_elem_stride * l_slot_cnt),
            .elem_stride = l_elem_stride,
            .elem_cnt    = 0,
            .slot_cnt    = l_slot_cnt,
            .rot         = 0,
#if ZETA_Core_DebugEnable
            .stamp = zeta::core::debug_utils::revision::GetStamp(),
#endif
        };

    zeta::core::circular_array::Cntr<
        zeta::core_test::sample_object::SampleObject>
        r_ca{
            .data = zeta::core::debug_utils::memory::Malloc<SampleObject>(
                r_elem_stride * r_slot_cnt),
            .elem_stride = r_elem_stride,
            .elem_cnt    = 0,
            .slot_cnt    = r_slot_cnt,
            .rot         = 0,
#if ZETA_Core_DebugEnable
            .stamp = zeta::core::debug_utils::revision::GetStamp(),
#endif
        };

    std::vector<SampleObject> l_elems;
    std::vector<SampleObject> r_elems;

    (CommonInit)(l_ca, r_ca, l_elems, r_elems, l_elem_cnt, r_elem_cnt);

    std::vector<SampleObject> ins_elems;
    (GenRandomSampleObj)(ins_elems, ins_cnt);

    {
        size_t l_vec_hash{ RollHash(0, l_elems, 0, l_elem_cnt) };
        size_t r_vec_hash{ RollHash(0, r_elems, 0, r_elem_cnt) };

        size_t l_ca_hash{ RollHash(0, l_ca, 0, l_elem_cnt) };
        size_t r_ca_hash{ RollHash(0, r_ca, 0, r_elem_cnt) };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(l_vec_hash == l_ca_hash);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(r_vec_hash == r_ca_hash);
    }

    zeta::core::seg_utils::SegInsertShoveR(
        l_ca,       // l_ca
        r_ca,       // r_ca
        lr_cnt,     // lr_cnt
        ins_cnt,    // ins_cnt
        shove_cnt,  // shove_cnt
        zeta::core::lin_seq_endpoint::provider::Provider<
            zeta::core::lifecycle::LeftObjView<SampleObject>>{
            .data{ ins_elems.data() },
            .elem_stride = sizeof(SampleObject),
            .elem_cnt    = ins_cnt,
        });

    ZETA_Core_DebugUtils_Diag_PromiseAssert(l_ca.elem_cnt ==
                                            l_elem_cnt + ins_cnt - shove_cnt);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(r_ca.elem_cnt ==
                                            r_elem_cnt + shove_cnt);

    {
        auto reader{ [](SampleObject const& elem) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                elem.statistics.default_construct == 0);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                elem.statistics.copy_construct_lhs +
                    elem.statistics.move_construct_lhs +
                    elem.statistics.copy_assign_lhs +
                    elem.statistics.move_assign_lhs <=
                1);
        } };

        (ReadCircularArray)(l_ca, 0, l_ca.elem_cnt, reader);
        (ReadCircularArray)(r_ca, 0, r_ca.elem_cnt, reader);
    }

    size_t expected_final_hash{ 0 };

    expected_final_hash =
        (RollHash)(expected_final_hash, l_elems, 0, l_elem_cnt - lr_cnt);
    expected_final_hash =
        (RollHash)(expected_final_hash, ins_elems, 0, ins_cnt);
    expected_final_hash =
        (RollHash)(expected_final_hash, l_elems, l_elem_cnt - lr_cnt, lr_cnt);
    expected_final_hash =
        (RollHash)(expected_final_hash, r_elems, 0, r_elem_cnt);

    size_t op_final_hash{ 0 };

    op_final_hash = (RollHash)(op_final_hash, l_ca, 0, l_ca.elem_cnt);
    op_final_hash = (RollHash)(op_final_hash, r_ca, 0, r_ca.elem_cnt);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(expected_final_hash ==
                                            op_final_hash);

    {
        std::vector<SampleObject> k;
        (GenRandomSampleObj)(k, l_ca.elem_cnt + r_ca.elem_cnt);

        size_t k_hash{ RollHash(0, k, 0, k.size()) };

        if (0 < k.size()) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                k_hash != expected_final_hash && k_hash != op_final_hash);
        }
    }

    l_ca.EraseAll(zeta::core::seq_cntr::Tag{},
                  zeta::core::seq_endpoint::acceptor::BasicAcceptor{});

    r_ca.EraseAll(zeta::core::seq_cntr::Tag{},
                  zeta::core::seq_endpoint::acceptor::BasicAcceptor{});

    zeta::core::debug_utils::memory::Free(l_ca.data);
    zeta::core::debug_utils::memory::Free(r_ca.data);
}

constexpr void TestSegEraseShoveL() {
    using SampleObject = zeta::core_test::sample_object::SampleObject;

    size_t l_elem_stride{ (GetRandomStride<SampleObject>)() };
    size_t r_elem_stride{ (GetRandomStride<SampleObject>)() };

    size_t l_slot_cnt{ (GenRandomSlotCnt)() };
    size_t r_slot_cnt{ (GenRandomSlotCnt)() };

    size_t l_elem_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, l_slot_cnt) };

    size_t r_elem_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, r_slot_cnt) };

    size_t rl_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, r_elem_cnt) };

    size_t ers_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, r_elem_cnt - rl_cnt) };

    size_t shove_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, std::min(r_elem_cnt - ers_cnt, l_slot_cnt - l_elem_cnt)) };

    zeta::core::circular_array::Cntr<
        zeta::core_test::sample_object::SampleObject>
        l_ca{
            .data = zeta::core::debug_utils::memory::Malloc<SampleObject>(
                l_elem_stride * l_slot_cnt),
            .elem_stride = l_elem_stride,
            .elem_cnt    = 0,
            .slot_cnt    = l_slot_cnt,
            .rot         = 0,
#if ZETA_Core_DebugEnable
            .stamp = zeta::core::debug_utils::revision::GetStamp(),
#endif
        };

    zeta::core::circular_array::Cntr<
        zeta::core_test::sample_object::SampleObject>
        r_ca{
            .data = zeta::core::debug_utils::memory::Malloc<SampleObject>(
                r_elem_stride * r_slot_cnt),
            .elem_stride = r_elem_stride,
            .elem_cnt    = 0,
            .slot_cnt    = r_slot_cnt,
            .rot         = 0,
#if ZETA_Core_DebugEnable
            .stamp = zeta::core::debug_utils::revision::GetStamp(),
#endif
        };

    std::vector<SampleObject> l_elems;
    std::vector<SampleObject> r_elems;

    (CommonInit)(l_ca, r_ca, l_elems, r_elems, l_elem_cnt, r_elem_cnt);

    {
        size_t l_vec_hash{ RollHash(0, l_elems, 0, l_elem_cnt) };
        size_t r_vec_hash{ RollHash(0, r_elems, 0, r_elem_cnt) };

        size_t l_ca_hash{ RollHash(0, l_ca, 0, l_elem_cnt) };
        size_t r_ca_hash{ RollHash(0, r_ca, 0, r_elem_cnt) };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(l_vec_hash == l_ca_hash);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(r_vec_hash == r_ca_hash);
    }

    zeta::core::seg_utils::SegEraseShoveL(
        l_ca,       // l_ca
        r_ca,       // r_ca
        rl_cnt,     // rl_cnt
        ers_cnt,    // ers_cnt
        shove_cnt,  // shove_cnt
        zeta::core::seq_endpoint::acceptor::BasicAcceptor{});

    ZETA_Core_DebugUtils_Diag_PromiseAssert(l_ca.elem_cnt ==
                                            l_elem_cnt + shove_cnt);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(r_ca.elem_cnt ==
                                            r_elem_cnt - ers_cnt - shove_cnt);

    {
        auto reader{ [](SampleObject const& elem) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                elem.statistics.default_construct == 0);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                elem.statistics.copy_construct_lhs +
                    elem.statistics.move_construct_lhs +
                    elem.statistics.copy_assign_lhs +
                    elem.statistics.move_assign_lhs <=
                1);
        } };

        (ReadCircularArray)(l_ca, 0, l_ca.elem_cnt, reader);
        (ReadCircularArray)(r_ca, 0, r_ca.elem_cnt, reader);
    }

    size_t expected_final_hash{ 0 };

    expected_final_hash =
        (RollHash)(expected_final_hash, l_elems, 0, l_elem_cnt);
    expected_final_hash = (RollHash)(expected_final_hash, r_elems, 0, rl_cnt);
    expected_final_hash =
        (RollHash)(expected_final_hash, r_elems, rl_cnt + ers_cnt,
                   r_elem_cnt - (rl_cnt + ers_cnt));

    size_t op_final_hash{ 0 };

    op_final_hash = (RollHash)(op_final_hash, l_ca, 0, l_ca.elem_cnt);
    op_final_hash = (RollHash)(op_final_hash, r_ca, 0, r_ca.elem_cnt);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(expected_final_hash ==
                                            op_final_hash);

    {
        std::vector<SampleObject> k;
        (GenRandomSampleObj)(k, l_ca.elem_cnt + r_ca.elem_cnt);

        size_t k_hash{ RollHash(0, k, 0, k.size()) };

        if (0 < k.size()) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                k_hash != expected_final_hash && k_hash != op_final_hash);
        }
    }

    l_ca.EraseAll(zeta::core::seq_cntr::Tag{},
                  zeta::core::seq_endpoint::acceptor::BasicAcceptor{});

    r_ca.EraseAll(zeta::core::seq_cntr::Tag{},
                  zeta::core::seq_endpoint::acceptor::BasicAcceptor{});

    zeta::core::debug_utils::memory::Free(l_ca.data);
    zeta::core::debug_utils::memory::Free(r_ca.data);
}

constexpr void TestSegEraseShoveR() {
    using SampleObject = zeta::core_test::sample_object::SampleObject;

    size_t l_elem_stride{ (GetRandomStride<SampleObject>)() };
    size_t r_elem_stride{ (GetRandomStride<SampleObject>)() };

    size_t l_slot_cnt{ (GenRandomSlotCnt)() };
    size_t r_slot_cnt{ (GenRandomSlotCnt)() };

    size_t l_elem_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, l_slot_cnt) };

    size_t r_elem_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, r_slot_cnt) };

    size_t lr_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, l_elem_cnt) };

    size_t ers_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, l_elem_cnt - lr_cnt) };

    size_t shove_cnt{ zeta::core_test::random::GenUniformRandomInt<size_t>(
        0, std::min(l_elem_cnt - ers_cnt, r_slot_cnt - r_elem_cnt)) };

    zeta::core::circular_array::Cntr<
        zeta::core_test::sample_object::SampleObject>
        l_ca{
            .data = zeta::core::debug_utils::memory::Malloc<SampleObject>(
                l_elem_stride * l_slot_cnt),
            .elem_stride = l_elem_stride,
            .elem_cnt    = 0,
            .slot_cnt    = l_slot_cnt,
            .rot         = 0,
#if ZETA_Core_DebugEnable
            .stamp = zeta::core::debug_utils::revision::GetStamp(),
#endif
        };

    zeta::core::circular_array::Cntr<
        zeta::core_test::sample_object::SampleObject>
        r_ca{
            .data = zeta::core::debug_utils::memory::Malloc<SampleObject>(
                r_elem_stride * r_slot_cnt),
            .elem_stride = r_elem_stride,
            .elem_cnt    = 0,
            .slot_cnt    = r_slot_cnt,
            .rot         = 0,
#if ZETA_Core_DebugEnable
            .stamp = zeta::core::debug_utils::revision::GetStamp(),
#endif
        };

    std::vector<SampleObject> l_elems;
    std::vector<SampleObject> r_elems;

    (CommonInit)(l_ca, r_ca, l_elems, r_elems, l_elem_cnt, r_elem_cnt);

    {
        size_t l_vec_hash{ RollHash(0, l_elems, 0, l_elem_cnt) };
        size_t r_vec_hash{ RollHash(0, r_elems, 0, r_elem_cnt) };

        size_t l_ca_hash{ RollHash(0, l_ca, 0, l_elem_cnt) };
        size_t r_ca_hash{ RollHash(0, r_ca, 0, r_elem_cnt) };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(l_vec_hash == l_ca_hash);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(r_vec_hash == r_ca_hash);
    }

    zeta::core::seg_utils::SegEraseShoveR(
        l_ca,       // l_ca
        r_ca,       // r_ca
        lr_cnt,     // lr_cnt
        ers_cnt,    // ers_cnt
        shove_cnt,  // shove_cnt
        zeta::core::seq_endpoint::acceptor::BasicAcceptor{});

    ZETA_Core_DebugUtils_Diag_PromiseAssert(l_ca.elem_cnt ==
                                            l_elem_cnt - ers_cnt - shove_cnt);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(r_ca.elem_cnt ==
                                            r_elem_cnt + shove_cnt);

    {
        auto reader{ [](SampleObject const& elem) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                elem.statistics.default_construct == 0);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                elem.statistics.copy_construct_lhs +
                    elem.statistics.move_construct_lhs +
                    elem.statistics.copy_assign_lhs +
                    elem.statistics.move_assign_lhs <=
                1);
        } };

        (ReadCircularArray)(l_ca, 0, l_ca.elem_cnt, reader);
        (ReadCircularArray)(r_ca, 0, r_ca.elem_cnt, reader);
    }

    size_t expected_final_hash{ 0 };

    expected_final_hash = (RollHash)(expected_final_hash, l_elems, 0,
                                     l_elem_cnt - lr_cnt - ers_cnt);
    expected_final_hash =
        (RollHash)(expected_final_hash, l_elems, l_elem_cnt - lr_cnt, lr_cnt);
    expected_final_hash =
        (RollHash)(expected_final_hash, r_elems, 0, r_elem_cnt);

    size_t op_final_hash{ 0 };

    op_final_hash = (RollHash)(op_final_hash, l_ca, 0, l_ca.elem_cnt);
    op_final_hash = (RollHash)(op_final_hash, r_ca, 0, r_ca.elem_cnt);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(expected_final_hash ==
                                            op_final_hash);

    {
        std::vector<SampleObject> k;
        (GenRandomSampleObj)(k, l_ca.elem_cnt + r_ca.elem_cnt);

        size_t k_hash{ RollHash(0, k, 0, k.size()) };

        if (0 < k.size()) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                k_hash != expected_final_hash && k_hash != op_final_hash);
        }
    }

    l_ca.EraseAll(zeta::core::seq_cntr::Tag{},
                  zeta::core::seq_endpoint::acceptor::BasicAcceptor{});

    r_ca.EraseAll(zeta::core::seq_cntr::Tag{},
                  zeta::core::seq_endpoint::acceptor::BasicAcceptor{});

    zeta::core::debug_utils::memory::Free(l_ca.data);
    zeta::core::debug_utils::memory::Free(r_ca.data);
}

constexpr void main1() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 1790008707 };

    unsigned seed{ random_seed };
    // unsigned seed{ fixed_seed };

    ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

    ZETA_Core_DebugUtils_Logging_ImmLogVar(random_seed);
    ZETA_Core_DebugUtils_Logging_ImmLogVar(fixed_seed);
    ZETA_Core_DebugUtils_Logging_ImmLogVar(seed);

    zeta::core_test::random::SetRandomSeed(seed);

    for (size_t i{ 0 }; i < 1024 * 16; ++i) {
        ZETA_Core_DebugUtils_Logging_ImmLogVar(i);

        (TestSegInsertShoveL)();
        (TestSegInsertShoveR)();
        (TestSegEraseShoveL)();
        (TestSegEraseShoveR)();
    }
}

int main() {
    unsigned long long beg_time{ zeta::core_test::GetTime() };
    ZETA_Core_DebugUtils_Logging_ImmLogVar(beg_time);

    main1();
    // main2();
    // main3();

    ZETA_Core_DebugUtils_Logging_ImmLogVar(beg_time);

    unsigned long long end_time{ zeta::core_test::GetTime() };
    ZETA_Core_DebugUtils_Logging_ImmLogVar(end_time);

    unsigned long long duration{ end_time - beg_time };
    ZETA_Core_DebugUtils_Logging_ImmLogVar(duration);

    ZETA_Core_DebugUtils_Logging_ImmLogVar("ok");

    return 0;
}
