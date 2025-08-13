#include <define.h>
#include <stdio.h>

int glb_x __attribute__((section("my_section"))) = 3;

extern int _start_my_section;
extern int _stop_my_section;

void main3() {
    ZETA_Core_PrintVar((void*)&_start_my_section);

    ZETA_Core_PrintVar((void*)&glb_x);
}

int main() {
    main3();
    printf("ok\n");
    return 0;
}
