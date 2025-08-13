

#include <zeta/core/define.h>

ZETA_Core_DeclareStruct(ClassA);
ZETA_Core_DeclareStruct(ClassB);
ZETA_Core_DeclareStruct(ClassB);
ZETA_Core_DeclareStruct(ClassA_VContext);

struct ClassA_VContext {
    int AF_context_offset;

    void (*AF)(void* context, void* vcontext, int x);
};

struct ClassA {
    int ax;
};

// ---

struct ClassB {
    ClassA a;

    //
};

void ClassB_AF(void* context, void* vcontext, int x) {
    printf("b.AF: %d\n", x);
}

ClassA_VContext class_b_class_a_vcontext = {
    .AF_context_offset = -offsetof(ClassB, a),

    .AF = ClassB_AF,
};

struct ClassB_VContext {
    int AF_context_offset;

    //
};
