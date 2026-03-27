// memory.c
#include "memory.h"
#include "libc.h"
#include "logger.h"

extern u32int __kernel_end;

static u32int *frame_bitmap = 0;
static u32int total_frames = 0;
static u32int used_frames = 0;
static u32int bitmap_words = 0;
static u32int placement_addr = 0;

static u32int align_up(u32int addr, u32int align) {
    return (addr + align - 1) & ~(align - 1);
}

static void placement_init(void) {
    placement_addr = align_up((u32int)&__kernel_end, FRAME_SIZE);
}

static void *placement_alloc(u32int size, u32int align) {
    if (align) {
        placement_addr = align_up(placement_addr, align);
    }
    u32int addr = placement_addr;
    placement_addr += size;
    return (void *)addr;
}

static int test_frame(u32int frame) {
    return (frame_bitmap[frame / 32] >> (frame % 32)) & 1U;
}

static void set_frame(u32int frame) {
    if (!test_frame(frame)) {
        frame_bitmap[frame / 32] |= (1U << (frame % 32));
        used_frames++;
    }
}

static void clear_frame(u32int frame) {
    if (test_frame(frame)) {
        frame_bitmap[frame / 32] &= ~(1U << (frame % 32));
        used_frames--;
    }
}

static void reserve_region(u32int start, u32int length) {
    u32int s = align_up(start, FRAME_SIZE);
    u32int e = align_up(start + length, FRAME_SIZE);
    for (u32int addr = s; addr < e; addr += FRAME_SIZE) {
        set_frame(addr / FRAME_SIZE);
    }
}

static void free_region(u32int start, u32int length) {
    u32int s = align_up(start, FRAME_SIZE);
    u32int e = align_up(start + length, FRAME_SIZE);
    for (u32int addr = s; addr < e; addr += FRAME_SIZE) {
        clear_frame(addr / FRAME_SIZE);
    }
}

static u32int clamp_u64_to_u32(u64int v) {
    if (v > 0xFFFFFFFFULL) return 0xFFFFFFFFU;
    return (u32int)v;
}

void frame_allocator_init(struct multiboot_info *mb) {
    if (!mb || !(mb->flags & MULTIBOOT_FLAG_MMAP)) {
        log_warn("Frame allocator icin mmap yok.\n");
        return;
    }

    u64int max_addr = 0;
    u32int mmap_end = mb->mmap_addr + mb->mmap_length;
    struct multiboot_mmap_entry *entry =
        (struct multiboot_mmap_entry *)mb->mmap_addr;

    while ((u32int)entry < mmap_end) {
        u64int end = entry->addr + entry->len;
        if (end > max_addr) {
            max_addr = end;
        }
        entry = (struct multiboot_mmap_entry *)((u32int)entry + entry->size + sizeof(entry->size));
    }

    u32int max_addr32 = clamp_u64_to_u32(max_addr);
    total_frames = max_addr32 / FRAME_SIZE;
    if (total_frames == 0) {
        log_warn("Frame allocator: total_frames=0\n");
        return;
    }

    placement_init();
    bitmap_words = (total_frames + 31) / 32;
    frame_bitmap = (u32int *)placement_alloc(bitmap_words * sizeof(u32int), 4);
    memset(frame_bitmap, 0xFF, bitmap_words * sizeof(u32int));
    used_frames = total_frames;

    entry = (struct multiboot_mmap_entry *)mb->mmap_addr;
    while ((u32int)entry < mmap_end) {
        if (entry->type == 1) {
            u64int start64 = entry->addr;
            u64int end64 = entry->addr + entry->len;
            if (start64 < 0x100000000ULL) {
                if (end64 > 0x100000000ULL) end64 = 0x100000000ULL;
                u32int start = (u32int)start64;
                u32int length = (u32int)(end64 - start64);
                free_region(start, length);
            }
        }
        entry = (struct multiboot_mmap_entry *)((u32int)entry + entry->size + sizeof(entry->size));
    }

    reserve_region(0, placement_addr);
    reserve_region(mb->mmap_addr, mb->mmap_length);
    reserve_region((u32int)mb, sizeof(*mb));

    log_info("Frame allocator: total=%u used=%u free=%u\n",
             total_frames, used_frames, total_frames - used_frames);
}

u32int frame_alloc(void) {
    if (!frame_bitmap) return 0;
    for (u32int i = 0; i < bitmap_words; i++) {
        if (frame_bitmap[i] != 0xFFFFFFFFU) {
            for (u32int b = 0; b < 32; b++) {
                if (!(frame_bitmap[i] & (1U << b))) {
                    u32int frame = (i * 32) + b;
                    set_frame(frame);
                    return frame * FRAME_SIZE;
                }
            }
        }
    }
    return 0;
}

void frame_free(u32int addr) {
    if (!frame_bitmap) return;
    u32int frame = addr / FRAME_SIZE;
    if (frame < total_frames) {
        clear_frame(frame);
    }
}

u32int frame_total(void) {
    return total_frames;
}

u32int frame_used(void) {
    return used_frames;
}

u32int frame_free_count(void) {
    if (total_frames < used_frames) return 0;
    return total_frames - used_frames;
}

u32int frame_alloc_contiguous(u32int count) {
    if (!frame_bitmap || count == 0) return 0;
    if (count > total_frames) return 0;

    for (u32int start = 0; start + count <= total_frames; start++) {
        int ok = 1;
        for (u32int i = 0; i < count; i++) {
            if (test_frame(start + i)) {
                ok = 0;
                start += i;
                break;
            }
        }
        if (ok) {
            for (u32int i = 0; i < count; i++) {
                set_frame(start + i);
            }
            return start * FRAME_SIZE;
        }
    }
    return 0;
}

u32int frame_largest_free_run(void) {
    if (!frame_bitmap) return 0;
    u32int best = 0;
    u32int current = 0;
    for (u32int i = 0; i < total_frames; i++) {
        if (!test_frame(i)) {
            current++;
            if (current > best) best = current;
        } else {
            current = 0;
        }
    }
    return best;
}

void frame_stats(u32int *free_frames, u32int *largest_run) {
    if (free_frames) *free_frames = frame_free_count();
    if (largest_run) *largest_run = frame_largest_free_run();
}
