#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/ptr_utils.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::basic_bin_tree_node {

template <typename LinkType_, typename PColorTag_, typename LColorTag_,
          typename RColorTag_, typename AccSizeTag_>
struct Node {
    using LinkType = LinkType_;
    using PColorTag = PColorTag_;
    using LColorTag = LColorTag_;
    using RColorTag = RColorTag_;
    using AccSizeTag = AccSizeTag_;

    ZETA_Core_StaticAssert(meta::IsAnyOf<AccSizeTag, value_wrapper::TrueType,
                                         value_wrapper::FalseType>);

    ptr_utils::AugPtrTpl<LinkType, PColorTag> p;
    ptr_utils::AugPtrTpl<LinkType, LColorTag> l;
    ptr_utils::AugPtrTpl<LinkType, RColorTag> r;

    static constexpr bool IsRelLink{
        ptr_utils::AugPtrTpl<LinkType, PColorTag>::IsRelLink
    };

    meta::Conditional<AccSizeTag::value, size_t, meta::Monostate> acc_size;

    void Init();

    Node* GetPPtr();
    Node* GetLPtr();
    Node* GetRPtr();

    Node const* GetPPtr() const;
    Node const* GetLPtr() const;
    Node const* GetRPtr() const;

    unsigned GetPColor() const;
    unsigned GetLColor() const;
    unsigned GetRColor() const;

    void SetPPtr(Node* m);
    void SetLPtr(Node* m);
    void SetRPtr(Node* m);

    void SetPColor(unsigned color);
    void SetLColor(unsigned color);
    void SetRColor(unsigned color);

    template <typename _ = void,
              typename = meta::EnableIf<AccSizeTag::value, _>>
    size_t GetAccSize() const;

    template <typename _ = void,
              typename = meta::EnableIf<AccSizeTag::value, _>>
    void SetAccSize(size_t acc_size);
};

}  // namespace zeta::core::basic_bin_tree_node
