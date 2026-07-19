#include <sanitizer/common_interface_defs.h>

#include <memory>

namespace zeta::core_test {

template <typename T>
T* alloc_mem(size_t size) {
    return static_cast<T*>(std::malloc(size));
}

template <typename T>
T* alloc_elem(size_t width, size_t stride, size_t cnt) {
    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= stride);

    if (cnt == 0) { return NULL; }

    if (width == stride) { return alloc_mem<T>(stride * cnt); }

    char* ret{ static_cast<char*>(std::malloc(stride * cnt)) };

#if __has_feature(address_sanitizer)
    for (size_t i{ 0 }; i < cnt; ++i) {
        char elem{ ret + stride * i };

        __asan_poison_memory_region(elem + width, stride - width);
    }
#endif

    return static_cast<T*>(static_cast<void*>(ret));
}

}  // namespace zeta::core_test
