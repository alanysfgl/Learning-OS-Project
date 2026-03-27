#ifndef IDT_H
#define IDT_H

#include "common.h"

struct idt_entry_struct {
    u16int base_low;    // Adresin alt 16 biti
    u16int sel;         // Kernel segment seçicisi (GDT'den)
    u8int  always0;     // Her zaman 0
    u8int  flags;       // Erişim hakları ve kapı tipi
    u16int base_high;   // Adresin üst 16 biti
} __attribute__((packed));

struct idt_ptr_struct {
    u16int limit;
    u32int base;
} __attribute__((packed));

struct registers {
    u32int ds;
    u32int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32int int_no, err_code;
    u32int eip, cs, eflags, useresp, ss;
};

typedef void (*irq_handler_t)(struct registers* regs);

void init_idt();

#endif
