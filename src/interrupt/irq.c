#include "irq.h"
#include "pic.h"
#include "kprintf.h"
#include "panic.h"
#include "logger.h"
#include "syscall.h"

static irq_handler_t irq_handlers[16] = { 0 };

static const char *exception_names[32] = {
    "Divide By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};

void register_irq_handler(u8int irq, irq_handler_t handler) {
    if (irq < 16) {
        irq_handlers[irq] = handler;
    }
}

void irq_handler(struct registers regs) {
    // Exception / ISR
    if (regs.int_no < 32) {
        const char *name = exception_names[regs.int_no];
        PANIC_REGS(name ? name : "Unknown Exception", &regs);
    }
    else if (regs.int_no == 128) {
        syscall_handler(&regs);
    }
    else if (regs.int_no >= 32 && regs.int_no <= 47) {
        u8int irq = (u8int)(regs.int_no - 32);
        if (irq_handlers[irq]) {
            irq_handlers[irq](&regs);
        } else {
            log_warn("Unhandled IRQ: %u\n", (u32int)irq);
        }
        pic_send_eoi((u8int)regs.int_no);
    }
}
