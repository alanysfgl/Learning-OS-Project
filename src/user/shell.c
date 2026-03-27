// shell.c
#include "shell.h"
#include "keyboard.h"
#include "monitor.h"
#include "kprintf.h"
#include "libc.h"
#include "vfs.h"
#include "fat.h"
#include "ramfs.h"
#include "heap.h"
#include "memory.h"

#define MAX_ARGS 8

static void shell_usage(const char *usage) {
    monitor_write((char *)usage);
    monitor_put('\n');
}

static char *ltrim(char *s) {
    while (*s == ' ' || *s == '\t') s++;
    return s;
}

static void rtrim(char *s) {
    int i = (int)strlen(s) - 1;
    while (i >= 0 && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n')) {
        s[i--] = 0;
    }
}

static int parse_line(char *line, char *argv[], int max_args) {
    int argc = 0;
    char *p = ltrim(line);
    rtrim(p);
    if (!*p) return 0;

    while (*p && argc < max_args) {
        if (*p == '"') {
            p++;
            argv[argc++] = p;
            while (*p && *p != '"') p++;
            if (*p == '"') {
                *p = 0;
                p++;
            }
        } else {
            argv[argc++] = p;
            while (*p && *p != ' ' && *p != '\t') p++;
        }
        while (*p == ' ' || *p == '\t') {
            *p = 0;
            p++;
        }
    }
    return argc;
}

static void shell_help(void) {
    monitor_write("Commands:\n");
    monitor_write("  help [cmd]\n");
    monitor_write("  ls [path]\n");
    monitor_write("  cat <path>\n");
    monitor_write("  fatls\n");
    monitor_write("  fatcat <file>\n");
    monitor_write("  fatwrite <file> <text>\n");
    monitor_write("  mkdir <name>\n");
    monitor_write("  touch <name>\n");
    monitor_write("  rm <name>\n");
    monitor_write("  heap\n");
    monitor_write("  mem\n");
}

static void shell_ls_path(const char *path) {
    struct vfs_node *node = 0;
    if (!path || !*path) {
        node = vfs_get_root();
    } else {
        node = vfs_resolve_path(path);
    }
    if (!node) {
        monitor_write("not found\n");
        return;
    }
    if (!(node->flags & VFS_DIR)) {
        monitor_write("not a directory\n");
        return;
    }
    for (u32int i = 0; ; i++) {
        struct vfs_dirent *ent = vfs_readdir(node, i);
        if (!ent) break;
        kprintf("%s\n", ent->name);
    }
}

static void shell_ls(void) {
    shell_ls_path(0);
}

static void shell_help_cmd(const char *cmd) {
    if (!cmd || !*cmd) {
        shell_help();
        return;
    }
    if (strcmp(cmd, "ls") == 0) {
        shell_usage("usage: ls [path]");
    } else if (strcmp(cmd, "cat") == 0) {
        shell_usage("usage: cat <path>");
    } else if (strcmp(cmd, "fatls") == 0) {
        shell_usage("usage: fatls");
    } else if (strcmp(cmd, "fatcat") == 0) {
        shell_usage("usage: fatcat <file>");
    } else if (strcmp(cmd, "fatwrite") == 0) {
        shell_usage("usage: fatwrite <file> <text>");
    } else if (strcmp(cmd, "mkdir") == 0) {
        shell_usage("usage: mkdir <name>");
    } else if (strcmp(cmd, "touch") == 0) {
        shell_usage("usage: touch <name>");
    } else if (strcmp(cmd, "rm") == 0) {
        shell_usage("usage: rm <name>");
    } else if (strcmp(cmd, "heap") == 0) {
        shell_usage("usage: heap");
    } else if (strcmp(cmd, "mem") == 0) {
        shell_usage("usage: mem");
    } else {
        monitor_write("unknown command\n");
    }
}
static void shell_cat(const char *path) {
    if (!path || !*path) {
        monitor_write("usage: cat <path>\n");
        return;
    }
    struct vfs_node *node = vfs_resolve_path(path);
    if (!node) {
        monitor_write("not found\n");
        return;
    }
    u8int buf[512];
    u32int off = 0;
    while (1) {
        u32int n = vfs_read(node, off, sizeof(buf), buf);
        if (n == 0) break;
        for (u32int i = 0; i < n; i++) {
            monitor_put((char)buf[i]);
        }
        off += n;
    }
    monitor_put('\n');
}

