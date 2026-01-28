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

void init_gdt();