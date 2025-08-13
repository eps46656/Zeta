#pragma

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

#define ZETA_Core_Environ_SectName ".environ"

#define ZETA_Core_Environ_DeclareVar(environ, type, name) \
    __attribute__((section(environ))) extern type name;

#define ZETA_Core_Environ_DefineVar(environ, type, name, val) \
    __attribute__((section(ZETA_Core_ToStr(environ)))) type name = (val);

#define ZETA_Core_Environ_Var(environ, base, name)               \
    ({                                                           \
        extern unsigned char ZETA_Core_Concat(__start_, environ) \
            section_start[];                                     \
    })

ZETA_Core_ExternC_End;
