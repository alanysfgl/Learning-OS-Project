// vfs.c
#include "vfs.h"
#include "logger.h"
#include "libc.h"

static struct vfs_node *vfs_root = 0;

void vfs_init(void) {
    vfs_root = 0;
}

void vfs_set_root(struct vfs_node *root) {
    vfs_root = root;
    log_info("VFS root set: %s\n", root ? root->name : "(null)");
}

struct vfs_node *vfs_get_root(void) {
    return vfs_root;
}

u32int vfs_read(struct vfs_node *node, u32int offset, u32int size, u8int *buffer) {
    if (!node || !node->read) return 0;
    return node->read(node, offset, size, buffer);
}

u32int vfs_write(struct vfs_node *node, u32int offset, u32int size, const u8int *buffer) {
    if (!node || !node->write) return 0;
    return node->write(node, offset, size, buffer);
}

struct vfs_dirent *vfs_readdir(struct vfs_node *node, u32int index) {
    if (!node || !node->readdir) return 0;
    return node->readdir(node, index);
}

struct vfs_node *vfs_finddir(struct vfs_node *node, const char *name) {
    if (!node || !node->finddir) return 0;
    return node->finddir(node, name);
}

struct vfs_node *vfs_resolve_path(const char *path) {
    if (!path || !*path) return 0;
    if (!vfs_root) return 0;

    struct vfs_node *cur = vfs_root;
    const char *p = path;

    while (*p == '/') p++;
    if (*p == 0) return cur;

    char name[VFS_NAME_MAX];
    while (*p) {
        u32int len = 0;
        while (*p && *p != '/') {
            if (len + 1 < VFS_NAME_MAX) {
                name[len++] = *p;
            }
            p++;
        }
        name[len] = 0;
        cur = vfs_finddir(cur, name);
        if (!cur) return 0;
        while (*p == '/') p++;
    }
    return cur;
}
