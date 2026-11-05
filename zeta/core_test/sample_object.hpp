#pragma once

#include <iostream>
#include <zeta/core/comparison.ipp>
#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils/lifecycle_sanity.ipp>
#include <zeta/core/hash.hpp>
#include <zeta/core/hash_utils.ipp>
#include <zeta/core/utils.ipp>
#include <zeta/core_test/random.hpp>

namespace zeta::core_test::sample_object {

struct Statictics {
    size_t default_construct{ 0 };

    size_t copy_construct_lhs{ 0 };
    size_t copy_construct_rhs{ 0 };

    size_t move_construct_lhs{ 0 };
    size_t move_construct_rhs{ 0 };

    size_t copy_assign_lhs{ 0 };
    size_t copy_assign_rhs{ 0 };

    size_t move_assign_lhs{ 0 };
    size_t move_assign_rhs{ 0 };

    size_t acc_copy_cnt{ 0 };
    size_t acc_move_cnt{ 0 };

    constexpr bool operator==(Statictics const&) const = default;

    constexpr bool operator!=(Statictics const&) const = default;

    constexpr void Reset() {
        this->default_construct = 0;

        this->copy_construct_lhs = 0;
        this->copy_construct_rhs = 0;

        this->move_construct_lhs = 0;
        this->move_construct_rhs = 0;

        this->copy_assign_lhs = 0;
        this->copy_assign_rhs = 0;

        this->move_assign_lhs = 0;
        this->move_assign_rhs = 0;

        this->acc_copy_cnt = 0;
        this->acc_move_cnt = 0;
    }
};

struct SampleObject {
    static constexpr size_t width{ 7 };

    core::debug_utils::lifecycle_sanity::ObjHook obj_hook;

    unsigned char pre_padding[8];

    std::array<unsigned char, width> data1;
    std::array<unsigned char, width> data2;

    unsigned char post_padding[8];

    mutable Statictics statistics;

#pragma push_macro("ObjHookInitList")

#define ObjHookInitList                                                        \
    this, sizeof(*this),                                                       \
        {                                                                      \
            ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_FieldMember(          \
                pre_padding),                                                  \
            ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_StorageMember(data1), \
            ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_StorageMember(data2), \
            ZETA_Core_DebugUtils_LifecycleSanity_ObjHook_FieldMember(          \
                post_padding),                                                 \
        },

    constexpr SampleObject() : obj_hook{ ObjHookInitList } {
        this->obj_hook.AssertIsConstructed();

        ++this->statistics.default_construct;
    }

    constexpr SampleObject(SampleObject const& rhs)
        : obj_hook{ ObjHookInitList } {
        this->obj_hook.AssertIsConstructed();
        rhs.AssertIsValid();

        ++this->statistics.copy_construct_lhs;
        ++rhs.statistics.copy_construct_rhs;

        this->statistics.acc_copy_cnt = rhs.statistics.acc_copy_cnt + 1;
        this->statistics.acc_move_cnt = rhs.statistics.acc_move_cnt;

        this->data1 = rhs.data1;
        this->data2 = rhs.data2;
    }

    constexpr SampleObject(SampleObject&& rhs) : obj_hook{ ObjHookInitList } {
        this->obj_hook.AssertIsConstructed();
        rhs.AssertIsValid();

        ++this->statistics.move_construct_lhs;
        ++rhs.statistics.move_construct_rhs;

        this->statistics.acc_copy_cnt = rhs.statistics.acc_copy_cnt;
        this->statistics.acc_move_cnt = rhs.statistics.acc_move_cnt + 1;

        this->data1 = rhs.data1;
        this->data2 = rhs.data2;

        rhs.data1 = random::GenRandom<decltype(rhs.data1)>();
        rhs.data2 = random::GenRandom<decltype(rhs.data2)>();
    }

    constexpr SampleObject& operator=(SampleObject const& rhs) {
        this->AssertIsValid();
        rhs.AssertIsValid();

        ++this->statistics.copy_assign_lhs;
        ++rhs.statistics.copy_assign_rhs;

        this->statistics.acc_copy_cnt = rhs.statistics.acc_copy_cnt + 1;
        this->statistics.acc_move_cnt = rhs.statistics.acc_move_cnt;

        this->data1 = rhs.data1;
        this->data2 = rhs.data2;

        return *this;
    }

