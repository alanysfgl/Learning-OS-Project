// heap.c
#include "heap.h"
#include "logger.h"

struct heap_block {
    u32int size;
    struct heap_block *next;
    u32int free;
};

static u32int heap_start = 0;
static u32int heap_end = 0;
static struct heap_block *heap_head = 0;

static u32int align8(u32int size) {
    return (size + 7) & ~7U;
}

void heap_init(u32int start, u32int size) {
    heap_start = start;
    heap_end = start + size;
    log_info("Heap init: start=0x%x size=%u\n", heap_start, size);

    heap_head = (struct heap_block *)heap_start;
    heap_head->size = size - sizeof(struct heap_block);
    heap_head->next = 0;
    heap_head->free = 1;
}

void *kmalloc(u32int size) {
    if (size == 0 || !heap_head) return 0;
    size = align8(size);

    struct heap_block *curr = heap_head;
    while (curr) {
        if (curr->free && curr->size >= size) {
            u32int remaining = curr->size - size;
            if (remaining > sizeof(struct heap_block) + 8) {
                struct heap_block *next = (struct heap_block *)((u32int)curr + sizeof(struct heap_block) + size);
                next->size = remaining - sizeof(struct heap_block);
                next->next = curr->next;
                next->free = 1;
                curr->next = next;
                curr->size = size;
            }
            curr->free = 0;
            return (void *)((u32int)curr + sizeof(struct heap_block));
        }
        curr = curr->next;
    }

    log_err("Heap out of memory (req=%u)\n", size);
    return 0;
}

static void coalesce(void) {
    struct heap_block *curr = heap_head;
    while (curr && curr->next) {
        u32int curr_end = (u32int)curr + sizeof(struct heap_block) + curr->size;
        if (curr->free && curr->next->free && curr_end == (u32int)curr->next) {
            curr->size += sizeof(struct heap_block) + curr->next->size;
            curr->next = curr->next->next;
            continue;
        }
        curr = curr->next;
    }
}

void kfree(void *ptr) {
    if (!ptr || !heap_head) return;
    u32int addr = (u32int)ptr;
    if (addr < heap_start + sizeof(struct heap_block) || addr >= heap_end) {
        return;
    }
    struct heap_block *block = (struct heap_block *)(addr - sizeof(struct heap_block));
    block->free = 1;
    coalesce();
}

void heap_stats(u32int *used, u32int *free, u32int *blocks, u32int *free_blocks) {
    u32int u = 0, f = 0, b = 0, fb = 0;
    for (struct heap_block *curr = heap_head; curr; curr = curr->next) {
        b++;
        if (curr->free) {
            f += curr->size;
            fb++;
        } else {
            u += curr->size;
        }
    }
    if (used) *used = u;
    if (free) *free = f;
    if (blocks) *blocks = b;
    if (free_blocks) *free_blocks = fb;
}
