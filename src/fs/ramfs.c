// ramfs.c
#include "ramfs.h"
#include "libc.h"
#include "logger.h"
#include "vfs.h"
#include "heap.h"

#define RAMFS_MAX_CHILDREN 16
#define RAMFS_MAX_FILES 8

struct ramfs_node {
    struct vfs_node vfs;
    struct ramfs_node *parent;
    struct ramfs_node *children[RAMFS_MAX_CHILDREN];
    u32int child_count;
    u8int *data;
    u32int size;
    int owns_data;
};

static struct ramfs_node nodes[RAMFS_MAX_FILES + 1];
static u32int node_count = 0;
static struct ramfs_node *ramfs_root = 0;

static struct vfs_dirent dirent_tmp;

static struct ramfs_node *ramfs_alloc_node(const char *name, u32int flags) {
    if (node_count >= (RAMFS_MAX_FILES + 1)) return 0;
    struct ramfs_node *node = &nodes[node_count++];
    memset(node, 0, sizeof(*node));
    if (name) {
        u32int len = strlen(name);
        if (len >= VFS_NAME_MAX) len = VFS_NAME_MAX - 1;
        memcpy(node->vfs.name, name, len);
        node->vfs.name[len] = 0;
    }
    node->vfs.flags = flags;
    return node;
}

static u32int ramfs_read(struct vfs_node *node, u32int offset, u32int size, u8int *buffer) {
    struct ramfs_node *r = (struct ramfs_node *)node->impl;
    if (!r || !r->data || offset >= r->size) return 0;
    if (offset + size > r->size) size = r->size - offset;
    memcpy(buffer, r->data + offset, size);
    return size;
}

static u32int ramfs_write(struct vfs_node *node, u32int offset, u32int size, const u8int *buffer) {
    struct ramfs_node *r = (struct ramfs_node *)node->impl;
    if (!r) return 0;
    if (offset + size > r->size) {
        if (!r->owns_data) {
            if (offset >= r->size) return 0;
            size = r->size - offset;
        } else {
            u32int new_size = offset + size;
            u8int *new_data = (u8int *)kmalloc(new_size);
            if (!new_data) return 0;
            if (r->data && r->size > 0) {
                memcpy(new_data, r->data, r->size);
                kfree(r->data);
            }
            r->data = new_data;
            r->size = new_size;
            r->vfs.length = new_size;
        }
    }
    if (!r->data) return 0;
    memcpy(r->data + offset, buffer, size);
    return size;
}

static struct vfs_dirent *ramfs_readdir(struct vfs_node *node, u32int index) {
    struct ramfs_node *r = (struct ramfs_node *)node->impl;
    if (!r || index >= r->child_count) return 0;
    struct ramfs_node *child = r->children[index];
    if (!child) return 0;
    memset(&dirent_tmp, 0, sizeof(dirent_tmp));
    memcpy(dirent_tmp.name, child->vfs.name, VFS_NAME_MAX - 1);
    dirent_tmp.ino = child->vfs.inode;
    return &dirent_tmp;
}

static struct vfs_node *ramfs_finddir(struct vfs_node *node, const char *name) {
    struct ramfs_node *r = (struct ramfs_node *)node->impl;
    if (!r || !name) return 0;
    for (u32int i = 0; i < r->child_count; i++) {
        struct ramfs_node *child = r->children[i];
        if (child && strcmp(child->vfs.name, name) == 0) {
            return &child->vfs;
        }
    }
    return 0;
}

void ramfs_init(void) {
    node_count = 0;
    ramfs_root = ramfs_alloc_node("/", VFS_DIR);
    struct ramfs_node *file = ramfs_alloc_node("hello.txt", VFS_FILE);
    static u8int hello_data[] = "hello from ramfs\n";
    file->data = hello_data;
    file->size = sizeof(hello_data) - 1;
    file->owns_data = 0;

    ramfs_root->children[ramfs_root->child_count++] = file;
    file->parent = ramfs_root;

    ramfs_root->vfs.readdir = ramfs_readdir;
    ramfs_root->vfs.finddir = ramfs_finddir;
    ramfs_root->vfs.impl = ramfs_root;

    file->vfs.read = ramfs_read;
    file->vfs.write = ramfs_write;
    file->vfs.length = file->size;
    file->vfs.impl = file;

    vfs_set_root(&ramfs_root->vfs);
    log_info("RAMFS initialized.\n");
}

static int ramfs_add_child(struct ramfs_node *parent, struct ramfs_node *child) {
    if (!parent || !child) return -1;
    if (parent->child_count >= RAMFS_MAX_CHILDREN) return -1;
    parent->children[parent->child_count++] = child;
    child->parent = parent;
    return 0;
}

int ramfs_mkdir(const char *name) {
    if (!ramfs_root || !name || !*name) return -1;
    if (ramfs_finddir(&ramfs_root->vfs, name)) return -1;
    struct ramfs_node *node = ramfs_alloc_node(name, VFS_DIR);
    if (!node) return -1;
    node->vfs.readdir = ramfs_readdir;
    node->vfs.finddir = ramfs_finddir;
    node->vfs.impl = node;
    return ramfs_add_child(ramfs_root, node);
}

int ramfs_create(const char *name) {
    if (!ramfs_root || !name || !*name) return -1;
    if (ramfs_finddir(&ramfs_root->vfs, name)) return -1;
    struct ramfs_node *node = ramfs_alloc_node(name, VFS_FILE);
    if (!node) return -1;
    node->data = 0;
    node->size = 0;
    node->owns_data = 1;
    node->vfs.read = ramfs_read;
    node->vfs.write = ramfs_write;
    node->vfs.length = 0;
    node->vfs.impl = node;
    return ramfs_add_child(ramfs_root, node);
}

int ramfs_unlink(const char *name) {
    if (!ramfs_root || !name || !*name) return -1;
    for (u32int i = 0; i < ramfs_root->child_count; i++) {
        struct ramfs_node *child = ramfs_root->children[i];
        if (!child) continue;
        if (strcmp(child->vfs.name, name) == 0) {
            if (child->vfs.flags == VFS_DIR && child->child_count > 0) {
                return -1;
            }
            if (child->owns_data && child->data) {
                kfree(child->data);
                child->data = 0;
            }
            for (u32int j = i; j + 1 < ramfs_root->child_count; j++) {
                ramfs_root->children[j] = ramfs_root->children[j + 1];
            }
            ramfs_root->children[ramfs_root->child_count - 1] = 0;
            ramfs_root->child_count--;
            return 0;
        }
    }
    return -1;
}
