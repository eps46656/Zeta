#pragma once

#include <zeta/core/basic_bin_tree_node.hpp>
#include <zeta/core/bin_tree.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/value_wrapper.hpp>

#pragma push_macro("MapperTplDeclParamList")
#define MapperTplDeclParamList typename SegAllocatorLike_

#pragma push_macro("MapperTplParamList")
#define MapperTplParamList typename SegAllocatorLike

#pragma push_macro("MapperTplArgList")
#define MapperTplArgList SegAllocatorLike

namespace zeta::core::lin_space_mapper {

using Node = basic_bin_tree_node::Node<
    void*,
    value_wrapper::TrueType,                     // PColorTag
    value_wrapper::FalseType,                    // LColorTag
    value_wrapper::FalseType,                    // RColorTag
    value_wrapper::FalseType,                    // AccSizeTag
    basic_bin_tree_node::PrimaryColorTagEnum::P  // PrimaryColoTag
    >;

struct Seg {
    Node n;

    unsigned long long src_beg;
    unsigned long long dst_beg;
    unsigned long long size;
};

template <MapperTplDeclParamList>
struct Mapper {
    using SegAllocatorLike = SegAllocatorLike_;

    Seg* root;

    SegAllocatorLike seg_alctr;
};

template <MapperTplParamList, typename SegAllocatorInitArg>
void Init(Mapper<MapperTplArgList>& mapper,
          SegAllocatorInitArg&& seg_alctr_init_arg);

template <MapperTplParamList>
void Deinit(Mapper<MapperTplArgList>& mapper);

template <MapperTplParamList>
pair::Pair<bool, unsigned long long> LookUp(
    Mapper<MapperTplArgList> const& mapper, unsigned long long pos);

template <MapperTplParamList>
void SetSeg(Mapper<MapperTplArgList>& mapper, unsigned long long src_beg,
            unsigned long long dst_beg, unsigned long long size);

template <MapperTplParamList>
void ClearSeg(Mapper<MapperTplArgList>& mapper, unsigned long long src_beg,
              unsigned long long size);

template <MapperTplParamList>
void Sanitize(Mapper<MapperTplArgList> const& mapper,
              mem_recorder::MemRecorder* dst_seg);

}  // namespace zeta::core::lin_space_mapper

namespace zeta::core {

template <MapperTplParamList>
struct lifecycle::Traits<lin_space_mapper::Mapper<MapperTplArgList>> {
    template <typename... Args>
    static void Init(lin_space_mapper::Mapper<MapperTplArgList>& mapper,
                     Args&&... args);

    static void Deinit(lin_space_mapper::Mapper<MapperTplArgList>& mapper);
};

}  // namespace zeta::core

#pragma pop_macro("MapperTplArgList")
#pragma pop_macro("MapperTplParamList")
#pragma pop_macro("MapperTplDeclParamList")
