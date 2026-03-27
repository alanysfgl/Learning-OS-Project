// memory.h
#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"
#include "multiboot.h"

#define FRAME_SIZE 4096

void frame_allocator_init(struct multiboot_info *mb);
u32int frame_alloc(void);
void frame_free(u32int addr);
u32int frame_total(void);
u32int frame_used(void);
u32int frame_free_count(void);
u32int frame_alloc_contiguous(u32int count);
u32int frame_largest_free_run(void);
void frame_stats(u32int *free_frames, u32int *largest_run);

#endif
