// panic.h
#ifndef PANIC_H
#define PANIC_H

#include "common.h"

struct registers;

void panic(const char *msg, const char *file, u32int line);
void panic_with_regs(const char *msg, const char *file, u32int line, struct registers *regs);

#define PANIC(msg) panic((msg), __FILE__, __LINE__)
#define PANIC_REGS(msg, regs) panic_with_regs((msg), __FILE__, __LINE__, (regs))

#endif
