// multiboot.c
#include "multiboot.h"
#include "kprintf.h"
#include "logger.h"

struct multiboot_info *multiboot_init(u32int magic, u32int addr) {
    if (magic != MULTIBOOT_MAGIC) {
        log_warn("Multiboot magic yok: 0x%x\n", magic);
        return 0;
    }

    struct multiboot_info *mb = (struct multiboot_info *)addr;
    log_info("Multiboot OK. flags=0x%x\n", mb->flags);

    if (mb->flags & MULTIBOOT_FLAG_MMAP) {
        u32int mmap_end = mb->mmap_addr + mb->mmap_length;
        struct multiboot_mmap_entry *entry =
            (struct multiboot_mmap_entry *)mb->mmap_addr;

        log_info("Memory map:\n");
        while ((u32int)entry < mmap_end) {
            kprintf("  base=%llu len=%llu type=%u\n",
                    (u64int)entry->addr,
                    (u64int)entry->len,
                    entry->type);
            entry = (struct multiboot_mmap_entry *)((u32int)entry + entry->size + sizeof(entry->size));
        }
    } else {
        log_warn("Memory map yok (flags=0x%x)\n", mb->flags);
    }

    return mb;
}