    constexpr SampleObject& operator=(SampleObject&& rhs) {
        this->AssertIsValid();
        rhs.AssertIsValid();

        ++this->statistics.move_assign_lhs;
        ++rhs.statistics.move_assign_rhs;

        this->statistics.acc_copy_cnt = rhs.statistics.acc_copy_cnt;
        this->statistics.acc_move_cnt = rhs.statistics.acc_move_cnt + 1;

        this->data1 = rhs.data1;
        this->data2 = rhs.data2;

        rhs.data1 = random::GenRandom<decltype(rhs.data1)>();
        rhs.data2 = random::GenRandom<decltype(rhs.data2)>();

        return *this;
    }

    constexpr void AssertIsValid() const {
        this->obj_hook.AssertIsConstructed();
    }

#pragma pop_macro("ObjHookInitList")
};

constexpr std::ostream& operator<<(std::ostream& os, SampleObject const& val) {
    char cs[]{ "0123456789ABCDEF" };

    for (size_t i{ SampleObject::width }; 0 < i--;) {
        os << cs[val.data1[i] / 16] << cs[val.data1[i] % 16];
    }

    for (size_t i{ SampleObject::width }; 0 < i--;) {
        os << cs[val.data2[i] / 16] << cs[val.data2[i] % 16];
    }

    return os;
}

}  // namespace zeta::core_test::sample_object

namespace zeta::core_test {

template <>
struct random::RandomCore<sample_object::SampleObject> {
    static sample_object::SampleObject F() {
        sample_object::SampleObject ret;

        for (size_t i{ 0 }; i < sample_object::SampleObject::width; ++i) {
            ret.data1[i] = GenUniformRandomInt<unsigned char>(0, 255);
        }

        for (size_t i{ 0 }; i < sample_object::SampleObject::width; ++i) {
            ret.data2[i] = (GenUniformRandomInt<unsigned char>)(0, 255);
        }

        return ret;
    }
};

}  // namespace zeta::core_test

namespace zeta::core {

template <>
struct hash::BasicHasher<core_test::sample_object::SampleObject> {
    static constexpr unsigned long long Hash(
        hash::Tag, core_test::sample_object::SampleObject const& x,
        unsigned long long salt) {
        return hash_utils::BasicMemHash(
            x.data1.data(), core_test::sample_object::SampleObject::width,
            salt);
    }
};

}  // namespace zeta::core

namespace zeta::core {

template <>
struct comparison::BasicComparator<core_test::sample_object::SampleObject,
                                   core_test::sample_object::SampleObject> {
    template <comparison::IsOpTag OpTag>
    static auto Compare(comparison::Tag, OpTag,
                        core_test::sample_object::SampleObject const& a,
                        core_test::sample_object::SampleObject const& b) {
        return comparison_utils::MemLexCompare(
            OpTag{}, a.data1.data(), b.data1.data(),
            core_test::sample_object::SampleObject::width,
            core_test::sample_object::SampleObject::width);
    }
};

}  // namespace zeta::core

namespace zeta::core_test::sample_object {

constexpr bool operator==(SampleObject const& x, SampleObject const& y) {
    return core::comparison::BasicCompare(core::comparison::OpTags::Equal{}, x,
                                          y);
}

constexpr bool operator!=(SampleObject const& x, SampleObject const& y) {
    return core::comparison::BasicCompare(core::comparison::OpTags::NotEqual{},
                                          x, y);
}

constexpr bool operator<(sample_object::SampleObject const& x,
                         SampleObject const& y) {
    return core::comparison::BasicCompare(core::comparison::OpTags::Less{}, x,
                                          y);
}

constexpr bool operator<=(sample_object::SampleObject const& x,
                          SampleObject const& y) {
    return core::comparison::BasicCompare(core::comparison::OpTags::LessEqual{},
                                          x, y);
}

constexpr bool operator>(SampleObject const& x, SampleObject const& y) {
    return core::comparison::BasicCompare(core::comparison::OpTags::Greater{},
                                          x, y);
}

constexpr bool operator>=(SampleObject const& x, SampleObject const& y) {
    return core::comparison::BasicCompare(
        core::comparison::OpTags::GreaterEqual{}, x, y);
}

}  // namespace zeta::core_test::sample_object
