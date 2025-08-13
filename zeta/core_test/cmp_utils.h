#pragma once

#include <zeta/core/define.h>

#define DefineCompareFunction_(Value)                    \
    int Compare(void const* context, Value x, Value y) { \
        ZETA_Core_Unused(context);                       \
        return ZETA_Core_ThreeWayCompare(x, y);          \
    }                                                    \
    ZETA_Core_StaticAssert(TRUE)

template <typename ValueA, typename ValueB>
struct CompareCore {
    int operator()(void const* context, ValueA const& a,
                   ValueB const& b) const {
        ZETA_Core_Unused(context);
        return ZETA_Core_ThreeWayCompare(a, b);
    }
};

template <typename ValueA, typename ValueB>
int Compare(void const* context, ValueA const& a, ValueB const& b) {
    return CompareCore<ValueA, ValueB>{}(context, a, b);
}

template <typename ValueA, typename ValueB>
int ZetaCompare(void const* context, void const* a, void const* b) {
    return Compare<ValueA, ValueB>(context, *(ValueA const*)a,
                                   *(ValueB const*)b);
}
