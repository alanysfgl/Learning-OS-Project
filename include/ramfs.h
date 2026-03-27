// ramfs.h
#ifndef RAMFS_H
#define RAMFS_H

#include "vfs.h"

void ramfs_init(void);
int ramfs_mkdir(const char *name);
int ramfs_create(const char *name);
int ramfs_unlink(const char *name);

#endif
