#include <zeta/core/lock.h>

void Zeta_Core_SpinLock_Init(Zeta_Core_SpinLock* spin_lock) {
    spin_lock->x = 0;
}

bool_t Zeta_Core_SpinLock_TryLock(Zeta_Core_SpinLock* spin_lock) {
    return __sync_lock_test_and_set(&spin_lock->x, 1) == 0;
}

void Zeta_Core_SpinLock_Lock(Zeta_Core_SpinLock* spin_lock) {
    while (__sync_lock_test_and_set(&spin_lock->x, 1) == 1) {
        ZETA_Core_CPU_RELAX();
    }
}

void Zeta_Core_SpinLock_Unlock(Zeta_Core_SpinLock* spin_lock) {
    __sync_lock_release(&spin_lock->x);
}
