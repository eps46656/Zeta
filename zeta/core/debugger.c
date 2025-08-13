#include <zeta/core/debug_str_pipe.h>
#include <zeta/core/debugger.h>
#include <zeta/core/pipe.h>
#include <zeta/core/utils.h>

Zeta_Core_Pipe Zeta_Core_debug_pipe = {
    .vtable = NULL,
    .context = NULL,
};

bool_t Zeta_Core_assert_stage = FALSE;

void Zeta_Core_Debugger_InitPipe() {
    if (Zeta_Core_debug_pipe.vtable != NULL) { return; }

    Zeta_Core_debug_pipe.vtable = &Zeta_Core_DebugStrPipe_pipe_vtable;
    Zeta_Core_debug_pipe.context = Zeta_Core_DebugStrPipe_Create();
}

void Zeta_Core_Debugger_ClearPipe() {
    if (Zeta_Core_debug_pipe.vtable != NULL) {
        ZETA_Core_Pipe_Clear(Zeta_Core_debug_pipe);
    }
}

void Zeta_Core_Debugger_FlushPipe() {
    if (Zeta_Core_debug_pipe.vtable == NULL) { return; }

    size_t const buffer_size = 32;
    unsigned char buffer[buffer_size + 1];

    for (size_t size = ZETA_Core_Pipe_GetSize(Zeta_Core_debug_pipe);
         0 < size;) {
        size_t cur_cnt = ZETA_Core_GetMinOf(size, buffer_size);
        size -= cur_cnt;

        ZETA_Core_Pipe_Read(Zeta_Core_debug_pipe, cur_cnt, buffer, 1, TRUE);
        buffer[cur_cnt] = '\0';

        printf("%s", buffer);
    }
}
