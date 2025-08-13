#include <stdio.h>
#include <zeta/core/debug_str_pipe.h>
#include <zeta/core/debugger.h>
#include <zeta/core/io.h>

void PrintPipe(Zeta_Core_Pipe* pipe) {
    unsigned char c;

    while (!pipe->IsEmpty(pipe->context)) {
        pipe->Read(pipe->context, 1, &c);
        printf("%c", c);
    }
}

void main2() {
    ZETA_Core_PrintCurPos;

    unsigned x = 5;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintCurPos;

    Zeta_Core_Debugger_FlushPipe();

    // unsigned char const str[] = ":";

    // printf("%s\n", "abcdefg\0");
}

int main() {
    main2();
    printf("ok\n");
    return 0;
}
