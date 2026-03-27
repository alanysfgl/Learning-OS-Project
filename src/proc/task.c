// task.c
#include "task.h"

void task_context_init(struct task_context *ctx, u32int entry, u32int stack_top) {
    if (!ctx) return;
    u32int *sp = (u32int *)stack_top;
    // Build an interrupt frame compatible with common_stub restore path.
    *--sp = 0x202;       // EFLAGS (IF=1)
    *--sp = 0x08;        // CS (kernel code)
    *--sp = entry;       // EIP
    *--sp = 0;           // ERR_CODE
    *--sp = 0;           // INT_NO
    *--sp = 0;           // EAX
    *--sp = 0;           // ECX
    *--sp = 0;           // EDX
    *--sp = 0;           // EBX
    *--sp = 0;           // ESP (dummy for popa)
    *--sp = 0;           // EBP
    *--sp = 0;           // ESI
    *--sp = 0;           // EDI
    *--sp = 0x10;        // DS (kernel data)
    ctx->irq_esp = (u32int)sp;
}
