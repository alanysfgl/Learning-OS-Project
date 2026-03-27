// smp.c
#include "smp.h"
#include "common.h"
#include "logger.h"

int smp_init(void) {
    u32int eax, ebx, ecx, edx;
    cpuid(1, &eax, &ebx, &ecx, &edx);
    if (!(edx & (1U << 9))) {
        log_warn("SMP: APIC not supported.\n");
        return -1;
    }
    log_info("SMP: APIC present (stub init).\n");
    return 0;
}
