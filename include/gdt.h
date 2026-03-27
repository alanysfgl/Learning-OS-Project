#ifndef GDT_H
#define GDT_H

#include "common.h"

struct gdt_entry_struct {
    u16int limit_low;           // Segmentin sınırı (alt 16 bit)
    u16int base_low;            // Başlangıç adresi (alt 16 bit)
    u8int  base_middle;         // Başlangıç adresi (orta 8 bit)
    u8int  access;              // Erişim hakları
    u8int  granularity;
    u8int  base_high;           // Başlangıç adresi (üst 8 bit)
} __attribute__((packed));

struct gdt_ptr_struct {
    u16int limit;               // Tablonun boyutu
    u32int base;                // Tablonun adresi
} __attribute__((packed));

struct tss_entry_struct {
    u32int prev_tss;
    u32int esp0;
    u32int ss0;
    u32int esp1;
    u32int ss1;
    u32int esp2;
    u32int ss2;
    u32int cr3;
    u32int eip;
    u32int eflags;
    u32int eax;
    u32int ecx;
    u32int edx;
    u32int ebx;
    u32int esp;
    u32int ebp;
    u32int esi;
    u32int edi;
    u32int es;
    u32int cs;
    u32int ss;
    u32int ds;
    u32int fs;
    u32int gs;
    u32int ldt;
    u16int trap;
    u16int iomap_base;
} __attribute__((packed));

void init_gdt();
void set_kernel_stack(u32int stack);

#endif
