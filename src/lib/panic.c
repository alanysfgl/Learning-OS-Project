// panic.c
#include "panic.h"
#include "kprintf.h"
#include "idt.h"

static void dump_backtrace(u32int ebp) {
    kprintf("Backtrace:\n");
    u32int *frame = (u32int *)ebp;
    for (int i = 0; i < 10; i++) {
        if (!frame) {
            break;
        }
        u32int next_ebp = frame[0];
        u32int ret_addr = frame[1];
        kprintf("  #%d ebp=%p ret=%p\n", i, (void *)frame, (void *)ret_addr);
        if ((next_ebp & 0x3) != 0 || next_ebp <= (u32int)frame) {
            break;
        }
        frame = (u32int *)next_ebp;
    }
}

static void dump_stack(u32int esp) {
    kprintf("Stack (top 16 words):\n");
    u32int *ptr = (u32int *)esp;
    for (int i = 0; i < 16; i++) {
        kprintf("  [%p] %p\n", (void *)&ptr[i], (void *)ptr[i]);
    }
}

void panic(const char *msg, const char *file, u32int line) {
    asm volatile("cli");
    kprintf("KERNEL PANIC: %s (%s:%u)\n", msg, file, line);
    for (;;) {
        asm volatile("hlt");
    }
}

void panic_with_regs(const char *msg, const char *file, u32int line, struct registers *regs) {
    asm volatile("cli");
    kprintf("KERNEL PANIC: %s (%s:%u)\n", msg, file, line);
    if (regs) {
        kprintf("INT=%u ERR=%u\n", (u32int)regs->int_no, (u32int)regs->err_code);
        kprintf("EIP=%p CS=%p EFLAGS=%p\n", (void *)regs->eip, (void *)regs->cs, (void *)regs->eflags);
        kprintf("EAX=%p EBX=%p ECX=%p EDX=%p\n",
                (void *)regs->eax, (void *)regs->ebx, (void *)regs->ecx, (void *)regs->edx);
        kprintf("ESI=%p EDI=%p EBP=%p ESP=%p\n",
                (void *)regs->esi, (void *)regs->edi, (void *)regs->ebp, (void *)regs->esp);
        dump_backtrace(regs->ebp);
        dump_stack(regs->esp);
    }
    for (;;) {
        asm volatile("hlt");
    }
}
