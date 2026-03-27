// vfs.h
#ifndef VFS_H
#define VFS_H

#include "common.h"

#define VFS_NAME_MAX 128

enum vfs_node_type {
    VFS_FILE = 1,
    VFS_DIR  = 2
};

struct vfs_node;
struct vfs_dirent {
    char name[VFS_NAME_MAX];
    u32int ino;
};

typedef u32int (*vfs_read_fn)(struct vfs_node *node, u32int offset, u32int size, u8int *buffer);
typedef u32int (*vfs_write_fn)(struct vfs_node *node, u32int offset, u32int size, const u8int *buffer);
typedef struct vfs_dirent *(*vfs_readdir_fn)(struct vfs_node *node, u32int index);
typedef struct vfs_node *(*vfs_finddir_fn)(struct vfs_node *node, const char *name);

struct vfs_node {
    char name[VFS_NAME_MAX];
    u32int flags;
    u32int inode;
    u32int length;
    void *impl;
    vfs_read_fn read;
    vfs_write_fn write;
    vfs_readdir_fn readdir;
    vfs_finddir_fn finddir;
};

void vfs_init(void);
void vfs_set_root(struct vfs_node *root);
struct vfs_node *vfs_get_root(void);

u32int vfs_read(struct vfs_node *node, u32int offset, u32int size, u8int *buffer);
u32int vfs_write(struct vfs_node *node, u32int offset, u32int size, const u8int *buffer);
struct vfs_dirent *vfs_readdir(struct vfs_node *node, u32int index);
struct vfs_node *vfs_finddir(struct vfs_node *node, const char *name);
struct vfs_node *vfs_resolve_path(const char *path);

#endif
