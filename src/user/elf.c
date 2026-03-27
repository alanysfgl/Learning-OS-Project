// elf.c
#include "elf.h"
#include "libc.h"
#include "logger.h"

#define ELF_MAGIC 0x464C457F

struct elf_header {
    u32int magic;
    u8int  class;
    u8int  data;
    u8int  version;
    u8int  osabi;
    u8int  pad[8];
    u16int type;
    u16int machine;
    u32int version2;
    u32int entry;
    u32int phoff;
    u32int shoff;
    u32int flags;
    u16int ehsize;
    u16int phentsize;
    u16int phnum;
    u16int shentsize;
    u16int shnum;
    u16int shstrndx;
} __attribute__((packed));

struct elf_phdr {
    u32int type;
    u32int offset;
    u32int vaddr;
    u32int paddr;
    u32int filesz;
    u32int memsz;
    u32int flags;
    u32int align;
} __attribute__((packed));

int elf_load(const u8int *image, u32int size, u32int *entry_out) {
    if (!image || size < sizeof(struct elf_header)) return -1;
    const struct elf_header *eh = (const struct elf_header *)image;
    if (eh->magic != ELF_MAGIC) return -1;
    if (eh->phoff + (eh->phnum * eh->phentsize) > size) return -1;

    for (u16int i = 0; i < eh->phnum; i++) {
        const struct elf_phdr *ph = (const struct elf_phdr *)(image + eh->phoff + (i * eh->phentsize));
        if (ph->type != 1) continue;
        if (ph->offset + ph->filesz > size) return -1;
        u8int *dest = (u8int *)ph->vaddr;
        memcpy(dest, image + ph->offset, ph->filesz);
        if (ph->memsz > ph->filesz) {
            memset(dest + ph->filesz, 0, ph->memsz - ph->filesz);
        }
    }

    if (entry_out) {
        *entry_out = eh->entry;
    }
    log_info("ELF loaded. entry=0x%x\n", eh->entry);
    return 0;
}

int elf_prepare_stack(void *stack_top, u32int stack_size, const char **argv, int argc, u32int *out_esp) {
    if (!stack_top || stack_size < 64 || !out_esp) return -1;
    u32int sp = (u32int)stack_top;
    const char *arg_ptrs[16];
    if (argc < 0 || argc > 16) return -1;

    for (int i = argc - 1; i >= 0; i--) {
        const char *arg = argv ? argv[i] : 0;
        u32int len = arg ? (strlen(arg) + 1) : 1;
        if (sp < (u32int)stack_top - stack_size + len) return -1;
        sp -= len;
        if (arg) {
            memcpy((void *)sp, arg, len);
        } else {
            ((char *)sp)[0] = 0;
        }
        arg_ptrs[i] = (const char *)sp;
    }

    sp &= ~3U;
    sp -= (argc + 1) * 4;
    u32int *argv_ptr = (u32int *)sp;
    for (int i = 0; i < argc; i++) {
        argv_ptr[i] = (u32int)arg_ptrs[i];
    }
    argv_ptr[argc] = 0;

    sp -= 4;
    *(u32int *)sp = (u32int)argc;
    *out_esp = sp;
    return 0;
}
