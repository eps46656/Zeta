#include <stdio.h>
#include <zeta/core/define.h>

__attribute__((__section__("my_section"))) int glb_x;

//  extern int __start_my_section[1];

void main3() {
    ZETA_Core_PrintVar(_start_my_section);

    ZETA_Core_PrintVar((void*)&glb_x);
}

int main() {
    main3();
    printf("ok\n");
    return 0;
}
