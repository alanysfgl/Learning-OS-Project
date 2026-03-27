// fat.h
#ifndef FAT_H
#define FAT_H

#include "common.h"

struct fat_file_info {
    char name[13];
    u32int size;
    u32int first_cluster;
};

void fat_init(void);
int fat_list_root(struct fat_file_info *out, u32int max);
int fat_read_file(const char *name, u8int *buffer, u32int max);
int fat_write_file(const char *name, const u8int *buffer, u32int size);

#endif
