// elf.h
#ifndef ELF_H
#define ELF_H

#include "common.h"

int elf_load(const u8int *image, u32int size, u32int *entry_out);
int elf_prepare_stack(void *stack_top, u32int stack_size, const char **argv, int argc, u32int *out_esp);

#endif