static void shell_fatls(void) {
    struct fat_file_info list[16];
    int count = fat_list_root(list, 16);
    if (count <= 0) {
        monitor_write("fat: no entries\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        kprintf("%s (%u)\n", list[i].name, list[i].size);
    }
}

static void shell_fatcat(const char *name) {
    if (!name || !*name) {
        monitor_write("usage: fatcat <file>\n");
        return;
    }
    static u8int buf[1024];
    int n = fat_read_file(name, buf, sizeof(buf));
    if (n <= 0) {
        monitor_write("fat: not found or read failed\n");
        return;
    }
    for (int i = 0; i < n; i++) {
        monitor_put((char)buf[i]);
    }
    monitor_put('\n');
}

static void shell_fatwrite(const char *name, const char *text) {
    if (!name || !*name || !text) {
        monitor_write("usage: fatwrite <file> <text>\n");
        return;
    }
    if (fat_write_file(name, (const u8int *)text, strlen(text)) == 0) {
        monitor_write("ok\n");
    } else {
        monitor_write("fatwrite failed\n");
    }
}

static void shell_heap(void) {
    u32int used = 0, free = 0, blocks = 0, free_blocks = 0;
    heap_stats(&used, &free, &blocks, &free_blocks);
    kprintf("heap: used=%u free=%u blocks=%u free_blocks=%u\n", used, free, blocks, free_blocks);
}

static void shell_mem(void) {
    u32int total = frame_total();
    u32int used = frame_used();
    u32int free = frame_free_count();
    u32int largest = frame_largest_free_run();
    kprintf("frames: total=%u used=%u free=%u largest_run=%u\n", total, used, free, largest);
}

void shell_run(void) {
    char line[128];
    shell_help();
    while (1) {
        monitor_write("\n> ");
        int len = keyboard_readline(line, sizeof(line));
        if (len <= 0) continue;

        char *argv[MAX_ARGS];
        int argc = parse_line(line, argv, MAX_ARGS);
        if (argc == 0) continue;

        const char *cmd = argv[0];
        if (strcmp(cmd, "help") == 0) {
            shell_help_cmd(argc > 1 ? argv[1] : 0);
        } else if (strcmp(cmd, "ls") == 0) {
            if (argc > 1) shell_ls_path(argv[1]);
            else shell_ls();
        } else if (strcmp(cmd, "cat") == 0) {
            if (argc < 2) shell_usage("usage: cat <path>");
            else shell_cat(argv[1]);
        } else if (strcmp(cmd, "fatls") == 0) {
            shell_fatls();
        } else if (strcmp(cmd, "fatcat") == 0) {
            if (argc < 2) shell_usage("usage: fatcat <file>");
            else shell_fatcat(argv[1]);
        } else if (strcmp(cmd, "fatwrite") == 0) {
            if (argc < 3) shell_usage("usage: fatwrite <file> <text>");
            else shell_fatwrite(argv[1], argv[2]);
        } else if (strcmp(cmd, "mkdir") == 0) {
            if (argc < 2) shell_usage("usage: mkdir <name>");
            else if (ramfs_mkdir(argv[1]) == 0) monitor_write("ok\n");
            else monitor_write("mkdir failed\n");
        } else if (strcmp(cmd, "touch") == 0) {
            if (argc < 2) shell_usage("usage: touch <name>");
            else if (ramfs_create(argv[1]) == 0) monitor_write("ok\n");
            else monitor_write("touch failed\n");
        } else if (strcmp(cmd, "rm") == 0) {
            if (argc < 2) shell_usage("usage: rm <name>");
            else if (ramfs_unlink(argv[1]) == 0) monitor_write("ok\n");
            else monitor_write("rm failed\n");
        } else if (strcmp(cmd, "heap") == 0) {
            shell_heap();
        } else if (strcmp(cmd, "mem") == 0) {
            shell_mem();
        } else {
            monitor_write("unknown command\n");
        }
    }
}
