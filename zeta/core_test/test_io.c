#include <zeta/core/define.h>

//

extern void __stdcall syscall__write_(unsigned fd, char const* data,
                                      size_t data_len);

int main() {
    char const str[] = "Hello World\n";

    syscall__write_(fileno(stdout), str, sizeof(str) / sizeof(char));

    printf("fileno(stdout): %d\n", fileno(stdout));
    printf("hakdow\n");

    return 0;
}
