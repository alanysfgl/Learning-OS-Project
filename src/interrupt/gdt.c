#include "gdt.h"
#include "libc.h"

struct gdt_entry_struct gdt_entries[6];
struct gdt_ptr_struct   gdt_ptr;
struct tss_entry_struct tss_entry;

extern void gdt_flush(u32int);
extern void tss_flush();

void gdt_set_gate(s32int num, u32int base, u32int limit, u8int access, u8int gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;
    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

static void write_tss(int num, u16int ss0, u32int esp0) {
    u32int base = (u32int)&tss_entry;
    u32int limit = sizeof(struct tss_entry_struct) - 1;

    gdt_set_gate(num, base, limit, 0x89, 0x40);
    memset(&tss_entry, 0, sizeof(struct tss_entry_struct));

    tss_entry.ss0 = ss0;
    tss_entry.esp0 = esp0;
    tss_entry.cs = 0x0B;
    tss_entry.ss = 0x13;
    tss_entry.ds = 0x13;
    tss_entry.es = 0x13;
    tss_entry.fs = 0x13;
    tss_entry.gs = 0x13;
    tss_entry.iomap_base = sizeof(struct tss_entry_struct);
}

void init_gdt() {
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct) * 6) - 1;
    gdt_ptr.base  = (u32int)&gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data
    write_tss(5, 0x10, 0);

    gdt_flush((u32int)&gdt_ptr);
    tss_flush();
}

void set_kernel_stack(u32int stack) {
    tss_entry.esp0 = stack;
}
