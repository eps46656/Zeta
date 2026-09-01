#pragma once

#include <zeta/core/allocator.ipp>
#include <zeta/core/basic_bin_tree_node.ipp>
#include <zeta/core/bin_tree.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/lin_space_mapper.hpp>
#include <zeta/core/rbtree.ipp>

#pragma push_macro("MapperTplParamList")
#define MapperTplParamList typename SegAllocatorLike

#pragma push_macro("MapperTplArgList")
#define MapperTplArgList SegAllocatorLike

namespace zeta::core {

namespace lin_space_mapper::detail {

template <MapperTplParamList>
void CheckMapper_(Mapper<MapperTplArgList> const&) {}

inline Seg* NToSeg_(Node* n) {
    ZETA_Core_DebugAssert(n != nullptr);
    return ZETA_Core_MemberToStruct(Seg, n, n);
}

template <typename SegAllocator>
Seg* AllocateSeg_(SegAllocator& seg_alctr) {
    Seg* seg{ static_cast<Seg*>(
        allocator::SafeAllocate(seg_alctr, alignof(Seg), sizeof(Seg))) };

    seg->n.Init();

    return seg;
}

template <typename SegAllocator>
void DeallocateSeg_(Seg* seg, SegAllocator& seg_alctr) {
    allocator::Deallocate(seg_alctr, seg);
}

}  // namespace lin_space_mapper::detail

template <MapperTplParamList, typename SegAllocatorInitArg>
void lin_space_mapper::Init(Mapper<MapperTplArgList>& mapper,
                            SegAllocatorInitArg&& seg_alctr_init_arg) {
    lifecycle::Init(mapper.seg_alctr,
                    meta::Forward<SegAllocatorInitArg>(seg_alctr_init_arg));

    allocator::CheckContract(meta::GetInstRef(mapper.seg_alctr));

    mapper.root = nullptr;
}

template <MapperTplParamList>
void lin_space_mapper::Deinit(Mapper<MapperTplArgList>& mapper) {
    detail::CheckMapper_(mapper);

    if (mapper.root == nullptr) { return; }

    constexpr size_t buffer_capacity{ rbtree::max_height };
    Seg* buffer[buffer_capacity];
    size_t buffer_size{ 0 };

    buffer[buffer_size++] = mapper.root;

    while (0 < buffer_size) {
        Seg* seg{ buffer[--buffer_size] };

        Node* nl{ bin_tree::GetL(&seg->n) };
        Node* nr{ bin_tree::GetR(&seg->n) };

        detail::DeallocateSeg_(meta::GetInstRef(mapper.seg_alctr), seg);

        if (nl != nullptr) {
            ZETA_Core_DebugAssert(buffer_size < buffer_capacity);
            buffer[buffer_size++] = detail::NToSeg_(nl);
        }

        if (nr != nullptr) {
            ZETA_Core_DebugAssert(buffer_size < buffer_capacity);
            buffer[buffer_size++] = detail::NToSeg_(nr);
        }
    }
}

template <MapperTplParamList>
pair::Pair<bool, unsigned long long> lin_space_mapper::LookUp(
    Mapper<MapperTplArgList> const& mapper, unsigned long long src_pos) {
    detail::CheckMapper_(mapper);

    if (mapper.root == nullptr) { return { false, 0 }; }

    for (Seg* seg{ mapper.root };;) {
        unsigned long long seg_src_beg{ seg->src_beg };
        unsigned long long seg_size{ seg->size };

        if (src_pos < seg_src_beg) {
            Node* nl{ bin_tree::GetL(&seg->n) };
            if (nl == nullptr) { return { false, 0 }; }
            seg = detail::NToSeg_(nl);
            continue;
        }

        if (src_pos < seg_src_beg + seg_size) {
            return { true, seg->dst_beg + (src_pos - seg_src_beg) };
        }

        Node* nr{ bin_tree::GetR(&seg->n) };
        if (nr == nullptr) { return { false, 0 }; }
        seg = detail::NToSeg_(nr);
    }

    ZETA_Core_Unreachable();
}

template <MapperTplParamList>
void lin_space_mapper::SetSeg(Mapper<MapperTplArgList>& mapper,
                              unsigned long long src_beg,
                              unsigned long long dst_beg,
                              unsigned long long size) {
    detail::CheckMapper_(mapper);

    auto& seg_alctr{ meta::GetInstRef(mapper.seg_alctr) };

    if (size == 0) { return; }

    if (mapper.root == nullptr) {
        Seg* new_seg{ detail::AllocateSeg_(seg_alctr) };

        new_seg->n.SetPColor(rbtree::black);

        new_seg->src_beg = src_beg;
        new_seg->dst_beg = dst_beg;
        new_seg->size = size;

        mapper.root = new_seg;

        return;
    }

    Seg* lt_seg{ nullptr };
    Seg* ge_seg{ nullptr };

    for (Seg* seg{ mapper.root };;) {
        if (seg->src_beg < src_beg) {
            lt_seg = seg;

            Node* nr{ bin_tree::GetR(&seg->n) };
            if (nr == nullptr) { break; }
            seg = detail::NToSeg_(nr);
        } else {
            ge_seg = seg;

            Node* nl{ bin_tree::GetL(&seg->n) };
            if (nl == nullptr) { break; }
            seg = detail::NToSeg_(nl);
        }
    }

    unsigned long long src_end{ src_beg + size };

    Seg* l_seg;
    Seg* r_seg;

    if (lt_seg != nullptr) {
        unsigned long long lt_seg_src_beg{ lt_seg->src_beg };
        unsigned long long lt_seg_size{ lt_seg->size };
        unsigned long long lt_seg_src_end{ lt_seg_src_beg + lt_seg_size };

        if (src_beg <= lt_seg_src_end) {
            if (lt_seg->dst_beg - lt_seg_src_beg == dst_beg - src_beg) {
                lt_seg->size = src_end - lt_seg_src_beg;

                if (src_end <= lt_seg_src_end) { return; }

                l_seg = lt_seg;
                r_seg = ge_seg;

                goto EAT;
            }

            lt_seg->size = src_beg - lt_seg_src_beg;
        }
    }

    if (ge_seg != nullptr) {
        unsigned long long ge_seg_src_beg{ ge_seg->src_beg };
        unsigned long long ge_seg_size{ ge_seg->size };
        unsigned long long ge_seg_src_end{ ge_seg_src_beg + ge_seg_size };

        if (ge_seg_src_beg <= src_end) {
            if (ge_seg_src_end <= src_end) {
                ge_seg->src_beg = src_beg;
                ge_seg->dst_beg = dst_beg;
                ge_seg->size = size;

                if (ge_seg_src_end == src_end) { return; }

                l_seg = ge_seg;
                Node* rn{ bin_tree::StepR(&ge_seg->n) };
                if (rn == nullptr) { return; }
                r_seg = detail::NToSeg_(rn);

                goto EAT;
            }

            if (ge_seg->dst_beg - ge_seg_src_beg == dst_beg - src_beg) {
                ge_seg->src_beg = src_beg;
                ge_seg->dst_beg = dst_beg;
                ge_seg->size = ge_seg_src_end - src_beg;

                return;
            }

            ge_seg->src_beg = src_end;
            ge_seg->dst_beg += src_end - ge_seg_src_beg;
            ge_seg->size = ge_seg_src_end - src_end;
        }
    }

    {
        Seg* new_seg{ detail::AllocateSeg_(seg_alctr) };

        new_seg->src_beg = src_beg;
        new_seg->dst_beg = dst_beg;
        new_seg->size = size;

        mapper.root = detail::NToSeg_(rbtree::Insert(
            lt_seg == nullptr ? nullptr : &lt_seg->n,
            ge_seg == nullptr ? nullptr : &ge_seg->n, &new_seg->n));

        return;
    }

EAT: {
    unsigned long long l_seg_src_beg{ l_seg->src_beg };
    unsigned long long l_seg_size{ l_seg->size };
    unsigned long long l_seg_src_end{ l_seg_src_beg + l_seg_size };
    unsigned long long l_seg_offset{ l_seg->dst_beg - l_seg_src_beg };

    for (;;) {
        if (r_seg == nullptr) { break; }

        unsigned long long r_seg_src_beg{ r_seg->src_beg };
        unsigned long long r_seg_size{ r_seg->size };
        unsigned long long r_seg_src_end{ r_seg_src_beg + r_seg_size };

        if (r_seg_src_end <= l_seg_src_end) {
            mapper.root = ({
                Node* new_root_n{ rbtree::Extract(&r_seg->n) };
                new_root_n == nullptr ? nullptr : detail::NToSeg_(new_root_n);
            });
            detail::DeallocateSeg_(seg_alctr, r_seg);
            Node* rn{ bin_tree::StepR(&l_seg->n) };
            if (rn == nullptr) { break; }
            r_seg = detail::NToSeg_(rn);
            continue;
        }

        if (l_seg_src_end < r_seg_src_beg) { break; }

        unsigned long long r_seg_offset{ r_seg->dst_beg - r_seg_src_beg };

        if (l_seg_offset == r_seg_offset) {
            l_seg->size = r_seg_src_end - l_seg_src_beg;
            mapper.root = ({
                Node* new_root_n{ rbtree::Extract(&r_seg->n) };
                new_root_n == nullptr ? nullptr : detail::NToSeg_(new_root_n);
            });
            detail::DeallocateSeg_(seg_alctr, r_seg);
        } else {
            r_seg->src_beg = l_seg_src_end;
            r_seg->dst_beg += l_seg_src_end - r_seg_src_beg;
            r_seg->size = r_seg_src_end - l_seg_src_end;
        }

        break;
    }
}
}

template <MapperTplParamList>
void lin_space_mapper::ClearSeg(Mapper<MapperTplArgList>& mapper,
                                unsigned long long src_beg,
                                unsigned long long size) {
    detail::CheckMapper_(mapper);

    if (mapper.root == nullptr) { return; }

    auto& seg_alctr{ meta::GetInstRef(mapper.seg_alctr) };

    Seg* le_seg{ nullptr };

    for (Seg* seg{ mapper.root };;) {
        unsigned long long seg_src_beg{ seg->src_beg };

        if (seg_src_beg <= src_beg) {
            le_seg = seg;

            if (seg_src_beg == src_beg) { break; }

            Node* nr{ bin_tree::GetR(&seg->n) };
            if (nr == nullptr) { break; }
            seg = detail::NToSeg_(nr);
        } else {
            Node* nl{ bin_tree::GetL(&seg->n) };
            if (nl == nullptr) { break; }
            seg = detail::NToSeg_(nl);
        }
    }

    unsigned long long src_end{ src_beg + size };

    Seg* ge_seg;

    if (le_seg == nullptr) {
        ge_seg = detail::NToSeg_(bin_tree::GetMostL(&mapper.root->n).first);
    } else {
        unsigned long long le_seg_src_beg{ le_seg->src_beg };
        unsigned long long le_seg_size{ le_seg->size };
        unsigned long long le_seg_src_end{ le_seg_src_beg + le_seg_size };

        if (le_seg_src_beg == src_beg) {
            ge_seg = le_seg;
        } else {
            le_seg->size = src_beg - le_seg_src_beg;

            if (src_end == le_seg_src_end) { return; }

            if (src_end < le_seg_src_end) {
                Seg* new_seg{ detail::AllocateSeg_(seg_alctr) };

                new_seg->src_beg = src_end;
                new_seg->dst_beg = le_seg->dst_beg + (src_end - le_seg_src_beg);
                new_seg->size = le_seg_src_end - src_end;

                mapper.root =
                    detail::NToSeg_(rbtree::InsertR(&le_seg->n, &new_seg->n));

                return;
            }

            Node* rn{ bin_tree::StepR(&le_seg->n) };
            if (rn == nullptr) { return; }
            ge_seg = detail::NToSeg_(rn);
        }
    }

    for (;;) {
        unsigned long long ge_seg_src_beg{ ge_seg->src_beg };
        unsigned long long ge_seg_size{ ge_seg->size };
        unsigned long long ge_seg_src_end{ ge_seg_src_beg + ge_seg_size };

        if (src_end < ge_seg_src_end) {
            ge_seg->src_beg = src_end;
            ge_seg->dst_beg += src_end - ge_seg_src_beg;
            ge_seg->size = ge_seg_src_end - src_end;
            break;
        }

        if (ge_seg_src_end == src_end) {
            mapper.root = ({
                Node* new_root_n{ rbtree::Extract(&ge_seg->n) };
                new_root_n == nullptr ? nullptr : detail::NToSeg_(new_root_n);
            });
            detail::DeallocateSeg_(seg_alctr, ge_seg);
            break;
        }

        Node* nr{ bin_tree::StepR(&ge_seg->n) };
        mapper.root = ({
            Node* new_root_n{ rbtree::Extract(&ge_seg->n) };
            new_root_n == nullptr ? nullptr : detail::NToSeg_(new_root_n);
        });
        detail::DeallocateSeg_(seg_alctr, ge_seg);
        if (nr == nullptr) { break; }

        ge_seg = detail::NToSeg_(nr);
    }
}

template <MapperTplParamList>
void lin_space_mapper::Sanitize(Mapper<MapperTplArgList> const& mapper,
                                mem_recorder::MemRecorder* dst_seg) {
    detail::CheckMapper_(mapper);

    rbtree::Sanitize(nullptr, &mapper.root->n);

    if (mapper.root == nullptr) { return; }

    Seg* seg{ detail::NToSeg_(bin_tree::GetMostL(&mapper.root->n).first) };

    for (;;) {
        if (dst_seg != nullptr) {
            mem_recorder::Record(*dst_seg, seg, sizeof(Seg));
        }

        Node* rn{ bin_tree::StepR(&seg->n) };
        if (rn == nullptr) { break; }

        Seg* r_seg{ detail::NToSeg_(rn) };

        unsigned long long seg_src_beg{ seg->src_beg };
        unsigned long long seg_dst_beg{ seg->dst_beg };
        unsigned long long seg_size{ seg->size };
        unsigned long long seg_src_end{ seg_src_beg + seg_size };

        unsigned long long r_seg_src_beg{ r_seg->src_beg };
        unsigned long long r_seg_dst_beg{ r_seg->dst_beg };

        ZETA_Core_DebugAssert(
            (seg_src_end < r_seg_src_beg) ||
            (seg_src_end == r_seg_src_beg) &&
                (seg_dst_beg - seg_src_beg != r_seg_dst_beg - r_seg_src_beg));

        seg = r_seg;
    }
}

}  // namespace zeta::core

#pragma pop_macro("MapperTplArgList")
#pragma pop_macro("MapperTplParamList")
