// heap.h
#ifndef HEAP_H
#define HEAP_H

#include "common.h"

void heap_init(u32int start, u32int size);
void *kmalloc(u32int size);
void kfree(void *ptr);
void heap_stats(u32int *used, u32int *free, u32int *blocks, u32int *free_blocks);

#endif
