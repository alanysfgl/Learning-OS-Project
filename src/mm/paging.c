// paging.c
#include "paging.h"
#include "common.h"
#include "libc.h"
#include "logger.h"

#define PAGE_PRESENT 0x1
#define PAGE_RW      0x2
#define PAGE_USER    0x4

#define PAGE_SIZE    4096
#define TABLE_ENTRIES 1024

static u32int page_directory[TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static u32int page_tables_low[8][TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static u32int page_tables_high[8][TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

static void map_identity_range(u32int start_mb, u32int count_mb) {
    for (u32int mb = 0; mb < count_mb; mb++) {
        u32int table_index = start_mb + mb;
        if (table_index >= 8) {
            break;
        }
        for (u32int i = 0; i < TABLE_ENTRIES; i++) {
            u32int phys = ((table_index * TABLE_ENTRIES) + i) * PAGE_SIZE;
            page_tables_low[table_index][i] = phys | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        }
        page_directory[table_index] =
            ((u32int)&page_tables_low[table_index]) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    }
}

static void map_high_kernel(u32int base_mb, u32int count_mb) {
    u32int dir_base = 768; // 0xC0000000 / 4MB
    for (u32int mb = 0; mb < count_mb; mb++) {
        u32int table_index = base_mb + mb;
        if (table_index >= 8) {
            break;
        }
        for (u32int i = 0; i < TABLE_ENTRIES; i++) {
            u32int phys = ((table_index * TABLE_ENTRIES) + i) * PAGE_SIZE;
            page_tables_high[table_index][i] = phys | PAGE_PRESENT | PAGE_RW;
        }
        page_directory[dir_base + table_index] =
            ((u32int)&page_tables_high[table_index]) | PAGE_PRESENT | PAGE_RW;
    }
}

void paging_init(void) {
    memset(page_directory, 0, sizeof(page_directory));
    memset(page_tables_low, 0, sizeof(page_tables_low));
    memset(page_tables_high, 0, sizeof(page_tables_high));

    // Identity map first 32MB (8 tables).
    map_identity_range(0, 8);
    // Map kernel base at 0xC0000000 to same physical range (first 32MB).
    map_high_kernel(0, 8);

    asm volatile("mov %0, %%cr3" : : "r"(page_directory));

    u32int cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));

    log_info("Paging enabled (identity + high mapping).\n");
}
