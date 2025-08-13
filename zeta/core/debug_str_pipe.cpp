#include <zeta/core/debug_str_pipe.h>
#include <zeta/core/debugger.h>

#include <deque>

struct Zeta_Core_DebugStrPipe {
    std::deque<unsigned char> buffer;
};

void* Zeta_Core_DebugStrPipe_Create() { return new Zeta_Core_DebugStrPipe{}; }

void Zeta_Core_DebugStrPipe_Destroy(void* debug_str_pipe_) {
    Zeta_Core_DebugStrPipe* debug_str_pipe =
        (Zeta_Core_DebugStrPipe*)debug_str_pipe_;
    ZETA_Core_DebugAssert(debug_str_pipe != NULL);

    delete debug_str_pipe;
}

unsigned Zeta_Core_DebugStrPipe_GetType(void* debug_str_pipe_) {
    Zeta_Core_DebugStrPipe* debug_str_pipe =
        (Zeta_Core_DebugStrPipe*)debug_str_pipe_;
    ZETA_Core_DebugAssert(debug_str_pipe != NULL);

    return 0;
}

unsigned Zeta_Core_DebugStrPipe_GetFlags(void* debug_str_pipe_) {
    Zeta_Core_DebugStrPipe* debug_str_pipe =
        (Zeta_Core_DebugStrPipe*)debug_str_pipe_;
    ZETA_Core_DebugAssert(debug_str_pipe != NULL);

    return 0;
}

size_t Zeta_Core_DebugStrPipe_GetWidth(void* debug_str_pipe_) {
    Zeta_Core_DebugStrPipe* debug_str_pipe =
        (Zeta_Core_DebugStrPipe*)debug_str_pipe_;
    ZETA_Core_DebugAssert(debug_str_pipe != NULL);

    return 1;
}

size_t Zeta_Core_DebugStrPipe_GetSize(void* debug_str_pipe_) {
    Zeta_Core_DebugStrPipe* debug_str_pipe =
        (Zeta_Core_DebugStrPipe*)debug_str_pipe_;
    ZETA_Core_DebugAssert(debug_str_pipe != NULL);

    return debug_str_pipe->buffer.size();
}

void Zeta_Core_DebugStrPipe_Peek(void* debug_str_pipe_, size_t cnt,
                                 void* dst_) {
    Zeta_Core_DebugStrPipe* debug_str_pipe =
        (Zeta_Core_DebugStrPipe*)debug_str_pipe_;
    ZETA_Core_DebugAssert(debug_str_pipe != NULL);

    unsigned char* dst = (unsigned char*)dst_;
    ZETA_Core_DebugAssert(dst != NULL);

    ZETA_Core_DebugAssert(cnt <= debug_str_pipe->buffer.size());
}

void Zeta_Core_DebugStrPipe_Read(void* debug_str_pipe_, size_t cnt, void* dst_,
                                 size_t dst_stride, bool pop) {
    Zeta_Core_DebugStrPipe* debug_str_pipe =
        (Zeta_Core_DebugStrPipe*)debug_str_pipe_;
    ZETA_Core_DebugAssert(debug_str_pipe != NULL);

    unsigned char* dst = (unsigned char*)dst_;
    ZETA_Core_DebugAssert(dst != NULL);

    ZETA_Core_DebugAssert(cnt <= debug_str_pipe->buffer.size());

    for (size_t i = 0; i < cnt; ++i) {
        *dst = debug_str_pipe->buffer[i];
        dst += dst_stride;
    }

    if (pop) {
        for (size_t i = 0; i < cnt; ++i) { debug_str_pipe->buffer.pop_front(); }
    }
}

size_t Zeta_Core_DebugStrPipe_Write(void* debug_str_pipe_, size_t cnt,
                                    void const* src_, size_t stride) {
    Zeta_Core_DebugStrPipe* debug_str_pipe =
        (Zeta_Core_DebugStrPipe*)debug_str_pipe_;
    ZETA_Core_DebugAssert(debug_str_pipe != NULL);

    unsigned char const* src = (unsigned char const*)src_;
    ZETA_Core_DebugAssert(src != NULL);

    for (size_t i = 0; i < cnt; ++i, src += stride) {
        debug_str_pipe->buffer.push_back(*src);
    }

    return cnt;
}

void Zeta_Core_DebugStrPipe_Clear(void* debug_str_pipe_) {
    Zeta_Core_DebugStrPipe* debug_str_pipe =
        (Zeta_Core_DebugStrPipe*)debug_str_pipe_;

    ZETA_Core_DebugAssert(debug_str_pipe != NULL);

    debug_str_pipe->buffer.clear();
}

void Zeta_Core_DebugStrPipe_Flush(void* debug_str_pipe_) {
    Zeta_Core_DebugStrPipe* debug_str_pipe =
        (Zeta_Core_DebugStrPipe*)debug_str_pipe_;
    ZETA_Core_DebugAssert(debug_str_pipe != NULL);
}

Zeta_Core_Pipe_VTable const Zeta_Core_DebugStrPipe_pipe_vtable = {
    .GetType = Zeta_Core_DebugStrPipe_GetType,

    .GetFlags = Zeta_Core_DebugStrPipe_GetFlags,

    .GetWidth = Zeta_Core_DebugStrPipe_GetWidth,

    .GetSize = Zeta_Core_DebugStrPipe_GetSize,

    .Read = Zeta_Core_DebugStrPipe_Read,

    .Write = Zeta_Core_DebugStrPipe_Write,

    .Clear = Zeta_Core_DebugStrPipe_Clear,

    .Flush = Zeta_Core_DebugStrPipe_Flush,
};
