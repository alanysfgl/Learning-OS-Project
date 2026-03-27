// storage.h
#ifndef STORAGE_H
#define STORAGE_H

#include "common.h"

struct storage_device;

typedef int (*storage_read_fn)(struct storage_device *dev, u32int lba, u8int count, void *buffer);
typedef int (*storage_write_fn)(struct storage_device *dev, u32int lba, u8int count, const void *buffer);

struct storage_device {
    const char *name;
    u32int sector_size;
    u32int sector_count;
    storage_read_fn read;
    storage_write_fn write;
    void *driver_data;
};

int storage_register(struct storage_device *dev);
struct storage_device *storage_get_default(void);

#endif
