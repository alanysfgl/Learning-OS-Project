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

void init_idt();