#pragma once

#include <zeta/core/define.h>

ZETA_Core_ExternC_Beg;

ZETA_Core_DeclareStruct(Zeta_Core_SpinLock);

struct Zeta_Core_SpinLock {
    volatile int x;
};

void Zeta_Core_SpinLock_Init(Zeta_Core_SpinLock* spin_lock);

bool_t Zeta_Core_SpinLock_TryLock(Zeta_Core_SpinLock* spin_lock);

void Zeta_Core_SpinLock_Lock(Zeta_Core_SpinLock* spin_lock);

void Zeta_Core_SpinLock_Unlock(Zeta_Core_SpinLock* spin_lock);

ZETA_Core_ExternC_End;
