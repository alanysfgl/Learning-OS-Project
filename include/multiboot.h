// multiboot.h
#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include "common.h"

#define MULTIBOOT_MAGIC 0x2BADB002
#define MULTIBOOT_FLAG_MMAP 0x40

struct multiboot_info {
    u32int flags;
    u32int mem_lower;
    u32int mem_upper;
    u32int boot_device;
    u32int cmdline;
    u32int mods_count;
    u32int mods_addr;
    u32int syms[4];
    u32int mmap_length;
    u32int mmap_addr;
} __attribute__((packed));

struct multiboot_mmap_entry {
    u32int size;
    u64int addr;
    u64int len;
    u32int type;
} __attribute__((packed));

struct multiboot_info *multiboot_init(u32int magic, u32int addr);

#endif
