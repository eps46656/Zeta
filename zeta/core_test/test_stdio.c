#define ZetaDir "../Zeta"

#include <unistd.h>

#include ZetaDir "/pipe.h"

#define ZETA_StdOutStream_buffer_size 128

ZETA_Core_DeclareStruct(Zeta_Core_StdOutStream);

struct Zeta_Core_StdOutStream {
    int fd;

    int buffer_i;
    int buffer_size;
    byte_t* buffer;
};

bool_t Zeta_Core_StdOutStream_IsEmpty(void* context) {
    ZETA_Core_Unused(context);
    return FALSE;
}

bool_t Zeta_Core_StdOutStream_Write(void* os_, void const* data_) {
    Zeta_Core_StdOutStream* os = os_;
    byte_t const* data = data_;

    int buffer_i = os->buffer_i;
    int buffer_size = os->buffer_size;

    if (buffer_size == 0) { write(os->fd, data, 1); }

    ZETA_Core_DebugAssert(0 <= buffer_i);
    ZETA_Core_DebugAssert(buffer_i < buffer_size);

    os->buffer[buffer_i] = *data;
    ++buffer_i;

    if (buffer_i == buffer_size) {
        write(os->fd, os->buffer, buffer_size);
        buffer_i = 0;
    }

    os->buffer_i = buffer_i;
}

void main1() {
    //

    //
}

int main() {
    main1();
    return 0;
}
